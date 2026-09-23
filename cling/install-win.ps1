param(
    [string]$InstallDir = (Join-Path $env:LOCALAPPDATA "qtcling"),
    [string]$QtRoot = $(
        if ($env:QTCLING_QT_ROOT) { $env:QTCLING_QT_ROOT }
        else { "C:\Qt\6.11.1\msvc2022_arm64" }
    ),
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$ClingInstallDir = Join-Path $PSScriptRoot "qtcling-win-arm64"
$LibEditInstallDir = Join-Path $PSScriptRoot "libedit-win-arm64"
$RuntimeBuildDir = Join-Path $PSScriptRoot "build-qtcling-win-runtime"
$RuntimeDll = Join-Path $RuntimeBuildDir "bin\$Configuration\qtcling-win.dll"

function Find-CMake {
    $command = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $qtBaseDir = Split-Path -Parent (Split-Path -Parent $QtRoot)
    $candidates = @(
        (Join-Path $qtBaseDir "Tools\CMake\bin\cmake.exe"),
        "C:\Qt\Tools\CMake\bin\cmake.exe"
    ) | Select-Object -Unique

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    throw "CMake was not found. Install Qt's CMake component or add cmake.exe to PATH."
}

function Invoke-Checked {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Command)

    $executable = $Command[0]
    $arguments = $Command[1..($Command.Length - 1)]
    & $executable @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed ($LASTEXITCODE): $($Command -join ' ')"
    }
}

if (-not (Test-Path -LiteralPath (Join-Path $ClingInstallDir "bin\cling.exe"))) {
    throw "Cling is not installed in $ClingInstallDir. Run ./build-win.sh first."
}
if (-not (Test-Path -LiteralPath (Join-Path $LibEditInstallDir "share\libedit\termcap"))) {
    throw "libedit is not installed in $LibEditInstallDir. Run ./build-win.sh first."
}
if (-not (Test-Path -LiteralPath (Join-Path $QtRoot "include\QtCore"))) {
    throw "Qt ARM64 headers were not found under $QtRoot."
}

$CMake = Find-CMake
Write-Host "Using CMake: $CMake"

Invoke-Checked $CMake -S (Join-Path $PSScriptRoot "windows") -B $RuntimeBuildDir `
    -G "Visual Studio 17 2022" -A ARM64 "-DCMAKE_PREFIX_PATH=$QtRoot"
Invoke-Checked $CMake --build $RuntimeBuildDir --config $Configuration -j 2

if (-not (Test-Path -LiteralPath $RuntimeDll)) {
    throw "Qt callback DLL was not generated: $RuntimeDll"
}

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
Copy-Item -Path (Join-Path $ClingInstallDir "*") -Destination $InstallDir -Recurse -Force

$InstallBinDir = Join-Path $InstallDir "bin"
$InstallSourceDir = Join-Path $InstallDir "src"
$InstallTermcapDir = Join-Path $InstallDir "share\libedit"
New-Item -ItemType Directory -Force -Path `
    $InstallBinDir, $InstallSourceDir, $InstallTermcapDir | Out-Null

Copy-Item -LiteralPath (Join-Path $PSScriptRoot "bin\qtcling.cmd") `
    -Destination (Join-Path $InstallBinDir "qtcling.cmd") -Force
Copy-Item -LiteralPath $RuntimeDll `
    -Destination (Join-Path $InstallBinDir "qtcling-win.dll") -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot "src\qtguiwin-startup.cpp") `
    -Destination (Join-Path $InstallSourceDir "qtguiwin-startup.cpp") -Force
Copy-Item -LiteralPath (Join-Path $LibEditInstallDir "share\libedit\termcap") `
    -Destination (Join-Path $InstallTermcapDir "termcap") -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot "QTCLING_VERSION") `
    -Destination (Join-Path $InstallDir "QTCLING_VERSION") -Force

Write-Host "qtcling Windows ARM64 installed in $InstallDir"
Write-Host "Run: $InstallBinDir\qtcling.cmd"
