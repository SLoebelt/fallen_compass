# Refactoring Analysis v1 (Updated + Critically Hardened)

Scope: Review current runtime layering (PlayerController + managers + interaction flow) and identify refactors to reduce coupling, clarify ownership, and unblock pending issues (POI interaction, camp movement/animation, UI blocking). This version integrates the v1 plan plus critical guardrails to maximize *shipping speed* (Feb prototype) while preventing *god-object regrowth* and future debt (June demo).

---

## Immediate risks / hotspots

### 1) PlayerController is a god-object

* Owns input routing, camera mode decisions, UI entry points, save/load shortcuts, and game-flow triggers.
* Hardcoded asset paths (`/Game/FC/Input/IA_*`) make testing and reuse brittle.

### 2) Interaction flow leaks UI concerns + double authority

* `UFCInteractionComponent` consults `UFCUIManager` for POI selection; PlayerController gates interaction on UI state.
* Semantics drifting: movement and interaction are initiated from multiple places (controller + interaction component), increasing double-execute / “sometimes nothing happens” bugs.

### 3) Movement depends on auto-discovery

* Convoy/explorer lookup via world scans / “first player controller” / tags; missing controllers/tags cause silent failures.
* Cross-travel actor references: storing raw actor pointers in GameInstance is unsafe (actors don’t persist through travel).

### 4) State handling duplicated

* Controller caches camera/input mode alongside `UFCGameStateManager` transitions; manual replays on BeginPlay hint at missing authoritative source of truth.

### 5) Camp explorer movement/animation mismatch

* `MoveExplorerToLocation` drives AI movement, but explorer acceleration reads as zero (logged).
* Likely mismatch between command layer and locomotion signals:

  * AnimBP using acceleration → may remain ~0 under AI/path-following.
  * Prefer velocity magnitude (`GetVelocity().Size()`) for AI locomotion blending.

### 6) Interaction ownership inconsistency (root cause)

* Component is controller-owned, but some callers fetch it from pawns (e.g., convoy asks FirstPerson pawn).
* Component queries `GetInstigatorController()` even when owner is already controller → silent nulls for prompt/tracing.

---

## Key root-cause statement (alignment)

**The project suffers less from “too many systems” and more from inconsistent ownership assumptions.**
Interaction is intended as controller-owned and persistent across pawn swaps, but some paths behave as if interaction is pawn-owned. That mismatch produces null controllers, missing prompts, inconsistent POI behavior, and hard-to-debug travel-mode issues.

---

## Updated architecture principles (enforced)

### A) One controller for Feb, but *strict modularity*

* PlayerController is a router and lifecycle anchor.
* Mode behavior is applied via a switchboard coordinator and executed in focused components.

**Realistic invariant:**
✅ *No branching on “mode” inside controller beyond calling coordinator->ApplyMode(…).*
(General UI actions and input events are allowed.)

### B) Coordinator is a switchboard, not a brain

`UFCPlayerModeCoordinator` responsibilities (ONLY):

* Hold `CurrentMode` + previous
* Subscribe to `UFCGameStateManager::OnStateChanged` and map state → mode
* Apply `FPlayerModeProfile` by calling other components:

  * InputManager, CameraManager, InteractionComponent, and controller cursor/input-mode rules

Coordinator must NOT:

* execute actions
* open UI
* perform movement
* contain per-mode behavior logic (beyond choosing which profile to apply)

### C) Profiles are config-only, not behavior

Profiles contain configuration:

* input config
* interaction probe config
* camera preset
* cursor/input mode rules
* click handling policy enum (MoveOrInteract / InteractOnly / MoveOnly)

Profiles must NOT:

* contain complex conditional logic
* contain “special case booleans” beyond ~5 flags
  If profile needs >10 fields or nested configs → split controllers or add a small strategy object.

### D) Dependency direction contract (prevents spaghetti)

Allowed calls (✅):

* PlayerController → ModeCoordinator/InputManager/CameraManager/InteractionComponent
* ModeCoordinator → InputManager/CameraManager/InteractionComponent + controller cursor rules
* InteractionComponent → UIManager **(view only)** + world actors **via interfaces**
* GameModes → set defaults + one-time setup + pass explicit refs to controller/coordinator

Forbidden calls (❌):

* World actors → “find controller and call interaction” (must broadcast delegates only)
* UIManager → gameplay branching / movement / execute actions (UI is view-only)
* InteractionComponent → `GetFirstPlayerController()` or world scans
* GameInstance → store actor pointers across travel (use IDs/tags/resolve per world)

---

## Target refactors (ordered for maximum impact / minimum churn)

### 1) Stabilize interaction ownership + wiring (highest priority)

