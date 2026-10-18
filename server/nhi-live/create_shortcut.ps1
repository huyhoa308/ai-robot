# Tao shortcut "Nhi Live" tren Desktop: bam dup = bat hotspot + chay Nhi.
# Dung:  powershell -ExecutionPolicy Bypass -File create_shortcut.ps1
$root = $PSScriptRoot
$desktop = [Environment]::GetFolderPath("Desktop")
$lnk = Join-Path $desktop "Nhi Live.lnk"
$sh = New-Object -ComObject WScript.Shell
$s = $sh.CreateShortcut($lnk)
$s.TargetPath = "$env:WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe"
$s.Arguments = "-NoProfile -ExecutionPolicy Bypass -File `"$root\start_live.ps1`""
$s.WorkingDirectory = $root
$s.IconLocation = "$env:WINDIR\System32\shell32.dll,138"
$s.Description = "Chay robot Nhi (Gemini Live)"
$s.Save()
Write-Host "Da tao shortcut: $lnk" -ForegroundColor Green
