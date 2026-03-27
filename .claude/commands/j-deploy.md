---
description: Deploy to production (junior workflow)
model: sonnet
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## Pre-check
- No uncommitted changes
- Tests must pass

## Steps

### 1. Verify Before Deploy
```bash
git branch --show-current
git status
```
- Show current branch (so user knows what will be deployed)
- If uncommitted changes → "Commit first"

### 2. Run Tests
Run test command from j-settings.md:
```bash
{@test-backend}
```
- If tests fail → FIX before deploying, DO NOT proceed

### 3. Read Deploy Documentation
Read `{@deploy-docs}` (from j-settings.md) for:
- How deploy works
- What the script checks
- Manual steps if any

If `@deploy-docs` is "none", skip this step.

### 4. Backup (Pre-Deploy)
Check `@backup-script` in j-settings.md.

**If @backup-script is NOT "none":**
```
Running backup before deploy...
```
```bash
{@backup-script}
```
- Show output to user
- If backup FAILS → STOP, do not deploy
```
Backup failed. Deploy cancelled.
Error: [show error]

Fix the backup issue before retrying.
```

**If @backup-script is "none":**
```
No backup script configured. Proceeding with deploy.
   (To configure: j-setup → edit @backup-script)
```

### 5. Execute Deploy
```bash
{@deploy-script}
```
(command from j-settings.md)

### 6. Verify Deploy / Handle Failure
- Check script output

**If deploy SUCCESS → go to step 7**

**If deploy FAILS:**

Check if `@restore-script` exists and is not "none":

**WITH restore available:**
```
Deploy failed. Error: [show error]

You have a backup ready. What do you want to do?

1. RESTORE
   → Run @restore-script to roll back to before the deploy

2. LET'S DEBUG
   → Debug the error together
```

**WITHOUT restore (no script configured):**
```
Deploy failed. Error: [show error]

No restore script configured.

What do you want to do?

1. ASK FOR HELP
   → Contact a senior, show them the error above

2. LET'S DEBUG
   → Debug the error together
```

**If user chooses RESTORE (option 1 with restore):**
```bash
{@restore-script}
```
- Show output
- If restore succeeds:
```
Restore completed. Environment rolled back to before the deploy.

The code is still in the branch, you can retry anytime.
Use j-deploy to retry after fixing the issue.
```
- If restore fails:
```
Restore also failed. Error: [show error]

ACTION REQUIRED:
1. Contact a senior immediately
2. Show them BOTH errors (deploy + restore)
3. DO NOT touch anything until help arrives
```
STOP.

**If user chooses DEBUG (option 2):**
Continue debugging with user. Apply Protocol D if needed.

### 7. Confirm Success
```
Deploy completed!

Verify at: {@domain} (from j-settings.md)

If something doesn't work online, check:
- Application logs
- Service status
```

## Rules
- NEVER deploy if tests fail
- ALWAYS run backup if @backup-script is configured
- If backup fails → STOP, do not deploy
- Always verify the site works after deploy
- If deploy fails and restore available → offer restore first
