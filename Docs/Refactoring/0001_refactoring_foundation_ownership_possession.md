# PRD — Foundation Refactor (Ownership + Camp Possession)

## 1) Introduction/Overview
This milestone stabilizes the Player Runtime layer by fixing the highest-impact failures:
- Interaction ownership inconsistencies (controller-owned component behaving like pawn-owned)
- Camp control complexity (AI-controlled explorer used to preserve static camera)
- Hardcoded input asset paths that make builds brittle

The result should be template-aligned: **the explorer is possessed**, the **camera can remain static via view target**, and **interaction is consistently controller-owned**.

## 2) Goals
- Eliminate silent null failures in interaction prompting/tracing caused by incorrect controller resolution.
- Ensure Overworld/Camp POI overlap reliably reaches the controller-owned interaction orchestrator.
- Make Camp exploration controllable with standard possession while keeping a static camera.
- Remove hardcoded `/Game/...` input asset paths in C++ and rely on configuration assets.
- Improve clarity by renaming misleading variables (e.g., “PossessedConvoy” → “ActiveConvoy”).

## 3) User Stories
- As a player, I can move the explorer in Camp while the camera remains static and stable.
- As a player, interacting with POIs works consistently across Office/Overworld/Camp (no “sometimes nothing happens”).
- As a developer, I can change input assets without recompiling C++ or breaking builds due to hardcoded paths.
- As a developer, logs clearly tell me when required scene actors are missing.

## 4) Functional Requirements
1. **Camp possession**
   1.1 The Camp explorer pawn must be **possessed by the PlayerController** in Camp levels.  
   1.2 The Camp camera must remain static by setting the controller’s view target to a designated CampCamera actor (e.g., `SetViewTargetWithBlend`), independent of possession.
2. **Interaction component controller reference**
   2.1 The interaction component (controller-owned) must cache `AFCPlayerController*` from `GetOwner()` and never rely on `GetInstigatorController()` for controller-owned logic.  
   2.2 All interaction logic that needs the controller must use the cached pointer and guard against null.
3. **Normalize overlap → interaction routing**
   3.1 World actors must not crawl ownership chains (no “ask the FP pawn for InteractionComponent”).  
   3.2 Convoy/explorer must broadcast overlap/arrival delegates; the controller binds these delegates to the controller-owned interaction component during initialization.
4. **Mode-gate FirstPerson tracing**
   4.1 First-person trace/prompt updates must run only in FirstPerson mode and must not tick in TopDown/Camp.
5. **Input config DataAsset**
   5.1 Introduce `UFCInputConfig` (DataAsset) that references required Input Actions and Mapping Contexts.  
   5.2 Remove hardcoded `ConstructorHelpers::FObjectFinder("/Game/FC/Input/IA_*")` patterns from C++.
6. **Naming cleanup**
   6.1 Rename `PossessedConvoy` to `ActiveConvoy` (and similar) to reflect non-possession reality.
7. **Logging & failure visibility (basic)**
   7.1 If essential scene actors (explorer/camp camera/convoy) are missing, log a high-signal error including map name and current mode and abort dependent flows safely (no crashes).

## 5) Security Requirements
1. Validate all soft/class references before use; fail safely (no null dereference).
2. Asset loading must not accept arbitrary player-provided paths; only configured assets may be used.
3. Avoid storing raw actor pointers in persistent singletons across level travel (use resolve-per-level techniques).
4. Ensure debug logs can be reduced/disabled in Shipping builds.

## 6) Non-Goals (Out of Scope)
- Full redesign of POI action selection UX.
- Adding a WorldSubsystem registry (defer to June unless needed).
- Splitting into multiple PlayerController classes.

## 7) Design Considerations (Optional)
- No UI redesign required.
- Interaction prompt should not appear outside FirstPerson mode.

## 8) Technical Considerations (Optional)
- Likely touched files/classes:
  - `AFCPlayerController` (delegate binding, possession/view target setup)
  - `UFCInteractionComponent` (controller caching, mode gating)
  - `AFCOverworldConvoy` / explorer actor (delegate broadcasting only)
  - `UFCInputManager` (consuming `UFCInputConfig`)
- Prefer editor-visible setup for CampCamera reference; tags as fallback for prototype.

## 9) Success Metrics
- 0 occurrences of “controller is null” in interaction logs during standard play.
- Camp: explorer moves under player control while camera remains stable.
- Overworld: POI overlap triggers interaction handling every time (100% repro success).
- No hardcoded IA paths remain in C++ for core runtime inputs.

## 10) Open Questions
- Single-action POI policy: auto-execute vs confirmation?
- Canonical Camp camera reference: tag lookup vs explicit property?
