## UFCLevelTransitionManager — Technical Documentation (Manager)

### Where to find it (paths)

* **Header:** `Core/FCLevelTransitionManager.h` 
* **Source:** `Core/FCLevelTransitionManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCLevelTransitionManager`** is a **`UGameInstanceSubsystem`** that orchestrates **high-level game flows that span multiple systems at once**: game-state transitions, level loads, screen fades, and major UI entry/exit points.

Concretely, it owns:

1. **Transition orchestration across subsystems**

   * Ensures a flow changes **state** (via `UFCGameStateManager`), triggers **fade** (via `UFCTransitionManager`), and loads the **target map** (via `UFCLevelManager`) in the intended order.

2. **Post-load “finalization”**

   * On level start, it completes transitions that used `TransitionViaLoading(...)` by inspecting:

     * current level name (normalized via `UFCLevelManager`)
     * loading’s target state (from `UFCGameStateManager`)

3. **Key flow helpers**

   * Start expedition from Office table view → Overworld travel
   * Return from gameplay → main menu hosted in Office
   * Return from Overworld → Office with expedition summary shown
   * Enter Camp via Loading flow
   * Office table view enter/exit state transitions

4. **Centralization / future refactor hook**

   * Provides `RequestTransition(...)` as a future unified entry point (currently logs only).

---

## Public API (Blueprint-facing)

### Expedition-related flow

* `StartExpeditionFromOfficeTableView()`

  * Assumes the caller already validated expedition data and spent money; this method focuses on:

    * `TransitionViaLoading(Overworld_Travel)`
    * `BeginFadeOut(1.0)`
    * Load level `"L_Overworld"` (currently hardcoded)

### Main menu return flow

* `ReturnToMainMenuFromGameplay()`

  * Goes through Loading → MainMenu, hides pause menu if open, fades out with loading indicator enabled, then loads `"L_Office"` (used as the main-menu host map).

### Overworld → Office with summary flow

* `ReturnFromOverworldToOfficeWithSummary()`

  * Allowed when current state is `Overworld_Travel`, or when current state is `Paused` (assumes underlying “paused over overworld travel” case).
  * Hides main menu (to avoid Office auto-menu being visible on return path).
  * Goes Loading → `ExpeditionSummary`, fades out (loading indicator true), loads `"L_Office"`.
  * Actual showing of the summary is completed in the post-load initializer.

* `CloseExpeditionSummaryAndReturnToOffice()`

  * Closes summary widget via `UFCUIManager` (if available), then transitions state to `Office_TableView`.

### Camp flow

* `EnterCampFromGameplay()`

  * Requests Loading → `Camp_Local`, begins fade out with loading indicator, loads `"L_Camp"`.
  * The post-load initializer finalizes `Loading -> Camp_Local` after the map finishes loading.

### Level-start finalization

* `InitializeLevelTransitionOnLevelStart()`

  * Only reacts when **current state is `Loading`**.
  * Reads:

    * `CurrentLevelName` from `LevelMgr->GetCurrentLevelName()` (normalizes PIE prefixes)
    * `LoadingTarget` from `StateMgr->GetLoadingTargetState()`
  * Handles known cases:

    * `"L_Office"` + target `ExpeditionSummary`: `TransitionTo(ExpeditionSummary)` then asks UIManager to show summary via `ProcessEvent` call to `ShowExpeditionSummary(PC)`.
    * `"L_Camp"` + target `Camp_Local`: `TransitionTo(Camp_Local)`.

* `InitializeOnLevelStart()`

  * “Unified startup” wrapper:

    * Handles Office startup/main menu detection:

      * If in `"L_Office"` and state is `None`, or `Loading(target=MainMenu)`, or already `MainMenu`, it ensures state is `MainMenu` and calls `AFCPlayerController::InitializeMainMenu()`.
    * Otherwise delegates to `InitializeLevelTransitionOnLevelStart()` for Loading-based flows.

### Office table view flows

* `EnterOfficeTableView(AActor* TableActor)`

  * Only meaningful from `Office_Exploration`.
  * Transitions `Office_Exploration -> Office_TableView`.
  * Leaves camera/input reaction to `AFCPlayerController::OnGameStateChanged` (by design).

* `ExitOfficeTableView()`

  * Only valid from `Office_TableView`.
  * Closes any open table widget via `UFCUIManager` (if one is open).
  * Transitions `Office_TableView -> Office_Exploration`.

### Generic transition entry point (currently stub)

* `RequestTransition(EFCGameStateID TargetState, FName TargetLevelName = NAME_None, bool bUseFade = true)`

  * Logs parameters and returns (no functional behavior yet).

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `UFCGameStateManager` (validated state machine)

**What it does here**

* Performs the authoritative state changes:

  * `TransitionViaLoading(Target)`
  * `TransitionTo(Target)`
  * `GetCurrentState()`, `GetLoadingTargetState()`

**Why delegated**

* Keeps rules/validation of state transitions centralized in a dedicated state system; LevelTransitionManager just sequences the flow.

---

### 2) `UFCLevelManager` (map travel + level name normalization)

**What it does here**

* Loads maps: `LoadLevel("L_Overworld")`, `LoadLevel("L_Office")`, `LoadLevel("L_Camp")`.
* Provides normalized current map name: `GetCurrentLevelName()` (important for PIE).

**Why delegated**

* Encapsulates the engine travel details and makes the transition manager’s logic map-name driven and consistent.

---

### 3) `UFCTransitionManager` (screen fade / loading overlay)

**What it does here**

* `BeginFadeOut(Duration, bShowLoading)` for transitions, consistently used before/around level loads.

**Why delegated**

* Visual transition/overlay complexity stays in a dedicated manager that persists through travel.

---

### 4) `UFCUIManager` (major UI flow + cleanup)

**What it does here**

* Hides pause menu/main menu when transitioning in ways that would otherwise leave UI visible.
* Closes expedition summary and/or table widget.
* Post-load: asked to show expedition summary (via calling `ShowExpeditionSummary`).

**Why delegated**

* UI creation and widget ownership stays centralized in UIManager; TransitionManager just triggers the “what should happen” flow.

---

### 5) `AFCPlayerController` (camera/input setup hooks)

**What it does here**

* On Office startup / main menu: `InitializeMainMenu()`
* For table view: camera/input changes are expected to react to game-state changes externally (commented migration strategy).

**Why delegated**

* PlayerController remains the right place to apply input modes, camera blends, and pawn/controller-specific behavior, while this manager sticks to orchestration.

---

### 6) `UFCExpeditionManager` (expedition domain)

**What it does here**

* Required as a dependency for `StartExpeditionFromOfficeTableView()` even though the method currently doesn’t call into it heavily; it’s part of the “required subsystem set” guard for starting an expedition flow.

**Why delegated**

* Expedition validation/state is intentionally not implemented in the transition layer; it’s expected to live in expedition/gameplay systems.

---

## Where to configure / extend (practical notes)

* **Hardcoded level names** are currently embedded in the helpers:

  * `"L_Overworld"`, `"L_Office"`, `"L_Camp"` (one TODO notes making overworld level configurable from expedition data).
* The header points to a design doc: `Docs/Technical_Documentation.md` section *“Transition Responsibilities & Orchestration (Week 4 - Step 4.7)”*. 
* There is a private `PendingLoadingTarget` field declared but not used in the current `.cpp` implementation (looks like a placeholder for future refactors).