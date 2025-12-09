## UFCUIManager — Technical Documentation (Manager)

### Where to find it (paths)

* **Header:** `Core/FCUIManager.h` 
* **Source:** `Core/FCUIManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCUIManager`** is a **`UGameInstanceSubsystem`** that centralizes **UI widget creation and lifecycle management** so UI can persist across level transitions and to reduce Blueprint ↔ PlayerController coupling.

Core responsibilities:

1. **Main menu flow**

   * Create/cache and show/hide **MainMenu** and **SaveSlotSelector** widgets.

2. **Pause menu orchestration**

   * Show/Hide pause menu with correct **input mode** and **mouse cursor**.
   * Conditionally uses **engine pause** depending on level (Overworld pauses engine; Office does not).

3. **Context widgets**

   * **Table widget system:** maps an interactable table actor class → widget class, ensures only one is open at a time.
   * **POI action selection:** spawns an action selector widget, populates it with actions, and forwards the chosen action back into gameplay via a mediator pattern.

4. **Blocking/focus handling**

   * Tracks a `FocusedBlockingWidget` to gate world interaction when certain UIs are open (e.g., overworld map HUD, expedition summary).

5. **Overworld/Expedition UI**

   * Creates and shows the view-only **Overworld map HUD**.
   * Creates, shows, and closes the **Expedition summary** widget (blocking).

---

## Public API (Blueprint-facing)

### Main menu widgets

* `ShowMainMenu()`
* `HideMainMenu()`
* `ShowSaveSlotSelector()`
* `HideSaveSlotSelector()`

**Behavior notes**

* Widgets are created once and cached (`MainMenuWidget`, `SaveSlotSelectorWidget`) and then reused.

---

### Pause menu

* `ShowPauseMenu()`
* `HidePauseMenu()`

**Key behavior**

* Adds pause menu to viewport at **Z-order 100** (above gameplay UI). 
* Detects the current map name and:

  * **Enables engine pause** only when the level name contains `L_Overworld`
  * **Disables engine pause** otherwise (notably Office) so input still works. 
* Sets `FInputModeGameAndUI`, focuses pause widget, shows cursor; restores `GameOnly` and hides cursor when closing. 

---

### Table widget system (actor → widget registry)

* `ShowTableWidget(AActor* TableObject)`
* `CloseTableWidget()`
* Queries:

  * `GetCurrentTableWidget() const`
  * `IsTableWidgetOpen() const`

**Behavior notes**

* Uses `TableWidgetMap` to resolve the widget class based on `TableObject->GetClass()`.
* Ensures only one table widget is on screen: opening a new one will close the existing widget first.

---

### POI action selection (mediator pattern)

* `ShowPOIActionSelection(const TArray<FFCPOIActionData>& Actions, UFCInteractionComponent* InteractionComponent) -> UUserWidget*`
* `ClosePOIActionSelection()`
* Queries:

  * `GetCurrentPOIActionSelectionWidget() const`
  * `IsPOIActionSelectionOpen() const`
* Callback:

  * `HandlePOIActionSelected(EFCPOIAction Action)`

**Behavior notes**

* Stores the `InteractionComponent` temporarily in `PendingInteractionComponent`, letting UI remain decoupled from gameplay actor logic.
* After creating the widget, it looks up and calls a Blueprint function named **`PopulateActions`** via `ProcessEvent` to push the `Actions` array into the widget. 
* When an action is selected, it calls `PendingInteractionComponent->OnPOIActionSelected(Action)` then closes the widget and clears references.
* **View-only invariant (0003/0004):** `UFCUIManager` never issues movement, executes POI actions, or mutates `UFCInteractionComponent`'s internal state; it only presents the POI action selection UI and forwards the chosen `EFCPOIAction` back via `OnPOIActionSelected`.

### Blocking widgets and world-input gating

`UFCUIManager` cooperates with `UFCUIBlockSubsystem` to express which widgets block world input:

