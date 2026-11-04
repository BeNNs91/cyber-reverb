param([string]$JucePath = '', [string]$Configuration = 'Release')
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$options = @('-S', '.', '-B', 'build', '-G', 'Visual Studio 18 2026', '-A', 'x64')
if ($JucePath) { $options += "-DFETCHCONTENT_SOURCE_DIR_JUCE=$JucePath" }
& cmake @options
if ($LASTEXITCODE) { throw 'CMake configure failed' }
& cmake --build build --config $Configuration -- /m:1 /v:minimal
if ($LASTEXITCODE) { throw 'Build failed' }
& ctest --test-dir build -C $Configuration --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed' }
