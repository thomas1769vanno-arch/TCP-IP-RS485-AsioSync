---
description: Help (junior workflow)
model: sonnet
---

## Prerequisites
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- MB: not required for help

## Show
```
=== JUNIOR WORKFLOW HELP ===

FIRST TIME SETUP:
  j-setup        → Configure project settings (REQUIRED before other commands)

START WORK:
  j-new-feature  → Start a new feature
  j-bug          → Fix a bug
  j-poc          → Experiment with something new (proof of concept)
  j-continue     → Resume where you left off

DURING WORK:
  j-develop      → Start development (after analysis)
  j-review-plan  → Review plan for methodology/project conformity
  j-debug        → Debug any problem (Protocol D)
  j-status       → See current status
  j-save         → Save everything and leave

TESTING:
  j-test-browser → Quick test in browser (starts services if needed)
  j-e2e          → Run full E2E test suite
  j-e2e-generate → Generate test checklist from use cases

FINISH:
  j-close        → Close feature and deploy
  j-deploy       → Deploy only (if already on main)

=== FIRST TIME? ===

Run j-setup first! It will:
1. Ask about your project (language, structure, commands)
2. Create j-settings.md with all configurations
3. Make all other j-* commands work properly

=== HOW IT WORKS ===

1. Run j-setup (only once per project)
2. Use j-new-feature or j-bug
3. Tell me what you need (I listen)
4. When you say "done", I analyze
5. I propose solution, we discuss
6. We create a plan, you review
7. We work together (j-develop)
8. When done, use j-close

=== IF YOU'RE STUCK ===

- Don't know what to do? → j-status to see where we are
- Need to leave? → j-save to save everything
- Something not working? → Tell me the problem
- Have doubts? → Ask! Better to ask than make mistakes
- Settings wrong? → j-setup to change them

=== IMPORTANT RULES ===

- I do NOTHING until you ask me
- Before modifying code, I ask confirmation
- If you need to leave, always use j-save
- Don't be afraid of mistakes, we can always go back

=== IF SOMETHING GOES VERY WRONG ===

1. Use j-save
2. Contact someone experienced
```
