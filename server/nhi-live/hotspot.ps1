# Bat Mobile hotspot cua Windows neu dang tat (dung ket noi Internet hien tai lam nguon).
# Tra ve $true neu hotspot dang bat sau khi chay.
# Dung rieng:  powershell -ExecutionPolicy Bypass -File hotspot.ps1
function Enable-NhiHotspot {
    try {
        Add-Type -AssemblyName System.Runtime.WindowsRuntime
        $asTaskGeneric = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
            $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and
            $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1' })[0]
        function Await($op, $type) {
            $t = $asTaskGeneric.MakeGenericMethod($type).Invoke($null, @($op))
            $t.Wait(-1) | Out-Null
            $t.Result
        }
        $profile = [Windows.Networking.Connectivity.NetworkInformation, Windows.Networking.Connectivity, ContentType = WindowsRuntime]::GetInternetConnectionProfile()
        if (-not $profile) {
            Write-Host "Chua co Internet (Wi-Fi hoac dien thoai 4G USB) - khong bat duoc hotspot." -ForegroundColor Red
            return $false
        }
        $mgr = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager, Windows.Networking.NetworkOperators, ContentType = WindowsRuntime]::CreateFromConnectionProfile($profile)
        if ($mgr.TetheringOperationalState -eq 1) {
            Write-Host "Mobile hotspot: dang BAT (nguon Internet: $($profile.ProfileName))" -ForegroundColor Green
            return $true
        }
        Write-Host "Dang bat Mobile hotspot (nguon Internet: $($profile.ProfileName))..." -ForegroundColor Yellow
        $res = Await ($mgr.StartTetheringAsync()) ([Windows.Networking.NetworkOperators.NetworkOperatorTetheringOperationResult])
        if ($res.Status -eq 0) {
            Write-Host "Mobile hotspot: da BAT" -ForegroundColor Green
            return $true
        }
        Write-Host "Khong bat duoc hotspot (ma loi $($res.Status)) - bat tay trong Settings > Network & internet > Mobile hotspot" -ForegroundColor Red
        return $false
    } catch {
        Write-Host "Khong bat duoc hotspot tu dong: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "Bat tay trong Settings > Network & internet > Mobile hotspot" -ForegroundColor Red
        return $false
    }
}

if ($MyInvocation.InvocationName -ne '.') { Enable-NhiHotspot | Out-Null }
