---
description: Start development after analysis (junior workflow)
model: sonnet
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## Pre-check
This command is used AFTER:
- User explained what's needed (LISTEN)
- Claude analyzed the code (ANALYZE)
- Claude proposed solution and user approved (PROPOSE)
- Plan created and approved (PLAN)

**State Validation (use intelligence):**
Read MB → check `@state`
- If `@state` is PLAN or DEVELOP → OK, proceed
- If `@state` is LISTEN, ANALYZE, or PROPOSE:
  - **But first, check reality:** Does plan.md exist in task folder? Are there commits suggesting progress?
  - If reality shows progress beyond MB state → ask user if MB needs updating
  - If reality matches MB → then:
```
The plan is not ready yet. Complete the previous phases first.
Current state: {@state}
Use j-continue to resume from the right point.
```
STOP.

## Steps

### 1. Find Task Folder
Read MB → get `@feature` → folder is `{@tasks}/{feature}/`

### 2. Check for Plan
Look for `{@tasks}/{feature}/plan.md`

**If plan.md does NOT exist → CREATE IT based on methodology:**

**Read `@backend-method` from j-settings.md:**

**If `tddab`** → Read the TDDAB reference file from `@tddab-file` in j-settings.md, then create plan following TDDAB format:
```markdown
# TDDAB Plan: {feature-name}
**Date:** YYYY-MM-DD
**Type:** Feature

## Goal
[from notes.md - what user wants]

## TDDAB-1: [First Atomic Block]
### 1.1 Tests First (RED)
[Complete failing tests with full paths, imports, assertions]
### 1.2 Implementation (GREEN)
[Exact code to make tests pass]
### 1.3 Verification
[Exact test commands]

## TDDAB-2: [Second Atomic Block]
[Same structure]

## Success Criteria
[What proves the feature works]
```

**If `tdd`** → Create plan with test-first structure (test + implementation pairs).

**If `manual`** → Create plan with generic structure:
```markdown
# Plan: {feature-name}

## Goal
[from notes.md - what user wants]

## Changes Required
### Backend
- [ ] [specific change]
### Frontend
- [ ] [specific change]

## Test Strategy
- Backend: [how to test - see j-settings.md @test-backend]
- Frontend: [how to verify - see j-settings.md @frontend-method]

## Order of Implementation
1. [first thing]
2. [second thing]
```

**Ask user:** "Here's the plan. Review it and say 'ok' to proceed or tell me what to change."

### 3. Verify Plan Approved
- If user hasn't approved → wait
- If user approved → continue

### 4. Update MB → DEVELOP
**Update MB:** `@state::DEVELOP`
**Update notes.md:** mark "Plan created" done

### 5. Determine What's Affected
- [ ] Backend only
- [ ] Frontend only
- [ ] Both

### 6A. If Backend
**Follow `@backend-method` from j-settings.md:**

**If TDDAB:**
1. Read TDDAB reference file (see `@tddab-file` in j-settings.md)
2. For each step: RED → GREEN → REFACTOR
3. Run tests: `{@test-backend}`
4. Update MB after each completed step

**If TDD:**
1. Write test first, then implementation
2. Run tests: `{@test-backend}`

**If Manual:**
1. Implement changes
2. Test manually

### 6B. If Frontend
**Follow `@frontend-method` from j-settings.md:**

**If ChromeDevTools:**
1. Identify components to modify/create
2. Make incremental changes
3. After each significant change → test with ChromeDevTools
4. Ask user for visual confirmation

**If Automated:**
1. Write/update tests
2. Run: `{@test-frontend}`

### 7. During Development
- After every good checkpoint → update MB
- If unexpected problem found → stop and discuss
- Never make changes not discussed in plan

### 8. When Complete → TEST
**Update MB:** `@state::TEST`
**Update plan.md** - mark all items as done `[x]`
**Update notes.md** - mark "Development done" checkbox
```
Development completed!

What was done:
- [list of changes]

Tests:
- Backend: [result of @test-backend]
- Frontend: [verified with ChromeDevTools]

Ready to test? Then run `j-close` to close and deploy.
```

## Rules
- Follow methodology from j-settings.md
- Update MB often during development
- If something unclear → stop and ask
- Never modify files not discussed in plan
