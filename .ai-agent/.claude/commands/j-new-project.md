---
description: Scaffold a new project with all foundations in place (junior workflow)
model: opus
---

## Prerequisites
- If not read this session: Read `.claude/commands/mind-sets/junior.md`
- Read (if not already done) MB skip all subdirs

## Purpose

Guided project scaffolding that establishes ALL project foundations from Day 1. Uses `project-foundations-{language}.md` as the blueprint to ensure the project starts with the right structure, configuration, and practices.

The cost of adding these foundations on Day 1 is zero. The cost of adding them later is massive.

## Steps

### 1. Gather Project Info

Ask user:
```
Let's set up a new project. I need some info:

1. Project name?
2. Language/framework?
   - C#/.NET (Web API, console, etc.)
   - TypeScript (Angular, NestJS, Node, etc.)
   - Java/Spring Boot
   - Kotlin/KMP (Multiplatform: Android, iOS, JVM, etc.)
   - Python (FastAPI, Django, Flask, etc.)
3. Project type?
   - Web API (backend only)
   - Full stack (frontend + backend)
   - Library/package
   - Console/CLI application
4. Will it have a database? If yes, which one?
5. Brief description (one sentence)?
```

### 2. Load Foundations

Based on language, read the appropriate foundations file:
- C#: `.claude/commands/mind-sets/project-foundations-csharp.md`
- TypeScript: `.claude/commands/mind-sets/project-foundations-typescript.md`
- Java: `.claude/commands/mind-sets/project-foundations-java.md`
- Kotlin/KMP: `.claude/commands/mind-sets/project-foundations-kotlin.md`
- Python: `.claude/commands/mind-sets/project-foundations-python.md`

### 3. Present Scaffolding Plan

Show the user what will be created:

```
## Project: {name}

### Structure
[show directory tree based on project type + language]

### Foundations that will be applied:
 1. ✓ Zero-tolerance warnings — [language-specific config]
 2. ✓ Central dependency management — [language-specific approach]
 3. ✓ Versioning strategy — [how]
 4. ✓ Structured error handling — [error types]
 5. ✓ Test configuration — [fast/slow split]
 6. ✓ Immutable DTOs — [record types / frozen dataclass / etc.]
 7. ✓ DI registration pattern — [framework-specific]
 8. ✓ Centralized config with validation — [config approach]
 9. ✓ Structured file logging — [logging framework]
10. ✓ Zero-dependency core module — [core project/package]
11. ✓ Interface-first design — [interface pattern]
12. ✓ Internal by default — [visibility approach]
13. ✓ Test module per production module — [test structure]
14. ✓ Convention over configuration — [conventions]
15. ✓ Deterministic build output — [build config]
16. ✓ Black Box Composition ready — [modular structure]

Does this look right? Any changes needed?
```

Ask user to confirm before proceeding.

### 4. Scaffold Project

Create the project structure applying all foundations AND architecture conventions below.

**4a. Project skeleton**
- Create directory structure
- Create core module (interfaces, DTOs, errors)
- Create `IEntity` base interface in core — all entities inherit from it
- Create solution/workspace/project files

**4b. Build configuration**
- Warnings as errors
- Central dependency management
- Version configuration
- Build output configuration
- **KMP-specific**: Gradle Kotlin DSL (`build.gradle.kts`), `libs.versions.toml` version catalog, KMP source set structure (`commonMain`/`androidMain`/`iosMain`/`jvmMain` + corresponding test source sets), Koin-first version compatibility verification

**4c. Application wiring**
- DI registration pattern
- Configuration with validation
- Structured file logging
- Error handling middleware/handler
- JSON serialization with naming policy "as is" (no camelCase/snake_case transformation — property names serialize exactly as defined)

**4d. Architecture patterns**
- **Enums everywhere** — use proper enum types for all domain values, never string constants. Enums serialize as string values (not numbers) everywhere (JSON, DB, API)
- **IEntity base** — all entities inherit from `IEntity`. This enables generic `Repository<T where T : IEntity>` and generic `Controller<T>` base classes
- **Generic base repo + controller** — one base implementation, specific repos/controllers only add what's different
- **Unit of Work** — for complex services/use cases that touch multiple entities, use UoW pattern with single SaveChanges

**4e. Test infrastructure**
- Test project(s) mirroring production
- Fast/slow test split configuration
- **Factory pattern for test data** — never `new Entity { ... }` inline, always `EntityFactory.Create(...)` with sensible defaults
- First test (health/version endpoint or smoke test)

**4f. Development files**
- `.gitignore` (appropriate for language)
- `README.md` (brief, with build/run commands)
- `.editorconfig` (consistent formatting)

### 5. Verify

Run verification:
```
- [ ] Project builds with zero warnings
- [ ] Tests run (at least the smoke test)
- [ ] Config validation works (missing required value = startup failure)
- [ ] Logging writes to file
- [ ] Version is reported
```

### 6. Setup j-settings

If using junior workflow, run `j-setup` to configure project-specific settings for the new project.

### 7. Initial Commit

Ask user:
```
Project scaffolded with all foundations. Ready to commit?

Suggested message: "feat: scaffold {name} with project foundations"
```

## Rules

- **All 16 foundations** — apply every single one, no exceptions
- **Architecture conventions** — enums (not strings), IEntity base, generic repo/controller, UoW, factories, JSON "as is"
- **Working from minute one** — the project must build and have at least one passing test
- **No placeholder code** — only real infrastructure (error handling, logging, config) not TODO comments
- **Follow language conventions** — use the patterns from `project-foundations-{language}.md`
- **Ask if unsure** — use AskUserQuestion for any ambiguity

## What NOT to do

- Don't create business logic — only infrastructure/foundations
- Don't add features — the project is an empty shell with solid foundations
- Don't over-scaffold — create what's needed for the project type, not every possible pattern
- Don't skip any foundation — if something doesn't apply (e.g., no DB = no DB config), note it but apply the rest
