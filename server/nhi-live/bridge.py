"""Nhi Live bridge: Xiaozhi device protocol <-> Gemini Live (native audio).

The robot (or the digital-human web tester) talks the normal Xiaozhi
WebSocket protocol; every connection gets its own Gemini Live session.

  robot --opus 16 kHz--> bridge --PCM 16 kHz--> Gemini Live
  robot <--opus 24 kHz-- bridge <--PCM 24 kHz-- Gemini Live

HTTP  :8003  /xiaozhi/ota/    -> tells the device where the WebSocket is
WS    :8000  /xiaozhi/v1/     -> device audio + JSON control messages

Run:  .venv\\Scripts\\python.exe bridge.py      (see start_live.ps1)
"""
import asyncio
import json
import logging
import os
import re
import sys
import time
import uuid

BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "bin")
os.environ["PATH"] = BIN + os.pathsep + os.environ.get("PATH", "")
if hasattr(os, "add_dll_directory"):
    os.add_dll_directory(BIN)

import opuslib_next  # noqa: E402  (needs opus.dll on PATH)
import yaml  # noqa: E402
from aiohttp import WSMsgType, web  # noqa: E402
from google import genai  # noqa: E402
from google.genai import types  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
CFG = yaml.safe_load(open(os.path.join(HERE, "config.yaml"), encoding="utf-8"))

IN_RATE = 16000          # device mic
OUT_RATE = 24000         # Gemini Live output
FRAME_MS = 60            # opus frame length both ways
OUT_FRAME = OUT_RATE * FRAME_MS // 1000   # samples per outgoing opus frame
PREBUFFER_FRAMES = 4     # sent at once, then real-time pacing

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
    datefmt="%H:%M:%S",
    handlers=[logging.StreamHandler(sys.stdout),
              logging.FileHandler(os.path.join(HERE, "bridge.log"), encoding="utf-8")],
)
log = logging.getLogger("nhi-live")

SENTENCE_END = re.compile(r"[.!?…。！？]\s*$")


def live_config():
    speech = types.SpeechConfig(language_code=CFG.get("language", "vi-VN"))
    if CFG.get("voice"):
        speech = types.SpeechConfig(
            language_code=CFG.get("language", "vi-VN"),
            voice_config=types.VoiceConfig(
                prebuilt_voice_config=types.PrebuiltVoiceConfig(voice_name=CFG["voice"])),
        )
    tools = [types.Tool(google_search=types.GoogleSearch())] if CFG.get("google_search", True) else []
    now = time.localtime()
    weekday = ["Thứ Hai", "Thứ Ba", "Thứ Tư", "Thứ Năm", "Thứ Sáu", "Thứ Bảy", "Chủ Nhật"][now.tm_wday]
    knowledge = ""
    kpath = os.path.join(HERE, CFG.get("knowledge_file", "kien_thuc.md"))
    if os.path.isfile(kpath):
        knowledge = "\n\n# KIẾN THỨC NỀN (dùng để trả lời, không đọc nguyên văn)\n" + \
            open(kpath, encoding="utf-8").read().strip()
    system = CFG["prompt"].rstrip() + knowledge + (
        f"\n\nHôm nay là {weekday}, ngày {now.tm_mday}/{now.tm_mon}/{now.tm_year}, "
        f"bây giờ là {now.tm_hour} giờ {now.tm_min:02d}, giờ Việt Nam.\n"
        "Kiến thức có sẵn của bạn có thể đã cũ. Với mọi câu hỏi về người đang giữ chức vụ "
        "(tổng thống, chủ tịch, thủ tướng...), tin tức, sự kiện gần đây, giá cả, kết quả thi đấu, "
        "thời tiết hay bất kỳ thông tin nào có thể đã thay đổi, BẮT BUỘC dùng Google Search "
        "trước khi trả lời, rồi trả lời theo kết quả tìm được."
    )
    return types.LiveConnectConfig(
        response_modalities=["AUDIO"],
        system_instruction=system,
        speech_config=speech,
        input_audio_transcription=types.AudioTranscriptionConfig(),
        output_audio_transcription=types.AudioTranscriptionConfig(),
        thinking_config=types.ThinkingConfig(thinking_budget=int(CFG.get("thinking_budget", 0))),
        tools=tools,
        realtime_input_config=types.RealtimeInputConfig(
            automatic_activity_detection=types.AutomaticActivityDetection(
                silence_duration_ms=int(CFG.get("silence_ms", 700)),
            )
        ),
        # lets a conversation run longer than the default session limit
        context_window_compression=types.ContextWindowCompressionConfig(
            sliding_window=types.SlidingWindow()),
    )


