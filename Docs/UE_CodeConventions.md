# Unreal Engine C++ & Blueprint Best Practices

_For scalable, modular, loosely coupled projects_

---

## 1. Overview

This document summarizes conventions and patterns for building Unreal Engine projects that are:

- **Scalable** – easy to extend as features grow
- **Modular** – systems are clearly separated
- **Loosely coupled** – minimal hard dependencies between classes and Blueprints

It covers:

- C++ best practices
- Blueprint best practices
- Architectural & workflow recommendations
- Concrete code examples illustrating the patterns

---

## 2. C++ Best Practices

### 2.1 Encapsulation

- **Private by default**

  - Declare data members as `private`.
  - Use `protected` or `public` **only when required** by derived classes or external users.

- **Clear inheritance intent**

  - Use `final` on classes not meant for inheritance to:

    - Signal design intent.
    - Allow compiler optimizations.

  - Favor **composition over deep inheritance hierarchies**.

---

### 2.2 Modular Code Organization

- **Unreal Modules**

  - Organize code into **Unreal Engine modules** for:

    - Clear feature boundaries.
    - Faster compile times.
    - Cleaner dependency management.

  - Example modules: `Combat`, `UI`, `Inventory`, `AI`, etc.

- **Include-What-You-Use (IWYU)**

  - Include headers **only where needed**.
  - Use **forward declarations** in headers where possible.
  - Include full headers in `.cpp` files, not in `.h`, when only pointers/references are required.

---

### 2.3 Controlled Blueprint Exposure

- **Expose selectively**

  - Use:

    - `UFUNCTION(BlueprintCallable)` / `UFUNCTION(BlueprintPure)`
    - `UPROPERTY(BlueprintReadWrite)` / `UPROPERTY(BlueprintReadOnly)`

  - Only expose what designers actually need.

- **Extend via Blueprint events**

  - Use:

    - `BlueprintImplementableEvent` – implementation only in Blueprint.
    - `BlueprintNativeEvent` – default C++ implementation, overridable in Blueprint.

  - This allows designers to extend behavior **without modifying C++**.

---

### 2.4 Function Design & Complexity

- **Split complex logic**

  - Break large methods into **small, well-named submethods**.
  - Aim for clear responsibilities per function.

- **Local initialization**

  - Initialize variables as close as possible to their point of use.
  - Reduces risk of accidental misuse and improves readability.

---

### 2.5 Memory Management & GC Safety

- **Garbage collection tracking**

  - Use `UPROPERTY()` on `UObject*` pointers so Unreal’s GC can track them.
  - Non-UPROPERTY `UObject` pointers **must be managed carefully**.

- **Avoid circular references**

  - Avoid mutual strong references between UObjects/Actors.
  - Use `TWeakObjectPtr` or weak references where cycles might occur.

---

### 2.6 Avoid Excessive Ticking

- Prefer **event-driven** systems:

  - Use timers (`FTimerManager`) for recurring or delayed actions.
  - Use delegates/events instead of checking conditions every Tick.

- Only enable Tick when necessary:

  ```cpp
  PrimaryActorTick.bCanEverTick = false;
  ```

  and turn it on explicitly for classes that really need it.

---

## 3. Blueprint Best Practices

### 3.1 Blueprint Interfaces for Loose Coupling

- Define **Blueprint Interfaces** to describe common behaviors (e.g. `BPI_Interactable`, `BPI_Damageable`).
- Any Blueprint can implement the interface and provide custom behavior.
- Call interface functions without knowing the concrete class:

  - Greatly reduces casting and hard coupling.

---

### 3.2 Minimize Casting

- Avoid heavy reliance on `Cast To <Class>` nodes.
- Prefer:

  - **Interfaces** for common actions.
  - **Event Dispatchers** for broadcasting events.

- Benefits:

  - Better modularity.
  - Easier refactoring (no chain of casts to update).

---

### 3.3 Event Dispatchers & Delegates

- Use **Event Dispatchers** in Blueprints to:

  - Broadcast events (e.g. `OnHealthChanged`, `OnItemPickedUp`).
  - Allow other Blueprints to **bind** without holding strong direct references.

- Pattern:

  - Actor A fires dispatcher.
  - UI, FX, or other listeners bind to that dispatcher and react independently.

---

### 3.4 Data-Driven Design

