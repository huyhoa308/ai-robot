# Chạy server Xiaozhi cho robot Nhi (Groq Whisper + Gemini + Edge TTS).
# Dùng: chuột phải -> Run with PowerShell, hoặc:  powershell -ExecutionPolicy Bypass -File start_server.ps1
# Dừng: Ctrl+C trong cửa sổ này.
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

# opus.dll + ffmpeg.exe nằm trong .\bin
$env:PATH = (Join-Path $PSScriptRoot "bin") + ";" + $env:PATH
$env:PYTHONUTF8 = "1"

# Robot kết nối vào Mobile Hotspot của laptop: IP hotspot luôn là 192.168.137.1
$hotspot = Get-NetIPAddress -AddressFamily IPv4 -IPAddress 192.168.137.1 -ErrorAction SilentlyContinue
if ($hotspot) {
    Write-Host "Mobile Hotspot: BAT (192.168.137.1)" -ForegroundColor Green
} else {
    Write-Host "CANH BAO: Mobile Hotspot dang TAT - bat trong Settings > Network & internet > Mobile hotspot" -ForegroundColor Red
}
Write-Host "OTA URL cho robot:  http://192.168.137.1:8003/xiaozhi/ota/" -ForegroundColor Cyan

& (Join-Path $PSScriptRoot ".venv\Scripts\python.exe") app.py
