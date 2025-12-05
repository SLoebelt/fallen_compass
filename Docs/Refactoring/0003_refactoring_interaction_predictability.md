# PRD — Interaction Predictability (Pending State + Arrival-Gated Execution)

## 1) Introduction/Overview
This milestone makes POI interaction predictable by:
- Adding explicit pending state in the interaction orchestrator
- Gating POI action execution strictly on arrival signals
- Ensuring exactly one movement authority per click

## 2) Goals
- POI actions execute exactly once and only when the mover arrives.
- Eliminate duplicate movement initiation paths.
- Add clear logging for interaction state to speed debugging.
- Keep UIManager view-only; gameplay decisions stay in interaction orchestration.

## 3) User Stories
- As a player, selecting a POI action and moving there triggers the action only upon arrival.
- As a player, clicking a POI opens selection when needed and behaves consistently when not.
- As a developer, logs show clear interaction states (Idle → Selecting → Moving → Executing).

## 4) Functional Requirements
1. **Explicit pending state**
   1.1 Interaction component must maintain:
   - `FocusedTarget`
   - `PendingPOI`
   - `PendingAction`
   - `bAwaitingSelection`
   - `bAwaitingArrival`
   1.2 An enum may be used for logging/visibility; avoid complex transition engines unless required.
2. **Arrival-gated execution**
   2.1 Implement `NotifyArrivedAtPOI(AActor* POI)` on the interaction component.  
   2.2 Execute only if `bAwaitingArrival == true && PendingPOI == POI`.  
   2.3 Clear latch immediately after execution to prevent double fire.  
   2.4 Overlaps without pending POI must not crash; log debug for tuning.
3. **Single authority for movement**
   3.1 For TopDown/Camp, exactly one function path issues the move command per click.  
   3.2 FirstPerson interact does not issue movement (unless explicitly introduced later).
4. **Multi-action selection**
   4.1 If multiple actions exist, enter Selecting state and wait for UI selection callback.  
   4.2 No default pick-first for multi-action POIs.
5. **Single-action policy**
   5.1 Choose and document policy: auto-run OR confirm.  
   5.2 Apply consistently across levels/modes.
6. **UIManager responsibilities**
   6.1 UIManager only shows selection UI and returns result via callback.  
   6.2 UIManager must not execute actions, move actors, or mutate interaction state.

## 5) Security Requirements
1. Actions execute only when explicitly selected (multi-action) or policy allows (single-action).
2. Validate PendingPOI/FocusedTarget at use-time (actors may be destroyed during travel).
3. Ensure UI interactions occur on game thread.

## 6) Non-Goals (Out of Scope)
- Plugin replacement.
- Full POI action DataAsset library.
- WorldSubsystem registry.

## 7) Design Considerations (Optional)
- Action selection UI should block world click until selection is made (if UI blocking polish is implemented).

## 8) Technical Considerations (Optional)
- Prototype arrival source: overlap volumes are acceptable; tune volumes to reduce noise.
- If overlaps prove noisy, consider path-follow completion events later.

## 9) Success Metrics
- No double execution in standard play.
- 100% reliable execution on arrival for convoy and explorer.
- Logs show clear state progression.

## 10) Open Questions
- Arrival definition for prototype: overlap only or overlap + move complete?
- Chosen single-action policy?
