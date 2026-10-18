# Chay Nhi Live (robot <-> Gemini Live). Dong cua so nay = Nhi ngung hoat dong.
# Dung:  powershell -ExecutionPolicy Bypass -File start_live.ps1   (hoac shortcut "Nhi Live" tren Desktop)
Set-Location $PSScriptRoot
$env:PYTHONUTF8 = "1"
$Host.UI.RawUI.WindowTitle = "Nhi Live - dung dong cua so nay"

# 1. Mobile hotspot: tu bat neu dang tat
. (Join-Path $PSScriptRoot "hotspot.ps1")
Enable-NhiHotspot | Out-Null
# cho card hotspot nhan IP 192.168.137.1 (toi da ~15 s)
$ip = $null
for ($i = 0; $i -lt 15 -and -not $ip; $i++) {
    $ip = Get-NetIPAddress -AddressFamily IPv4 -IPAddress 192.168.137.1 -ErrorAction SilentlyContinue
    if (-not $ip) { Start-Sleep -Seconds 1 }
}
if ($ip) {
    Write-Host "Hotspot IP: 192.168.137.1 - robot ket noi duoc" -ForegroundColor Green
} else {
    Write-Host "CANH BAO: chua thay IP 192.168.137.1 - robot se khong ket noi duoc" -ForegroundColor Red
}

# 2. nhi-server dung chung cong 8000/8003: tat no (hoac ban Nhi Live cu) neu dang chay
Get-NetTCPConnection -LocalPort 8000,8003 -State Listen -ErrorAction SilentlyContinue |
    ForEach-Object { Stop-Process -Id $_.OwningProcess -Force -ErrorAction SilentlyContinue }

Write-Host "OTA URL cho robot:  http://192.168.137.1:8003/xiaozhi/ota/" -ForegroundColor Cyan
Write-Host "Bat robot (hoac tat/bat lai) - noi 'Chao Nhi' de thu." -ForegroundColor Cyan
Write-Host ""

# 3. Chay cau noi
& (Join-Path $PSScriptRoot ".venv\Scripts\python.exe") bridge.py
Write-Host "`nNhi Live da dung. Nhan Enter de dong." -ForegroundColor Yellow
Read-Host | Out-Null
