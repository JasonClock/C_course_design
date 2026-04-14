param(
    [switch]$SkipBuild
)

Set-Location $PSScriptRoot

chcp 65001 | Out-Null
$OutputEncoding = [Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()

$buildDir = Join-Path $PSScriptRoot "cmake-build-debug"
$exePath = Join-Path $buildDir "C.exe"

if (-not $SkipBuild) {
    cmake -S . -B $buildDir
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    cmake --build $buildDir
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if (-not (Test-Path $exePath)) {
    Write-Error "未找到可执行文件: $exePath"
    exit 1
}

& $exePath
exit $LASTEXITCODE

