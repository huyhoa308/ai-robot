# hardware-tests – kiểm tra phần cứng robot Nhi (PlatformIO / Arduino)

Mở thư mục này bằng VS Code + PlatformIO, hoặc dùng `pio` trên dòng lệnh.

| Env | File | Mục đích |
|---|---|---|
| `test_all` | `src/test_all.cpp` + `test_all.ps1` | Test toàn bộ ngoại vi, in `RESULT|id|tên|PASS/FAIL/WARN|chi tiết`; script tự tổng hợp bảng màu và lưu log vào `logs\` |
| `test_mic` | `src/test_mic.cpp` | Mức âm mic 2 kênh L/R mỗi 0,5 s và chẩn đoán dây |
| `test_audio` | `src/test_audio.cpp` | Loa phát nhạc và bíp 1 kHz; mic đo loopback |
| `test_sensors`, `test_servo`, `test_servo_basic` | | Test cảm biến và servo riêng lẻ |

```bash
powershell -ExecutionPolicy Bypass -File test_all.ps1                  # test, tự tìm cổng
powershell -ExecutionPolicy Bypass -File test_all.ps1 -RestoreXiaozhi  # test xong nạp lại chatbot
powershell -ExecutionPolicy Bypass -File test_all.ps1 -OnlyRestore     # chỉ nạp lại chatbot
pio run -e test_mic -t upload -t monitor
```

> Firmware Arduino ghi đè vùng NVS nên robot **mất Wi-Fi và OTA URL** của chatbot. Nạp lại chatbot rồi cài lại (README chính, bước C).
> `Restore-Xiaozhi` trong `test_all.ps1` dùng thư mục `xiaozhi-esp32` nằm cạnh thư mục repo này (README chính, bước B).
