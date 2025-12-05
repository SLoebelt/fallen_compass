## UFCGameInstance — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `UFCGameInstance.h` 
* **Source:** `UFCGameInstance.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCGameInstance`** is the project’s **long-lived configuration + persistence hub**. It exists across level transitions and is responsible for wiring up core subsystems and storing campaign context data.

1. **Subsystem bootstrap/configuration**

   * Configures **`UFCLevelManager`** with the **LevelMetadataTable** at init.
   * Configures **`UFCUIManager`** with widget classes + table widget registry on init.

2. **Global “expedition context” (persistent meta state)**

   * Persists `CurrentExpeditionId`, `DiscoveredRegions`, `ExpeditionsCounter`.
   * Tracks `bIsSessionDirty` and broadcasts `OnExpeditionContextChanged` for UI/gameplay refresh.

3. **Resources/economy state**

   * Owns `FFCGameStateData` (Supplies, Money, Day) and provides add/consume helpers that mark the session dirty and broadcast context changes.

4. **Save/Load orchestration**

   * Creates and writes `UFCSaveGame` to slots.
   * Loads saves, handles cross-level load via fade + delayed `OpenLevel`, and restores player transform after load.

5. **Post-map-load handling**

   * Subscribes to `FCoreUObjectDelegates::PostLoadMapWithWorld` and triggers fade-in if the screen is black (with a longer delay when restoring a save). 

6. **Centralized editor-configurable project hooks**

   * Holds UI widget class references, transition widget class reference, level metadata table, and expedition/world-map configuration used by other managers (e.g., ExpeditionManager world-map setup values). 

---

## Public API (Blueprint-facing)

### Expedition context

* `SetCurrentExpeditionId(const FString& InExpeditionId)`

  * Sets id, marks dirty, broadcasts `OnExpeditionContextChanged`.
* `MarkSessionSaved()`

  * Clears `bIsSessionDirty`.
* `OnExpeditionContextChanged` (multicast event)

### Version / meta

* `GetGameVersion() -> FString`

  * Reads `ProjectVersion` from config (`GeneralProjectSettings`).

### Resources / economy

* `GetGameStateData() -> const FFCGameStateData&` 
* `GetCurrentSupplies() -> int32` *(deprecated field exists but getter uses `GameStateData.Supplies`)*
* `AddSupplies(int32 Amount)`
* `ConsumeSupplies(int32 Amount, bool& bSuccess) -> int32`
* `GetMoney() -> int32` 
* `AddMoney(int32 Delta)` (clamps at 0)
* `ConsumeMoney(int32 Amount) -> bool`

### Save / load

* `SaveGame(const FString& SlotName) -> bool`

  * Saves: slot name, timestamp, game version, current map name, player location/rotation (if pawn is `AFCFirstPersonCharacter`), and expedition context fields.
* `LoadGameAsync(const FString& SlotName)`

  * Loads a `UFCSaveGame`, caches it as `PendingLoadData`, then:

    * if target level differs: fades out (with loading indicator) and opens level after a timer delay
    * if same level: restores position immediately
  * Broadcasts `OnGameLoaded(bool bSuccess)` (note: success is broadcast after loading/queuing, not after restoration completes).
* `GetAvailableSaveSlots() -> TArray<FString>`

  * Detects slots: `AutoSave_001..003`, `QuickSave`, `Manual_001..010` if they exist.
* `GetMostRecentSave() -> FString`

  * Loads each found slot and chooses the highest timestamp.
* `RestorePlayerPosition()`

  * Applies cached `PendingLoadData` transform to the pawn, updates controller rotation, calls `PC->TransitionToGameplay()`, updates `UFCLevelManager` current level, then clears `PendingLoadData`.
* `IsRestoringSaveGame() -> bool` 
* `OnPostLoadMapWithWorld(UWorld* LoadedWorld)`

  * If transition is black, schedules `BeginFadeIn(1.0)` after a small delay (0.5s when restoring, else 0.2s).

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `UFCLevelManager` (level metadata + name normalization)

**Delegated:** data-driven level configuration via `SetLevelMetadataTable()`; also used during load to normalize map names and detect same-level loads.
**Why:** level identity/type rules are centralized in the level manager, while GameInstance handles save/load orchestration.

### 2) `UFCUIManager` (widget ownership + registry)

**Delegated:** UI widget lifecycle; UIManager receives widget classes and the `TableWidgetMap` registry from GI at `Init()`.
**Why:** avoids UI ownership in GameInstance; GI only acts as a configuration source of truth.

### 3) `UFCTransitionManager` (fades / loading overlay)

**Delegated:** visual fade-out/fade-in during cross-level loads and post-load reveal.
**Why:** the fade widget needs to persist across travel and stay consistent; TransitionManager owns that layer, GI only triggers it.

### 4) Save system (`UFCSaveGame` + `UGameplayStatics`)

**Delegated:** creation, serialization, slot I/O, existence checks, and level travel (`OpenLevel`).
**Why:** uses Unreal’s standard persistence/travel flow; GI just fills and consumes the save payload.

### 5) Player runtime (`AFCPlayerController`, `AFCFirstPersonCharacter`)

**Delegated:** restore target pawn/controller and re-enter gameplay state (`TransitionToGameplay()`).
**Why:** controller/pawn are responsible for gameplay input/camera state; GI restores raw transform and triggers the appropriate controller flow.

### 6) Engine delegates & config (`FCoreUObjectDelegates`, `GConfig`)

**Delegated:** post-load map notifications and project version lookup.
**Why:** makes fade-in and version reporting “engine-native” and reliable across maps.

---

## Where to find / configure key pieces (paths & editor-facing knobs)

### Configure in your GameInstance Blueprint (e.g., `BP_FCGameInstance`)

* **UI Widget Classes:** `MainMenuWidgetClass`, `SaveSlotSelectorWidgetClass`, `PauseMenuWidgetClass`, `ActionSelectionWidgetClass`, `OverworldMapHUDWidgetClass`, `ExpeditionSummaryWidgetClass`, and `TableWidgetMap`.
* **Level metadata:** set `LevelMetadataTable` so `UFCLevelManager` can become data-driven.
* **Screen transitions:** set `TransitionWidgetClass` so the fade system can spawn. 

### Expedition / WorldMap configuration (consumed by Expedition/world-map systems)

* `WorldMapLandMaskTexture`, `OverworldWorldMin/Max`, `OfficeGridId/SubId`, `AvailableStartGridId/SubId`, `PreviewTargetGridId/SubId`, `DefaultRevealedWorldMapGridIds`. 

### Save slot naming conventions used by the runtime

* Auto: `AutoSave_001..003`
* Quick: `QuickSave`
* Manual: `Manual_001..010`