* Cache `AFCPlayerController*` inside `UFCInteractionComponent`

  * Use `Cast<AFCPlayerController>(GetOwner())` (owner is controller).
  * Remove all `GetInstigatorController()` usage in controller-owned components.
* Normalize calls: overlap/POI delegation must target the controller-owned component

  * Bind convoy/explorer overlap delegates to controller’s interaction component.
  * Stop fetching interaction component from other pawns.
* Gate FirstPerson trace/prompt updates to FirstPerson mode only

  * Prevent spurious traces/prompt updates in TopDown/Camp.

**Acceptance impact:** removes silent null failures and wrong-owner interaction calls.

---

### 2) Harmonize interaction semantics per mode (remove double authority)

Pick one authority per mode and enforce it:

* TopDown/Camp: click-to-move-or-POI (if POI hit → interact/select; else move)
* FirstPerson: Interact input uses focus target (trace)

Remove competing movement triggers (component and controller cannot both issue move on same click).

---

### 3) Interaction/POI pipeline cleanup (seams + responsibilities)

* Introduce a pure POI action contract (struct + delegates)
* InteractionComponent owns pipeline:

  * resolve → optional selection → pending action → move → **execute only on arrival**
* UIManager is a dumb view producer:

  * show selection UI, return result via callback/delegate
  * no UI->gameplay branching in UIManager
* Arrival triggers:

  * overlap triggers `NotifyArrivedAtPOI(POI)`
  * execute only if `PendingPOI == POI` and `bAwaitingArrival == true`
  * clear latch immediately on first arrival to avoid double fire
* Single-action policy: decide explicitly and document:

  * auto-run single-action OR require confirm; keep consistent

---

### 4) Movement / pawn ownership clarity (reduce magic wiring)

* For Feb: **GameMode imperative wiring** + *loud logging*

  * Do not introduce Registry subsystem unless requirements demand it (June hardening).
* Make missing actor failures explicit:

  * log with map name + mode + which actor missing, abort safely
* Explorer locomotion alignment:

  * AnimBP reads speed from `GetVelocity().Size()`
  * acceleration only secondary
* Camp simplification (mandatory for Feb):

  * possess explorer
  * keep static camera via `SetViewTargetWithBlend`

---

### 5) State / camera / input synchronization (single source of truth)

* `UFCGameStateManager` is authoritative
* Derive mode/camera/input via coordinator in one place
* Normalize post-load restoration:

  * `OnLevelReady` handshake applies state → camera → input → interaction in fixed order
  * avoids scattered manual replays

---

### 6) UI blocking and focus rules (reduce scattered checks)

* Centralize queries in UIManager as read-only:

  * `CanWorldClick()` (TopDown/Camp)
  * `CanWorldInteract()` (FirstPerson interact input)
* Controller and InteractionComponent query these; no circular notifications.
* Add checklist tests (map HUD, summary, pause, selection modal).

---

## Tactical tasks (short term / concrete)

* Cache controller pointer in InteractionComponent; remove instigator retrieval.
* Delegate-bind overlaps to controller interaction; stop pawn-fetching.
* Enforce single movement authority per click.
* Add arrival gating + latch to prevent double execution.
* Decide single-action policy and implement consistently.
* Replace hardcoded input asset paths with `UFCInputConfig` DataAsset.
* Possess explorer + static view target camera.
* AnimBP uses velocity magnitude; log velocity during moves.
* Rename misleading variables (PossessedConvoy → ActiveConvoy).
* Add loud map+mode logging for missing actor references in setup.

---

## Reconciliation: Final phased plan (Feb prototype scope)

### Week 1 (8 hours): Foundation — ownership + possession (non-negotiable stop line)

1. ✅ Possess explorer + static view target (2h)
2. ✅ Cache `AFCPlayerController*` in InteractionComponent (1h)
3. ✅ Route overlaps via delegate to controller component (2h)
4. ✅ Gate FP trace/prompt to FP mode only (30m)
5. ✅ Input config DataAsset (2h)
6. ✅ Rename misleading variables (30m)

**Acceptance:**

* Camp: explorer moves with static camera, animations can be driven reliably
* Overworld: convoy overlap reaches controller-owned interaction
* No FP prompts in TopDown/Camp
* No hardcoded IA paths in C++

---

### Week 2 (6 hours): Thin GameModes + Mode Coordinator (stop line)

7. ✅ Thin per-level GameModes (2h)

* `AFCBaseGameMode` + `AFCOfficeGameMode` / `AFCCampGameMode` / `AFCOverworldGameMode`
* Each child: <50 lines; sets defaults + calls one setup function
* Prefer instance-editable refs (or tags as fallback) for:

  * CampCameraActor
  * ActiveConvoy
  * Any scene-critical actors needed for wiring

