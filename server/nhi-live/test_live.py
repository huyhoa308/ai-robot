"""Quick Gemini Live probe: latency to first audio, transcript, quota errors.

Reads the Gemini key from this folder's config.yaml.
Usage: .venv\\Scripts\\python.exe test_live.py [model]
"""
import asyncio
import re
import sys
import time
import wave

from google import genai
from google.genai import types

import os

import yaml

CFG = os.path.join(os.path.dirname(os.path.abspath(__file__)), "config.yaml")


def gemini_key():
    return yaml.safe_load(open(CFG, encoding="utf-8"))["gemini_api_key"]


async def probe(model, question):
    client = genai.Client(api_key=gemini_key())
    config = types.LiveConnectConfig(
        response_modalities=["AUDIO"],
        system_instruction="Bạn là Nhi, robot trợ lý người Việt vui vẻ. Luôn nói tiếng Việt, ngắn gọn 1-3 câu.",
        speech_config=types.SpeechConfig(language_code="vi-VN"),
        output_audio_transcription=types.AudioTranscriptionConfig(),
        tools=[types.Tool(google_search=types.GoogleSearch())],
        thinking_config=types.ThinkingConfig(thinking_budget=0),
    )
    pcm = bytearray()
    text = []
    t0 = time.time()
    first = None
    async with client.aio.live.connect(model=model, config=config) as session:
        t_conn = time.time() - t0
        t1 = time.time()
        await session.send_client_content(
            turns=types.Content(role="user", parts=[types.Part(text=question)]),
            turn_complete=True,
        )
        async for msg in session.receive():
            if msg.data:
                if first is None:
                    first = time.time() - t1
                pcm += msg.data
            sc = msg.server_content
            if sc and sc.output_transcription and sc.output_transcription.text:
                text.append(sc.output_transcription.text)
            if sc and sc.turn_complete:
                break
    total = time.time() - t1
    secs = len(pcm) / 2 / 24000
    with wave.open(f"probe_{model.replace('/', '_')}.wav", "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(24000); w.writeframes(bytes(pcm))
    print(f"[{model}] connect {t_conn:.1f}s | first audio {first and round(first, 2)}s | "
          f"done {total:.1f}s | audio {secs:.1f}s\n   -> {''.join(text)[:220]}")


async def main():
    models = sys.argv[1:-1] if len(sys.argv) > 2 else ["gemini-3.8-live", "gemini-2.5-flash-native-audio-latest"]
    q = sys.argv[-1] if len(sys.argv) > 2 else "Năm 1945 ở Việt Nam có sự kiện gì quan trọng?"
    for m in models:
        try:
            await probe(m, q)
        except Exception as e:
            print(f"[{m}] ERR {type(e).__name__}: {str(e)[:300]}")


asyncio.run(main())
