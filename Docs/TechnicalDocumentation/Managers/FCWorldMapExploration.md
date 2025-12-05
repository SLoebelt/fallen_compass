## FFCWorldMapExploration — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `WorldMap/FCWorldMapExploration.h` 
* **Source:** `WorldMap/FCWorldMapExploration.cpp` 

---

## Responsibilities (what this “manager” owns)

**`FFCWorldMapExploration`** is a pure helper/domain class that owns the **world-map grid representation** and algorithms used for fog-of-war + traversal.

It owns:

1. **Global map mask storage**

   * `RevealMask`: 256×256 (65,536 bytes). Revealed when value ≥ 128 (internally set to 255 or 0).
   * `LandMask`: 256×256 (65,536 bytes). Defaults to “land everywhere” until replaced.

2. **Coordinate conversion helpers**

   * Converts between:

     * **Grid** (16×16 areas) → `GridId`
     * **Subcell** (16×16 inside each area) → `SubId`
     * **Global** (256×256) → `GlobalId`, `(GX, GY)`

3. **New game initialization**

   * Applies “default revealed” grid areas by fully revealing all subcells of each grid id.

4. **Route/pathfinding support**

  * Provides BFS shortest path on the global grid with traversal rules tuned for "expedition preview route" behavior.
  * Serves as a conceptual reference for path-based movement elsewhere (e.g., Explorer Camp movement uses NavMesh paths, not this grid, but follows the same idea of "precompute path, then step through points").

---

## Public API (Blueprint-facing / usable surface)

### Reveal mask

* `GetRevealMask() const -> const TArray<uint8>&` 
* `GetRevealMaskMutable() -> TArray<uint8>&` 
* `IsRevealed_Global(int32 GlobalId) const -> bool`

  * Returns true if `RevealMask[GlobalId] >= 128`.
* `SetRevealed_Global(int32 GlobalId, bool bRevealed) -> bool`

  * Writes 255/0; returns false if invalid id or no state change.

### Land mask

* `GetLandMask() const -> const TArray<uint8>&` 
* `SetLandMask(const TArray<uint8>& InLandMask)`

  * Only accepts input if it matches `GlobalCount` (65,536).
* `IsLand_Global(int32 GlobalId) const -> bool`
* `IsWater_Global(int32 GlobalId) const -> bool` (inline: `!IsLand_Global`) 

### Coordinate helpers (static)

Validation:

* `IsValidGridId(int32 GridId) -> bool`
* `IsValidSubId(int32 SubId) -> bool`
* `IsValidGlobal(int32 X, int32 Y) -> bool`
* `IsValidGlobalId(int32 GlobalId) -> bool`

Conversions:

* `GridIdToXY(GridId, OutX, OutY)` / `XYToGridId(X,Y)`
* `SubIdToXY(SubId, OutSX, OutSY)` / `XYToSubId(SX,SY)`
* `AreaSubToGlobalId(GridId, SubId) -> GlobalId`
* `GlobalIdToAreaSub(GlobalId, OutGridId, OutSubId)`
* `GlobalIdToXY(GlobalId, OutGX, OutGY)` / `XYToGlobalId(GX,GY)`

### Initialization

* `ApplyDefaultRevealedAreas_NewGame(const TArray<int32>& DefaultGridIds)`

  * Reveals each valid grid id fully (all 16×16 subcells).

### Pathfinding

* `FindShortestPath_BFS(int32 StartGlobalId, int32 GoalGlobalId, TArray<int32>& OutPath) const -> bool`

  * BFS over 4-neighborhood (up/down/left/right).
  * Traversal rules (implemented via `IsTraversable_Global`):

    * **Water** is always traversable.
    * **Land** is traversable only if **revealed**, except the **goal cell** must be **land** (even if unrevealed).

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) Expedition / world-map orchestration (consumer)

**Used by:** the expedition/world-map managers to compute preview routes, apply default reveals, and maintain fog/terrain masks. (This file includes `FCExpeditionManager.h` for the `LogFCWorldMap` category.)

**Why delegated:** keeps “grid math + traversal rules” in one focused helper while higher-level managers own textures, saving, UI, and gameplay flow.

### 2) Unreal containers + algorithms

* Uses `TQueue` for BFS frontier and `Algo::Reverse` for path reconstruction. 
* Uses `UE_LOG(LogFCWorldMap, ...)` for diagnostics. 

---

## Where to configure / extend (practical notes)

* **Grid sizing is fixed** by constants:

  * `GridSize = 16`, `SubSize = 16`, `GlobalSize = 256`, `GlobalCount = 65536`. 
* **Terrain comes from outside:** call `SetLandMask(...)` with a 65,536-byte array (usually derived from a mask texture). If you don’t set it, everything is treated as land.
* **Default reveals come from outside:** call `ApplyDefaultRevealedAreas_NewGame(DefaultGridIds)` with configured grid ids (invalid ids are ignored with warnings).
* **Changing traversal behavior:** edit `IsTraversable_Global(...)` (currently: water always ok; land requires reveal; goal must be land).
