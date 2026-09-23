param(
    [string]$SourceDir = (Join-Path $PSScriptRoot "libedit"),
    [string]$BuildDir = (Join-Path $PSScriptRoot "build-libedit-win"),
    [string]$InstallDir = (Join-Path $PSScriptRoot "libedit-win-arm64"),
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$LibEditRepository = "https://github.com/michaeljclark/libedit.git"
$LibEditRevision = "f3a98319002d0eca05e2a370a68b28a2afa6a822"
$LibEditPatch = Join-Path $PSScriptRoot "patch/libedit-windows-arm64.patch"

function Invoke-Checked {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Command)

    $executable = $Command[0]
    $arguments = $Command[1..($Command.Length - 1)]
    & $executable @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed ($LASTEXITCODE): $($Command -join ' ')"
    }
}

if (-not (Test-Path -LiteralPath (Join-Path $SourceDir ".git"))) {
    Invoke-Checked git clone $LibEditRepository $SourceDir
}

$currentRevision = (& git -C $SourceDir rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0) {
    throw "Cannot read the libedit revision in $SourceDir"
}

if ($currentRevision -ne $LibEditRevision) {
    & git -C $SourceDir diff --quiet
    if ($LASTEXITCODE -ne 0) {
        throw "libedit has tracked changes; refusing to change its revision."
    }
    Invoke-Checked git -C $SourceDir fetch origin $LibEditRevision
    Invoke-Checked git -C $SourceDir checkout --detach $LibEditRevision
}

& git -C $SourceDir apply --reverse --check $LibEditPatch 2>$null
if ($LASTEXITCODE -ne 0) {
    Invoke-Checked git -C $SourceDir apply --check $LibEditPatch
    Invoke-Checked git -C $SourceDir apply $LibEditPatch
}

Invoke-Checked cmake -S $SourceDir -B $BuildDir -G "Visual Studio 17 2022" -A ARM64 `
    "-DCMAKE_INSTALL_PREFIX=$InstallDir"
Invoke-Checked cmake --build $BuildDir --config $Configuration --target install

Write-Host "libedit Windows ARM64 installed in $InstallDir"
