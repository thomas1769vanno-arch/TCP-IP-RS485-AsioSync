---
description: Save and leave (junior workflow)
model: sonnet
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## Steps

### 1. Commit Work
```bash
git status
```
**Review what will be committed:**
- No sensitive files (.env, credentials, API keys)?
- No large binary files accidentally staged?
- Changes make sense for current work?

If something unexpected → ask user before proceeding.

```bash
git add .
git commit -m "WIP: [brief description of where we are]"
```

### 2. Update Memory Bank
Update `memory-bank/activeContext.md` with:
- Current state (LISTEN, ANALYZE, PROPOSE, PLAN, DEVELOP, TEST)
- What we were doing
- What's left to do
- Any open issues

### 3. Push
```bash
git push origin [current-branch]
```

### 4. Confirm
```
Everything saved!

Branch: [name]
Commit: [message]
Pushed: yes

WHERE WE WERE:
[brief summary]

NEXT TIME:
Use j-continue to resume from here.

Bye!
```

## Rules
- Always push (so it's saved on remote too)
- Always update MB (so Claude remembers)
- Give clear summary
