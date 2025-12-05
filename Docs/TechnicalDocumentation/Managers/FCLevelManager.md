## UFCLevelManager — Technical Documentation (Manager)

### Where to find it (paths)

* **Header:** `Core/FCLevelManager.h` 
* **Source:** `Core/FCLevelManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCLevelManager`** is a **`UGameInstanceSubsystem`** that acts as the **single source of truth for “what level are we in?”** and “what kind of level is it?” across the game.

Core responsibilities:

1. **Track level identity**

   * Caches `CurrentLevelName` and `PreviousLevelName`.
   * Normalizes map names by stripping PIE prefixes like `UEDPIE_0_…` and trimming whitespace.

2. **Determine level category (type)**

   * Computes `CurrentLevelType` (Office / Overworld / Camp / POI / etc.).
   * Prefers **data-driven metadata** via a DataTable; falls back to string matching for backward compatibility.

3. **Provide centralized queries for other systems**

   * `IsMenuLevel()`, `IsGameplayLevel()`, and metadata queries (`GetCurrentLevelMetadata`, `GetLevelMetadata`).

4. **Perform level loads with a fade-out handshake**

   * `LoadLevel()` initiates a fade-out via `UFCTransitionManager`, then opens the target level when fade completes.
   * Updates its cached current level immediately after calling `OpenLevel` so other subsystems querying it on BeginPlay see correct data.

---

## Public API (Blueprint-facing)

### Level identity / navigation

* `GetCurrentLevelName() -> FName`
* `GetCurrentLevelType() -> EFCLevelType`
* `GetPreviousLevelName() -> FName` 

### Level type queries

* `IsMenuLevel() -> bool`

  * True if `CurrentLevelType == MainMenu`. 
* `IsGameplayLevel() -> bool`

  * True for: Office, Overworld, Camp, POI, Village.

### Data-driven metadata

* `GetLevelMetadata(FName LevelName, FFCLevelMetadata& Out) -> bool`

  * Normalizes the level name and looks up a row in a DataTable of type `FFCLevelMetadata`.
* `GetCurrentLevelMetadata(FFCLevelMetadata& Out) -> bool`

  * Convenience wrapper for current level.
* `SetLevelMetadataTable(UDataTable* InMetadataTable)`

  * Validates table row struct is `FFCLevelMetadata`, logs rows for debugging, then re-evaluates current level type.

### Level loading

* `LoadLevel(FName LevelName, bool bShowLoadingScreen = false)`

  * Validates name, normalizes it, binds a one-shot handler to transition manager’s `OnFadeOutComplete`, then calls `BeginFadeOut()` (note: it currently ignores the `bShowLoadingScreen` flag and just calls `BeginFadeOut()` with defaults).

### Utility / internal-but-public helpers

* `NormalizeLevelName(const FName& RawLevelName) -> FName`

  * Strips PIE prefix `UEDPIE_X_` and trims whitespace.
* `DetermineLevelType(const FName& LevelName) -> EFCLevelType`

  * Priority 1: use DataTable metadata if available.
  * Priority 2: fallback substring checks (“Office”, “Overworld”, “Camp”, “Combat”, “POI”, “Village”), else Unknown.

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `UFCTransitionManager` (fade timing / visual transition)

**What/Why**

* `LoadLevel` does not immediately `OpenLevel`. It delegates the fade-out to `UFCTransitionManager` and waits for `OnFadeOutComplete`, ensuring a clean black-screen cut.
* The manager binds and then unbinds a one-shot delegate (`OnFadeOutCompleteForLevelLoad`) to avoid repeated callbacks.

---

### 2) `UFCGameInstance` (subsystem access + configuration hub)

**What/Why**

* Used to retrieve `UFCTransitionManager` via `GI->GetSubsystem<UFCTransitionManager>()`.
* Also implied as the place where `SetLevelMetadataTable` should be called (“set via Blueprint in BP_FC_GameInstance”).

---

### 3) Unreal Engine level travel (`UGameplayStatics::OpenLevel`)

**What/Why**

* Once fade is complete, performs the actual travel using `OpenLevel(this, LevelToLoad)`. 
* Immediately calls `UpdateCurrentLevel(LevelToLoad)` so other subsystems reading `GetCurrentLevelName()` during new level BeginPlay get normalized name instead of the old one. 

---

### 4) DataTable-driven configuration (`UDataTable` + `FFCLevelMetadata`)

**What/Why**

* Designers can configure each level’s behavior in `DT_LevelMetadata` instead of hardcoding name checks.
* If metadata is missing, the system still works via fallback substring matching (backward compatibility).

---

## Level metadata model (what designers can configure)

`FFCLevelMetadata` rows define: 

* `LevelType` (EFCLevelType)
* `DefaultInputMode` (0 UIOnly, 1 GameAndUI, 2 GameOnly)
* `bRequiresFadeTransition`
* `bShowCursor`
* `bRequiresLoadingScreen` (future)
* `DisplayName`, `Description`

---

## Where to configure / extend (practical notes)

* **Assign the metadata DataTable** by calling `SetLevelMetadataTable()` from your GameInstance init (Blueprint).
* If you want `LoadLevel(..., bShowLoadingScreen=true)` to actually show a loading indicator, you’ll need to forward that flag into `TransitionMgr->BeginFadeOut(Duration, bShowLoadingIndicator)` (the parameter exists in your TransitionManager API, but this manager currently doesn’t use it). 
