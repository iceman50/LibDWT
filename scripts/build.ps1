param(
    [string[]]$Configurations = @("Release"),
    [string[]]$Platforms = @("x64"),
    [string]$PlatformToolset = "",
    [switch]$Clean,
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"

$Configurations = @($Configurations | ForEach-Object { $_ -split "," } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$Platforms = @($Platforms | ForEach-Object { $_ -split "," } | ForEach-Object { $_.Trim() } | Where-Object { $_ })

function Get-MSBuildPath {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $latest = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
        if ($latest) {
            return $latest
        }
    }

    $fromPath = Get-Command MSBuild.exe -ErrorAction SilentlyContinue
    if ($fromPath) {
        return $fromPath.Source
    }

    throw "MSBuild.exe not found. Install Visual Studio Build Tools or Visual Studio 2022."
}

function Get-DetectedPlatformToolset {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        return $null
    }

    $vcToolsVersionFile = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt" | Select-Object -First 1
    if (-not $vcToolsVersionFile -or -not (Test-Path $vcToolsVersionFile)) {
        return $null
    }

    $versionText = (Get-Content -Path $vcToolsVersionFile -TotalCount 1).Trim()
    if (-not $versionText) {
        return $null
    }

    $version = [Version]$versionText
    if ($version.Major -ge 15) { return "v150" }
    if ($version.Major -eq 14 -and $version.Minor -ge 50) { return "v145" }
    if ($version.Major -eq 14 -and $version.Minor -ge 30) { return "v143" }
    if ($version.Major -eq 14 -and $version.Minor -ge 20) { return "v142" }
    if ($version.Major -eq 14 -and $version.Minor -ge 10) { return "v141" }
    return $null
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$msbuild = Get-MSBuildPath

if (-not $PlatformToolset) {
    $PlatformToolset = Get-DetectedPlatformToolset
}

$dwtProject = Join-Path $repoRoot "projects\msvc\dwt\dwt.vcxproj"
$exampleProjectsRoot = Join-Path $repoRoot "projects\msvc\examples"
$exampleProjects = @()
if (Test-Path $exampleProjectsRoot) {
    $exampleProjects = Get-ChildItem -Path $exampleProjectsRoot -Recurse -Filter *.vcxproj -File | ForEach-Object { $_.FullName }
}

$testProjectsRoot = Join-Path $repoRoot "projects\msvc\tests"
$testProjects = @()
if (Test-Path $testProjectsRoot) {
    $testProjects = Get-ChildItem -Path $testProjectsRoot -Recurse -Filter *.vcxproj -File | ForEach-Object { $_.FullName }
}

$projects = @($dwtProject) + $exampleProjects + $testProjects

if ($projects.Count -eq 0) {
    throw "No projects found to build."
}

$target = if ($Clean) { "Clean;Build" } else { "Build" }

Write-Host "Using MSBuild: $msbuild"
if ($PlatformToolset) {
    Write-Host "Using platform toolset: $PlatformToolset"
} else {
    Write-Host "Using project default platform toolset."
}
Write-Host "Build target: $target"

$buildStopwatch = [System.Diagnostics.Stopwatch]::StartNew()

try {
    foreach ($configuration in $Configurations) {
        foreach ($platform in $Platforms) {
            foreach ($project in $projects) {
                Write-Host "Building $project ($configuration|$platform)"
                $properties = "/p:Configuration=$configuration;Platform=$platform"
                if ($PlatformToolset) {
                    $properties = "$properties;DwtPlatformToolset=$PlatformToolset"
                }
                if ($project -ne $dwtProject) {
                    # dwt is built first for every configuration. Avoid rebuilding
                    # the same project through every example/test reference,
                    # especially for Clean;Build runs.
                    $properties = "$properties;BuildProjectReferences=false"
                }
                & $msbuild $project "/m" "/nologo" "/verbosity:minimal" "/t:$target" $properties
                if ($LASTEXITCODE -ne 0) {
                    throw "Build failed for $project ($configuration|$platform)."
                }
            }
            if (-not $SkipTests) {
                $testExecutable = Join-Path $testProjectsRoot "FrameworkTests\build\$platform\$configuration\FrameworkTests.exe"
                if (-not (Test-Path $testExecutable)) {
                    throw "Test executable not found: $testExecutable"
                }
                Write-Host "Running $testExecutable"
                & $testExecutable
                if ($LASTEXITCODE -ne 0) {
                    throw "Tests failed ($configuration|$platform)."
                }

                $validationExecutable = Join-Path $exampleProjectsRoot "FrameworkValidation\build\$platform\$configuration\FrameworkValidation.exe"
                if (-not (Test-Path $validationExecutable)) {
                    throw "Validation executable not found: $validationExecutable"
                }
                Write-Host "Running $validationExecutable --self-test"
                & $validationExecutable --self-test
                if ($LASTEXITCODE -ne 0) {
                    throw "Framework validation self-test failed ($configuration|$platform)."
                }
            }
        }
    }

    Write-Host "All requested builds completed successfully."
}
finally {
    $buildStopwatch.Stop()
    $elapsed = $buildStopwatch.Elapsed
    $formattedElapsed = "{0:00}:{1:00}:{2:00}.{3:000}" -f $elapsed.Hours, $elapsed.Minutes, $elapsed.Seconds, $elapsed.Milliseconds
    Write-Host "Total build time: $formattedElapsed"
}
