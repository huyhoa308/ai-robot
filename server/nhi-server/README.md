# nhi-server – server dự phòng (Groq Whisper + Gemini + Edge TTS)

Dùng khi không chạy được Gemini Live. Đây là [xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server)
cấu hình cho tiếng Việt:

| Khâu | Dịch vụ (đều có gói miễn phí) |
|---|---|
| Nhận giọng nói (ASR) | Groq `whisper-large-v3`, `language: vi` |
| Trả lời (LLM) | Gemini `gemini-flash-lite-latest` qua cổng tương thích OpenAI |
| Đọc (TTS) | Microsoft Edge TTS `vi-VN-HoaiMyNeural` |

Repo chỉ chứa **phần thay đổi** so với upstream (commit `eb9dc8c`):

| File | Nội dung |
|---|---|
| `upstream-changes.diff` | Sửa 3 file Python: gửi lại `thought_signature` của Gemini (`core/connection.py`), thêm `language` / `prompt` cho ASR OpenAI (`core/providers/asr/openai.py`), bỏ plugin `get_lunar` (`plugin_executor.py`) |
| `overlay/agent-base-prompt-vi.txt` | Prompt hệ thống tiếng Việt |
| `overlay/data/.config.template.yaml` | Cấu hình: điền 2 API key (Groq, Gemini) rồi đổi tên thành `.config.yaml` |
| `overlay/start_server.ps1` | Script chạy server trên Windows |

## Cài đặt

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server.git
cd xiaozhi-esp32-server/main/xiaozhi-server
git checkout eb9dc8c
git apply <đường-dẫn>/ai-robot/server/nhi-server/upstream-changes.diff
```

Sau đó:
1. Copy toàn bộ `overlay/` vào `main/xiaozhi-server/`, đổi `data/.config.template.yaml` thành `data/.config.yaml`
   rồi điền key Groq (<https://console.groq.com/keys>) và Gemini (<https://aistudio.google.com/apikey>).
2. Tạo môi trường Python 3.10 và cài thư viện: `python -m venv .venv` rồi `.venv\Scripts\pip install -r requirements.txt`.
3. Tải model Silero VAD theo hướng dẫn upstream vào `models/snakers4_silero-vad`.
4. Tạo thư mục `bin\` chứa `opus.dll` (có sẵn ở `server/nhi-live/bin`) và `ffmpeg.exe`, `ffprobe.exe` (bản essentials của gyan.dev).
5. Chạy `start_server.ps1`. OTA URL cho robot: `http://192.168.137.1:8003/xiaozhi/ota/`.
