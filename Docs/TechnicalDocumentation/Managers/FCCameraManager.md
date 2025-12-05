## UFCCameraManager — Technical Documentation (Manager-style)

### Where to find it (paths)

* **Header:** `Components/FCCameraManager.h` 
* **Source:** `Components/FCCameraManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCCameraManager`** is a **PlayerController-attached ActorComponent** responsible for **camera mode transitions** and **view-target management**. It encapsulates blending logic, temporary camera spawning, and restoring previous views.

It owns:

1. **Camera mode state**

   * Tracks `CurrentCameraMode`, `PreviousCameraMode`, and whether a blend is active (`bIsTransitioning`).

2. **Original view target capture**

   * On BeginPlay, stores an `OriginalViewTarget` (prefers pawn; falls back to current view target), so “return to first-person” can reliably restore the correct actor.

3. **Unified blend API**

   * Provides explicit entry points: menu, first-person, table-object view, top-down overworld view, POI/camp view.

4. **Temporary camera management**

   * Spawns/destroys a transient `ACameraActor` for table views and cleans up safely after transitions.
   * Maintains `PreviousTableViewCamera` so switching between table objects can restore the prior table camera when closing a widget.

5. **Overworld integration via discovery/reflection**

   * Locates an “OverworldCamera” actor by name, attaches it to the convoy’s `GetCameraAttachPoint()` (called via reflection), and calls `SetPlayerPawn(APawn*)` on the camera (also via reflection) for distance limiting. 

---

## Public API (Blueprint-facing)

### Queries

* `GetCameraMode() -> EFCPlayerCameraMode` 
* `GetOriginalViewTarget() -> AActor*` 
* `IsInTransition() -> bool` 

### Camera transition API

* `BlendToMenuCamera(float BlendTime = -1.0f)`
* `BlendToFirstPerson(float BlendTime = -1.0f)`
* `BlendToTableObject(AActor* TableObject, float BlendTime = -1.0f)`

  * Finds a component containing `"CameraTarget"` on the target actor; falls back to root component. Spawns a temp camera at that transform and blends to it.
* `BlendToTopDown(float BlendTime = -1.0f)`

  * Finds the overworld camera by name containing `"OverworldCamera"`, optionally attaches it to the overworld convoy attach point, then blends to it.
* `BlendToPOISceneCamera(ACameraActor* POICamera, float BlendTime = -1.0f)`
* `RestorePreviousViewTarget(float BlendTime = -1.0f)`

  * Restores based on `PreviousCameraMode` (FirstPerson or MainMenu; otherwise defaults to FirstPerson).
* `RestorePreviousTableCamera(float BlendTime = -1.0f)`

  * Blends back to `PreviousTableViewCamera` if present; otherwise returns to FirstPerson.

### Configuration

* `SetMenuCamera(ACameraActor* InMenuCamera)`

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) Owning `APlayerController`

**Delegated:** executing the blend via `SetViewTargetWithBlend(...)`.
**Why:** the PlayerController is the canonical owner of the view target; the component simply standardizes how targets are chosen and blended.

### 2) Level actors (camera actors, convoy actor)

**Delegated:** world-provided camera actors and convoy attach points.

* Overworld camera is found via actor name search and expected to exist in the map (e.g., “BP_OverworldCamera” style). 
* Convoy is found by class name containing `"OverworldConvoy"` and is expected (for full behavior) to provide:

  * a `GetCameraAttachPoint()` function
  * the camera actor to implement `SetPlayerPawn(APawn*)` 

**Why:** keeps camera manager generic and avoids hard compile-time dependencies on Blueprint classes (uses reflection instead).

### 3) Engine timer system

**Delegated:** scheduling cleanup and “transition complete” flag reset after a blend.
**Why:** blend completion is time-based; timers provide a simple, deterministic lifecycle for transient cameras. 

---

## Where to configure / extend (practical notes)

* **Menu camera** should be set per-instance in Blueprint (`MenuCamera` property) or via `SetMenuCamera(...)`.
* **Table objects** should include a component whose name contains `"CameraTarget"`; otherwise the system falls back to the root component transform. 
* **Overworld requirements:** place an overworld camera actor with `"OverworldCamera"` in its name in the overworld map; optionally provide an overworld convoy actor that exposes `GetCameraAttachPoint()`. 
