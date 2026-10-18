# Cai dat Nhi Live tren may Windows moi (chay 1 lan).
# Dung:  chuot phai setup.ps1 -> Run with PowerShell
#   hoac: powershell -ExecutionPolicy Bypass -File setup.ps1
# Chay bang quyen Administrator neu muon script tu mo tuong lua.
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
Set-Location $root
function Step($t) { Write-Host "`n==> $t" -ForegroundColor Cyan }

# ---- 1. uv (trinh quan ly Python) ------------------------------------------
Step "Tim uv"
$uv = (Get-Command uv -ErrorAction SilentlyContinue).Source
if (-not $uv) {
    $py = (Get-Command py -ErrorAction SilentlyContinue).Source
    if (-not $py) { $py = (Get-Command python -ErrorAction SilentlyContinue).Source }
    if ($py) {
        Write-Host "Cai uv bang pip ($py)..."
        & $py -m pip install --user --quiet uv
        $uv = & $py -c "import sysconfig,os;print(os.path.join(sysconfig.get_path('scripts','nt_user'),'uv.exe'))"
    } else {
        Write-Host "May chua co Python. Script se tai uv tu trang chinh thuc astral.sh." -ForegroundColor Yellow
        $ok = Read-Host "Tiep tuc tai uv? (y/n)"
        if ($ok -ne "y") { throw "Huy. Hay cai Python tu python.org roi chay lai." }
        Invoke-RestMethod https://astral.sh/uv/install.ps1 | Invoke-Expression
        $uv = Join-Path $env:USERPROFILE ".local\bin\uv.exe"
    }
}
if (-not (Test-Path $uv)) { throw "Khong tim thay uv: $uv" }
Write-Host "uv: $uv"

# ---- 2. Python 3.12 rieng trong thu muc + thu vien --------------------------
Step "Cai Python 3.12 vao .\python va tao .venv"
$env:UV_PYTHON_INSTALL_DIR = Join-Path $root "python"
$env:UV_CACHE_DIR = Join-Path $root ".uv-cache"
if (Test-Path .venv) { Remove-Item -Recurse -Force .venv }
# uv tu tai Python 3.12 vao .\python (khong tao loi tat o ~\.localin)
& $uv venv --python 3.12 --python-preference only-managed .venv
Step "Cai thu vien (requirements.txt)"
& $uv pip install --python .venv\Scripts\python.exe -r requirements.txt
Remove-Item -Recurse -Force $env:UV_CACHE_DIR -ErrorAction SilentlyContinue

if (-not (Test-Path bin\opus.dll)) { throw "Thieu bin\opus.dll (copy tu goi cai dat)" }
$env:PATH = (Join-Path $root "bin") + ";" + $env:PATH
& .venv\Scripts\python.exe -c "import opuslib_next, google.genai, aiohttp, yaml; print('Thu vien: OK')"

# ---- 3. API key ----------------------------------------------------------------
Step "Cau hinh"
if (-not (Test-Path config.yaml)) {
    Copy-Item config.template.yaml config.yaml
    $sec = Read-Host "Dan Gemini API key (lay o aistudio.google.com/apikey)" -AsSecureString
    $key = [Runtime.InteropServices.Marshal]::PtrToStringAuto(
        [Runtime.InteropServices.Marshal]::SecureStringToBSTR($sec))
    if ($key) {
        $c = [IO.File]::ReadAllText((Join-Path $root "config.yaml"), [Text.Encoding]::UTF8)
        $c = $c.Replace("DAN_GEMINI_API_KEY_VAO_DAY", $key.Trim())
        [IO.File]::WriteAllText((Join-Path $root "config.yaml"), $c, (New-Object Text.UTF8Encoding $false))
        Write-Host "Da luu key vao config.yaml"
    } else {
        Write-Host "Chua nhap key - sua gemini_api_key trong config.yaml sau." -ForegroundColor Yellow
    }
} else {
    Write-Host "config.yaml da co, giu nguyen."
}

# ---- 4. Tuong lua --------------------------------------------------------------
# Mo cong TCP 8000/8003 cho moi loai mang. (Rule theo file python.exe khong du:
# card Mobile hotspot cua Windows co the khong thuoc profile Private/Public.)
Step "Tuong lua"
$ruleName = "Nhi Live (TCP 8000, 8003)"
$cmd = "New-NetFirewallRule -DisplayName '$ruleName' -Direction Inbound -Protocol TCP -LocalPort 8000,8003 -Action Allow -Profile Any"
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
           ).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (Get-NetFirewallRule -DisplayName $ruleName -ErrorAction SilentlyContinue) {
    Write-Host "Rule tuong lua '$ruleName' da co." -ForegroundColor Green
} elseif ($isAdmin) {
    $ok = Read-Host "Mo cong 8000 + 8003 tren tuong lua cho robot ket noi? (y/n)"
    if ($ok -eq "y") {
        Invoke-Expression $cmd | Out-Null
        Write-Host "Da tao rule tuong lua." -ForegroundColor Green
    }
} else {
    Write-Host "Can mo cong tuong lua. Mo PowerShell bang 'Run as administrator' va chay:" -ForegroundColor Yellow
    Write-Host "   $cmd" -ForegroundColor White
}

Step "Shortcut tren Desktop"
$ok = Read-Host "Tao shortcut 'Nhi Live' tren Desktop? (y/n)"
if ($ok -eq "y") { & (Join-Path $root "create_shortcut.ps1") }

Step "XONG"
Write-Host "1. Lan dau: vao Settings > Network & internet > Mobile hotspot, chon Band 2.4 GHz,"
Write-Host "   dat TEN + MAT KHAU giong laptop cu de robot tu ket noi."
Write-Host "2. Moi lan dung: bam dup shortcut 'Nhi Live' (tu bat hotspot + chay Nhi)."
