param(
    [string]$Port = "",          # de trong = tu tim (uu tien CH343, sau do USB native)
    [switch]$NoUpload,           # chi doc log, khong nap lai
    [switch]$RestoreXiaozhi,     # test xong nap lai firmware chatbot Xiaozhi (ESP-IDF)
    [switch]$OnlyRestore,        # khong test, chi nap lai firmware chatbot
    [switch]$ShowLog             # in TOAN BO log serial ra man hinh (khong chi dong ket qua)
)
# Tu kiem tra toan bo ngoai vi robot: nap env test_all, doc log, bao cai nao FAIL.
# Dung:  powershell -ExecutionPolicy Bypass -File test_all.ps1 [-RestoreXiaozhi]
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
$pio = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\pio.exe"
$py  = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\python.exe"

# ---- 1. tim cong ----------------------------------------------------------------
if (-not $Port) {
    $list = & $pio device list --serial --json-output | ConvertFrom-Json
    $ch343 = $list | Where-Object { $_.hwid -match "1A86:55D3" } | Select-Object -First 1
    $native = $list | Where-Object { $_.hwid -match "303A:1001" } | Select-Object -First 1
    if ($ch343) { $Port = $ch343.port } elseif ($native) { $Port = $native.port }
}
if (-not $Port) { throw "Khong thay robot. Cam cap USB (cong UART CH343 hoac USB native) roi chay lai." }
Write-Host "Cong robot: $Port" -ForegroundColor Cyan

function Restore-Xiaozhi($p) {
    Write-Host "`n==> Nap lai firmware chatbot Xiaozhi..." -ForegroundColor Cyan
    $env:IDF_TOOLS_PATH = "C:\Espressif\tools6"
    . "C:\esp\v6.0.1\esp-idf\export.ps1" | Out-Null
    Push-Location "E:\Claude\xiaozhi-dog\xiaozhi-esp32"
    idf.py -p $p flash
    Pop-Location
    Write-Host "`n==> Log khoi dong chatbot (30 s):" -ForegroundColor Cyan
    $mon = @"
import serial, time, sys
s = None; t0 = time.time()
while s is None and time.time() - t0 < 15:
    try:
        s = serial.Serial(); s.port = sys.argv[1]; s.baudrate = 115200; s.timeout = 0.3
        s.dtr = False; s.rts = False; s.open()
    except Exception:
        s = None; time.sleep(0.3)
if s is None: sys.exit(0)
end = time.time() + 30; buf = b''
while time.time() < end:
    buf += s.read(4096)
    while b'\n' in buf:
        line, buf = buf.split(b'\n', 1)
        print('  | ' + line.decode('utf-8', 'replace').rstrip(), flush=True)
s.close()
"@
    $tmpMon = Join-Path $env:TEMP "test_all_monitor.py"
    Set-Content -Path $tmpMon -Value $mon -Encoding UTF8
    & $py $tmpMon $p
}
if ($OnlyRestore) { Restore-Xiaozhi $Port; return }

# ---- 2. nap firmware test --------------------------------------------------------
if (-not $NoUpload) {
    Write-Host "==> Build + nap env test_all (ghi de firmware chatbot)..." -ForegroundColor Cyan
    & $pio run -e test_all -t upload --upload-port $Port
    if ($LASTEXITCODE -ne 0) { throw "Nap that bai." }
}

# ---- 3. doc log ------------------------------------------------------------------
New-Item -ItemType Directory -Force logs | Out-Null
$log = Join-Path $PSScriptRoot ("logs\test_all_{0}.log" -f (Get-Date -Format "yyyyMMdd_HHmmss"))
$reader = @"
import serial, sys, time
port, out = sys.argv[1], sys.argv[2]
s = None
t0 = time.time()
while s is None and time.time() - t0 < 15:
    try:
        s = serial.Serial(); s.port = port; s.baudrate = 115200; s.timeout = 0.3
        s.dtr = False; s.rts = False; s.open()
    except Exception:
        s = None; time.sleep(0.3)
