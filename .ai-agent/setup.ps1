# AI Agent Setup Script (Windows PowerShell)
# Run from .ai-agent/ directory to set up AI agent in parent project

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SubrepoName = Split-Path -Leaf $ScriptDir
$ProjectRoot = Split-Path -Parent $ScriptDir

Write-Host "=== AI Agent Setup ===" -ForegroundColor Cyan
Write-Host "Subrepo: $ScriptDir"
Write-Host "Project: $ProjectRoot"
Write-Host ""

# 1. Create .claude symlink
$ClaudeLink = Join-Path $ProjectRoot ".claude"
$ClaudeTarget = Join-Path $SubrepoName ".claude"

if (Test-Path $ClaudeLink) {
    $item = Get-Item $ClaudeLink -Force
    if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
        Write-Host "[OK] .claude symlink already exists" -ForegroundColor Green
    } elseif ($item.PSIsContainer) {
        # Auto-migrate: Claude Code creates .claude/ automatically, we need to replace it
        Write-Host "[MIGRATE] .claude directory found (created by Claude Code)" -ForegroundColor Yellow

        # Backup user settings if they exist
        $SettingsFile = Join-Path $ClaudeLink "settings.local.json"
        $SettingsBackup = $null
        if (Test-Path $SettingsFile) {
            $SettingsBackup = [System.IO.Path]::GetTempFileName()
            Copy-Item $SettingsFile $SettingsBackup
            Write-Host "[BACKUP] settings.local.json saved" -ForegroundColor Yellow
        }

        # Remove the directory
        Remove-Item $ClaudeLink -Recurse -Force
        Write-Host "[REMOVE] .claude directory removed" -ForegroundColor Yellow

        # Create symlink
        cmd /c mklink /D "$ClaudeLink" "$ClaudeTarget" 2>$null
        if (-not $?) {
            try {
                New-Item -ItemType SymbolicLink -Path $ClaudeLink -Target $ClaudeTarget -Force | Out-Null
            } catch {
                Write-Host "[ERROR] Failed to create symlink. Enable Developer Mode or run as Admin." -ForegroundColor Red
                exit 1
            }
        }
        Write-Host "[CREATE] .claude -> $ClaudeTarget" -ForegroundColor Yellow

        # Restore user settings
        if ($SettingsBackup -and (Test-Path $SettingsBackup)) {
            $NewSettingsFile = Join-Path $ClaudeLink "settings.local.json"
            Copy-Item $SettingsBackup $NewSettingsFile
            Remove-Item $SettingsBackup
            Write-Host "[RESTORE] settings.local.json restored" -ForegroundColor Yellow
        }
    } else {
        Write-Host "[ERROR] .claude exists but is not a symlink or directory" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "[CREATE] .claude -> $ClaudeTarget" -ForegroundColor Yellow
    # Use cmd mklink for directory symlink (requires admin or developer mode)
    $targetFull = Join-Path $ProjectRoot $ClaudeTarget
    cmd /c mklink /D "$ClaudeLink" "$ClaudeTarget" 2>$null
    if (-not $?) {
        # Fallback: try New-Item (requires PowerShell 5+ and permissions)
        try {
            New-Item -ItemType SymbolicLink -Path $ClaudeLink -Target $ClaudeTarget -Force | Out-Null
        } catch {
            Write-Host "[ERROR] Failed to create symlink. Enable Developer Mode or run as Admin." -ForegroundColor Red
            exit 1
        }
    }
}

# 2. Copy src/ contents to project root (including dotfiles)
$SrcDir = Join-Path $ScriptDir "src"
if (Test-Path $SrcDir) {
    Write-Host ""
    Write-Host "=== Copying src/ contents ===" -ForegroundColor Cyan
    Get-ChildItem -Path $SrcDir -Force | ForEach-Object {
        $dest = Join-Path $ProjectRoot $_.Name
        if (Test-Path $dest) {
            Write-Host "[SKIP] $($_.Name) already exists" -ForegroundColor Gray
        } else {
            Write-Host "[COPY] $($_.Name)" -ForegroundColor Yellow
            Copy-Item -Path $_.FullName -Destination $dest -Recurse
        }
    }
}

# 3. Ensure .gitignore has .claude entry
$Gitignore = Join-Path $ProjectRoot ".gitignore"
Write-Host ""
Write-Host "=== Updating .gitignore ===" -ForegroundColor Cyan

if (-not (Test-Path $Gitignore)) {
    Write-Host "[CREATE] .gitignore" -ForegroundColor Yellow
    # Use ASCII to avoid BOM issues
    Set-Content -Path $Gitignore -Value ".claude`n.cvm" -Encoding ASCII
} else {
    $content = Get-Content $Gitignore -Raw
    if ($content -match "(?m)^\.claude$") {
        Write-Host "[OK] .claude already in .gitignore" -ForegroundColor Green
    } else {
        Write-Host "[ADD] .claude to .gitignore" -ForegroundColor Yellow
        Add-Content -Path $Gitignore -Value ".claude" -Encoding ASCII
    }
    if ($content -match "(?m)^\.cvm$") {
        Write-Host "[OK] .cvm already in .gitignore" -ForegroundColor Green
    } else {
        Write-Host "[ADD] .cvm to .gitignore" -ForegroundColor Yellow
        Add-Content -Path $Gitignore -Value ".cvm" -Encoding ASCII
    }
}

# 4. Symlink memory-bank/README.md (grammar comes from submodule)
$MBDir = Join-Path $ProjectRoot "memory-bank"
$MBReadme = Join-Path $MBDir "README.md"
$MBReadmeTarget = Join-Path ".." (Join-Path $SubrepoName "src/memory-bank/README.md")

if (Test-Path $MBDir) {
    Write-Host ""
    Write-Host "=== Memory Bank Setup ===" -ForegroundColor Cyan

    $item = if (Test-Path $MBReadme) { Get-Item $MBReadme -Force } else { $null }
    if ($item -and ($item.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        Write-Host "[OK] memory-bank/README.md symlink already exists" -ForegroundColor Green
    } else {
        if (Test-Path $MBReadme) { Remove-Item $MBReadme -Force }
        Write-Host "[LINK] memory-bank/README.md -> $MBReadmeTarget" -ForegroundColor Yellow
        cmd /c mklink "$MBReadme" "$MBReadmeTarget" 2>$null
        if (-not $?) {
            try {
                New-Item -ItemType SymbolicLink -Path $MBReadme -Target $MBReadmeTarget -Force | Out-Null
            } catch {
                Write-Host "[WARN] Could not create symlink, keeping copy" -ForegroundColor Yellow
            }
        }
    }

    # Ensure archive folder exists
    $ArchiveDir = Join-Path $MBDir "archive"
    if (-not (Test-Path $ArchiveDir)) {
        New-Item -ItemType Directory -Path $ArchiveDir -Force | Out-Null
    }
}

Write-Host ""
Write-Host "=== Setup Complete ===" -ForegroundColor Green
Write-Host "AI agent is now enabled for this project."
