#Requires -Version 5.1

<#
.SYNOPSIS
Interactively builds LibDWT for MSVC x64, MinGW-w64 x64, or both.

.DESCRIPTION
With no arguments, this script presents menus for the toolchain, build
configuration, clean mode, and test execution. Supplying both -Toolchain and
-Configuration skips the menus, making the same entry point suitable for CI.
Completed executables, libraries, and debug symbols are staged under
Builds\<compiler>\<configuration> at the repository root.

.EXAMPLE
.\scripts\build-interactive.ps1

.EXAMPLE
.\scripts\build-interactive.ps1 -Toolchain Both -Configuration Both -Clean

.EXAMPLE
.\scripts\build-interactive.ps1 -Toolchain MinGW -Configuration Debug -SkipTests

.EXAMPLE
.\scripts\build-interactive.ps1 -Toolchain MinGW -Configuration Release -j 12
#>

param(
    [ValidateSet("MSVC", "MinGW", "Both")]
    [string]$Toolchain,

    [ValidateSet("Debug", "Release", "Both")]
    [string]$Configuration,

    [string]$PlatformToolset = "",

    [Alias("j")]
    [ValidateRange(1, 256)]
    [int]$Jobs = [Environment]::ProcessorCount,

    [switch]$Clean,
    [switch]$SkipTests,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

function Read-MenuChoice {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Title,

        [Parameter(Mandatory = $true)]
        [object[]]$Options
    )

    while ($true) {
        Write-Host ""
        Write-Host $Title -ForegroundColor Cyan
        foreach ($option in $Options) {
            Write-Host ("  {0}) {1}" -f $option.Key, $option.Label)
        }

        $answer = (Read-Host "Select an option").Trim()
        $selected = $Options | Where-Object { $_.Key -eq $answer } |
            Select-Object -First 1
        if ($selected) {
            return $selected.Value
        }

        Write-Host "Invalid selection. Please try again." -ForegroundColor Yellow
    }
}

function Read-JobCount {
    param(
        [int]$Default
    )

    while ($true) {
        Write-Host ""
        $answer = (Read-Host "MinGW-w64 parallel jobs [$Default]").Trim()
        if (-not $answer) {
            return $Default
        }

        $parsed = 0
        if ([int]::TryParse($answer, [ref]$parsed) -and
            $parsed -ge 1 -and $parsed -le 256) {
            return $parsed
        }

        Write-Host "Enter a number from 1 through 256." -ForegroundColor Yellow
    }
}

function Format-Command {
    param(
        [string]$Executable,
        [string[]]$Arguments
    )

    $formattedArguments = @($Arguments | ForEach-Object {
        if ($_ -match '\s') { '"{0}"' -f $_ } else { $_ }
    })
    return (@($Executable) + $formattedArguments) -join " "
}

