# Task Template Schema for LLM-Driven Implementation

> **Purpose**: Standardized task structure for systematic feature implementation with strict compliance checks and step-by-step verification.  
> **Usage**: Copy this template for each major feature or sprint, customize numbered sections, and follow the structure exactly.

---

## Task Metadata

```yaml
Task ID: [XXXX] # e.g., 0008
Sprint/Phase: [Week X / Phase Y]
Feature Name: [Brief descriptive name]
Dependencies: [List prior task IDs this depends on]
Estimated Complexity: [Low/Medium/High]
Primary Files Affected: [List key files/directories]
```

---

## Overview & Context

### Purpose

[1-2 sentence description of what this task accomplishes and why it's needed]

### Architecture Notes

[Key architectural decisions, design patterns, or approaches to follow from GDD/DRM]

### Reference Documents

- `/Docs/Fallen_Compass_GDD_v0.2.md` - [Relevant sections]
- `/Docs/Fallen_Compass_DRM.md` - [Relevant phase/week]
- `/Docs/UE_CodeConventions.md` - [Relevant conventions]
- `/Docs/UE_NamingConventions.md` - [Relevant naming rules]
- `Docs/TechnicalDocumentation/FCRuntime.md` and linked files - [Relevant technical specs]

---

## Pre-Implementation Phase

### Step X.0: Analysis & Discovery

- [ ] **Analysis of Existing Implementations**

  - [ ] Read `Docs/TechnicalDocumentation/FCRuntime.md` and linked files sections: [list specific sections]
  - [ ] Check project file system for existing classes/Blueprints: [list paths to check]
  - [ ] Identify existing patterns to follow (e.g., subsystem initialization, widget management)
  - [ ] Document what already exists and what needs to be created:

    ```
    Existing:
    - [Class/Blueprint/Asset name] at [path]

    To Create:
    - [Class/Blueprint/Asset name] at [path]
    ```

- [ ] **Code Conventions Compliance Check**

  - [ ] Review `/Docs/UE_CodeConventions.md` for applicable rules:
    - [ ] **Encapsulation (§2.1)**: Private members with public accessors planned?
    - [ ] **Modular Organization (§2.2)**: Code organized in proper module folders?
    - [ ] **Blueprint Exposure (§2.3)**: Only necessary methods marked BlueprintCallable?
    - [ ] **Memory Management (§2.5)**: All UObject\* pointers will use UPROPERTY()?
    - [ ] **Event-Driven Design (§2.6)**: Avoiding Tick usage, using timers/delegates?
    - [ ] **Interface Usage (§3.1)**: Using C++ BlueprintNativeEvent interfaces for extensibility?
    - [ ] **Separation of Concerns (§4.3)**: Proper delegation (e.g., Controller → UIManager)?

- [ ] **Naming Conventions Compliance Check**

  - [ ] Review `/Docs/UE_NamingConventions.md` for applicable rules:
    - [ ] C++ classes use `FC` prefix (A for Actor, U for UObject, F for struct)?
    - [ ] Blueprints use `BP_` prefix (WBP\_ for widgets)?
    - [ ] Files organized in `/Source/FC/[Module]/` and `/Content/FC/[Category]/`?
    - [ ] Input actions use `IA_` prefix, mapping contexts use `IMC_` prefix?

- [ ] **Implementation Plan**
  - [ ] List all classes/Blueprints to create with correct naming
  - [ ] List all properties/methods to add with visibility (public/protected/private)
  - [ ] Identify integration points with existing systems
  - [ ] Document expected call flow (e.g., User Input → Controller → Subsystem → Widget)

---

## Implementation Phase

### Step X.1: [First Major Component - e.g., "Create C++ Base Classes"]

#### Step X.1.1: [Specific Implementation Step]

- [ ] **Analysis**

  - [ ] What implementations already exist: Check technical documentation and project file system
  - [ ] What patterns to follow: [Reference existing similar implementations]

- [ ] **Implementation ([Filename.h])**

  - [ ] Create/modify file at: [exact path]
  - [ ] Add includes (forward declarations in .h, full includes in .cpp):
    ```cpp
    // Example:
    #pragma once
    #include "CoreMinimal.h"
    #include "[BaseClass].h"
    #include "[ThisClass].generated.h"
    ```
  - [ ] Define class with correct prefix and inheritance:
    ```cpp
    UCLASS([Specifiers])
    class FC_API [UClassName] : public [UBaseClass]
    {
        GENERATED_BODY()
    public:
        // Public interface
    protected:
        // Protected implementation
    private:
        // Private data members
    };
    ```
  - [ ] Add properties with UPROPERTY() for GC tracking:
    ```cpp
    UPROPERTY([Specifiers])
    [Type] [PropertyName];
    ```
  - [ ] Add methods with appropriate specifiers:
    ```cpp
    UFUNCTION(BlueprintCallable, Category = "[Category]")
    void [MethodName]([Parameters]);
    ```

- [ ] **Implementation ([Filename.cpp])**

  - [ ] Implement constructor with CDO initialization
  - [ ] Implement all declared methods
  - [ ] Add logging statements using custom log category:
    ```cpp
    UE_LOG(Log[ProjectPrefix][ClassName], [Verbosity], TEXT("[Message]"), [Args]);
    ```
  - [ ] Compile successfully (check for errors)

- [ ] **Blueprint Configuration (if applicable)**

  - [ ] Create Blueprint at: `/Game/FC/[Category]/BP_[Name]`
  - [ ] Set parent class to C++ class
  - [ ] Configure properties in Class Defaults
  - [ ] Implement Blueprint-exposed events/functions if needed
  - [ ] Compile Blueprint

- [ ] **Testing After Step X.1.1** ✅ CHECKPOINT
  - [ ] Compile succeeds without errors
  - [ ] PIE starts successfully without crashes
  - [ ] [Specific test case 1]
  - [ ] [Specific test case 2]
  - [ ] No errors or warnings in Output Log
  - [ ] No "Accessed None" Blueprint errors

**COMMIT POINT X.1.1**: `git add -A && git commit -m "feat([scope]): [description]"`

---

#### Step X.1.2: [Next Specific Implementation Step]

[Repeat structure from X.1.1 with new analysis/implementation/testing]

---

### Step X.2: [Second Major Component - e.g., "Implement Widget System"]

[Repeat structure from X.1 with appropriate steps]

---

### Step X.3: [Third Major Component - e.g., "Wire Up Input/Interaction"]

[Repeat structure from X.1 with appropriate steps]

---

## Verification & Testing Phase

### Step X.Y: Comprehensive Functional Testing

#### Step X.Y.1: [Feature Flow Name] Verification

- [ ] **Analysis**

  - [ ] What implementations already exist: Check technical documentation and project file system
  - [ ] What test cases to cover: [List based on feature requirements]

- [ ] **Test Sequence**
  - [ ] [Test step 1]: Expected behavior: [description]
  - [ ] [Test step 2]: Expected behavior: [description]
  - [ ] [Test step 3]: Expected behavior: [description]
  - [ ] Verify no crashes, errors, or Blueprint warnings in Output Log
  - [ ] Document actual behavior vs. expected behavior

#### Step X.Y.2: Edge Cases & Regression Testing

- [ ] **Test Sequence**
  - [ ] [Edge case 1]: [description] → Expected: [behavior]
  - [ ] [Edge case 2]: [description] → Expected: [behavior]
  - [ ] [Regression test 1]: [description] → Expected: [behavior]
  - [ ] Verify state persistence across level transitions
  - [ ] Verify no memory leaks or degradation after multiple cycles
  - [ ] Check for "Accessed None" errors throughout all tests

**FINDINGS DOCUMENTATION**: Record any discovered issues in "Known Issues & Backlog" section

---

### Step X.Z: Technical Architecture Verification

#### Step X.Z.1: Core Framework Integration Check

- [ ] **Check Project Settings**

  - [ ] [Setting 1]: [Expected value] ✅
  - [ ] [Setting 2]: [Expected value] ✅

- [ ] **Verify Subsystem/Component Initialization**
  - [ ] Launch PIE, check Output Log for:
    - [ ] `[LogCategory]: [Expected initialization message]` ✅
    - [ ] No "Failed to initialize" errors

#### Step X.Z.2: Code Conventions Compliance Audit

- [ ] **UE_CodeConventions.md Compliance Check**
  - [ ] **Encapsulation (§2.1)**:
    - [ ] All data members private with public accessors ✅
    - [ ] No public member variables except UPROPERTY BlueprintReadWrite where needed ✅
  - [ ] **Modular Organization (§2.2)**:
    - [ ] Code organized in `/Source/FC/[Module]` modules ✅
    - [ ] Forward declarations used in headers ✅
    - [ ] Full includes only in .cpp files ✅
  - [ ] **Blueprint Exposure (§2.3)**:
    - [ ] Only necessary methods exposed as BlueprintCallable ✅
    - [ ] Proper delegation (e.g., Controller → Subsystem → Widget) ✅
  - [ ] **Memory Management (§2.5)**:
    - [ ] All `UObject*` pointers use `UPROPERTY()` for GC tracking ✅
    - [ ] Widget outer set correctly (GameInstance for persistent, Controller for temporary) ✅
    - [ ] No dangling pointers or memory leaks ✅
  - [ ] **Event-Driven Design (§2.6)**:
    - [ ] No excessive Tick usage ✅
    - [ ] Timers used for delays ✅
    - [ ] Delegates used for async operations ✅
  - [ ] **Interface Usage (§3.1)** (if applicable):
    - [ ] C++ BlueprintNativeEvent interfaces used for extensibility ✅
    - [ ] No hard casting to specific implementations ✅
  - [ ] **Separation of Concerns (§4.3)**:
    - [ ] [Component 1] delegates to [Component 2] for [responsibility] ✅
    - [ ] Clear separation of UI/logic/data layers ✅

#### Step X.Z.3: Output Log Review

- [ ] **Log Analysis**
  - [ ] Launch PIE, complete full playthrough of feature
  - [ ] Review Output Log for:
    - [ ] No `Error:` messages (except intentional test cases) ✅
    - [ ] No `Warning:` messages for missing assets or null pointers ✅
    - [ ] Expected logs present:
      - [ ] [Log message 1] ✅
      - [ ] [Log message 2] ✅
      - [ ] [Log message 3] ✅
  - [ ] Document any unexpected warnings for investigation:
    - [ ] [Warning description]: [Context/root cause/priority]

---

### Step X.W: Naming Conventions Compliance

#### Step X.W.1: C++ Class Naming Verification

- [ ] **Check Against UE_NamingConventions.md**
  - [ ] All classes use `FC` prefix ✅
  - [ ] Classes created:
    - [ ] `[AClassName]` (A prefix for Actor-derived) ✅
    - [ ] `[UClassName]` (U prefix for UObject-derived) ✅
    - [ ] `[FStructName]` (F prefix for structs) ✅

#### Step X.W.2: Blueprint Naming Verification

- [ ] **Check Against UE_NamingConventions.md**
  - [ ] Blueprints use `BP_` prefix (or `WBP_` for widgets) ✅
  - [ ] Blueprints created:
    - [ ] `BP_[Name]` at `/Game/FC/[Category]/` ✅
    - [ ] `WBP_[WidgetName]` at `/Game/FC/UI/` ✅

#### Step X.W.3: Folder Structure Verification

- [ ] **Check Against UE_NamingConventions.md**
  - [ ] Source code structure:
    - [ ] `/Source/FC/[Module]/` (appropriate module folder) ✅
  - [ ] Content structure:
    - [ ] `/Content/FC/[Category]/` (appropriate category folder) ✅
  - [ ] No loose files in inappropriate locations ✅

---

### Step X.V: Git Repository Hygiene

#### Step X.V.1: Working Tree Cleanup

- [ ] **Verify Clean State**

  - [ ] From repo root, run: `cd [RepoPath]; git status`
  - [ ] Check for untracked files:
    - [ ] No loose `.uasset` or `.umap` files in unexpected locations ✅
    - [ ] No editor temporary files (`.tmp`, `.log` outside `/Saved`) ✅
    - [ ] No build artifacts outside `/Binaries`, `/Intermediate` ✅
  - [ ] Check modified files:
    - [ ] Only intentional changes staged ✅
    - [ ] No accidental modifications to engine content ✅

- [ ] **Stage All Task Changes**

  - [ ] Run: `git add Source/ Content/ Config/ Docs/`
  - [ ] Run: `git status` → verify all intended changes staged
  - [ ] Commit: `git commit -m "feat([scope]): [detailed description]"`

- [ ] **Create Milestone Tag (if applicable)**

  - [ ] Run: `git tag -a milestone-[name] -m "[Description of milestone]"`
  - [ ] Run: `git push origin master --tags`

- [ ] **Update Documentation**
  - [ ] Update `Docs/TechnicalDocumentation/FCRuntime.md` and linked files with new systems/classes
  - [ ] Update `/Docs/Fallen_Compass_DRM.md` to mark task/week complete
  - [ ] Commit: `git commit -am "docs: Update technical docs and roadmap for [task name]"`

---

## Known Issues & Backlog

> Document any bugs, limitations, or technical debt discovered during implementation.

### [Category 1 - e.g., "Visual Polish"]

- **Issue**: [Description]
  - **Context**: [When it occurs, reproduction steps]
  - **Priority**: [Low/Medium/High]
  - **Proposed Solution**: [Ideas for fixing]

### [Category 2 - e.g., "Performance"]

- **Issue**: [Description]
  - **Context**: [When it occurs, reproduction steps]
  - **Priority**: [Low/Medium/High]
  - **Proposed Solution**: [Ideas for fixing]

---

## Task Complete ✅

**Acceptance Criteria Met**: [List key deliverables verified]

- [ ] [Deliverable 1]
- [ ] [Deliverable 2]
- [ ] [Deliverable 3]

**Next Steps**: [Reference next task ID or phase]

---

## LLM Instructions for Using This Template

### Mandatory Workflow

1. **Read Before Writing**: Always check `Docs/TechnicalDocumentation/FCRuntime.md` and linked files before creating new code
2. **Compliance First**: Verify code/naming conventions before any implementation step
3. **Compile and Test Immediately**: Let the user compile and manually test checkpoints after EVERY implementation step (X.Y.Z format)
4. **Document Findings**: Record all warnings/errors in "Known Issues" section as discovered
5. **Commit Frequently**: Use commit points after each completed step

### Strict Rules

- ❌ **NEVER** skip the Analysis section of any step
- ❌ **NEVER** mark a test checkpoint complete without actually running PIE
- ❌ **NEVER** create code without verifying naming conventions first
- ❌ **NEVER** merge multiple steps into one commit
- ❌ **NEVER** compile on your own
- ❌ **NEVER** edit other files than the ones mentioned in the task checklist wihtout asking the user
- ✅ **ALWAYS** check for "Accessed None" errors in Output Log
- ✅ **ALWAYS** verify UPROPERTY() on all UObject\* pointers
- ✅ **ALWAYS** use forward declarations in .h files
- ✅ **ALWAYS** log state changes with custom log categories

### Response Format

When executing a step:

1. Quote the step number and name
2. Perform analysis (if applicable)
3. Execute implementation
4. Run tests and report results
5. Request user confirmation before proceeding to next step

Example:

```
Executing Step X.1.1: Create Base Character Class

Analysis:
- Checked `Docs/TechnicalDocumentation/FCRuntime.md` and linked files §3.2 - No existing character class found
- Convention check: Will use AFCCharacterName (Actor prefix)
- Will create at /Source/FC/Characters/FCCharacterName.h

Implementation:
[Tool calls for file creation]

Testing:
- Compile by User: ✅ Success
- PIE: ✅ No crashes and implemented Features working
- Output Log: ✅ No errors

Ready to proceed to Step X.1.2?
```