- Move **data out of logic**:

  - Use **Data Tables**, **Curve Assets**, and **Data-Only Blueprints**.
  - Example: item stats, enemy properties, damage curves, etc.

- Benefits:

  - Tweak values without modifying graphs or C++.
  - Easier balancing and iteration.

---

### 3.5 Blueprint Function Libraries

- Place reusable utility logic in **Blueprint Function Libraries**:

  - No need for object context.
  - Shared across many Blueprints.
  - Good for math helpers, formatting utilities, generic queries, etc.

---

## 4. Architectural & Workflow Recommendations

### 4.1 Modules for Code Encapsulation

- Group related functionality into logical modules:

  - Example: `MyGame.Combat`, `MyGame.Inventory`, `MyGame.UI`.

- Encapsulation rules:

  - Public API: minimal and clear.
  - Internal details: kept private to the module.

---

### 4.2 Minimal Dependencies

- Reduce cross-module dependencies:

  - Avoid circular dependencies between modules.
  - Use interfaces and abstract base classes at module boundaries.

- Aim for:

  - **High cohesion** inside modules.
  - **Low coupling** between modules.

---

### 4.3 Clean APIs

- Design small, clear public interfaces:

  - C++ module API: well-documented public classes and functions.
  - Blueprint APIs: minimal set of nodes exposed for designers.

- Hide implementation details:

  - Prevents external code from depending on internal behavior.

---

### 4.4 Latent Blueprint Functions

- For asynchronous or time-consuming operations:

  - Expose them as **latent Blueprint nodes** (`UK2Node`s / latent `UFUNCTION`s) for clean flow.

- Useful for:

  - Loading assets.
  - Network calls.
  - Timed sequences.

---

### 4.5 Consistent Naming & Coding Standards

- Follow Epic’s C++ coding standards:

  - Class prefixes (`U`, `A`, `F`, `I`).
  - PascalCase for types, camelCase for variables.
  - Consistent function naming (`VerbNoun`).

- Maintain consistent naming between:

  - C++ functions.
  - Blueprint events and variables.

---

## 5. Practical Implementation Examples

### 5.1 C++ Blueprintable Interface for Loose Coupling

**ReactToTriggerInterface.h**

```cpp
#pragma once

#include "UObject/Interface.h"
#include "ReactToTriggerInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UReactToTriggerInterface : public UInterface
{
    GENERATED_BODY()
};

class IReactToTriggerInterface
{
    GENERATED_BODY()

public:
    // BlueprintNativeEvent allows C++ and Blueprint implementation
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    bool ReactToTrigger();
};
```

**Trap.h – Interface Implementation**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReactToTriggerInterface.h"
#include "Trap.generated.h"

UCLASS(Blueprintable)
class ATrap : public AActor, public IReactToTriggerInterface
{
    GENERATED_BODY()

public:
    virtual bool ReactToTrigger_Implementation() override;
};
```

**Trap.cpp**

```cpp
#include "Trap.h"
#include "Engine/Engine.h"

bool ATrap::ReactToTrigger_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Trap triggered via interface!"));
    // Your custom logic here
    return true;
}
```

**Safe Interface Call (C++)**

```cpp
AActor* Actor = /* ... */;

if (Actor && Actor->GetClass()->ImplementsInterface(UReactToTriggerInterface::StaticClass()))
{
    const bool bResult = IReactToTriggerInterface::Execute_ReactToTrigger(Actor);
}
```

> **Why?**
> This enforces loose coupling and allows polymorphic behavior across different actor types **without casting**.

---

### 5.2 UPROPERTY & UFUNCTION for GC Safety & Blueprint Exposure

**MyActor.h**

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

UCLASS(Blueprintable)
class AMyActor : public AActor
{
    GENERATED_BODY()

public:
    AMyActor();

    // Tracked by GC and editable/visible in Blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health;

    // Exposed to Blueprints as callable action
    UFUNCTION(BlueprintCallable, Category = "Actions")
    void TakeDamage(float DamageAmount);
};
```

**MyActor.cpp**

```cpp
#include "MyActor.h"
#include "Engine/Engine.h"

AMyActor::AMyActor()
{
    Health = 100.f;
}

void AMyActor::TakeDamage(float DamageAmount)
{
    Health = FMath::Max(Health - DamageAmount, 0.f);
    UE_LOG(LogTemp, Warning, TEXT("Health now: %f"), Health);
}
```

