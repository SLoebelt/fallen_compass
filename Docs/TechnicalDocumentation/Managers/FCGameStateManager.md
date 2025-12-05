## UFCGameStateManager — Technical Documentation (Manager)

### Where to find it (paths)

* **Header:** `Core/FCGameStateManager.h` 
* **Source:** `Core/FCGameStateManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCGameStateManager`** is a **`UGameInstanceSubsystem`** that implements the game’s explicit **global state machine** (“what mode is the game in?”) and enforces allowed transitions between those states.

It owns:

1. **Authoritative current/previous game state**

   * `CurrentState`, `PreviousState`
   * Starts in `None` on subsystem init.

2. **Transition validation**

   * A `ValidTransitions` map defines which target states are legal from each current state.

3. **Transition execution + notification**

   * Executes the state change and broadcasts `OnStateChanged(OldState, NewState)` so other systems react (UI, input, camera, gameplay logic).

4. **Optional nested-state support**

   * A simple `StateStack` supports “push/pop” flows (e.g., Paused/modals).

5. **Loading “two-hop” helper**

   * `TransitionViaLoading(TargetState)` stores a `LoadingTargetState` so other systems can do:

     * `Current -> Loading -> (level load) -> TargetState`
     * without MainMenu/other systems overriding the intended post-load state.

---

## Public API (Blueprint-facing)

### State queries

* `GetCurrentState() -> EFCGameStateID` 
* `GetPreviousState() -> EFCGameStateID` 
* `GetLoadingTargetState() -> EFCGameStateID`

  * Only meaningful after `TransitionViaLoading(...)` when current is `Loading`.

### Transition operations

* `CanTransitionTo(EFCGameStateID NewState) -> bool`

  * Allows “transition to same state” as a no-op (returns true).
* `TransitionTo(EFCGameStateID NewState) -> bool`

  * Validates against `ValidTransitions`, updates `PreviousState`, sets `CurrentState`, then broadcasts `OnStateChanged`.
* `TransitionViaLoading(EFCGameStateID TargetState) -> bool`

  * Sets `LoadingTargetState = TargetState`.
  * Transitions to `Loading` immediately (if not already), broadcasts `OnStateChanged`.
  * **Does not** automatically hop to `TargetState`; external systems finalize after the level finishes loading by calling `TransitionTo(TargetState)`.

### State stack (nested states)

* `PushState(EFCGameStateID NewState) -> bool`

  * Validates the transition from current → new.
  * Pushes current state onto stack, then transitions to NewState + broadcasts.
* `PopState() -> bool`

  * Pops a state and restores it **without transition validation** (“trust the stack”), then broadcasts.
* `GetStateStackDepth() -> int32` 
* `GetStateAtDepth(int32 Depth) -> EFCGameStateID`

  * Supports negative indices (`-1` = top). Returns `None` if invalid.

### Events

* `OnStateChanged(OldState, NewState)` (BlueprintAssignable multicast)

---

## Game states covered (enum)

`EFCGameStateID` includes (as defined in the header): 

* `None`, `MainMenu`
* `Office_Exploration`, `Office_TableView`, `ExpeditionSummary`
* `Overworld_Travel`, `Camp_Local`
* `Combat_PlayerTurn`, `Combat_EnemyTurn`
* `Paused`, `Loading`

---

## Connected systems (“connected managers”) and what/why is delegated

This manager intentionally **does not** implement UI, input, camera, or level-loading behavior itself. Instead:

### 1) UI / PlayerController / gameplay systems (reactive responsibilities)

**What is delegated**

* Actual behavior changes when state switches (show/hide menus, input mode changes, camera swaps, starting combat, etc.).

**Why delegated**

* Keeps `UFCGameStateManager` a pure, testable “rules + event” state machine. Everything else subscribes to `OnStateChanged` and reacts.

### 2) Level loading orchestration (external finalization of Loading → Target)

**What is delegated**

* After `TransitionViaLoading(Target)`, some other system performs the level load and then calls `TransitionTo(Target)`.

**Why delegated**

* Your code explicitly states level-loading responsibilities should remain outside this manager to avoid coupling state rules to travel logic.

---

## Where to configure / extend (practical notes)

* **Add new states:** extend `EFCGameStateID` in the header and update `InitializeValidTransitions()` to include rules for the new state.
* **Change allowed flows:** edit the per-state arrays in `InitializeValidTransitions()` (this is the single source of truth for legality). 
* **If you rely on `PopState()`:** note it bypasses validation—by design—so only push states you truly want to restore later. 
