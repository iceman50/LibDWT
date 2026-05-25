param(
	[string]$OutputDir = "dist",
	[string]$Name = "LibDWT-source",
	[switch]$NoClean,
	[switch]$IncludeGit
)

$ErrorActionPreference = "Stop"

function Get-DwtVersionString {
	param([string]$RepoRoot)

	$versionHeaderPath = Join-Path $RepoRoot "dwt\include\dwt\Version.h"
	if (-not (Test-Path -LiteralPath $versionHeaderPath)) {
		throw "Version header not found: $versionHeaderPath"
	}

	$versionHeaderContent = Get-Content -LiteralPath $versionHeaderPath -Raw
	$versionMatch = [Regex]::Match($versionHeaderContent, '#define\s+DWT_VERSION_STRING\s+"([^"]+)"')
	if (-not $versionMatch.Success) {
		throw "Failed to extract DWT version string from: $versionHeaderPath"
	}

	return $versionMatch.Groups[1].Value
}

function Remove-PathIfExists {
	param([string]$Path)

	if (Test-Path -LiteralPath $Path) {
		Remove-Item -LiteralPath $Path -Recurse -Force
		Write-Host "Removed: $Path"
	}
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$resolvedRepoRoot = (Resolve-Path -LiteralPath $repoRoot).Path
$dwtVersion = Get-DwtVersionString -RepoRoot $resolvedRepoRoot

$tempDirectories = @(
	(Join-Path $resolvedRepoRoot "projects\mingw-w64\build")
	(Join-Path $resolvedRepoRoot "projects\mingw-w64\dist")
	(Join-Path $resolvedRepoRoot "dwt\Build")
	(Join-Path $resolvedRepoRoot "projects\msvc\Build")
	(Join-Path $resolvedRepoRoot "projects\msvc\x64")
	Join-Path $resolvedRepoRoot "projects\msvc\x86"
)

$tempFileExtensions = @(".obj", ".pdb", ".idb", ".ilk", ".pch", ".tlog", ".lastbuildstate", ".log", ".tmp", ".cache")

if (-not $NoClean) {
	Write-Host "Cleaning temporary folders..."
	foreach ($dir in $tempDirectories) {
		Remove-PathIfExists -Path $dir
	}

	Write-Host "Cleaning temporary files..."
	Get-ChildItem -Path $resolvedRepoRoot -Recurse -File -ErrorAction SilentlyContinue |
		Where-Object { $tempFileExtensions -contains $_.Extension.ToLowerInvariant() } |
		ForEach-Object {
			Remove-Item -LiteralPath $_.FullName -Force -ErrorAction SilentlyContinue
		}
}

$resolvedOutputDir = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
	$OutputDir
} else {
	Join-Path $resolvedRepoRoot $OutputDir
}

$repoRootWithSlash = $resolvedRepoRoot.TrimEnd('\') + '\'
$repoRootPrefixRegex = '^' + [Regex]::Escape($repoRootWithSlash)

if (-not (Test-Path -LiteralPath $resolvedOutputDir)) {
	New-Item -ItemType Directory -Path $resolvedOutputDir | Out-Null
}

$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$zipPath = Join-Path $resolvedOutputDir ("{0}-{1}-{2}.zip" -f $Name, $dwtVersion, $timestamp)

$includePatterns = @(
	"*.c", "*.cc", "*.cpp", "*.cxx",
	"*.h", "*.hh", "*.hpp", "*.hxx", "*.inl", "*.ipp",
	"*.rc", "*.ico", "*.manifest", "*.def",
	"*.vcxproj", "*.vcproj", "*.sln", "*.filters", "*.props", "*.targets", "*.mk",
	"Makefile", "makefile", "GNUmakefile"
)

$excludeRootNames = @(".vs", ".vscode", "projects\mingw-w64\build", "projects\mingw-w64\dist", "dist")
if (-not $IncludeGit) {
	$excludeRootNames += ".git"
}

$relativeOutputDir = $null
if ($resolvedOutputDir.StartsWith($repoRootWithSlash, [System.StringComparison]::OrdinalIgnoreCase)) {
	$relativeOutputDir = $resolvedOutputDir.Substring($repoRootWithSlash.Length)
}

Write-Host "Collecting files to package..."
$candidateFiles = New-Object 'System.Collections.Generic.List[System.IO.FileInfo]'
foreach ($pattern in $includePatterns) {
	Get-ChildItem -Path $resolvedRepoRoot -Recurse -File -Filter $pattern -ErrorAction SilentlyContinue |
		ForEach-Object { $candidateFiles.Add($_) | Out-Null }
}

$filesToPack = New-Object 'System.Collections.Generic.List[string]'
$seen = New-Object 'System.Collections.Generic.HashSet[string]' ([System.StringComparer]::OrdinalIgnoreCase)
foreach ($file in $candidateFiles) {
	$relativePath = $file.FullName -replace $repoRootPrefixRegex, ""
	if ($relativePath -eq $file.FullName) { continue }

	$isExcludedRoot = $false
	foreach ($excluded in $excludeRootNames) {
		if ($relativePath -ieq $excluded -or $relativePath.StartsWith($excluded + "\", [System.StringComparison]::OrdinalIgnoreCase)) {
			$isExcludedRoot = $true
			break
		}
	}
	if (-not $isExcludedRoot -and $relativeOutputDir) {
		if ($relativePath -ieq $relativeOutputDir -or $relativePath.StartsWith($relativeOutputDir + "\", [System.StringComparison]::OrdinalIgnoreCase)) {
			$isExcludedRoot = $true
		}
	}
	if ($isExcludedRoot) { continue }

	if ($seen.Add($relativePath)) {
		$filesToPack.Add($relativePath) | Out-Null
	}
}

if (Test-Path -LiteralPath $zipPath) {
	Remove-Item -LiteralPath $zipPath -Force
}

if ($filesToPack.Count -eq 0) {
	throw "No files matched packaging criteria."
}

Write-Host "Creating archive: $zipPath"
$stagingRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("libdwt-package-" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $stagingRoot | Out-Null

try {
	foreach ($relativePath in $filesToPack) {
		$sourcePath = Join-Path $resolvedRepoRoot $relativePath
		$targetPath = Join-Path $stagingRoot $relativePath
		$targetDir = Split-Path -Parent $targetPath
		if (-not (Test-Path -LiteralPath $targetDir)) {
			New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
		}
		Copy-Item -LiteralPath $sourcePath -Destination $targetPath -Force
	}

	Compress-Archive -Path (Join-Path $stagingRoot "*") -DestinationPath $zipPath -CompressionLevel Optimal
}
finally {
	if (Test-Path -LiteralPath $stagingRoot) {
		Remove-Item -LiteralPath $stagingRoot -Recurse -Force
	}
}

Write-Host "Done. Source package created at: $zipPath"