8. ✅ Add `UFCPlayerModeCoordinator` component (3h)

* Switchboard-only; subscribes to game state changes and applies profiles by delegating.

9. ✅ Add `FPlayerModeProfile` struct (1h) — **updated**

* Reference `UFCInputConfig`, not raw mapping contexts (prevents duplicate sources of truth):

```cpp
USTRUCT(BlueprintType)
struct FPlayerModeProfile
{
  UPROPERTY(EditAnywhere)
  TSoftObjectPtr<UFCInputConfig> InputConfig;

  UPROPERTY(EditAnywhere)
  EFCPlayerCameraMode CameraMode;

  UPROPERTY(EditAnywhere)
  TSoftObjectPtr<UInteractionProfile> InteractionProfile;

  UPROPERTY(EditAnywhere)
  EFCClickHandlingPolicy ClickPolicy;

  UPROPERTY(EditAnywhere)
  bool bShowMouseCursor = true;

  UPROPERTY(EditAnywhere)
  EMouseCaptureMode MouseCaptureMode = EMouseCaptureMode::NoCapture;
};
```

**Critical staging tweak:**
Start Week 2 with hard refs/default assets if needed; only move to soft refs after travel is stable.

**Acceptance:**

* Correct GameMode defaults per level
* Mode changes apply profile through coordinator
* Controller has no mode-branching other than `ApplyMode()`

---

### Week 3a (6 hours): Interaction predictability (must-have stop line)

10. ✅ Add explicit “pending” state + enum-for-logs (2h)

* Keep state machine minimal; use enum mainly for visibility/logging:

  * `PendingPOI`, `PendingAction`, `bAwaitingSelection`, `bAwaitingArrival`
  * enum reflects those states but doesn’t become a complex engine

11. ✅ Arrival-gated execution + latch (2h)

* Execute only when:

  * `bAwaitingArrival == true && PendingPOI == POI`
* Clear latch immediately on first execution
* Log overlap arrivals without pending POI for tuning (no crash)

12. ✅ Harmonize click semantics (one authority) (2h)

* FirstPerson: `OnInteractPressed()` → InteractionComponent
* TopDown/Camp: click → controller routes to InteractionComponent, but only one initiator calls move

**Acceptance:**

* POI executes only on arrival and never twice
* Click initiates movement from exactly one place
* Logs show pending state transitions clearly

---

### Week 3b (optional polish, 4 hours): Nice-to-have (skip if time tight)

13. ⚠️ Centralize UI blocking queries (2h)

* `CanWorldClick()` and `CanWorldInteract()` in UIManager

14. ⚠️ Add `UInteractionProfile` DataAsset (2h)

* If not already needed, keep probe config in code for prototype and migrate later.

**Acceptance (if done):**

* Input/interaction/camera config is data-driven and stable across levels

---

### Week 4–5: Polish + evaluate (Feb buffer)

15. ✅ AnimBP velocity fix (2h, only if visible in prototype)
16. ✅ Missing actor error logging (1h)
17. ⚠️ Decision point: does controller remain clean?

* If coordinator pattern insufficient → plan controller split for June

18. ✅ Update docs (`FCRuntime.md`) (1h)

* Include dependency direction contract + stop lines

---

## June demo hardening (defer until after Feb)

* Add `UFCActorRegistrySubsystem` **only if needed** (runtime spawning, multi-actor scenarios, persistent identity constraints)
* Split controllers **only if needed** (if mode complexity still causes giant branching)
* Move POI action definitions to DataAssets for balancing/localization
* Expand interaction profile system to per-pawn configs if required

---

## Critical success factors (prevent regressions)

* Coordinator stays dumb (switchboard only)
* Profiles stay config-only (no hidden behavior)
* UIManager remains view + blocking state (no gameplay branching)
* No world scans or “first player controller” in gameplay logic
* No actor pointers persisted across travel in GameInstance
* Timebox refactors: if a fix keeps causing edge cases, allow surgical internal rewrite while keeping API stable

---

## Suggested acceptance checks (final prototype validation)

1. Ownership sanity: convoy overlap reaches controller interaction (no pawn fetch)
2. Arrival gating: POI actions execute only on arrival overlap, never twice
3. Single authority: only one system triggers movement per click
4. Mode gating: FP prompts only in FP mode
5. Camp movement: explorer moves with static camera + animations blend via velocity magnitude
6. UI blocking: blocking widgets stop click/interact appropriately (`CanWorldClick/Interact`)
7. Input config: no hardcoded IA paths remain in C++
8. Error visibility: missing actors log map + mode and abort safely (no silent failures)