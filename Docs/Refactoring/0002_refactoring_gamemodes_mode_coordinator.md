# PRD — Thin GameModes + Player Mode Coordinator

## 1) Introduction/Overview
This milestone reduces mode-switching complexity by:
- Introducing **thin per-level GameModes** to enforce correct pawn/controller defaults per scene
- Introducing a **mode coordinator** component that applies configuration-only profiles to input/camera/interaction without embedding behavior

## 2) Goals
- Prevent config branching spaghetti (e.g., `if LevelType == ...`).
- Ensure PIE/testing of a single level always runs correct defaults.
- Make mode state authoritative via `UFCGameStateManager` and a simple coordinator mapping.
- Keep profile data configuration-only (no behavior creep).

## 3) User Stories
- As a developer, opening Camp level in PIE always uses Camp defaults without extra steps.
- As a developer, I can change mode configuration without rewriting controller logic.
- As a developer, I can trace mode changes and profile application via logs.

## 4) Functional Requirements
1. **Thin per-level GameModes**
   1.1 Implement `AFCBaseGameMode` with shared setup helpers.  
   1.2 Implement `AFCOfficeGameMode`, `AFCCampGameMode`, `AFCOverworldGameMode` as thin wrappers (<50 lines each): set defaults + call one setup function.  
   1.3 Each GameMode must support explicit scene wiring via editor-visible properties for key actors (camp camera, active convoy). Tags may be used as fallback for prototype.
2. **Mode coordinator component**
   2.1 Implement `UFCPlayerModeCoordinator` as an ActorComponent on the controller.  
   2.2 Subscribe to `UFCGameStateManager::OnStateChanged` and map state → `EFCPlayerMode`.  
   2.3 Apply `FPlayerModeProfile` by delegating to:
   - `UFCInputManager` (apply `UFCInputConfig`)
   - `UFCCameraManager` (apply camera preset/mode)
   - `UFCInteractionComponent` (apply interaction probe profile)
   - PlayerController (cursor + input mode rules)
   2.4 Coordinator must NOT: open UI, execute actions, perform movement, or contain per-mode behavior logic.
3. **Profiles (configuration-only)**
   3.1 Add `FPlayerModeProfile` with:
   - `TSoftObjectPtr<UFCInputConfig> InputConfig`
   - camera mode/preset id
   - `TSoftObjectPtr<UInteractionProfile> InteractionProfile` (may begin hard-referenced)
   - click handling policy enum
   - cursor visibility and mouse capture mode
   3.2 Staging rule: begin with hard refs/default assets; move to soft refs after travel is stable.
4. **Controller responsibilities**
   4.1 Controller must not branch on mode beyond requesting coordinator to apply current profile.
   4.2 Pause/menu/debug keys may remain in controller for prototype.

## 5) Security Requirements
1. Validate referenced assets before applying configuration.
2. Profile application must be idempotent and safe to call multiple times (e.g., after travel).
3. Avoid circular dependencies: UIManager must not call coordinator; coordinator must not call UI.

## 6) Non-Goals (Out of Scope)
- Multiple PlayerController classes.
- WorldSubsystem actor registry.
- Full data-table level metadata for this milestone.

## 7) Design Considerations (Optional)
- None.

## 8) Technical Considerations (Optional)
- Apply ordering after travel should be consistent:
  state → mode → camera → input → interaction.
- Prefer explicit editor references to reduce “tag magic” debugging.

## 9) Success Metrics
- Any level PIE-run selects correct defaults (pawn/controller).
- Mode changes reliably re-apply mapping contexts, cursor rules, and camera modes without manual “replay” calls.
- Controller contains no mode branching beyond `ApplyMode()`.

## 10) Open Questions
- Minimal player modes required for prototype (FP / TopDown / POIScene)?
- Should GameModes resolve refs primarily via explicit properties with tag fallback?
