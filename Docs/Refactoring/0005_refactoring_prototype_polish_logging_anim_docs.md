# PRD — Prototype Polish (AnimBP Velocity, Setup Logging, Runtime Docs)

## 1) Introduction/Overview
This milestone improves prototype quality and maintainability by:
- Ensuring movement animations reflect motion (velocity-driven)
- Adding high-signal error logging for missing scene dependencies
- Updating runtime architecture documentation (ownership + dependency contract)

## 2) Goals
- Explorer animations play correctly while moving in Camp.
- Missing actor config is loud and actionable (map + mode + missing actor).
- Runtime docs reflect the refactor and prevent regressions.

## 3) User Stories
- As a player, the explorer visibly walks/runs while moving.
- As a developer, I immediately see which dependency is missing when scene wiring is wrong.
- As a developer, I can onboard quickly using current runtime docs.

## 4) Functional Requirements
1. **AnimBP velocity-driven locomotion**
   1.1 Explorer AnimBP drives locomotion primarily from `GetVelocity().Size()`.  
   1.2 Add optional debug logging for speed/velocity while moving (debug builds only).
2. **Setup logging**
   2.1 GameMode setup functions validate critical references and log:
   - map name
   - current mode
   - which actor/reference is missing
   2.2 Dependent flows abort safely.
3. **Runtime documentation**
   3.1 Update `FCRuntime.md` to include:
   - ownership rules (controller owns interaction orchestrator)
   - coordinator constraints (switchboard only)
   - allowed/forbidden dependency direction
   - prototype stop lines
4. **Decision checkpoint**
   4.1 Record whether controller stayed clean and whether splitting controllers is needed for June.

## 5) Security Requirements
1. Debug logging reduced/disabled in Shipping builds (build flags).
2. Avoid logging sensitive info (unlikely, but keep discipline).
3. Validate pointers before reading velocity/owner references.

## 6) Non-Goals (Out of Scope)
- Full gameplay polish pass.
- Controller splitting (unless scheduled for June).
- Actor registry subsystem.

## 7) Design Considerations (Optional)
- None.

## 8) Technical Considerations (Optional)
- AI/path-follow movers may show low acceleration; base animation on velocity magnitude.

## 9) Success Metrics
- Explorer locomotion animations reliably trigger while moving.
- Missing scene wiring produces immediate actionable logs.
- Updated docs used as team source of truth.

## 10) Open Questions
- Preferred log categories/verbosity levels for setup vs interaction?
