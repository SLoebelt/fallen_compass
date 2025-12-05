## UFCTransitionManager — Technical Documentation (Manager)

### Where to find it (paths)

* **Header:** `Core/FCTransitionManager.h` 
* **Source:** `Core/FCTransitionManager.cpp` 

---

## Responsibilities (what this manager owns)

**`UFCTransitionManager`** is a **`UGameInstanceSubsystem`** that owns the game’s **screen transition layer** (fade to/from black, optional loading indicator) and keeps it **persistent across level loads**.

Core responsibilities:

1. **Persistent transition overlay**

   * Creates a single `UFCScreenTransitionWidget` instance (once) using the **GameInstance as outer** so it survives controller/lvl changes.
   * Adds it via `UGameViewportClient::AddViewportWidgetContent` at very high Z (1000) to always stay on top. 

2. **Unified fade control**

   * Exposes `BeginFadeOut` / `BeginFadeIn` to Blueprint.
   * Prevents overlapping fades with `bCurrentlyFading`.

3. **Startup cleanliness**

   * On initialize, it creates the widget early and tries to start from **black** (avoids showing the world before the first fade). If the viewport isn’t ready, it retries via a timer and then auto-fade-ins. 

4. **Transition completion signaling**

   * Broadcasts `OnFadeOutComplete` / `OnFadeInComplete` when widget animations finish.

5. **Level tracking helper**

   * Tracks `CurrentLevelName` and provides `IsSameLevelLoad` to detect loads to the currently active map.

---

## Public API (Blueprint-facing)

### Fades / screen state

* `BeginFadeOut(float Duration = 1.0f, bool bShowLoadingIndicator = false)`

  * Starts fade to black (optionally shows loading spinner/indicator).
* `BeginFadeIn(float Duration = 1.0f)`

  * Starts fade from black to clear.
* `IsFading() -> bool`

  * True while a fade is in progress (guarded by `bCurrentlyFading`).
* `IsBlack() -> bool`

  * True if the transition widget exists and reports fully black.

### Level-load helpers

* `IsSameLevelLoad(const FName& TargetLevelName) -> bool`

  * Compares `TargetLevelName` with internally tracked `CurrentLevelName`.
* `UpdateCurrentLevel(const FName& NewLevelName)`

  * Updates `CurrentLevelName` (intended to be called after a successful load).

### Events (delegates)

* `OnFadeOutComplete` (BlueprintAssignable) 
* `OnFadeInComplete` (BlueprintAssignable) 

---

## Connected systems (“connected managers”) and what/why is delegated

### 1) `UFCGameInstance` (configuration + timers)

**What/Why**

* Pulls the transition widget class from GameInstance configuration (`TransitionWidgetClass` is a soft class ref).
* Uses the GameInstance timer manager to retry viewport insertion and to trigger the initial auto fade-in. 

---

### 2) `UFCScreenTransitionWidget` (actual visuals + animation completion)

**What/Why**

* The widget implements the real transition behavior: `BeginFadeOut`, `BeginFadeIn`, `InitializeToBlack`, and state query `IsBlack()`.
* The manager binds to widget delegates (`OnFadeOutComplete`, `OnFadeInComplete`) and re-broadcasts them in a manager-level API.

---

### 3) `UGameViewportClient` / Slate (persistence across level loads)

**What/Why**

* Adds the transition widget using `AddViewportWidgetContent(...)` rather than typical `AddToViewport()` so it remains present even when player controllers are destroyed during travel. 
* On shutdown, removes it using `RemoveViewportWidgetContent(...)` with the widget’s cached Slate widget. 

---

## Where to configure / extend (practical notes)

* **Configure widget class** in `BP_FCGameInstance` (the code logs a warning if not set).
* If you need different transition “styles” (e.g., different overlays per mode), you typically extend/replace **`UFCScreenTransitionWidget`** and switch the configured soft class reference.