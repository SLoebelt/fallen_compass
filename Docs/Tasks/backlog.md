# Fallen Compass - Known Issues & Future Backlog

> **Living Document**: This backlog tracks known bugs, technical debt, enhancements, and architectural decisions deferred to future sprints. Items are organized by priority and include detailed context for future implementation.

**Last Updated**: November 23, 2025

---

## Table of Contents

2. [Issue #2: Pause Menu Abort Button Visibility Logic](#issue-2-pause-menu-abort-button-visibility-logic)
4. [Issue #4: ESC Key Doesn't Close Pause Menu When Engine is Paused](#issue-4-esc-key-doesnt-close-pause-menu-when-engine-is-paused)

---

## Issue #2: Pause Menu Abort Button Visibility Logic

**Status**: 🔧 Enhancement - UX Improvement  
**Severity**: Low  
**Priority**: Low  
**Affected Systems**: Pause menu widget, UI conditional display  
**Discovered**: Week 3 (Task 7)  
**Target Sprint**: Week 4 (low priority polish)

### Problem Summary

"Abort Expedition" button in WBP_PauseMenu is visible in both Office and Overworld levels because it only checks game state (`Paused`). The button should only appear when pausing from Overworld, not when pausing in Office.

**Impact**: Minor UX inconsistency. User sees button in Office but clicking it just loads L_Office (no harmful effect, just confusing).

### Root Cause

Button visibility is based on game state alone (`Paused`), but doesn't check:

1. Which level is currently loaded (L_Office vs L_Overworld)
2. Which state was pushed onto the stack before pause (Office_Exploration vs Overworld_Travel)

**Current Logic**:

```
WBP_PauseMenu Event Construct:
- Get Game State
- If state == Paused: Show button
- (Always shows because we're always paused when menu opens)
```

### Expected Behavior

- ESC in L_Overworld → Pause menu shows "Resume" + "Abort Expedition" buttons
- ESC in L_Office → Pause menu shows "Resume" + "Settings" + "Quit" buttons (no abort)
- Button visibility should be contextual based on level/previous state

### Current Workaround

User sees Abort button in Office. Clicking it loads L_Office (same level), so no crash or data loss. Just confusing UX.

### Proposed Solutions

#### Option A: Check Current Level (Simple)

In WBP_PauseMenu Event Construct, check level name:

```
Event Construct
├─ Get Owning Player World
├─ Get Map Name
├─ Contains "L_Overworld"?
    ├─ True: Set AbortExpeditionButton Visibility = Visible
    └─ False: Set AbortExpeditionButton Visibility = Collapsed
```

**Pros**: Simple, 5 minutes to implement  
**Cons**: Hardcoded level name check

#### Option B: Check State Stack (Robust)

Check GameStateManager state stack to see previous state before pause:

```
Event Construct
├─ Get Game Instance
├─ Get GameStateManager Subsystem
├─ Get State At Depth (0) // Bottom of stack = pre-pause state
├─ Branch (State == Overworld_Travel)
    ├─ True: Show Abort button
    └─ False: Hide Abort button
```

**Pros**: Robust, doesn't rely on level names  
**Cons**: Requires understanding state stack (10-15 min implementation)

#### Option C: Separate Pause Menus (Advanced)

Create two pause menu widgets:

- `WBP_PauseMenu_Office`: Resume, Settings, Quit
- `WBP_PauseMenu_Overworld`: Resume, Abort Expedition

UIManager spawns correct widget based on level/state.

**Pros**: Clean separation, extensible  
**Cons**: More work, duplicate widget layouts

### Recommendation

**Option A** (Check Level) for quick fix in Week 4 polish sprint. Can refactor to Option B later if needed.

### Related Code

**Files to Modify**:

- `Content/FC/UI/Menus/WBP_PauseMenu.uasset` (Designer Tab + Event Graph)

**Files to Reference**:

- `Source/FC/Core/FCPlayerController.cpp::HandlePausePressed()` (lines ~353)
- `Source/FC/Core/FCGameStateManager.h` (GetStateAtDepth method)

### Testing Checklist

After implementing fix:

- [ ] ESC in Office → Abort button NOT visible ✓
- [ ] ESC in Overworld → Abort button IS visible ✓
- [ ] Resume button works in both levels ✓
- [ ] No layout issues (button collapse doesn't break spacing) ✓

---

## Issue #4: ESC Key Doesn't Close Pause Menu When Engine is Paused

**Status**: 🐛 Bug - Needs Fix  
**Severity**: Low  
**Priority**: Medium  
**Affected Systems**: Enhanced Input, Pause menu, Engine pause behavior  
**Discovered**: Week 3 (Task 8)  
**Target Sprint**: Week 4 or 5

### Problem Summary

When the pause menu is open in L_Overworld (with engine pause active via `SetPause(true)`), pressing ESC key a second time does not close the pause menu. This breaks the expected toggle behavior where ESC opens and closes the menu.

**Impact**: User must click "Resume" button instead of pressing ESC. Minor UX annoyance, not game-breaking.

### Root Cause

Engine pause (`SetPause(true)`) prevents Enhanced Input actions from firing. The IA_Escape input action is blocked when the game is paused, so `HandlePausePressed()` is never called to unpause.

**Why This Happens**:

```
1. User presses ESC in Overworld
2. HandlePausePressed() called (input active)
3. GameStateManager->PushState(Paused)
4. UIManager->ShowPauseMenu() called
5. ShowPauseMenu() detects L_Overworld, calls SetPause(true)
6. Engine pause activates, ALL input processing stops
7. User presses ESC again
8. Input system is paused, IA_Escape action never fires
9. HandlePausePressed() never called, menu stays open
```

**Why Office Works**:

- Office doesn't use engine pause (SetPause stays false)
- Enhanced Input continues processing
- ESC key toggles normally

### Expected Behavior

- Press ESC in Overworld → Pause menu opens, engine pauses
- Press ESC again → Pause menu closes, engine unpauses
- Consistent toggle behavior like in Office (where engine pause is not used)

### Current Workaround

Use "Resume" button instead of ESC key to close menu. Button works because it's UMG widget input, not Enhanced Input action.

### Proposed Solutions

#### Option A: Widget OnKeyDown Event (Quick Fix - Recommended)

WBP_PauseMenu captures ESC key directly via UMG input system:

**Implementation (Blueprint)**:

```
WBP_PauseMenu Widget Blueprint:
├─ Override: OnKeyDown(MyGeometry, InKeyEvent)
│  ├─ Get Key from InKeyEvent
│  ├─ Branch: Key == Escape?
│  │  ├─ True:
│  │  │  ├─ Get Owning Player
│  │  │  ├─ Cast to AFCPlayerController
│  │  │  ├─ Call ResumeGame()
│  │  │  └─ Return: Handled
│  │  └─ False: Return: Unhandled
```

**Pros**:

- Simple, 10-15 minutes to implement
- Works immediately
- No C++ changes needed

**Cons**:

- Duplicates input handling logic (ESC handled in two places)
- Couples widget to specific input key
- Less maintainable

#### Option B: FInputModeUIOnly + Manual Movement Stop (Robust)

Replace engine pause with input mode change + manual convoy disable:

**Implementation (C++ UFCUIManager)**:

```cpp
void UFCUIManager::ShowPauseMenu()
{
    // ... widget creation ...

    if (bIsOverworld)
    {
        // Don't use SetPause(true) - use input mode instead
        PC->SetInputMode(FInputModeUIOnly()); // Blocks gameplay input

        // Manually stop convoy AI
        ABP_OverworldConvoy* Convoy = FindConvoyInLevel();
        if (Convoy)
        {
            Convoy->StopMovement();
            Convoy->SetActorTickEnabled(false); // Pause AI tick
        }
    }
}
```

**Pros**:

- Enhanced Input continues working (ESC toggles properly)
- More control over what's paused vs active
- Cleaner architecture

**Cons**:

- More code (manual convoy management)
- 1-2 hours to implement and test
- Need reference to convoy actor

#### Option C: PlayerController Input While Paused (Experimental)

Configure PlayerController to accept input while paused:

```cpp
AFCPlayerController::AFCPlayerController()
{
    bShouldPerformFullTickWhenPaused = true; // Allow input processing when paused
}
```

**Pros**:

- Minimal code change
- Fixes ESC toggle immediately

**Cons**:

- May have unintended side effects (other systems might expect no input during pause)
- Needs extensive testing
- Affects all input, not just ESC

### Recommendation

**Option A** (Widget OnKeyDown) for **quick fix in Week 4**. Implements in Blueprint, no C++ rebuild needed, works immediately.

**Option B** (Input Mode + Manual Stop) for **proper fix in Week 5**. Better architecture, more maintainable, but requires more time.

### Related Code

**Option A Implementation (Blueprint)**:

- `Content/FC/UI/Menus/WBP_PauseMenu.uasset` (Override OnKeyDown function)
- `Source/FC/Core/FCPlayerController.h` (ResumeGame method already exists)

**Option B Implementation (C++)**:

- `Source/FC/Core/UFCUIManager.cpp::ShowPauseMenu()` (line ~110)
- `Source/FC/Core/UFCUIManager.cpp::HidePauseMenu()` (line ~147)
- `Content/FC/Overworld/BP_OverworldConvoy.uasset` (add StopMovement method)

**Option C Implementation (C++)**:

- `Source/FC/Core/FCPlayerController.cpp` (constructor)

### Testing Checklist

After implementing fix:

- [ ] ESC in Overworld → Menu opens ✓
- [ ] ESC again → Menu closes (NEW - should work) ✓
- [ ] Resume button still works ✓
- [ ] Convoy stops when paused ✓
- [ ] Convoy resumes when unpaused ✓
- [ ] ESC in Office still works (regression check) ✓
- [ ] Round trip ESC toggle 5x in Overworld ✓
- [ ] No "Accessed None" errors ✓

### Trade-off Analysis

| Criterion               | Option A               | Option B                  | Option C                 |
| ----------------------- | ---------------------- | ------------------------- | ------------------------ |
| **Implementation Time** | 15 min                 | 1-2 hours                 | 5 min                    |
| **Code Quality**        | Fair (duplicate logic) | Good (clean architecture) | Fair (side effects)      |
| **Maintainability**     | Low                    | High                      | Low                      |
| **Risk**                | Low                    | Low                       | Medium (unknown effects) |
| **Recommended For**     | Quick fix              | Long-term solution        | Experimental only        |

---

## Enhancement #5: Dynamic Texture-Based Fog of War System

**Status**: 💡 Enhancement - Future Feature  
**Severity**: N/A (Quality of Life)  
**Priority**: Low (Post-Week 4)  
**Affected Systems**: World Map, UFCWorldMapManager, Rendering Pipeline  
**Proposed For**: Week 6+ or Polish Phase  
**Dependencies**: Task 4.2 (Basic Map System with Sub-Grid)

### Enhancement Summary

Upgrade the Week 4 sub-grid fog of war system (12×12 boolean array) to a dynamic texture-based rendering approach for smooth, high-resolution fog unveiling. This will improve visual polish and enable real-time fog transitions, cinematic effects, and higher fidelity exploration tracking.

### Current Implementation (Week 4)

The Week 4 map system uses a **12×12 sub-grid** per 2km×2km area:

- **Data Structure**: `TArray<bool> ExploredSubCells` (144 entries per area) in `FFCMapAreaData`
- **Cell Size**: ~166m × 166m per sub-cell
- **Update Frequency**: Every frame via `UFCWorldMapManager->UpdateExplorationAtWorldPosition()`
- **Rendering**: Either Uniform Grid Panel (144 widgets) or Material Instance Dynamic with bool array parameter
- **Pros**: Simple, predictable performance, easy to debug, sufficient for Week 4 scope
- **Cons**: Blocky unveiling, limited visual fidelity, 144-cell granularity may show "stepping" at convoy edges

### Proposed Enhancement: Dynamic Texture Rendering

Replace boolean sub-grid with **512×512 grayscale texture** per area for smooth fog transitions:

#### Architecture Changes

1. **Data Structure**

   - Replace `TArray<bool> ExploredSubCells` with `UTexture2D* FogOfWarTexture` in `FFCMapAreaData`
   - Each pixel represents ~3.9m × 3.9m of world space (2000m / 512px)
   - Grayscale values: 0 = unexplored (black), 255 = fully explored (white)
   - Intermediate values (1-254) support smooth transitions and "partial visibility" zones

2. **UFCWorldMapManager Enhancements**

   - **Method**: `UpdateFogTextureAtWorldPosition(FVector WorldPosition, float RevealRadius)`
     - Convert world position to texture UV coordinates
     - Draw radial gradient (e.g., 50m radius = ~13 pixels) centered on convoy
     - Use CPU-side pixel manipulation (`FUpdateTextureRegion2D`) or RenderTarget approach
     - Lock/unlock texture for write access per frame or batch updates
   - **Method**: `GetFogTextureForArea(FName AreaID)` → returns `UTexture2D*` for widget binding
   - **Optimization**: Update only dirty regions (convoy bounding box + reveal radius), not entire 512×512 texture

3. **Rendering Pipeline**

   - **Material**: Create `M_FogOfWar` material with texture parameter (`FogTexture`)
     - Sample texture at world UV coordinates
     - Lerp between fog color (dark gray/black) and map color (full saturation) based on pixel value
     - Support animated "wave" effect for newly unveiled areas (optional shader animation)
   - **Widget Integration**: `WBP_WorldMap` uses single Image widget per area with Material Instance Dynamic
     - Bind `FogTexture` parameter to `GetFogTextureForArea(AreaID)`
     - Automatic refresh when texture updates (no manual widget iteration)

4. **Convoy Integration**
   - In convoy pawn's `Tick()`:
     ```cpp
     WorldMapManager->UpdateFogTextureAtWorldPosition(GetActorLocation(), RevealRadius = 50.0f);
     ```
   - **Reveal Radius**: Configurable (e.g., 50m, 100m) for fog "brush size"
   - **Fade-In**: Optional smooth transition over 1-2 seconds for cinematic effect
   - **Persistence**: Serialize texture to save file (use PNG compression or RLE encoding for ~100KB per area)

#### Visual Features Enabled

- **Smooth Gradients**: Fog recedes smoothly around convoy path, no blocky edges
- **Partial Visibility**: "Fog of exploration" vs "fog of war" distinction (e.g., once-seen areas are lighter gray, not black)
- **Animated Transitions**: Shader-based wave/ripple effect when fog clears
- **High Fidelity**: 512×512 = 262,144 "cells" vs 144 sub-cells (1,820× resolution increase)
- **Cinematic Moments**: Support for scripted fog reveals (e.g., cutscene flyover unveils entire region)

#### Performance Considerations

**GPU Cost**:

- Texture sampling per pixel: Negligible (single grayscale texture lookup)
- Material complexity: Low (1 texture sample + lerp operation)
- No performance difference between 512×512 and 12×12 for rendering (both are single material instances)

**CPU Cost**:

- **Texture Update**: ~5-15ms per frame if updating entire 512×512 texture (262KB data)
- **Optimization A**: Update only dirty 64×64 region around convoy → ~0.5ms
- **Optimization B**: Batch updates every 5-10 frames → amortize cost
- **Optimization C**: Use RenderTarget + GPU compute for convoy "brush" drawing → <1ms

**Memory Cost**:

- 512×512 grayscale = 256KB per area (uncompressed)
- 10 areas = 2.56MB total (negligible for modern GPUs)
- Save file: PNG compression reduces to ~50-100KB per area

**Trade-Off**: 3-5× CPU cost vs sub-grid, but still well within frame budget (<1ms optimized)

#### Implementation Roadmap

**Phase 1: Core Texture System** (2-3 days)

- [ ] Extend `FFCMapAreaData` with `UTexture2D* FogOfWarTexture` property
- [ ] Implement `UFCWorldMapManager::InitializeFogTextures()` – create blank 512×512 textures
- [ ] Implement `UpdateFogTextureAtWorldPosition()` with CPU-side pixel writes
- [ ] Test texture updates in isolation (draw radial gradient at convoy position)

**Phase 2: Material & Rendering** (1 day)

- [ ] Create `M_FogOfWar` material (texture sample + lerp)
- [ ] Update `WBP_WorldMap` to use Material Instance Dynamic per area
- [ ] Bind `FogTexture` parameter to area data
- [ ] Verify fog renders correctly at rest (static texture test)

**Phase 3: Convoy Integration** (1 day)

- [ ] Hook convoy `Tick()` to call `UpdateFogTextureAtWorldPosition()`
- [ ] Tune reveal radius (50m, 100m, etc.)
- [ ] Test fog unveiling during convoy movement
- [ ] Verify save/load persistence (serialize texture to PNG)

**Phase 4: Polish & Optimization** (1-2 days)

- [ ] Implement dirty region optimization (64×64 bounding box updates)
- [ ] Add optional fade-in shader animation (wave effect)
- [ ] Profile CPU cost, ensure <1ms per frame
- [ ] Add debug visualization (show texture UV overlay, dirty regions)

**Phase 5: Testing & Edge Cases** (1 day)

- [ ] Test with 10+ areas (memory/performance)
- [ ] Test save/load with partially unveiled areas
- [ ] Test convoy teleportation (large position deltas)
- [ ] Test rapid back-and-forth across area boundaries

**Total Estimated Time**: 5-8 days (assuming one engineer)

#### Why Defer to Post-Week 4?

1. **Scope Management**: Week 4 focuses on functional expedition loop, not visual polish
2. **Risk Mitigation**: Sub-grid approach is proven and simple; texture system requires rendering pipeline knowledge
3. **Dependencies**: Requires stable convoy movement and save system (both in flux during Week 4)
4. **Player Testing**: Week 4 sub-grid validates fog of war gameplay before committing to texture overhaul
5. **Incremental Quality**: Sub-grid is "good enough" for MVP; texture is "excellent" for final release

#### Success Criteria

- [ ] Fog unveils smoothly around convoy with no visible "steps" or blocky edges
- [ ] Texture updates cost <1ms per frame (profiled via Unreal Insights)
- [ ] Save/load preserves fog state with <100KB overhead per area
- [ ] Visual quality matches AAA standards (e.g., Civilization VI, Total War series)
- [ ] No regressions: Sub-grid functionality remains as fallback option (config toggle)

#### Notes

- **Fallback Strategy**: If performance issues arise, revert to sub-grid or use hybrid (low-res texture: 128×128)
- **Art Direction**: Coordinate with art team on fog color, transition speed, and "wave" animation style
- **Accessibility**: Ensure high contrast between explored/unexplored for colorblind players (use luminance, not hue)

---

## Priority Summary

### High Priority (Week 4)

1. **Issue #1**: Office State Reset - Medium severity, blocks gameplay
   - Estimated: 2-3 hours (C++ implementation + testing)

### Medium Priority (Week 4-5)

2. **Issue #4**: ESC Toggle in Overworld - Low severity, UX annoyance
   - Estimated: 15 min (Option A) or 1-2 hours (Option B)

### Low Priority (Week 5+)

3. **Issue #2**: Abort Button Visibility - Low severity, minor UX inconsistency

   - Estimated: 15 min (simple Blueprint change)

4. **Issue #3**: Level Loading Order - Low severity, documentation task
   - Estimated: 30 min (standardize existing widgets)

### Future Enhancements (Week 6+ / Polish Phase)

5. **Enhancement #5**: Dynamic Texture-Based Fog of War - High visual impact, low urgency
   - Estimated: 5-8 days (full implementation + testing)
   - Requires: Task 4.2 completion (sub-grid baseline)
   - Benefits: Smooth fog transitions, 1,820× resolution increase, cinematic effects

---

## Document History

| Date       | Change                                                 | Author       |
| ---------- | ------------------------------------------------------ | ------------ |
| 2025-11-23 | Initial backlog extraction from 0003-tasks.md          | AI Assistant |
| 2025-11-26 | Added Enhancement #5: Dynamic Texture-Based Fog of War | AI Assistant |

---

**END OF BACKLOG.MD**