if s is None:
    print('ERR khong mo duoc cong'); sys.exit(1)
if '$NoUpload' == 'True':  # reset de chay lai tu dau
    s.rts = True; time.sleep(0.1); s.rts = False
buf = b''; done = False; t0 = time.time()
with open(out, 'wb') as f:
    while time.time() - t0 < 150 and not done:
        d = s.read(4096)
        if not d: continue
        f.write(d); buf += d
        while b'\n' in buf:
            line, buf = buf.split(b'\n', 1)
            t = line.decode('utf-8', 'replace').strip()
            if t.startswith(('RESULT|', 'ACTION|', 'SUMMARY|', 'TEST_ALL_START')):
                print(t, flush=True)
            elif '$ShowLog' == 'True' and t:
                print('LOG|' + t, flush=True)
            if t == 'TEST_ALL_DONE':
                done = True
s.close()
print('DONE' if done else 'TIMEOUT', flush=True)
"@
$tmp = Join-Path $env:TEMP "test_all_reader.py"
Set-Content -Path $tmp -Value $reader -Encoding UTF8
Write-Host "==> Doc log (toi da 150 s). Khi duoc nhac, hay NHAN NUT BOOT tren robot." -ForegroundColor Cyan
$lines = & $py $tmp $Port $log | ForEach-Object {
    if ($_ -like "ACTION|*") { Write-Host (">>> " + $_.Substring(7)) -ForegroundColor Magenta }
    elseif ($_ -like "RESULT|*") { Write-Host ("    " + ($_ -split '\|')[1] + " ... " + ($_ -split '\|')[3]) }
    elseif ($_ -like "LOG|*") { Write-Host ("  | " + $_.Substring(4)) -ForegroundColor DarkGray }
    $_
}

# ---- 4. bang ket qua -------------------------------------------------------------
$seen = @{}
$rows = foreach ($l in $lines) {
    if ($l -like "RESULT|*") {
        $p = $l -split '\|', 5
        if (-not $seen.ContainsKey($p[1])) { $seen[$p[1]] = 1; [pscustomobject]@{Id=$p[1]; Ten=$p[2]; KetQua=$p[3]; ChiTiet=$p[4]} }
    }
}
Write-Host ""
Write-Host "================ KET QUA TEST NGOAI VI ================" -ForegroundColor White
foreach ($r in $rows) {
    $c = switch ($r.KetQua) { "PASS" {"Green"} "FAIL" {"Red"} "WARN" {"Yellow"} default {"Cyan"} }
    Write-Host ("{0,-6} {1,-7} {2}" -f $r.Id, $r.KetQua, $r.Ten) -ForegroundColor $c
    if ($r.KetQua -ne "PASS") { Write-Host ("       -> " + $r.ChiTiet) -ForegroundColor DarkGray }
}
$fail = @($rows | Where-Object KetQua -eq "FAIL").Count
$warn = @($rows | Where-Object KetQua -eq "WARN").Count
Write-Host "=======================================================" -ForegroundColor White
if (-not $rows) { Write-Host "Khong doc duoc ket qua nao - xem log: $log" -ForegroundColor Red }
elseif ($fail) { Write-Host "$fail FAIL, $warn WARN. MANUAL = nhin/nghe tren robot de xac nhan." -ForegroundColor Red }
else { Write-Host "Khong co FAIL ($warn WARN). MANUAL = nhin/nghe tren robot de xac nhan." -ForegroundColor Green }
Write-Host "Log day du: $log"

# ---- 5. nap lai chatbot ------------------------------------------------------------
if ($RestoreXiaozhi) {
    Restore-Xiaozhi $Port
} else {
    Write-Host "`nLUU Y: robot dang chay firmware TEST. Nap lai chatbot:  powershell -ExecutionPolicy Bypass -File test_all.ps1 -OnlyRestore" -ForegroundColor Yellow
}
