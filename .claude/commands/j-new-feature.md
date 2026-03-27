---
description: Start new feature (junior workflow)
model: opus
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## Steps

### 1. Clean State
- `git status` - check for uncommitted changes
- If changes exist:
  - **Review what will be committed** - no sensitive files (.env, credentials)?
  - If something unexpected → ask user before committing
  - `git add . && git commit -m "WIP: save before new feature"`
- `git push`

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
- Extract number and increment (e.g., if 03-xxx exists, next is 04)
- If no numbered folders exist, start with 01

### 4. Ask Feature Name
Ask user: "What do you want to call this feature? (short, like: login-social, dark-mode)"

### 4b. Confirm Before Creating
```
I'll create:
- Branch: feature/NN-{name}
- Folder: {@tasks}/NN-{name}/

Is this correct? (yes/no)
```
If no → ask for correct name again.
If yes → proceed.

### 5. Create Branch, Folder, Files, and Push
```bash
git checkout -b feature/NN-feature-name
mkdir -p {@tasks}/NN-feature-name
git push -u origin feature/NN-feature-name
```
**IMPORTANT:** Push branch immediately so user can see it on remote.

### 6. Create Notes File in Task Folder
Create `{@tasks}/NN-feature-name/notes.md`:
```markdown
# Feature: NN-feature-name

## Requirements (from user)
<!-- Claude writes here what user says during LISTEN -->

## Analysis
<!-- Claude writes here findings from ANALYZE -->

## Proposed Solution
<!-- Claude writes here the proposal from PROPOSE -->

## Status
- [ ] Requirements gathered
- [ ] Code analyzed
- [ ] Solution proposed
- [ ] Plan created
- [ ] Development done
- [ ] Tested
- [ ] Deployed
```

### 7. Update MB
In `activeContext.md` set:
```
@state::LISTEN
@feature::NN-feature-name
@branch::feature/NN-feature-name
```

### 8. Enter LISTEN State
Say:
```
Ready!
Branch: feature/NN-feature-name
Folder: {@tasks}/NN-feature-name/

Now TELL ME what you need. I will listen and do nothing until you say "done".
You can tell me:
- What this feature should do
- How it should work
- What you see in your mind

Go!
```

## State Transitions (UPDATE MB AT EACH!)

### When user says "done" → ANALYZE
1. **Update MB:** `@state::ANALYZE`
2. **Update notes.md:** mark "Requirements gathered" done
3. Read code, use ChromeDevTools, analyze
4. **Update notes.md:** write Analysis section

### After analysis complete → PROPOSE
1. **Update MB:** `@state::PROPOSE`
2. **Update notes.md:** mark "Code analyzed" done
3. Present solution to user
4. **Update notes.md:** write Proposed Solution section

### When user approves → PLAN
1. **Update MB:** `@state::PLAN`
2. **Update notes.md:** mark "Solution proposed" done
3. **Read `@backend-method` from j-settings.md:**
   - If `tddab` → Read the TDDAB reference file from `@tddab-file` in j-settings.md, then create `{@tasks}/NN-feature-name/plan.md` following TDDAB format (atomic blocks, tests first, RED→GREEN→VERIFY)
   - If `tdd` → Create plan with test-first structure
   - If `manual` → Create plan with generic checklist structure (see j-develop for template)
4. Ask user to review plan

### When user approves plan → ready for j-develop
Tell user: "Plan approved! Use `j-develop` to start implementation."

## Rules
- Do NOT read code during LISTEN
- Do NOT propose solutions during LISTEN
- **Write what user says to notes.md** as they speak
- **UPDATE MB at EVERY state change!**
