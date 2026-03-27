# AI Agent Lite

Drop-in AI agent configuration for junior developers. Add as git submodule, run setup, done.

## Quick Start

```bash
# 1. Add as submodule
git submodule add <repo-url> .ai-agent

# 2. Run setup
cd .ai-agent && ./setup.sh   # Unix
cd .ai-agent; .\setup.ps1    # Windows

# 3. Done - AI agent enabled
```

## What's Included

### Junior Workflow (`j-*` commands)
Full guided development workflow:
- `j-setup` — Configure project settings
- `j-new-feature` — Start a new feature (guided: LISTEN → ANALYZE → PROPOSE → PLAN → DEVELOP → TEST)
- `j-bug` — Fix a bug with Protocol D (systematic debugging)
- `j-develop` — Start development after planning
- `j-continue` — Resume work on existing feature
- `j-deploy` — Deploy to production (with backup/restore)
- `j-close` — Close feature, merge, and deploy
- `j-save` — Save work and leave
- `j-status` — Show current status
- `j-help` — Get help
- `j-debug` / `j-debug-mb` — Debug any problem with Protocol D
- `j-review-plan` — Review plan for TDDAB conformity
- `j-new-project` — Scaffold new project with all foundations
- `j-poc` — Create proof of concept
- `j-e2e` / `j-e2e-generate` — E2E testing
- `j-test-browser` — Test in browser with ChromeDevTools

### Architecture Audit (`x-audit`)
Comprehensive code audit covering KISS, DRY, YAGNI, overengineering, project foundations (16 principles), security (OWASP), and TCO.

### Memory Bank
Persistent project memory across sessions. Claude reads Memory Bank at session start to understand project context.

## Structure After Setup

```
your-project/
├── .ai-agent/           ← submodule (this repo)
├── .claude/             ← symlink → .ai-agent/.claude
├── CLAUDE.md            ← project instructions for Claude
├── j-settings.md        ← project-specific settings (created by j-setup)
└── memory-bank/         ← persistent AI memory
    └── README.md        ← symlink → .ai-agent/src/memory-bank/README.md
```

## First Steps

1. Run `j-setup` to configure your project
2. Use `j-new-feature` to start your first feature
3. Use `j-help` if you get stuck
