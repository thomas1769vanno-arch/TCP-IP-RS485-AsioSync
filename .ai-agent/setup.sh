#!/bin/bash
# AI Agent Setup Script (Unix)
# Run from .ai-agent/ directory to set up AI agent in parent project

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SUBREPO_NAME="$(basename "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== AI Agent Setup ==="
echo "Subrepo: $SCRIPT_DIR"
echo "Project: $PROJECT_ROOT"
echo ""

# Helper: create a link (.claude or memory-bank/README.md)
# Uses symlink on Unix/Dev-mode Windows, junction on Windows without dev mode
create_link() {
    local target="$1"
    local link_path="$2"
    local is_dir="$3"  # "dir" or "file"

    # Try symlink first
    if ln -s "$target" "$link_path" 2>/dev/null; then
        return 0
    fi

    # Symlink failed - try Windows junction for directories
    if [ "$is_dir" = "dir" ] && command -v cmd.exe >/dev/null 2>&1; then
        local win_target
        local win_link
        win_target="$(cd "$(dirname "$link_path")" && cd "$target" && pwd -W 2>/dev/null || cygpath -w "$(dirname "$link_path")/$target" 2>/dev/null)" || true
        win_link="$(pwd -W 2>/dev/null && echo "\\$(basename "$link_path")" || cygpath -w "$link_path" 2>/dev/null)" || true

        if [ -n "$win_target" ] && [ -n "$win_link" ]; then
            cmd.exe /c "mklink /J \"$win_link\" \"$win_target\"" >/dev/null 2>&1 && return 0
        fi
    fi

    # All methods failed - copy as last resort
    if [ "$is_dir" = "dir" ]; then
        cp -r "$(dirname "$link_path")/$target" "$link_path"
        echo "[WARN] Could not create symlink or junction, copied files instead"
        echo "[WARN] Changes to $SUBREPO_NAME won't auto-propagate to $link_path"
    else
        cp "$(dirname "$link_path")/$target" "$link_path"
        echo "[WARN] Could not create symlink, copied file instead"
    fi
    return 0
}

# 1. Create .claude symlink (or junction on Windows)
CLAUDE_LINK="$PROJECT_ROOT/.claude"
CLAUDE_TARGET="$SUBREPO_NAME/.claude"

if [ -L "$CLAUDE_LINK" ]; then
    echo "[OK] .claude symlink already exists"
elif [ -d "$CLAUDE_LINK" ]; then
    # Check if it's a Windows junction (not a symlink but points to the right place)
    if [ -f "$CLAUDE_LINK/commands/j-help.md" ] 2>/dev/null; then
        echo "[OK] .claude directory exists (junction or copy)"
    else
        # Auto-migrate: Claude Code creates .claude/ automatically, we need to replace it
        echo "[MIGRATE] .claude directory found (created by Claude Code)"

        # Backup user settings if they exist
        SETTINGS_BACKUP=""
        if [ -f "$CLAUDE_LINK/settings.local.json" ]; then
            SETTINGS_BACKUP=$(mktemp)
            cp "$CLAUDE_LINK/settings.local.json" "$SETTINGS_BACKUP"
            echo "[BACKUP] settings.local.json saved"
        fi

        # Remove the directory
        rm -rf "$CLAUDE_LINK"
        echo "[REMOVE] .claude directory removed"

        # Create link
        cd "$PROJECT_ROOT"
        create_link "$CLAUDE_TARGET" "$CLAUDE_LINK" "dir"
        echo "[CREATE] .claude -> $CLAUDE_TARGET"
        cd "$SCRIPT_DIR"

        # Restore user settings
        if [ -n "$SETTINGS_BACKUP" ] && [ -f "$SETTINGS_BACKUP" ]; then
            cp "$SETTINGS_BACKUP" "$CLAUDE_LINK/settings.local.json"
            rm "$SETTINGS_BACKUP"
            echo "[RESTORE] settings.local.json restored"
        fi
    fi
elif [ -e "$CLAUDE_LINK" ]; then
    echo "[ERROR] .claude exists but is not a symlink or directory"
    exit 1
else
    cd "$PROJECT_ROOT"
    create_link "$CLAUDE_TARGET" "$CLAUDE_LINK" "dir"
    echo "[CREATE] .claude -> $CLAUDE_TARGET"
    cd "$SCRIPT_DIR"
fi

