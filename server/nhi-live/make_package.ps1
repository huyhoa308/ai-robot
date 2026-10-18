# Dong goi Nhi Live thanh dist\nhi-live-portable.zip (KHONG kem API key va .venv).
# Dung:  powershell -ExecutionPolicy Bypass -File make_package.ps1
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
$files = @("bridge.py", "config.template.yaml", "kien_thuc.md", "requirements.txt",
           "setup.ps1", "start_live.ps1", "hotspot.ps1", "create_shortcut.ps1", "test_live.py", "README.md", ".gitignore")
$stage = Join-Path $env:TEMP "nhi-live-stage"
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force "$stage\nhi-live\bin" | Out-Null
foreach ($f in $files) { Copy-Item $f "$stage\nhi-live\" }
Copy-Item bin\opus.dll "$stage\nhi-live\bin\"

# chot chan: khong de lot key vao goi
$leak = Select-String -Path "$stage\nhi-live\*" -Pattern "AQ\.[A-Za-z0-9_-]{20,}|AIza[0-9A-Za-z_-]{20,}|gsk_[0-9A-Za-z]{20,}" -ErrorAction SilentlyContinue
if ($leak) { throw "Phat hien API key trong goi: $($leak.Path) - dung lai!" }

New-Item -ItemType Directory -Force dist | Out-Null
$zip = Join-Path $PSScriptRoot "dist\nhi-live-portable.zip"
if (Test-Path $zip) { Remove-Item $zip }
Compress-Archive -Path "$stage\nhi-live" -DestinationPath $zip
Remove-Item -Recurse -Force $stage
Write-Host "Da tao: $zip ($([math]::Round((Get-Item $zip).Length/1KB)) KB)" -ForegroundColor Green
