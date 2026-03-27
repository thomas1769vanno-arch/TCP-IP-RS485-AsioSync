---
description: Create proof of concept for unknown/complex features (junior workflow)
model: sonnet
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## When to Use
Use this command when:
- Adding something NEW that you haven't done before
- Integrating an external library/API you don't know well
- The feature is complex and you're not sure how to approach it
- You want to experiment WITHOUT touching production code

**Better to experiment in POC than mess up real code!**

## Steps

### 1. Understand What's Needed
Ask user:
```
You want to try something new. Tell me:
- What do you want to achieve?
- What technology/library/API is involved?
- What's the unknown part you want to figure out?

When done, say "done".
```

### 2. Create POC Folder
- Check existing **folders** in `{@docs}/` matching pattern `poc-NN-*`
- Use: `ls -d {@docs}/poc-[0-9][0-9]-*/ 2>/dev/null | sort | tail -1`
- Extract number and increment, or start with 01 if none exist
```bash
mkdir -p {@docs}/poc-NN-description
```

### 3. Research Phase
- Search for documentation
- Look at examples
- Check if similar POCs exist in `{@docs}/`
- Use web search if needed

### 4. Simple Plan
Create `{@docs}/poc-NN-description/README.md`:
```markdown
# POC: [description]

## Goal
What we want to verify/learn

## Questions to Answer
- [ ] Does X work with Y?
- [ ] How do we configure Z?
- [ ] What's the performance like?

## Approach
1. Step 1
2. Step 2
3. ...

## Results
(fill after POC)

## Conclusion
(fill after POC: YES we can use it / NO we need alternative)
```

### 5. Execute POC
- Create minimal code to test the concept
- Keep it simple - just enough to answer the questions
- NO need for tests, clean code, or perfect structure
- Document what you learn

### 6. Evaluate Results
```
POC completed!

GOAL: [what we wanted to verify]

RESULTS:
- Question 1: [answer]
- Question 2: [answer]

CONCLUSION:
[Can we use this? Yes/No/With modifications]

NEXT:
- If YES → ready to implement in real code with j-new-feature
- If NO → need alternative approach
```

### 7. Save POC
- Commit POC code: `git add . && git commit -m "poc(NN): description"`
- Keep it for future reference
- Update MB if relevant learnings

## Rules
- POC code stays in `{@docs}/`, NEVER in `{@code}/`
- Keep POC minimal - just enough to answer questions
- Document everything you learn
- If POC successful → start real feature with `j-new-feature`
- If POC fails → discuss alternatives before proceeding
- POC doesn't follow full workflow - no state tracking needed