* Widgets based on blocking-aware base classes (pause menu, table widgets, POI action selection, overworld map, expedition summary) automatically register/unregister as blockers in `UFCUIBlockSubsystem` with `bBlocksWorldClick` / `bBlocksWorldInteract` flags.
* Legacy blocking widgets are still driven by `UFCUIManager` calling into the subsystem to set/clear a focused blocker until they are migrated.
* Controllers and interaction components do not inspect widgets directly; they query `AFCPlayerController::CanWorldClick()` / `CanWorldInteract()`, which are backed by `UFCUIBlockSubsystem`.
* **View-only invariant (0003):** `UFCUIManager` never issues movement, executes POI actions, or mutates `UFCInteractionComponent`’s internal state; it only presents UI and forwards `EFCPOIAction` back to `OnPOIActionSelected`.

---

### Focus / blocking helpers

* `SetFocusedBlockingWidget(UUserWidget* Widget)`
* `IsFocusedWidgetOpen() const`

**Meaning**

* “Blocking” widgets (like map HUD / summary) can be tracked so other systems can prevent world interaction while UI is active. 

---

### Overworld & expedition summary UI

* `ShowOverworldMapHUD(APlayerController* OwningPlayer) -> UUserWidget*`
* `ShowExpeditionSummary(APlayerController* OwningPlayer) -> UUserWidget*`
* `CloseExpeditionSummary()`

**Behavior notes**

* Both add to viewport and set `FocusedBlockingWidget` to the created widget.
* Summary auto-closes any existing summary widget before creating a new one.

---

### Button callback methods (called from UI Blueprints)

* `HandleNewLegacyClicked()`

  * Hides main menu and asks `AFCPlayerController` to `TransitionToGameplay()`.
* `HandleContinueClicked()`

  * Uses `UFCGameInstance::GetMostRecentSave()` and `LoadGameAsync(Slot)`; hides main menu first.
* `HandleLoadSaveClicked()`

  * Hides main menu and shows save slot selector.
* `HandleOptionsClicked()`

  * Placeholder (not implemented yet).
* `HandleQuitClicked()`

  * Uses `UKismetSystemLibrary::QuitGame(...)`.
* `HandleBackFromSaveSelector()`

  * Hides selector, returns to main menu.
* `HandleSaveSlotSelected(const FString& SlotName)`

  * Hides selector and calls `LoadGameAsync(SlotName)`.

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `UFCGameInstance` (configuration + save orchestration)

**What/Why**

* UIManager expects widget classes to be configured from the GameInstance (e.g., `MainMenuWidgetClass`, `OverworldMapHUDWidgetClass`, etc.).
* Uses `GetMostRecentSave()` and `LoadGameAsync()` for the continue/load flows instead of managing save logic inside UI code.

---

### 2) `AFCPlayerController` (gameplay transitions)

**What/Why**

* UIManager triggers high-level transitions (New Legacy → gameplay) by delegating to the PlayerController’s `TransitionToGameplay()`.

---

### 3) `UFCInteractionComponent` (gameplay interaction execution)

**What/Why**

* UIManager acts as a **mediator**: it collects the selected POI action from UI and forwards it to `UFCInteractionComponent::OnPOIActionSelected`. This keeps widgets “dumb” and interaction logic in gameplay code.

---

### 4) Unreal Engine UI framework (`UUserWidget`, viewport) + input system

**What/Why**

* Uses `CreateWidget`, `AddToViewport`, `RemoveFromParent` for widget lifecycle.
* Uses `FInputModeGameAndUI` / `FInputModeGameOnly`, cursor visibility, and conditional `SetPause(true/false)` for pause behavior.

---

## Where to configure / extend (practical notes)

* **Widget class references** are `UPROPERTY()` fields on the subsystem; they’re intended to be set from the GameInstance Blueprint (comment says “configured by GameInstance”).
* **TableWidgetMap** must include entries for each table actor class you want UI for (ActorClass → WidgetClass). 
* **POI Action selection widget** must implement a Blueprint function named **`PopulateActions`** that accepts `TArray<FFCPOIActionData>`, otherwise UIManager logs an error and the widget won’t be initialized correctly.