# 2. Copy src/ contents to project root (including dotfiles)
if [ -d "$SCRIPT_DIR/src" ]; then
    echo ""
    echo "=== Copying src/ contents ==="
    shopt -s dotglob
    for item in "$SCRIPT_DIR/src"/*; do
        if [ -e "$item" ]; then
            name="$(basename "$item")"
            dest="$PROJECT_ROOT/$name"
            if [ -e "$dest" ]; then
                echo "[SKIP] $name already exists"
            else
                echo "[COPY] $name"
                cp -r "$item" "$dest"
            fi
        fi
    done
    shopt -u dotglob
fi

# 3. Ensure .gitignore has .claude entry
GITIGNORE="$PROJECT_ROOT/.gitignore"
echo ""
echo "=== Updating .gitignore ==="

if [ ! -f "$GITIGNORE" ]; then
    echo "[CREATE] .gitignore"
    printf ".claude\n.cvm\n" > "$GITIGNORE"
else
    if grep -qx ".claude" "$GITIGNORE" 2>/dev/null; then
        echo "[OK] .claude already in .gitignore"
    else
        echo "[ADD] .claude to .gitignore"
        echo ".claude" >> "$GITIGNORE"
    fi
    if grep -qx ".cvm" "$GITIGNORE" 2>/dev/null; then
        echo "[OK] .cvm already in .gitignore"
    else
        echo "[ADD] .cvm to .gitignore"
        echo ".cvm" >> "$GITIGNORE"
    fi
fi

# 4. Symlink memory-bank/README.md (grammar comes from submodule)
MB_README="$PROJECT_ROOT/memory-bank/README.md"
MB_README_TARGET="../$SUBREPO_NAME/src/memory-bank/README.md"

if [ -d "$PROJECT_ROOT/memory-bank" ]; then
    echo ""
    echo "=== Memory Bank Setup ==="

    # Replace README with symlink
    if [ -L "$MB_README" ]; then
        echo "[OK] memory-bank/README.md symlink already exists"
    else
        [ -f "$MB_README" ] && rm "$MB_README"
        echo "[LINK] memory-bank/README.md -> $MB_README_TARGET"
        cd "$PROJECT_ROOT/memory-bank"
        create_link "../$SUBREPO_NAME/src/memory-bank/README.md" "$MB_README" "file"
        cd "$SCRIPT_DIR"
    fi

    # Ensure archive folder exists
    mkdir -p "$PROJECT_ROOT/memory-bank/archive"
fi

# 5. Install pre-commit hook to prevent .ai-agent being committed as regular directory
HOOKS_DIR="$PROJECT_ROOT/.git/hooks"
PRECOMMIT="$HOOKS_DIR/pre-commit"
HOOK_MARKER="# ai-agent-submodule-guard"

echo ""
echo "=== Installing pre-commit hook ==="

if [ -d "$HOOKS_DIR" ]; then
    # Check if our hook is already installed
    if [ -f "$PRECOMMIT" ] && grep -q "$HOOK_MARKER" "$PRECOMMIT" 2>/dev/null; then
        echo "[OK] pre-commit hook already installed"
    else
        # Create or append to pre-commit hook
        if [ ! -f "$PRECOMMIT" ]; then
            cat > "$PRECOMMIT" << 'HOOKEOF'
#!/bin/bash
HOOKEOF
        fi

        cat >> "$PRECOMMIT" << 'HOOKEOF'

# ai-agent-submodule-guard
# Prevents .ai-agent from being committed as a regular directory (must be a submodule)
ai_agent_entry=$(git ls-files --stage .ai-agent 2>/dev/null | head -1)
if echo "$ai_agent_entry" | grep -q "^040000"; then
    echo ""
    echo "ERROR: .ai-agent is staged as a regular directory, not a submodule!"
    echo "This would break the submodule structure for other developers."
    echo ""
    echo "Fix: git rm -r --cached .ai-agent && git submodule add <url> .ai-agent"
    echo ""
    exit 1
fi
# Prevents .claude from being committed (should be in .gitignore)
if git ls-files --cached .claude/ 2>/dev/null | grep -q .; then
    echo ""
    echo "ERROR: .claude/ is staged for commit but should be in .gitignore!"
    echo ".claude is a local symlink/junction and must not be tracked by git."
    echo ""
    echo "Fix: git rm -r --cached .claude && echo '.claude' >> .gitignore"
    echo ""
    exit 1
fi
HOOKEOF

        chmod +x "$PRECOMMIT"
        echo "[CREATE] pre-commit hook installed"
    fi
else
    echo "[SKIP] .git/hooks not found (not a git repo?)"
fi

# 6. Verify setup
echo ""
echo "=== Verification ==="

ERRORS=0

# Check .claude is accessible
if [ -f "$CLAUDE_LINK/commands/j-help.md" ] 2>/dev/null; then
    echo "[OK] .claude/commands accessible"
else
    echo "[ERROR] .claude/commands not accessible"
    ERRORS=$((ERRORS + 1))
fi

# Check submodule (only if in a git repo)
if [ -d "$PROJECT_ROOT/.git" ]; then
    TREE_MODE=$(git -C "$PROJECT_ROOT" ls-tree HEAD .ai-agent 2>/dev/null | awk '{print $1}')
    if [ "$TREE_MODE" = "160000" ]; then
        echo "[OK] .ai-agent is a proper submodule (160000 commit)"
    elif [ -n "$TREE_MODE" ]; then
        echo "[WARN] .ai-agent is tracked as $TREE_MODE (should be 160000)"
        echo "       Run: git rm -r --cached .ai-agent && git submodule add <url> .ai-agent"
    fi
fi

if [ $ERRORS -eq 0 ]; then
    echo ""
    echo "=== Setup Complete ==="
    echo "AI agent is now enabled for this project."
else
    echo ""
    echo "=== Setup Complete (with warnings) ==="
    echo "Some issues detected. Check the messages above."
fi