function Invoke-NativeBuildCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Executable,

        [string[]]$Arguments = @(),

        [Parameter(Mandatory = $true)]
        [string]$WorkingDirectory
    )

    Write-Host -Object ("> {0}" -f (Format-Command $Executable $Arguments)) `
        -ForegroundColor DarkGray
    if ($DryRun) {
        return
    }

    Push-Location $WorkingDirectory
    try {
        & $Executable @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "Command failed with exit code ${LASTEXITCODE}: $(Format-Command $Executable $Arguments)"
        }
    }
    finally {
        Pop-Location
    }
}

function Get-MinGWMakePath {
    foreach ($candidate in @("mingw32-make.exe", "mingw32-make", "make.exe", "make")) {
        $command = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($command) {
            return $command.Source
        }
    }

    throw "A Make executable was not found. Install MinGW-w64 and add mingw32-make to PATH."
}

function Publish-Artifacts {
    param(
        [Parameter(Mandatory = $true)]
        [AllowEmptyCollection()]
        [object[]]$Files,

        [Parameter(Mandatory = $true)]
        [string]$Destination
    )

    if ($DryRun) {
        Write-Host -Object ("> Stage build artifacts in {0}" -f $Destination) `
            -ForegroundColor DarkGray
        return
    }

    $retiredArtifacts = @("WindowsSdkValidation.exe", "WindowsSdkValidation.pdb")
    $filesToPublish = @($Files | Where-Object {
        $_ -and $_.Exists -and $retiredArtifacts -notcontains $_.Name
    })
    if ($filesToPublish.Count -eq 0) {
        throw "No build artifacts were found to stage in $Destination."
    }

    New-Item -ItemType Directory -Path $Destination -Force | Out-Null

    # The staging folders are flat. Remove previously staged binary files so a
    # renamed or removed target cannot leave a stale artifact behind.
    $artifactExtensions = @(".exe", ".dll", ".lib", ".a", ".pdb", ".exp")
    Get-ChildItem -Path $Destination -File -ErrorAction SilentlyContinue |
        Where-Object { $artifactExtensions -contains $_.Extension.ToLowerInvariant() } |
        Remove-Item -Force

    Write-Host ("Staging artifacts in {0}" -f $Destination) -ForegroundColor Green
    foreach ($file in $filesToPublish) {
        Copy-Item -LiteralPath $file.FullName -Destination $Destination -Force
        Write-Host ("  + {0}" -f $file.Name)
    }

    # Keep the repository-level example icons beside every staged toolchain.
    # MultiControlExample embeds them, while other examples may load the same
    # stable res\<name>.ico paths at runtime.
    $repositoryRoot = Split-Path -Parent $PSScriptRoot
    $resourceSource = Join-Path $repositoryRoot "res"
    if (Test-Path $resourceSource) {
        $resourceDestination = Join-Path $Destination "res"
        New-Item -ItemType Directory -Path $resourceDestination -Force | Out-Null
        Copy-Item -Path (Join-Path $resourceSource "*.ico") `
            -Destination $resourceDestination -Force
        Write-Host "  + res\*.ico"
    }
}

function Publish-MSVCArtifacts {
    param(
        [string]$Configuration,
        [string]$RepositoryRoot,
        [string]$ArtifactRoot
    )

    $destination = Join-Path $ArtifactRoot ("MSVC\{0}" -f $Configuration)
    if ($DryRun) {
        Publish-Artifacts @() $destination
        return
    }

    $msvcRoot = Join-Path $RepositoryRoot "projects\msvc"
    $sourceDirectories = @(
        (Join-Path $msvcRoot "dwt\build\x64\$Configuration")
    )

    foreach ($group in @("examples", "tests")) {
        $groupRoot = Join-Path $msvcRoot $group
        if (Test-Path $groupRoot) {
            $sourceDirectories += Get-ChildItem -Path $groupRoot -Directory |
                ForEach-Object { Join-Path $_.FullName "build\x64\$Configuration" }
        }
    }

    $extensions = @(".exe", ".dll", ".lib", ".pdb", ".exp")
    $files = @($sourceDirectories | ForEach-Object {
        if (Test-Path $_) {
            Get-ChildItem -Path $_ -File | Where-Object {
                $extensions -contains $_.Extension.ToLowerInvariant()
            }
        }
    })

    Publish-Artifacts $files $destination
}

function Publish-MinGWArtifacts {
    param(
        [string]$Configuration,
        [string]$ProjectDirectory,
        [string]$ArtifactRoot
    )

    $destination = Join-Path $ArtifactRoot ("MinGW-w64\{0}" -f $Configuration)
    if ($DryRun) {
        Publish-Artifacts @() $destination
        return
    }

    $configurationName = $Configuration.ToLowerInvariant()
    $configurationRoot = Join-Path $ProjectDirectory "build\x64\$configurationName"
    $sourceDirectories = @(
        (Join-Path $configurationRoot "bin"),
        (Join-Path $configurationRoot "lib")
    )
    $extensions = @(".exe", ".dll", ".a")
    $files = @($sourceDirectories | ForEach-Object {
        if (Test-Path $_) {
            Get-ChildItem -Path $_ -File | Where-Object {
                $extensions -contains $_.Extension.ToLowerInvariant()
            }
        }
    })

    Publish-Artifacts $files $destination
}

function Invoke-MSVCBuild {
    param(
        [string[]]$Configurations,
        [string]$BuildScript
    )

    Write-Host ""
    Write-Host "=== MSVC x64 ===" -ForegroundColor Cyan

    $arguments = @{
        Configurations = $Configurations
        Platforms = @("x64")
        Clean = $Clean
        SkipTests = $SkipTests
    }
    if ($PlatformToolset) {
        $arguments.PlatformToolset = $PlatformToolset
    }

    $displayArguments = @(
        "-Configurations", ($Configurations -join ","),
        "-Platforms", "x64"
    )
    if ($Clean) { $displayArguments += "-Clean" }
    if ($SkipTests) { $displayArguments += "-SkipTests" }
    if ($PlatformToolset) {
        $displayArguments += @("-PlatformToolset", $PlatformToolset)
    }
    Write-Host -Object ("> {0}" -f (Format-Command $BuildScript $displayArguments)) `
        -ForegroundColor DarkGray

    if (-not $DryRun) {
        & $BuildScript @arguments
    }
}

function Invoke-MinGWBuild {
    param(
        [string[]]$Configurations,
        [string]$ProjectDirectory
    )

    Write-Host ""
    Write-Host "=== MinGW-w64 x64 ===" -ForegroundColor Cyan

    $make = if ($DryRun) {
        $detected = Get-Command "mingw32-make.exe" -ErrorAction SilentlyContinue
        if ($detected) { $detected.Source } else { "mingw32-make" }
    } else {
        Get-MinGWMakePath
    }

    Write-Host "Using Make: $make"
    Write-Host "Parallel jobs: $Jobs"

    # The Makefile clean target removes the complete MinGW build tree, so clean
    # once before building multiple configurations rather than between them.
    if ($Clean) {
        Invoke-NativeBuildCommand $make @("clean") $ProjectDirectory
    }

    foreach ($item in $Configurations) {
        $makeConfiguration = $item.ToLowerInvariant()
        Write-Host ""
        Write-Host "Building MinGW-w64 x64 $item" -ForegroundColor Green
        $buildArguments = @("-j$Jobs", "build", "ARCH=x64", "CONFIG=$makeConfiguration")
        Invoke-NativeBuildCommand $make $buildArguments $ProjectDirectory

        if (-not $SkipTests) {
            Write-Host "Running MinGW-w64 x64 $item tests" -ForegroundColor Green
            $testArguments = @("-j$Jobs", "test", "ARCH=x64", "CONFIG=$makeConfiguration")
            Invoke-NativeBuildCommand $make $testArguments $ProjectDirectory
        }
    }
}