class DeviceSession:
    """One robot WebSocket connection <-> one Gemini Live session."""

    def __init__(self, ws, device_id):
        self.ws = ws
        self.device_id = device_id
        self.session_id = str(uuid.uuid4())
        self.decoder = opuslib_next.Decoder(IN_RATE, 1)
        self.encoder = opuslib_next.Encoder(OUT_RATE, 1, opuslib_next.APPLICATION_VOIP)
        self.live = None
        self.speaking = False
        self.drop_output = False      # set by abort until the model finishes its turn
        self.out_pcm = bytearray()
        self.frames_sent = 0
        self.speak_t0 = 0.0
        self.user_text = []
        self.bot_text = []
        self.last_activity = time.time()
        self.send_lock = asyncio.Lock()

    # ---- to device -------------------------------------------------------
    async def send_json(self, obj):
        obj.setdefault("session_id", self.session_id)
        if not self.ws.closed:
            await self.ws.send_str(json.dumps(obj, ensure_ascii=False))

    async def start_speaking(self):
        if self.speaking:
            return
        self.speaking = True
        self.frames_sent = 0
        self.speak_t0 = time.monotonic()
        if self.user_text:
            text = "".join(self.user_text).strip()
            self.user_text = []
            if text:
                log.info("[%s] >> %s", self.device_id, text)
                await self.send_json({"type": "stt", "text": text})
        await self.send_json({"type": "llm", "text": "😊", "emotion": CFG.get("speak_emotion", "happy")})
        await self.send_json({"type": "tts", "state": "start"})

    async def flush_sentence(self, force=False):
        text = "".join(self.bot_text).strip()
        if text and (force or SENTENCE_END.search(text) or len(text) > 80):
            self.bot_text = []
            log.info("[%s] << %s", self.device_id, text)
            await self.send_json({"type": "tts", "state": "sentence_start", "text": text})

    async def send_audio(self, pcm: bytes):
        """Encode 24 kHz PCM into 60 ms opus frames, paced at real time."""
        self.out_pcm += pcm
        frame_bytes = OUT_FRAME * 2
        while len(self.out_pcm) >= frame_bytes:
            chunk = bytes(self.out_pcm[:frame_bytes])
            del self.out_pcm[:frame_bytes]
            await self._send_frame(chunk)

    async def _send_frame(self, chunk: bytes):
        packet = self.encoder.encode(chunk, OUT_FRAME)
        # pacing: a few frames ahead of real time, never a flood
        ahead = self.frames_sent - PREBUFFER_FRAMES
        if ahead > 0:
            due = self.speak_t0 + ahead * FRAME_MS / 1000.0
            delay = due - time.monotonic()
            if delay > 0:
                await asyncio.sleep(delay)
        if not self.ws.closed and not self.drop_output:
            await self.ws.send_bytes(packet)
        self.frames_sent += 1

    async def stop_speaking(self):
        if self.out_pcm:  # pad the last partial frame
            pad = OUT_FRAME * 2 - len(self.out_pcm)
            self.out_pcm += b"\x00" * pad
            await self.send_audio(b"")
        await self.flush_sentence(force=True)
        if self.speaking:
            # wait until the device has played what we sent
            remaining = self.speak_t0 + self.frames_sent * FRAME_MS / 1000.0 - time.monotonic()
            if remaining > 0 and not self.drop_output:
                await asyncio.sleep(remaining)
            await self.send_json({"type": "tts", "state": "stop"})
        self.speaking = False
        self.out_pcm = bytearray()

    # ---- from Gemini -----------------------------------------------------
    async def pump_gemini(self):
        while True:
            async for msg in self.live.receive():
                sc = msg.server_content
                if sc and sc.input_transcription and sc.input_transcription.text:
                    self.user_text.append(sc.input_transcription.text)
                    self.last_activity = time.time()
                if msg.data:
                    if self.drop_output:
                        continue
                    await self.start_speaking()
                    await self.send_audio(msg.data)
                if sc and sc.output_transcription and sc.output_transcription.text:
                    self.bot_text.append(sc.output_transcription.text)
                    await self.flush_sentence()
                if sc and sc.interrupted:
                    log.info("[%s] interrupted", self.device_id)
                    self.drop_output = True
                if sc and (sc.turn_complete or sc.interrupted):
                    await self.stop_speaking()
                    self.drop_output = False
                    self.last_activity = time.time()
                if msg.go_away:
                    log.warning("[%s] Gemini go_away: %s", self.device_id, msg.go_away)

    # ---- from device -----------------------------------------------------
    async def on_audio(self, packet: bytes):
        if self.speaking:
            return  # half duplex: ignore mic while the robot talks (no AEC)
        try:
            pcm = self.decoder.decode(packet, IN_RATE * FRAME_MS // 1000)
        except Exception:
            return
        await self.live.send_realtime_input(
            audio=types.Blob(data=pcm, mime_type=f"audio/pcm;rate={IN_RATE}"))

    async def on_json(self, msg):
        t = msg.get("type")
        if t == "hello":
            await self.send_json({
                "type": "hello", "transport": "websocket", "version": msg.get("version", 1),
                "audio_params": {"format": "opus", "sample_rate": OUT_RATE, "channels": 1,
                                 "frame_duration": FRAME_MS},
            })
        elif t == "listen":
            state = msg.get("state")
            self.last_activity = time.time()
            if state == "detect" and msg.get("text"):
                # wake word: let Nhi greet
                log.info("[%s] wake word: %s", self.device_id, msg["text"])
                await self.live.send_client_content(
                    turns=types.Content(role="user", parts=[types.Part(text=msg["text"])]),
                    turn_complete=True)
            elif state == "stop":
                await self.live.send_realtime_input(audio_stream_end=True)
        elif t == "abort":
            log.info("[%s] abort (%s)", self.device_id, msg.get("reason", ""))
            if self.speaking:
                self.drop_output = True
                await self.send_json({"type": "tts", "state": "stop"})
                self.speaking = False
                self.out_pcm = bytearray()
        elif t == "mcp":
            pass  # device tools are not used by this bridge

    async def idle_watchdog(self):
        limit = int(CFG.get("idle_close_s", 120))
        while not self.ws.closed:
            await asyncio.sleep(5)
            if not self.speaking and time.time() - self.last_activity > limit:
                log.info("[%s] idle %ss, closing", self.device_id, limit)
                await self.ws.close()
                return


async def ws_handler(request):
    ws = web.WebSocketResponse(heartbeat=30, max_msg_size=4 * 1024 * 1024)
    await ws.prepare(request)
    device_id = request.headers.get("Device-Id", request.query.get("device-id", "?"))
    log.info("device connected: %s from %s", device_id, request.remote)
    sess = DeviceSession(ws, device_id)
    client = genai.Client(api_key=CFG["gemini_api_key"])
    try:
        async with client.aio.live.connect(model=CFG["model"], config=live_config()) as live:
            sess.live = live
            pump = asyncio.create_task(sess.pump_gemini())
            watchdog = asyncio.create_task(sess.idle_watchdog())
            try:
                async for m in ws:
                    if m.type == WSMsgType.BINARY:
                        await sess.on_audio(m.data)
                    elif m.type == WSMsgType.TEXT:
                        try:
                            await sess.on_json(json.loads(m.data))
                        except json.JSONDecodeError:
                            pass
                    elif m.type in (WSMsgType.CLOSE, WSMsgType.ERROR):
                        break
                    if pump.done():
                        pump.result()  # surface Gemini errors
            finally:
                pump.cancel()
                watchdog.cancel()
    except Exception as e:
        log.error("[%s] session error: %s: %s", device_id, type(e).__name__, str(e)[:300])
        if not ws.closed:
            await sess.send_json({"type": "tts", "state": "stop"})
    finally:
        if not ws.closed:
            await ws.close()
        log.info("device disconnected: %s", device_id)
    return ws


async def ota_handler(request):
    if request.method == "POST":
        device_id = request.headers.get("Device-Id", "?")
        log.info("OTA from %s (%s)", device_id, request.remote)
    # Answer with the address the device itself used to reach us, so the robot
    # works on the laptop hotspot (192.168.137.1) or on any shared Wi-Fi.
    host = (request.host or "").rsplit(":", 1)[0]
    ws_url = CFG["websocket_url"]
    if host and host not in ("localhost",):
        ws_url = f"ws://{host}:{int(CFG.get('ws_port', 8000))}/xiaozhi/v1/"
    body = {
        "server_time": {"timestamp": int(time.time() * 1000),
                        "timezone_offset": int(CFG.get("timezone_offset_min", 420))},
        "firmware": {"version": "0.0.0", "url": ""},
        "websocket": {"url": ws_url, "token": ""},
    }
    if request.method == "GET":
        return web.Response(text=f"Nhi Live OTA OK. WebSocket: {ws_url}")
    return web.json_response(body)


@web.middleware
async def cors(request, handler):
    if request.method == "OPTIONS":
        resp = web.Response()
    else:
        resp = await handler(request)
    resp.headers["Access-Control-Allow-Origin"] = "*"
    resp.headers["Access-Control-Allow-Headers"] = "*"
    resp.headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS"
    return resp


async def main():
    if not CFG.get("gemini_api_key") or "VAO_DAY" in CFG["gemini_api_key"]:
        log.error("Chua co gemini_api_key trong config.yaml")
        return
    http = web.Application(middlewares=[cors])
    http.router.add_route("*", "/xiaozhi/ota/", ota_handler)
    http.router.add_route("*", "/xiaozhi/ota", ota_handler)
    wsapp = web.Application()
    wsapp.router.add_get("/xiaozhi/v1/", ws_handler)
    wsapp.router.add_get("/xiaozhi/v1", ws_handler)
    r1, r2 = web.AppRunner(http), web.AppRunner(wsapp)
    await r1.setup(); await r2.setup()
    await web.TCPSite(r1, "0.0.0.0", int(CFG.get("http_port", 8003))).start()
    await web.TCPSite(r2, "0.0.0.0", int(CFG.get("ws_port", 8000))).start()
    log.info("Nhi Live bridge | model %s | OTA http://<ip>:%s/xiaozhi/ota/ | WS %s",
             CFG["model"], CFG.get("http_port", 8003), CFG["websocket_url"])
    await asyncio.Event().wait()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
