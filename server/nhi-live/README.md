# Nhi Live – robot Nhi nói chuyện bằng Gemini Live

Cầu nối giữa robot (firmware Xiaozhi, board `esp32s3-weact-dog`) và Google **Gemini Live**:
robot gửi giọng nói qua Wi-Fi, Gemini nghe – hiểu – tra Google – trả lời bằng giọng nói tự nhiên.

```
Robot ──Wi-Fi──► Laptop (Mobile hotspot 192.168.137.1 + bridge.py) ──internet──► Gemini Live
```

## Cài trên laptop mới (1 lần)

1. Giải nén `nhi-live-portable.zip` vào ổ đĩa, **đường dẫn không có dấu tiếng Việt**, ví dụ `D:\nhi-live`.
2. Chuột phải `setup.ps1` → **Run with PowerShell** (nên chạy bằng quyền Administrator để script tự mở tường lửa).
   Script sẽ:
   - cài Python 3.12 ngay trong thư mục `python\` và thư viện vào `.venv\`;
   - hỏi **Gemini API key** (lấy ở aistudio.google.com/apikey) và lưu vào `config.yaml`;
   - tạo rule tường lửa (hoặc hướng dẫn tự thêm).
3. Bật **Mobile hotspot**: Settings → Network & internet → Mobile hotspot, **Band 2.4 GHz**.
   Đặt **tên + mật khẩu giống laptop cũ** thì robot tự kết nối, không phải cài lại.

## Mỗi lần dùng (kể cả sau khi tắt/bật laptop)

1. Có Internet (Wi-Fi, hoặc điện thoại 4G cắm USB + bật chia sẻ Internet qua USB).
2. **Bấm đúp shortcut "Nhi Live" trên Desktop**: script tự bật Mobile hotspot rồi chạy Nhi.
   Để nguyên cửa sổ đó (đóng = Nhi ngừng). Không có shortcut thì chạy `create_shortcut.ps1`
   hoặc `powershell -ExecutionPolicy Bypass -File start_live.ps1`.
3. Bật robot (hoặc tắt/bật lại nếu robot đã bật trước laptop) → nói "Chào Nhi".

## Cài robot (chỉ khi đổi tên/mật khẩu hotspot)

Bật robot, nhấn **BOOT** lúc "Đang quét Wi-Fi…" → điện thoại vào Wi-Fi `Xiaozhi-xxxx` → mở `192.168.4.1`
→ chọn Wi-Fi hotspot của laptop → ô **OTA URL**: `http://192.168.137.1:8003/xiaozhi/ota/` → Lưu.

## Tuỳ chỉnh

| Muốn đổi | Sửa |
|---|---|
| Tính cách, cách nói chuyện | `config.yaml` → `prompt` |
| Kiến thức về trường, bộ môn, sự kiện | `kien_thuc.md` |
| Giọng (Aoede, Kore, Leda, Zephyr / Puck, Charon, Fenrir, Orus) | `config.yaml` → `voice` |
| Hay bị cắt lời / chờ lâu | `config.yaml` → `silence_ms` |

Sửa xong chạy lại `start_live.ps1`.

## Kiểm tra nhanh

```
.venv\Scripts\python.exe test_live.py gemini-2.5-flash-native-audio-latest "Xin chào Nhi"
```
Nhật ký hội thoại: `bridge.log`.

## Lưu ý

- `config.yaml` chứa **API key** – không chia sẻ, không đưa lên GitHub (`.gitignore` đã chặn).
- Gói miễn phí Gemini Live có giới hạn số phiên/phút mỗi ngày.
- Đóng gói lại để mang đi: `powershell -ExecutionPolicy Bypass -File make_package.ps1` → `dist\nhi-live-portable.zip`.