$interactive = -not $Toolchain -or -not $Configuration

if (-not $Toolchain) {
    $Toolchain = Read-MenuChoice "Choose toolchain" @(
        [pscustomobject]@{ Key = "1"; Label = "MSVC x64"; Value = "MSVC" },
        [pscustomobject]@{ Key = "2"; Label = "MinGW-w64 x64"; Value = "MinGW" },
        [pscustomobject]@{ Key = "3"; Label = "Both toolchains"; Value = "Both" }
    )
}

if (-not $Configuration) {
    $Configuration = Read-MenuChoice "Choose configuration" @(
        [pscustomobject]@{ Key = "1"; Label = "Debug"; Value = "Debug" },
        [pscustomobject]@{ Key = "2"; Label = "Release"; Value = "Release" },
        [pscustomobject]@{ Key = "3"; Label = "Debug and Release"; Value = "Both" }
    )
}

if ($interactive -and ($Toolchain -eq "MinGW" -or $Toolchain -eq "Both") -and
    -not $PSBoundParameters.ContainsKey("Jobs")) {
    $Jobs = Read-JobCount $Jobs
}

if ($interactive -and -not $PSBoundParameters.ContainsKey("Clean")) {
    $Clean = (Read-MenuChoice "Choose build mode" @(
        [pscustomobject]@{ Key = "1"; Label = "Incremental build"; Value = $false },
        [pscustomobject]@{ Key = "2"; Label = "Clean then build"; Value = $true }
    ))
}

if ($interactive -and -not $PSBoundParameters.ContainsKey("SkipTests")) {
    $runTests = Read-MenuChoice "Run FrameworkTests after building?" @(
        [pscustomobject]@{ Key = "1"; Label = "Yes"; Value = $true },
        [pscustomobject]@{ Key = "2"; Label = "No"; Value = $false }
    )
    $SkipTests = -not $runTests
}

$configurations = if ($Configuration -eq "Both") {
    @("Debug", "Release")
} else {
    @($Configuration)
}

$toolchains = if ($Toolchain -eq "Both") {
    @("MSVC", "MinGW")
} else {
    @($Toolchain)
}

Write-Host ""
Write-Host "=== Build plan ===" -ForegroundColor Cyan
Write-Host ("Toolchains:     {0}" -f ($toolchains -join ", "))
Write-Host "Architecture:   x64"
Write-Host ("Configurations: {0}" -f ($configurations -join ", "))
Write-Host "Artifacts:      Builds\<compiler>\<configuration>"
if ($toolchains -contains "MinGW") {
    Write-Host ("MinGW jobs:    {0}" -f $Jobs)
}
Write-Host ("Build mode:     {0}" -f $(if ($Clean) { "clean" } else { "incremental" }))
Write-Host ("Tests:          {0}" -f $(if ($SkipTests) { "skip" } else { "run" }))
Write-Host ("Execution:      {0}" -f $(if ($DryRun) { "dry run" } else { "build" }))

if ($interactive) {
    $confirmation = (Read-Host "Continue? [Y/n]").Trim()
    if ($confirmation -and $confirmation -notmatch '^[Yy]$') {
        Write-Host "Build cancelled."
        return
    }
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$msvcBuildScript = Join-Path $PSScriptRoot "build.ps1"
$mingwProjectDirectory = Join-Path $repoRoot "projects\mingw-w64"
$artifactRoot = Join-Path $repoRoot "Builds"

if (-not (Test-Path $msvcBuildScript)) {
    throw "MSVC build script not found: $msvcBuildScript"
}
if (-not (Test-Path (Join-Path $mingwProjectDirectory "Makefile"))) {
    throw "MinGW-w64 Makefile not found: $mingwProjectDirectory"
}

$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
try {
    if ($toolchains -contains "MSVC") {
        Invoke-MSVCBuild $configurations $msvcBuildScript
        foreach ($item in $configurations) {
            Publish-MSVCArtifacts $item $repoRoot $artifactRoot
        }
    }
    if ($toolchains -contains "MinGW") {
        Invoke-MinGWBuild $configurations $mingwProjectDirectory
        foreach ($item in $configurations) {
            Publish-MinGWArtifacts $item $mingwProjectDirectory $artifactRoot
        }
    }

    Write-Host ""
    Write-Host "All requested builds completed successfully." -ForegroundColor Green
}
finally {
    $stopwatch.Stop()
    $elapsed = $stopwatch.Elapsed
    $formattedElapsed = "{0:00}:{1:00}:{2:00}.{3:000}" -f $elapsed.Hours,
        $elapsed.Minutes, $elapsed.Seconds, $elapsed.Milliseconds
    Write-Host "Total build time: $formattedElapsed"
}
