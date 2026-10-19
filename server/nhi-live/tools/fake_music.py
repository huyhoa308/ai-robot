# Offline test of the music feature: runs a second bridge on ports 8100/8103
# (the real one on 8000/8003 keeps serving the robot) and asks for music by text.
import asyncio, json, os, sys, time
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import bridge  # noqa: E402
import aiohttp, opuslib_next  # noqa: E402
from aiohttp import web  # noqa: E402

ASK = sys.argv[1] if len(sys.argv) > 1 else "Nhi ơi, mở một bài nhạc cho mình nghe đi"


async def main():
    wsapp = web.Application()
    wsapp.router.add_get("/xiaozhi/v1/", bridge.ws_handler)
    runner = web.AppRunner(wsapp)
    await runner.setup()
    await web.TCPSite(runner, "127.0.0.1", 8100).start()
    dec = opuslib_next.Decoder(24000, 1)
    async with aiohttp.ClientSession() as s:
        async with s.ws_connect("ws://127.0.0.1:8100/xiaozhi/v1/", headers={"Device-Id": "test-music"}) as ws:
            await ws.send_str(json.dumps({"type": "hello", "version": 1}))
            await ws.receive()
            await ws.send_str(json.dumps({"type": "listen", "state": "detect", "text": ASK}))
            frames, music, t0 = 0, False, time.time()
            async for m in ws:
                if m.type == aiohttp.WSMsgType.BINARY:
                    dec.decode(m.data, 1440)
                    frames += 1
                    if music and frames % 50 == 0:
                        print(f"  music frames {frames}")
                    if music and frames > 200:  # ~12 s of music: test the stop path
                        print("  -> abort (like the hand sensor)")
                        await ws.send_str(json.dumps({"type": "abort", "reason": "test"}))
                        await asyncio.sleep(1.0)
                        break
                else:
                    d = json.loads(m.data)
                    d.pop("session_id", None)
                    print(f"{time.time() - t0:5.1f}s <- {d}")
                    if d.get("text", "").startswith("🎵 "):
                        music, frames = True, 0
                if time.time() - t0 > 60:
                    print("TIMEOUT")
                    break
    print("RESULT music_played =", music, "frames =", frames)
    await runner.cleanup()

asyncio.run(main())
