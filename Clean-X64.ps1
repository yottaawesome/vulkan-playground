<#
.SYNOPSIS
    Recursively deletes all 'x64' build folders under .\src.

.DESCRIPTION
    Useful for forcing Visual Studio's IntelliSense to re-scan C++ modules
    when it gets into a bad state.
#>
[CmdletBinding(SupportsShouldProcess)]
param(
    [string]$Root = (Join-Path $PSScriptRoot 'src')
)

if (-not (Test-Path -LiteralPath $Root)) {
    Write-Error "Source directory not found: $Root"
    exit 1
}

$folders = Get-ChildItem -LiteralPath $Root -Directory -Recurse -Force -Filter 'x64' -ErrorAction SilentlyContinue

if (-not $folders) {
    Write-Host "No x64 folders found under $Root."
    return
}

foreach ($folder in $folders) {
    if ($PSCmdlet.ShouldProcess($folder.FullName, 'Remove directory')) {
        try {
            Remove-Item -LiteralPath $folder.FullName -Recurse -Force -ErrorAction Stop
            Write-Host "Deleted: $($folder.FullName)"
        }
        catch {
            Write-Warning "Failed to delete $($folder.FullName): $_"
        }
    }
}
