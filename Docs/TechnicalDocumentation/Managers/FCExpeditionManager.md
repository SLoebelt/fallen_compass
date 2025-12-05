## UFCExpeditionManager — Technical Documentation (Manager)

### Where to find it (paths)

* **Header:** `Expedition/FCExpeditionManager.h` 
* **Source:** `Expedition/FCExpeditionManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCExpeditionManager`** is a **`UGameInstanceSubsystem`** that persists for the lifetime of the Game Instance and orchestrates two main domains: 

1. **Expedition lifecycle**

   * Creates/owns the current expedition data object (`CurrentExpedition`).
   * Starts, ends, and reports expedition state changes via a broadcast delegate.

2. **World map exploration + planning support**

   * Maintains and updates **fog-of-war** state (reveal mask) and a **route preview** mask.
   * Creates and updates transient textures (`FogTexture`, `RouteTexture`) for UI/material usage.
   * Records visited world locations and reveals map pixels accordingly.
   * Saves world-map exploration to a save slot with **debounced autosave**.

---

## Public API (Blueprint-facing)

### Expedition lifecycle

* `StartNewExpedition(const FString& ExpeditionName, int32 AllocatedSupplies) -> UFCExpeditionData*`

  * Ends any existing active expedition defensively, then creates a new expedition object and broadcasts.
* `GetCurrentExpedition() -> UFCExpeditionData*` 
* `EndExpedition(bool bSuccess)`

  * Marks expedition Completed/Failed, broadcasts, clears `CurrentExpedition`.
* `IsExpeditionActive() -> bool`

  * True only when `CurrentExpedition` exists and status is `InProgress`.
* `OnExpeditionStateChanged` (multicast delegate)

  * UI/gameplay can subscribe to react to expedition state changes.

### World map: textures

* `WorldMap_GetFogTexture() -> UTexture2D*` 
* `WorldMap_GetRouteTexture() -> UTexture2D*` 

### World map: planning / route preview

* `WorldMap_SelectGridArea(int32 GridId) -> bool`

  * Validates selection (must match `AvailableStartGridId`), creates a “Planning” expedition if needed, sets selection fields, then builds preview route.
* `WorldMap_BuildPreviewRoute() -> bool`

  * Computes shortest path (BFS) from office to preview target; computes money/risk costs; stores into `CurrentExpedition`.
* `WorldMap_BeginRoutePreviewPaint(float StepSeconds = 0.03f)`

  * Starts a timer-driven “animated paint” of the route mask into `RouteTexture`.
* `WorldMap_ClearRoutePreview()`

  * Stops paint timer, clears route mask, updates route texture (includes a debug-marked GlobalId 4240).

### World map: overworld exploration

* `WorldMap_RecordVisitedWorldLocation(const FVector& WorldLocation)`

  * Converts world (X,Y) into normalized UV using configured bounds, maps to 256×256 global id, reveals it, updates fog pixel, debounced autosave, broadcasts `OnWorldMapChanged`.
* `OnWorldMapChanged` (multicast delegate) 

---

## Connected systems (“connected managers”) and what/why is delegated

Even though this class is “the manager”, it **delegates specialized responsibilities** to other objects/types:

### 1) `UFCGameInstance` (configuration source)

**What it provides**

* Land mask texture reference, overworld bounds, grid/sub ids, and default revealed grid ids.

**Why delegated**

* GameInstance is the natural “configuration hub” for subsystem initialization; this avoids hardcoding project-specific values into the subsystem and keeps tuning centralized.

---

### 2) `FFCWorldMapExploration` (world map domain model + algorithms)

**What it provides**

* Storage for `RevealMask` and `LandMask`, and helpers like:

  * global-id conversions (`XYToGlobalId`, `GlobalIdToXY`, `AreaSubToGlobalId`)
  * reveal/water queries (`SetRevealed_Global`, `IsWater_Global`, `IsRevealed_Global`)
  * default reveal application
  * **pathfinding** via `FindShortestPath_BFS`

**Why delegated**

* Keeps the manager focused on orchestration and integration (textures, timers, saving, events), while the exploration struct encapsulates grid math + pathfinding + state rules in one place.

---

### 3) `UFCExpeditionData` (expedition state container)

**What it provides**

* A UObject container holding expedition name/status and planning details (selected ids, route, computed costs, etc.). The manager creates and fills it, then broadcasts it.

**Why delegated**

* Keeps expedition state as a standalone UObject suitable for Blueprint/UI binding, saving, or inspection without exposing internal manager machinery.

---

### 4) Save system: `UFCWorldMapSaveGame` + `UGameplayStatics`

**What it provides**

* SaveGame object type storing `RevealMask` + `LandMask`, and engine API to save to slot.

**Why delegated**

* Uses Unreal’s standard persistence mechanism; the manager only triggers and populates the save object.

> Note: In `WorldMap_InitOrLoad()`, the actual “load from slot” logic is currently commented out (debug state). Right now it always applies defaults + loads land mask from texture. 

---

### 5) Engine runtime services (timers + textures)

**TimerManager**

* Used for:

  * debounced autosave (`0.75s` one-shot)
  * animated route paint ticks (repeating timer)

**UTexture2D transient textures**

* Fog and route are created as 256×256 grayscale (PF_G8) with nearest filtering.
* Updated via `UpdateTextureRegions` (async), with heap allocation + cleanup callback to avoid dangling pointers.

**Why delegated**

* Leverages engine primitives for scheduling and GPU resource updates rather than reinventing these systems inside gameplay code.

---

## Key internal flows (how it behaves)

### Initialization

1. Resets `CurrentExpedition`.
2. Pulls configuration from `UFCGameInstance` (if available).
3. Initializes/loads world map exploration state.
4. Creates fog/route textures and syncs them with masks.

### Exploration → fog update → autosave

`WorldMap_RecordVisitedWorldLocation`:

* WorldPosition → UV → grid XY → GlobalId
* `SetRevealed_Global(GlobalId, true)`
* If changed: update fog pixel, start debounced autosave, broadcast `OnWorldMapChanged`.

### Planning selection → route preview

`WorldMap_SelectGridArea(GridId)`:

* Only accepts the configured “available start” grid.
* Ensures a planning expedition exists.
* Sets selection info, then `WorldMap_BuildPreviewRoute()`.

### Path cost logic

`ComputeCostsForPath(Path, OutMoney, OutRisk)`:

* Each step (excluding the first):

  * **Money:** water=3, land=1
  * **Risk:** water=2 if revealed else 3; land=1 