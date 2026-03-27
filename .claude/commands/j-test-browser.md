---
description: Test in browser with ChromeDevTools (junior workflow)
model: sonnet
---

## Prerequisites
- Read `j-settings.md` from project root (REQUIRED - run `j-setup` if missing)
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## Steps

### 0. Check ChromeDevTools MCP
If `j-settings.md @chromedevtools` is "no" OR MCP is not responding:
```
ChromeDevTools MCP is not configured.
It is required for automated browser tests.

Ask a senior to configure ChromeDevTools MCP, then retry.
```
STOP.

### 1. Open Browser NOW
**Immediately use ChromeDevTools MCP - don't waste time checking:**

Read `j-settings.md` for `@frontend-port`, then:
```
mcp__chrome-devtools__navigate_page
url: http://localhost:{@frontend-port}
```

### 2. If Page Doesn't Load
Only if navigate fails, tell user:
```
Browser can't reach http://localhost:{@frontend-port}

Start the frontend service.
Command hint: {@start-frontend} (from j-settings.md)

Say "running" when ready.
```

### 3. Take Snapshot
```
mcp__chrome-devtools__take_snapshot
```
This shows page structure with element UIDs for clicking.

### 4. Test What's Needed
- Navigate pages: `mcp__chrome-devtools__navigate_page`
- Click elements: `mcp__chrome-devtools__click` with uid
- Fill forms: `mcp__chrome-devtools__fill` with uid and value
- Take snapshots to verify results
- Check console: `mcp__chrome-devtools__list_console_messages`

### 5. Report
```
✅ PASS / ❌ FAIL
[what was tested]
[what happened]
[console errors if any]
```

## Rules
- Open browser FIRST - don't check with curl
- If page doesn't load, THEN tell user to start services
- Snapshot before AND after actions
- Always check console for errors