---

### 5.3 Event Dispatchers for Decoupled Blueprint Communication

**Conceptual Setup (Blueprint-side)**

1. In your character Blueprint (e.g. `BP_MyCharacter`):

   - Add an **Event Dispatcher**: `OnHealthChanged(float NewHealth)`.

2. Whenever `Health` changes (e.g. after `TakeDamage`):

   - Call `OnHealthChanged` dispatcher with the new health value.

3. In your HUD / UI Blueprint:

   - Bind to `OnHealthChanged` from the character.
   - Update health bar when the event is received.

> This lets UI and FX react to character state **without hard references or casts** from C++.

---

### 5.4 Blueprint Interface Usage Pattern

- Create `BPI_Interactable` with function `Interact`.
- Implement `Interact` in:

  - Door Blueprints
  - Item Blueprints
  - NPC Blueprints

**Player interaction flow:**

1. Player presses “Use”.
2. Player Blueprint traces for an actor in front.
3. If the hit actor **implements** `BPI_Interactable`:

   - Call `Interact` via the interface.

4. Each implementer responds according to its own logic.

> No casting to specific class types (Door, Item, NPC) is required.

---

### 5.5 Modular Class Structure & IWYU Example

**Folder Organization Example**

```text
/Source/MyGame/Combat
    EnemyCharacter.h/.cpp
    WeaponComponent.h/.cpp

/Source/MyGame/UI
    PlayerHUD.h/.cpp
```

**Forward Declaration in Header**

```cpp
// WeaponComponent is only referenced as a pointer here
class UWeaponComponent;

UCLASS()
class AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

private:
    UPROPERTY()
    UWeaponComponent* WeaponComp;
};
```

**Include in .cpp instead**

```cpp
#include "EnemyCharacter.h"
#include "WeaponComponent.h" // Full include here
```

> This reduces compile times and avoids unnecessary header dependencies.

---

### 5.6 Avoid Excessive Casting (Use Interfaces Instead)

**Casting approach (to avoid):**

```cpp
AMyCharacter* Char = Cast<AMyCharacter>(Actor);
if (Char)
{
    Char->DoSomething();
}
```

**Interface-based approach:**

```cpp
if (Actor && Actor->GetClass()->ImplementsInterface(UMyInterface::StaticClass()))
{
    IMyInterface::Execute_DoSomething(Actor);
}
```

> Interface-based calls are more flexible and reduce coupling.

---

### 5.7 Blueprint Function Library for Utilities

**MyBlueprintFunctionLibrary.h**

```cpp
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"

UCLASS()
class UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    static int32 MultiplyByTwo(int32 Input);
};
```

**MyBlueprintFunctionLibrary.cpp**

```cpp
#include "MyBlueprintFunctionLibrary.h"

int32 UMyBlueprintFunctionLibrary::MultiplyByTwo(int32 Input)
{
    return Input * 2;
}
```

> This function is now available as a node in any Blueprint, without needing object references.

---

## 6. Quick Reference Summary

| Practice                      | Description                                              | Example / Tool                        |
| ----------------------------- | -------------------------------------------------------- | ------------------------------------- |
| C++ Blueprintable Interfaces  | Loose coupling via `BlueprintNativeEvent`                | `ReactToTriggerInterface`             |
| Encapsulation & `final`       | Private members; `final` for sealed classes              | Class design & access control         |
| UPROPERTY & GC safety         | Track UObject pointers for GC & BP exposure              | `Health` + `TakeDamage` in `AMyActor` |
| Event Dispatchers             | Broadcast changes (e.g. health) to listeners             | `OnHealthChanged` in BP               |
| Blueprint Interfaces          | Shared contracts for varied BP implementations           | `BPI_Interactable`                    |
| Modular code structure & IWYU | Clean folder/modular organization & forward declarations | `Combat`, `UI` modules                |
| Avoid Casting                 | Prefer Interfaces over `Cast<>`                          | `IMyInterface::Execute_DoSomething`   |
| Data-driven design            | Use Data Tables, Curves, Data Assets                     | Enemy/item configs                    |
| Blueprint Function Libraries  | Shared utility logic                                     | `UMyBlueprintFunctionLibrary`         |
| Latent Blueprint functions    | Clean async/time-based BP flows                          | Custom latent UFUNCTIONs              |
