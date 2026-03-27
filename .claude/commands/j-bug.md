---
description: Fix bug (junior workflow)
model: opus
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- If not read this session: Read `.claude/commands/mind-sets/debug-protocol.md` (Protocol D - systematic debugging)
- Read (if not already done) MB skip all subdirs

## Steps

### 1. Clean State
- `git status` - check for uncommitted changes
- If changes exist:
  - **Review what will be committed** - no sensitive files (.env, credentials)?
  - If something unexpected → ask user before committing
  - `git add . && git commit -m "WIP: save before bugfix"`

### 2. Sync with Base Branch
**Read `@merge-strategy` from j-settings.md:**
- If `gitflow` or `gitflow-simple` → base branch is `{@dev-branch}` (develop)
- Otherwise → base branch is `{@main-branch}`

```bash
git checkout {base-branch}
git pull origin {base-branch}
```
**IMPORTANT:** Always start from latest base branch to avoid conflicts later.

### 3. Find Next Number
- Check existing **folders** (not files) in `{@tasks}/` matching pattern `NN-*`
- Use: `ls -d {@tasks}/[0-9][0-9]-*/ 2>/dev/null | sort | tail -1`
- Extract number and increment, or start with 01 if none exist

### 3b. Confirm Before Creating
```
I'll create:
- Branch: bugfix/NN-{bug-description}
- Folder: {@tasks}/NN-{bug-description}/

Is this correct? (yes/no)
```
If no → ask for correct name again.
If yes → proceed.

### 3c. Create Branch, Folder, and Push
```bash
git checkout -b bugfix/NN-bug-description
mkdir -p {@tasks}/NN-bug-description
git push -u origin bugfix/NN-bug-description
```
**IMPORTANT:** Push branch immediately so user can see it on remote.

### 4. Create Bug Notes File
Create `{@tasks}/NN-bug-description/bug-notes.md`:
```markdown
# Bug: NN-bug-description

## Problem
**Expected:** [what should happen]
**Actual:** [what happens instead]
**Where:** [page/function]

## Analysis
<!-- Claude writes here what was found -->

## Fix
<!-- Claude writes here what was changed -->

## Verification
- [ ] Bug reproduced
- [ ] Fix applied
- [ ] Tested working
- [ ] Deployed
```

### 5. Update MB → LISTEN
In `activeContext.md` set:
```
@state::LISTEN
@bug::NN-bug-description
@branch::bugfix/NN-bug-description
```

### 6. Enter LISTEN State
Say:
```
Tell me about the bug:
- What SHOULD happen?
- What happens INSTEAD?
- Where do you see it? (which page/function)

If you have screenshots, share them!
When done explaining, say "done".
```

### 7. After "done" - ANALYZE (apply Protocol D)
**Update MB:** `@state::ANALYZE`
**Update bug-notes.md** with what user said.

**Apply Protocol D:**
1. **READ** - Quote exact error message/behavior
2. **ISOLATE** - Find precise location (file:line)
3. **DOCS** - Check framework docs if relevant

Tools:
- Use ChromeDevTools to verify the bug
- Or ask for screenshots
- Search code for the problem
- Read `{@docs}/` if context needed (from j-settings.md)

### 8. PROPOSE Fix (Protocol D - HYPOTHESIZE)
**Update MB:** `@state::PROPOSE`
**Update bug-notes.md** with analysis findings.

**Apply Protocol D - HYPOTHESIZE:**
```
I found the problem: [simple explanation]

**Hypothesis:** [what I think causes it]
**Reasoning:** [why I think this]
**Prediction:** [what will happen after fix]

To fix it I need to: [what you'll do]

Proceed?
```

### 9. Fix (follow methodology!)
**Update MB:** `@state::DEVELOP`
**Update bug-notes.md** with what was changed.

**Read j-settings.md for methodology:**

**If Backend bug:**
- Simple fix (1-2 files) → fix directly
- Complex fix (3+ files) → create mini plan in bug-notes.md:
  - Read `@backend-method` from j-settings.md
  - If `tddab` → Read `@tddab-file`, create mini TDDAB plan (atomic blocks, tests first, RED→GREEN→VERIFY)
  - If `tdd` → Create plan with test-first pairs
  - If `manual` → Create checklist plan
- Run tests: `{@test-backend}` (from j-settings.md)
- **ZERO warnings** - fix any warnings introduced

**If Frontend bug:**
- Fix the code
- Test with ChromeDevTools (j-test-browser)
- Ask user for visual confirmation

### 10. Verify Fix → TEST (Protocol D - VERIFY)
**Update MB:** `@state::TEST`

**Apply Protocol D - VERIFY:**
- Make ONE change at a time
- Backend: run `{@test-backend}` - ALL must pass
- Frontend: verify in browser with ChromeDevTools
- Compare result with your PREDICTION from step 7
- **Update bug-notes.md:** mark "Fix applied" and "Tested working"

If tests fail or prediction doesn't match → go back to step 7 (ANALYZE - READ the new error)

### 11. Close Bug (NO auto-deploy)
**Update bug-notes.md** - mark "Fix applied" and "Tested working" checkboxes.
If it works:
```bash
git add . && git commit -m "fix(area): bug description"
git push origin bugfix/NN-xxx
```
**IMPORTANT:** Do NOT delete the bugfix branch! User needs it for review/control.

```
Bug fixed and tested!

Branch: bugfix/NN-xxx (pushed to remote)

To finish, use:
- j-close → merge to main + deploy (or create PR if @merge-strategy: pr)
- j-deploy → deploy only (if already merged)
```

**Update MB:**
- Keep `@state::TEST` (not IDLE yet - needs deploy)
- Keep `@bug` and `@branch` until j-close

## Rules
- **Protocol D always** - READ exact error, ISOLATE precisely, HYPOTHESIZE before fixing, VERIFY one change at a time
- **Backend = methodology from j-settings.md** (run tests per @test-backend)
- **Frontend = ChromeDevTools** (manual visual test)
- **ZERO warnings** - fix any warnings before close
- **ONE change at a time** - never change multiple things without verifying
- Always test before merge
- NEVER delete branches (user needs them for review/control)
- ALWAYS push branch before merging
- If "bug" is actually a feature → use `j-new-feature`
