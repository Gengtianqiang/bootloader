[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$InputElf,
    [Parameter(Mandatory = $true)][string]$OutputPackage,
    [Parameter(Mandatory = $true)][string]$BuildCache
)

$ErrorActionPreference = 'Stop'
$elfPath = (Resolve-Path -LiteralPath $InputElf).ProviderPath
$cachePath = (Resolve-Path -LiteralPath $BuildCache).ProviderPath
$packagePath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputPackage)
if ($elfPath -eq $packagePath) {
    throw 'The output package must be a different file from the ELF input.'
}

$cache = [IO.File]::ReadAllText($cachePath)
if ($cache -notmatch '(?m)^OTATEST_OTA:BOOL=ON\r?$') {
    throw 'Configure this App with the App-Debug CMake preset before packaging.'
}
if ($cache -notmatch '(?m)^CMAKE_HOME_DIRECTORY:INTERNAL=(.+)\r?$' -or
    [IO.Path]::GetFullPath($Matches[1]) -ine (Get-Location).Path) {
    throw 'The CMake build cache does not belong to this otatest project.'
}
$objcopyCommand = Get-Command arm-none-eabi-objcopy -CommandType Application -ErrorAction Stop
$packageDirectory = [IO.Path]::GetDirectoryName($packagePath)
[IO.Directory]::CreateDirectory($packageDirectory) | Out-Null
$temporaryBase = '.otatest-' + [guid]::NewGuid().ToString('N')
$temporaryBin = Join-Path $packageDirectory ($temporaryBase + '.bin')
$temporaryPackage = Join-Path $packageDirectory ($temporaryBase + '.step2.bin')
$packer = Join-Path $PSScriptRoot 'Pack-Step2.ps1'

try {
    & $objcopyCommand.Source -O binary $elfPath $temporaryBin
    if ($LASTEXITCODE -ne 0) {
        throw "arm-none-eabi-objcopy failed with exit code $LASTEXITCODE."
    }

    & $packer -InputBin $temporaryBin -OutputPackage $temporaryPackage
    if ($LASTEXITCODE -ne 0) {
        throw "Step2 packaging failed with exit code $LASTEXITCODE."
    }

    $package = [IO.File]::ReadAllBytes($temporaryPackage)
    if ($package.Length -lt 32 -or ($package.Length % 16) -ne 0) {
        throw 'The generated Step2 package has an invalid length.'
    }

    Move-Item -LiteralPath $temporaryPackage -Destination $packagePath -Force
    Write-Output "Created $packagePath from $elfPath"
} finally {
    Remove-Item -LiteralPath $temporaryBin, $temporaryPackage -Force -ErrorAction SilentlyContinue
}
