---
description: Review plan for TDDAB and project conformity (junior workflow)
model: opus
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## Purpose
Review a TDDAB plan or development plan to ensure it conforms with:
1. Project methodology (from j-settings.md)
2. Project directives and constraints
3. Code quality standards

## Steps

### 1. Read Methodology Reference
**Check `j-settings.md @backend-method`:**
- If `tddab` → Read the TDDAB reference file from `@tddab-file`
- If `tdd` → Standard TDD rules apply
- If `manual` → No strict methodology

Read the chosen file to understand methodology:
- RED → GREEN → REFACTOR cycle (if TDDAB/TDD)
- Test-first approach
- Plan structure requirements

### 2. Read Project Settings
From `j-settings.md`, check:
- `@backend-method` → methodology for backend
- `@frontend-method` → methodology for frontend
- `@language` → stack constraints
- Project-specific rules

### 3. Find the Plan
Look for plan in:
- Current task folder: `{@tasks}/NN-*/plan.md` or `tddab-plan.md`
- Or ask user: "Where is the plan file?"

### 4. Review Checklist

#### Methodology Conformity (if TDDAB/TDD)
- [ ] **Test cases defined BEFORE implementation code**
- [ ] Each feature has corresponding test(s)
- [ ] Tests are specific and atomic (one assertion per test ideal)
- [ ] RED phase clearly shows what test to write
- [ ] GREEN phase shows minimal code to pass
- [ ] REFACTOR phase identified (if needed)

#### Project Conformity
- [ ] **Backend uses correct method** (from j-settings.md @backend-method)
- [ ] **Frontend uses correct method** (from j-settings.md @frontend-method)
- [ ] No migrations that recreate DB (ADD only)
- [ ] Follows project architecture patterns
- [ ] Uses project conventions

#### Code Quality
- [ ] No obvious ZERO warnings violations
- [ ] No hardcoded values that should be in config
- [ ] No security issues (SQL injection, XSS, etc.)
- [ ] Follows existing code patterns in codebase

### 5. Report Issues

If issues found:
```
PLAN REVIEW - ISSUES FOUND

❌ Methodology Issues:
- [list specific issues]

❌ Project Issues:
- [list specific issues]

❌ Quality Issues:
- [list specific issues]

SUGGESTED FIXES:
1. [specific fix]
2. [specific fix]

Fix these before proceeding with j-develop.
```

If plan is OK:
```
PLAN REVIEW - APPROVED ✅

✓ Methodology conformity: OK
✓ Project directives: OK
✓ Code quality: OK

Ready to proceed with j-develop.
```

### 6. Update Plan (if needed)
If user agrees to fixes:
- Edit the plan file with corrections
- Mark reviewed sections
- Add review timestamp

## Common Issues to Catch

### Methodology Violations (if TDDAB/TDD)
- Writing implementation before tests
- Tests that are too broad (testing multiple things)
- Missing edge case tests
- No clear RED/GREEN/REFACTOR phases

### Project Violations
- Not following methodology from j-settings.md
- Recreating migrations instead of adding
- Hardcoded URLs/ports (should come from j-settings.md)
- Not following project architecture

### Quality Violations
- Potential warnings (unused variables, etc.)
- Missing null checks where needed
- Inconsistent naming conventions
- Copy-paste code that should be abstracted

## Rules
- Be strict but constructive
- Explain WHY something is wrong
- Suggest specific fixes, not vague advice
- If plan is good, say so quickly and move on
