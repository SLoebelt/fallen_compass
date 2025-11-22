# Week 3 – Task 5 & 6 Updates: Multi-Actor Convoy System

## Updated Task 5: Convoy Actor System with AI Navigation

**Purpose**: Create `BP_FC_OverworldConvoy` as a parent actor containing multiple child convoy member actors, each with their own mesh and AI controller. The leader follows player commands via NavMesh, and followers use a breadcrumb system. The convoy manages camera constraints and POI overlap detection.

---

### Architecture Overview

#### Core Components

1. **AOverworldConvoy** (C++ Base Class) + **BP_FC_OverworldConvoy** (Blueprint Child)

   - C++ base class handles:
     - Convoy member array (TArray<AConvoyMember\*>)
     - Spawning and attaching convoy members
     - POI overlap aggregation logic
     - GetLeaderMember() method
     - NotifyPOIOverlap() method
   - Blueprint child configures:
     - Spawn point locations (Scene components)
     - Camera attachment point
     - Member composition and types
   - Manages camera attachment (camera follows convoy center)
   - Coordinates movement commands to leader

2. **AConvoyMember** (C++ Base Class) + **BP_FC_ConvoyMember** (Blueprint Child)

   - C++ base class handles:
     - Capsule collision and overlap detection logic
     - POI overlap notification to parent convoy
     - Reference to parent AOverworldConvoy
   - Blueprint child configures:
     - Static/skeletal mesh component (unique per member)
     - Mesh materials and visual properties
     - CharacterMovement parameters
   - AI Controller possession for navigation
   - Leader: Receives NavMesh move commands
   - Followers: Follow breadcrumb trail left by leader

3. **BP_FC_ConvoyAIController** (AI Controller)

   - Handles NavMesh pathfinding for leader
   - Implements breadcrumb following for followers
   - Manages movement speed and stopping behavior

4. **Camera Integration**

   - BP_OverworldCamera attached to convoy center point
   - Spring arm length and pan distance constraints bound to convoy
   - Camera follows convoy smoothly during movement

5. **Manager Integration**
   - Uses `UFCInputManager` for TopDown context
   - Player controller sends move commands to convoy's AI controller
   - Uses `UFCGameStateManager` for state-based behavior

---

### Step 5.1: Add Left Mouse Button Binding to IMC_FC_TopDown

- [x] **Analysis**

  - [x] Left Mouse Button will trigger click-to-move commands
  - [x] Will bind to existing IA_Interact action
  - [x] No modifiers needed for simple click detection

- [x] **Implementation (Unreal Editor)**

  - [x] Open `/Game/FC/Input/IMC_FC_TopDown
IMC_FC_TopDown`
  - [x] Add Mapping: IA_Interact
  - [x] Add Key: **Left Mouse Button**
    - [x] No modifiers needed
  - [x] Save IMC_FC_TopDown

- [x] **Testing After Step 5.1** ✅ CHECKPOINT
  - [x] Left Mouse Button bound to IA_Interact
  - [x] IMC_FC_TopDown now has 3 mappings (Pan, Zoom, Interact)
  - [x] Asset saves without errors

**COMMIT POINT 5.1**: `git add Content/FC/Input/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Add left mouse button binding for click-to-move in IMC_FC_TopDown"`

---

### Step 5.2: Create BP_FC_ConvoyMember Actor

#### Step 5.2.1: Create C++ AConvoyMember Class

- [x] **Analysis**

  - [x] C++ base class inheriting from ACharacter for CharacterMovement and AI support
  - [x] Handles capsule overlap detection and POI notification logic
  - [x] Stores reference to parent AOverworldConvoy
  - [x] Blueprint child will configure mesh and materials

- [x] **Implementation (Visual Studio - ConvoyMember.h)**

  - [x] Create `Source/FC/Characters/Convoy/ConvoyMember.h`
  - [x] Add class declaration:

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Character.h"
    #include "ConvoyMember.generated.h"

    class AOverworldConvoy;

    UCLASS()
    class FC_API AConvoyMember : public ACharacter
    {
        GENERATED_BODY()

    public:
        AConvoyMember();

    protected:
        virtual void BeginPlay() override;

    private:
        /** Reference to parent convoy actor */
        UPROPERTY()
        TObjectPtr<AOverworldConvoy> ParentConvoy;

        /** Handle capsule overlap events */
        UFUNCTION()
        void OnCapsuleBeginOverlap(
            UPrimitiveComponent* OverlappedComponent,
            AActor* OtherActor,
            UPrimitiveComponent* OtherComp,
            int32 OtherBodyIndex,
            bool bFromSweep,
            const FHitResult& SweepResult
        );

    public:
        /** Set parent convoy reference */
        void SetParentConvoy(AOverworldConvoy* InConvoy);

        /** Notify parent convoy of POI overlap */
        void NotifyPOIOverlap(AActor* POIActor);
    };
    ```

  - [ ] Save file

- [x] **Implementation (Visual Studio - ConvoyMember.cpp)**

  - [x] Create `Source/FC/Characters/Convoy/ConvoyMember.cpp`
  - [x] Add implementation:

    ```cpp
    #include "Characters/Convoy/ConvoyMember.h"
    #include "World/FCOverworldConvoy.h"
    #include "Components/CapsuleComponent.h"

    AConvoyMember::AConvoyMember()
    {
        PrimaryActorTick.bCanEverTick = false;

        // Configure capsule
        UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
        if (CapsuleComp)
        {
            CapsuleComp->SetCapsuleHalfHeight(100.0f);
            CapsuleComp->SetCapsuleRadius(50.0f);
            CapsuleComp->SetGenerateOverlapEvents(true);
        }

        // AI controller auto-possession
        AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    }

    void AConvoyMember::BeginPlay()
    {
        Super::BeginPlay();

        // Bind overlap event
        UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
        if (CapsuleComp)
        {
            CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AConvoyMember::OnCapsuleBeginOverlap);
        }
    }

    void AConvoyMember::OnCapsuleBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult)
    {
        if (!OtherActor || OtherActor == this)
        {
            return;
        }

        // Check if actor implements BPI_InteractablePOI interface
        // (Blueprint interface check - simplified for now)
        if (OtherActor->GetClass()->ImplementsInterface(UBPI_InteractablePOI::StaticClass()))
        {
            NotifyPOIOverlap(OtherActor);
        }
    }

    void AConvoyMember::SetParentConvoy(AOverworldConvoy* InConvoy)
    {
        ParentConvoy = InConvoy;
    }

    void AConvoyMember::NotifyPOIOverlap(AActor* POIActor)
    {
        if (ParentConvoy)
        {
            ParentConvoy->NotifyPOIOverlap(POIActor);
        }
    }
    ```

  - [x] Save file

- [x] **Compilation**

  - [x] Build solution in Visual Studio
  - [x] Verify no compilation errors
  - [x] Check includes and forward declarations

- [x] **Testing After Step 5.2.1** ✅ CHECKPOINT
  - [x] Compilation succeeds
  - [x] AConvoyMember class visible in Unreal Editor
  - [x] Can derive Blueprint from AConvoyMember

**COMMIT POINT 5.2.1**: `git add Source/FC/Characters/Convoy/ConvoyMember.h Source/FC/Characters/Convoy/ConvoyMember.cpp && git commit -m "feat(convoy): Create C++ AConvoyMember base class"`

---

#### Step 5.2.2: Create BP_FC_ConvoyMember Blueprint Child Class

- [x] **Analysis**

  - [x] Blueprint derived from AConvoyMember C++ class
  - [x] Configures mesh, materials, and CharacterMovement parameters
  - [x] POI overlap logic already handled in C++ parent

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Characters/Convoy/Blueprints/`
  - [x] Right-click → Blueprint Class → Select **ConvoyMember** (C++ class)
  - [x] Name: `BP_FC_ConvoyMember`
  - [x] Open BP_FC_ConvoyMember
  - [x] Components Panel:
    - [x] Select Mesh component (inherited from Character)
      - [x] Rename to "MemberMesh"
      - [x] Set Skeletal Mesh: Choose placeholder (mannequin or starter content)
      - [x] Set Scale: X=1, Y=1, Z=1
      - [x] Set Material: Unique color per member type (e.g., blue for prototype)
    - [x] Select CharacterMovement component (inherited):
      - [x] Max Walk Speed: 300.0
      - [x] Max Acceleration: 500.0
      - [x] Braking Deceleration Walking: 1000.0
      - [x] Enable **Orient Rotation to Movement**: True
      - [x] Disable **Use Controller Desired Rotation**: False
  - [x] Class Defaults:
    - [x] AI Controller Class: BP_FC_ConvoyAIController (will create in next step)
  - [x] Compile and save

- [x] **Testing After Step 5.2.2** ✅ CHECKPOINT
  - [x] Blueprint compiles without errors
  - [x] Inherits from AConvoyMember C++ class
  - [x] Mesh and materials configured
  - [x] Can place in level viewport (test, then remove)

**COMMIT POINT 5.2.2**: `git add Content/FC/Characters/Convoy/Blueprints/BP_FC_ConvoyMember.uasset && git commit -m "feat(convoy): Create BP_FC_ConvoyMember Blueprint child class"`

---

### Step 5.3: Create BP_FC_ConvoyAIController

#### Step 5.3.1: Create AI Controller Blueprint

- [x] **Analysis**

  - [x] AI controller handles NavMesh pathfinding for convoy members
  - [x] Leader: Receives move-to-location commands from player controller
  - [x] Followers: Follow breadcrumb trail (deferred to Step 5.5)
  - [x] Uses built-in AI MoveTo nodes

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Characters/Convoy/Blueprints/`
  - [x] Right-click → Blueprint Class → AIController
  - [x] Name: `BP_FC_ConvoyAIController`
  - [x] Open BP_FC_ConvoyAIController
  - [x] Event Graph:
    - [x] Event BeginPlay:
      ```
      BeginPlay → Print String "ConvoyAIController: Initialized"
      ```
  - [x] Create Custom Event: **MoveTo**
    - [x] Input: Vector (Target Location)
    - [x] Implementation:
      ```
      MoveTo (Vector input)
      → AI MoveToLocation (Simple Move To Location)
        - Pawn: Get Controlled Pawn
        - Goal Location: Target Location input
      → Print String "AI Controller moving to: [Target Location]"
      ```
  - [x] Compile and save

- [x] **Testing After Step 5.3.1** ✅ CHECKPOINT
  - [x] BP_FC_ConvoyAIController created
  - [x] MoveTo custom event functional
  - [x] Blueprint compiles without errors

**COMMIT POINT 5.3.1**: `git add Content/FC/World/Blueprints/AI/BP_FC_ConvoyAIController.uasset && git commit -m "feat(convoy): Create BP_FC_ConvoyAIController with MoveTo"`

---

### Step 5.4: Create BP_FC_OverworldConvoy Parent Actor

#### Step 5.4.1: Create C++ AOverworldConvoy Class

- [x] **Analysis**

  - [x] C++ base class inheriting from AActor
  - [x] Manages array of AConvoyMember pointers
  - [x] Handles member spawning and POI overlap aggregation
  - [x] Provides GetLeaderMember() and NotifyPOIOverlap() methods
  - [x] Blueprint child will configure spawn point locations

- [x] **Implementation (Visual Studio - FCOverworldConvoy.h)**

  - [x] Create `Source/FC/Characters/Convoy/FCOverworldConvoy.h`
  - [x] Add class declaration:

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Actor.h"
    #include "FCOverworldConvoy.generated.h"

    class AConvoyMember;
    class USceneComponent;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConvoyPOIOverlap, AActor*, POIActor);

    UCLASS()
    class FC_API AFCOverworldConvoy : public AActor
    {
        GENERATED_BODY()

    public:
        AFCOverworldConvoy();

    protected:
        virtual void BeginPlay() override;
        virtual void OnConstruction(const FTransform& Transform) override;

    private:
        /** Array of convoy member actors */
        UPROPERTY()
        TArray<AFCConvoyMember*> ConvoyMembers;

        /** Reference to leader member */
        UPROPERTY()
        AFCConvoyMember* LeaderMember;

        /** Blueprint class to spawn for convoy members */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        TSubclassOf<AFCConvoyMember> ConvoyMemberClass;

        /** Blueprint-exposed spawn points */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* ConvoyRoot;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* CameraAttachPoint;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* LeaderSpawnPoint;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* Follower1SpawnPoint;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|Convoy", meta = (AllowPrivateAccess = "true"))
        USceneComponent* Follower2SpawnPoint;

        /** Spawn convoy members at spawn points */
        void SpawnConvoyMembers();

    public:
        /** Get leader member reference */
        UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
        AFCConvoyMember* GetLeaderMember() const { return LeaderMember; }

        /** Called by convoy members when they overlap a POI */
        UFUNCTION(BlueprintCallable, Category = "FC|Convoy")
        void NotifyPOIOverlap(AActor* POIActor);

        /** Event dispatcher for POI overlap */
        UPROPERTY(BlueprintAssignable, Category = "FC|Convoy|Events")
        FOnConvoyPOIOverlap OnConvoyPOIOverlap;
    };
    ```

  - [x] Save file

- [x] **Implementation (Visual Studio - FCOverworldConvoy.cpp)**

  - [x] Create `Source/FC/Characters/Convoy/FCOverworldConvoy.cpp`
  - [x] Add implementation:

    ```cpp
    #include "Characters/Convoy/FCOverworldConvoy.h"
    #include "Characters/Convoy/FCConvoyMember.h"
    #include "Components/SceneComponent.h"
    #include "Engine/World.h"
    #include "FC.h"

    DEFINE_LOG_CATEGORY_STATIC(LogFCOverworldConvoy, Log, All);

    AFCOverworldConvoy::AFCOverworldConvoy()
    {
        PrimaryActorTick.bCanEverTick = false;

        // Create component hierarchy
        ConvoyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ConvoyRoot"));
        SetRootComponent(ConvoyRoot);

        CameraAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CameraAttachPoint"));
        CameraAttachPoint->SetupAttachment(ConvoyRoot);
        CameraAttachPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));

        LeaderSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeaderSpawnPoint"));
        LeaderSpawnPoint->SetupAttachment(ConvoyRoot);
        LeaderSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

        Follower1SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Follower1SpawnPoint"));
        Follower1SpawnPoint->SetupAttachment(ConvoyRoot);
        Follower1SpawnPoint->SetRelativeLocation(FVector(-150.0f, 0.0f, 0.0f));

        Follower2SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Follower2SpawnPoint"));
        Follower2SpawnPoint->SetupAttachment(ConvoyRoot);
        Follower2SpawnPoint->SetRelativeLocation(FVector(-300.0f, 0.0f, 0.0f));
    }

    void AFCOverworldConvoy::BeginPlay()
    {
        Super::BeginPlay();

        UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: BeginPlay"), *GetName());

        // Spawn convoy members at runtime
        SpawnConvoyMembers();
    }

    void AFCOverworldConvoy::OnConstruction(const FTransform& Transform)
    {
        Super::OnConstruction(Transform);

        // Note: Spawning moved to BeginPlay to avoid editor duplication
        // OnConstruction spawns actors in editor, but they get destroyed at PIE start
    }

    void AFCOverworldConvoy::SpawnConvoyMembers()
    {
        // Clear existing members
        for (AFCConvoyMember* Member : ConvoyMembers)
        {
            if (Member)
            {
                Member->Destroy();
            }
        }
        ConvoyMembers.Empty();

        UWorld* World = GetWorld();
        if (!World)
        {
            UE_LOG(LogFCOverworldConvoy, Warning, TEXT("OverworldConvoy %s: No world context"), *GetName());
            return;
        }

        // Check if ConvoyMemberClass is set
        if (!ConvoyMemberClass)
        {
            UE_LOG(LogFCOverworldConvoy, Error, TEXT("OverworldConvoy %s: ConvoyMemberClass not set! Please set it in Blueprint."), *GetName());
            return;
        }

        // Spawn parameters
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // Spawn leader
        AFCConvoyMember* Leader = World->SpawnActor<AFCConvoyMember>(
            ConvoyMemberClass,
            LeaderSpawnPoint->GetComponentLocation(),
            LeaderSpawnPoint->GetComponentRotation(),
            SpawnParams
        );

        if (Leader)
        {
            Leader->AttachToComponent(LeaderSpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Leader->SetParentConvoy(this);
            LeaderMember = Leader;
            ConvoyMembers.Add(Leader);
            UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned leader"), *GetName());
        }

        // Spawn follower 1
        AFCConvoyMember* Follower1 = World->SpawnActor<AFCConvoyMember>(
            ConvoyMemberClass,
            Follower1SpawnPoint->GetComponentLocation(),
            Follower1SpawnPoint->GetComponentRotation(),
            SpawnParams
        );

        if (Follower1)
        {
            Follower1->AttachToComponent(Follower1SpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Follower1->SetParentConvoy(this);
            ConvoyMembers.Add(Follower1);
            UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned follower 1"), *GetName());
        }

        // Spawn follower 2
        AFCConvoyMember* Follower2 = World->SpawnActor<AFCConvoyMember>(
            ConvoyMemberClass,
            Follower2SpawnPoint->GetComponentLocation(),
            Follower2SpawnPoint->GetComponentRotation(),
            SpawnParams
        );

        if (Follower2)
        {
            Follower2->AttachToComponent(Follower2SpawnPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            Follower2->SetParentConvoy(this);
            ConvoyMembers.Add(Follower2);
            UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned follower 2"), *GetName());
        }

        UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned %d total members"), *GetName(), ConvoyMembers.Num());
    }

    void AFCOverworldConvoy::NotifyPOIOverlap(AActor* POIActor)
    {
        if (!POIActor)
        {
            return;
        }

        // Get POI name (simplified - will use interface later)
        FString POIName = POIActor->GetName();

        UE_LOG(LogFCOverworldConvoy, Log, TEXT("Convoy %s detected POI: %s"), *GetName(), *POIName);

        // Display on-screen message
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                FString::Printf(TEXT("Convoy detected POI: %s"), *POIName));
        }

        // Broadcast event
        OnConvoyPOIOverlap.Broadcast(POIActor);
    }
    ```

  - [x] Save file

- [x] **Compilation**

  - [x] Build solution in Visual Studio
  - [x] Verify no compilation errors
  - [x] Check includes and forward declarations

- [x] **Testing After Step 5.4.1** ✅ CHECKPOINT
  - [x] Compilation succeeds
  - [x] AOverworldConvoy class visible in Unreal Editor
  - [x] Can derive Blueprint from AOverworldConvoy

**COMMIT POINT 5.4.1**: `git add Source/FC/Characters/Convoy/FCOverworldConvoy.h Source/FC/Characters/Convoy/FCOverworldConvoy.cpp && git commit -m "feat(convoy): Create C++ AOverworldConvoy base class"`

---

#### Step 5.4.2: Create BP_FC_OverworldConvoy Blueprint Child Class

- [x] **Analysis**

  - [x] Blueprint derived from AOverworldConvoy C++ class
  - [x] Member spawning already handled in C++ OnConstruction
  - [x] Blueprint can override spawn point locations if needed
  - [x] Will configure camera attachment in Event Graph

- [x] **Implementation (Unreal Editor)**

  - [x] Content Browser → `/Game/FC/Characters/Convoy/Blueprints/`
  - [x] Right-click → Blueprint Class → Select **FCOverworldConvoy** (C++ class)
  - [x] Name: `BP_FC_OverworldConvoy`
  - [x] Open BP_FC_OverworldConvoy
  - [x] Components Panel:
    - [x] Verify hierarchy inherited from C++ class:
      - [x] ConvoyRoot (Root)
      - [x] CameraAttachPoint (child)
      - [x] LeaderSpawnPoint (child)
      - [x] Follower1SpawnPoint (child)
      - [x] Follower2SpawnPoint (child)
    - [x] Optionally adjust spawn point locations in Details panel
  - [x] Class Defaults → FC|Convoy category:
    - [x] Set **Convoy Member Class** to `BP_FC_ConvoyMember`
    - [x] This tells C++ which Blueprint class to spawn at runtime
  - [x] Compile and save

- [x] **Testing After Step 5.4.2** ✅ CHECKPOINT
  - [x] Blueprint compiles without errors
  - [x] Inherits from AFCOverworldConvoy C++ class
  - [x] Place in level viewport
  - [x] Set Convoy Member Class property to BP_FC_ConvoyMember
  - [x] PIE: Verify 3 convoy members spawn at runtime (C++ BeginPlay)
  - [x] Check World Outliner: 3 members appear with meshes visible
  - [x] Members have capsule collision and AI controllers

**IMPLEMENTATION NOTES:**

- C++ spawns members in `BeginPlay()` (not `OnConstruction()`) to avoid PIE destruction issues
- C++ uses `ConvoyMemberClass` property (TSubclassOf<AFCConvoyMember>) to spawn Blueprint children
- Must set `Convoy Member Class` in Blueprint Details panel to `BP_FC_ConvoyMember`
- Spawning base C++ class directly shows no mesh; Blueprint child provides visual mesh

**COMMIT POINT 5.4.2**: `git add Content/FC/Characters/Convoy/Blueprints/BP_FCOverworldConvoy.uasset && git commit -m "feat(convoy): Create BP_FC_OverworldConvoy Blueprint child class"`

---

#### Step 5.4.3: Verify POI Overlap Aggregation (Already in C++)

- [ ] **Analysis**

  - [ ] POI overlap aggregation already implemented in C++ AOverworldConvoy
  - [ ] NotifyPOIOverlap() method callable from AConvoyMember
  - [ ] OnConvoyPOIOverlap event dispatcher already exposed to Blueprint
  - [ ] This step verifies functionality only

- [ ] **Verification (Unreal Editor)**

  - [ ] Open BP_FC_OverworldConvoy
  - [ ] Verify NotifyPOIOverlap method visible in Blueprint (inherited from C++)
  - [ ] Verify OnConvoyPOIOverlap event dispatcher in Event Graph (My Blueprint panel)
  - [ ] No additional implementation needed

- [ ] **Testing After Step 5.4.3** ✅ CHECKPOINT
  - [ ] NotifyPOIOverlap method visible in Blueprint
  - [ ] OnConvoyPOIOverlap event dispatcher accessible
  - [ ] Blueprint compiles without errors

**COMMIT POINT 5.4.3**: N/A (functionality already in C++ base class)

---

### Step 5.5: Implement Click-to-Move for Convoy Leader

#### Step 5.5.1: Add Click-to-Move Handler to AFCOverworldPlayerController

- [ ] **Analysis**

  - [ ] Player controller detects left-click on terrain
  - [ ] Raycasts from mouse position to world
  - [ ] Sends move command to convoy leader's AI controller
  - [ ] Uses UFCInputManager for TopDown context

- [ ] **Implementation (FCOverworldPlayerController.h)**

  - [ ] Open `Source/FC/Core/FCOverworldPlayerController.h`
  - [ ] Add forward declarations:
    ```cpp
    class UInputAction;
    class AOverworldConvoy;
    class AConvoyMember;
    ```
  - [ ] Add private members:

    ```cpp
    private:
        /** Input action for click-to-move */
        UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
        TObjectPtr<UInputAction> ClickMoveAction;

        /** Reference to possessed convoy */
        UPROPERTY()
        TObjectPtr<AOverworldConvoy> PossessedConvoy;

        /** Handle click-to-move input */
        void HandleClickMove();
    ```

  - [ ] Save file

- [ ] **Implementation (FCOverworldPlayerController.cpp)**

  - [ ] Open `Source/FC/Core/FCOverworldPlayerController.cpp`
  - [ ] Add includes:
    ```cpp
    #include "InputAction.h"
    #include "NavigationSystem.h"
    #include "AIController.h"
    #include "GameFramework/Character.h"
    #include "World/FCOverworldConvoy.h"
    #include "Characters/Convoy/ConvoyMember.h"
    ```
  - [ ] Update BeginPlay to find convoy in level:

    ```cpp
    void AFCOverworldPlayerController::BeginPlay()
    {
        Super::BeginPlay();

        / Existing InputManager setup...

        / Find AOverworldConvoy in level
        TArray<AActor*> FoundConvoys;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOverworldConvoy::StaticClass(), FoundConvoys);

        if (FoundConvoys.Num() > 0)
        {
            PossessedConvoy = Cast<AOverworldConvoy>(FoundConvoys[0]);
            UE_LOG(LogFCOverworldController, Log, TEXT("BeginPlay: Found convoy: %s"), *PossessedConvoy->GetName());
        }
        else
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("BeginPlay: No convoy found in level!"));
        }
    }
    ```

  - [ ] Update SetupInputComponent:

    ```cpp
    void AFCOverworldPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
        if (EnhancedInput && ClickMoveAction)
        {
            EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Started, this, &AFCOverworldPlayerController::HandleClickMove);
            UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Bound ClickMoveAction"));
        }
    }
    ```

  - [ ] Implement HandleClickMove:

    ```cpp
    void AFCOverworldPlayerController::HandleClickMove()
    {
        / Get mouse cursor hit result
        FHitResult HitResult;
        bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

        if (!bHit)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: No hit under cursor"));
            return;
        }

        / Get convoy leader
        if (!PossessedConvoy)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: No convoy reference"));
            return;
        }

        / Get leader member from convoy (C++ method)
        AConvoyMember* LeaderMember = PossessedConvoy->GetLeaderMember();
        AActor* LeaderActor = Cast<AActor>(LeaderMember);
        if (!LeaderActor)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: No leader member found"));
            return;
        }

        / Get leader's AI controller (AConvoyMember inherits from ACharacter)
        if (!LeaderMember)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: Leader member is null"));
            return;
        }

        AAIController* AIController = Cast<AAIController>(LeaderMember->GetController());
        if (!AIController)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: Leader has no AI controller"));
            return;
        }

        / Project hit location to NavMesh
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLocation;
            bool bFoundPath = NavSys->ProjectPointToNavigation(HitResult.Location, NavLocation);

            if (bFoundPath)
            {
                / Send move command to AI controller
                AIController->MoveTo(NavLocation.Location);
                UE_LOG(LogFCOverworldController, Log, TEXT("HandleClickMove: Moving convoy to %s"), *NavLocation.Location.ToString());
            }
            else
            {
                UE_LOG(LogFCOverworldController, Warning, TEXT("HandleClickMove: Failed to project to NavMesh"));
            }
        }
    }
    ```

  - [ ] Save file

- [ ] **Compilation**

  - [ ] Build solution in Visual Studio
  - [ ] Verify no compilation errors
  - [ ] Check includes and forward declarations

- [ ] **Testing After Step 5.5.1** ✅ CHECKPOINT
  - [ ] Compilation succeeds
  - [ ] HandleClickMove method implemented
  - [ ] Can open Unreal Editor

**COMMIT POINT 5.5.1**: `git add Source/FC/Core/FCOverworldPlayerController.h Source/FC/Core/FCOverworldPlayerController.cpp && git commit -m "feat(convoy): Implement click-to-move for convoy leader in player controller"`

---

#### Step 5.5.2: Verify GetLeaderMember Method (Already in C++)

- [ ] **Analysis**

  - [ ] GetLeaderMember() method already implemented in C++ AOverworldConvoy
  - [ ] Marked as BlueprintCallable, accessible from both C++ and Blueprint
  - [ ] Returns AConvoyMember\* (leader member reference)
  - [ ] This step verifies functionality only

- [ ] **Verification (Unreal Editor)**

  - [ ] Open BP_FC_OverworldConvoy
  - [ ] Verify GetLeaderMember method visible in Blueprint (inherited from C++)
  - [ ] Test in Event Graph: Call GetLeaderMember → Print String with result
  - [ ] Remove test code after verification

- [ ] **Testing After Step 5.5.2** ✅ CHECKPOINT
  - [ ] GetLeaderMember method visible and callable in Blueprint
  - [ ] Returns valid AConvoyMember reference
  - [ ] Blueprint compiles without errors

**COMMIT POINT 5.5.2**: N/A (functionality already in C++ base class)

---

#### Step 5.5.3: Configure ClickMoveAction in BP_FCOverworldPlayerController

- [ ] **Analysis**

  - [ ] ClickMoveAction property needs IA_Interact assigned in Blueprint
  - [ ] Follow same pattern as other input actions

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FCOverworldPlayerController
  - [ ] Class Defaults → FC | Input | Actions: - [ ] Set ClickMoveAction: `/Game/FC/Input/Actions
/Game/FC/Input/IA_Interact`
  - [ ] Compile and save

- [ ] **Testing After Step 5.5.3** ✅ CHECKPOINT
  - [ ] ClickMoveAction assigned to IA_Interact
  - [ ] Blueprint compiles without errors

**COMMIT POINT 5.5.3**: `git add Content/FC/Core/BP_FCOverworldPlayerController.uasset && git commit -m "feat(convoy): Assign IA_Interact to ClickMoveAction in BP_FCOverworldPlayerController"`

---

### Step 5.6: Integrate Camera with Convoy

#### Step 5.6.1: Attach BP_OverworldCamera to Convoy

- [ ] **Analysis**

  - [ ] Camera should follow convoy's CameraAttachPoint
  - [ ] Spring arm and pan constraints bound to convoy center
  - [ ] Camera remains smooth during movement

- [ ] **Implementation (BP_OverworldCamera or AFCOverworldPlayerController)**

  - [ ] **Option A: Dynamic Attachment in Controller BeginPlay**
    - [ ] Open `FCOverworldPlayerController.cpp` → BeginPlay()
    - [ ] After finding convoy:
      ```cpp
      if (PossessedConvoy && OverworldCamera)
      {
          / Attach camera to convoy's camera attach point
          USceneComponent* CameraAttachPoint = PossessedConvoy->FindComponentByClass<USceneComponent>();
          / (Need proper component getter - will use Blueprint helper)
          OverworldCamera->AttachToActor(PossessedConvoy, FAttachmentTransformRules::KeepWorldTransform);
          UE_LOG(LogFCOverworldController, Log, TEXT("BeginPlay: Attached camera to convoy"));
      }
      ```
  - [ ] **Option B: Blueprint Attachment (Simpler for Week 3)**
    - [ ] Open BP_FC_OverworldConvoy → Event Graph
    - [ ] Event BeginPlay:
      ```
      BeginPlay
      → Get Actor of Class (BP_OverworldCamera)
      → Attach Actor To Component (Camera → CameraAttachPoint, Keep World Transform)
      → Print String "Convoy: Attached camera"
      ```
  - [ ] Choose Option B for simplicity
  - [ ] Compile and save

- [ ] **Testing After Step 5.6.1** ✅ CHECKPOINT
  - [ ] Place convoy and camera in level
  - [ ] PIE: Camera attaches to convoy
  - [ ] Camera follows convoy when it moves
  - [ ] No jittering or offset issues

**COMMIT POINT 5.6.1**: `git add Content/FC/World/Blueprints/Pawns/BP_FCOverworldConvoy.uasset && git commit -m "feat(convoy): Attach camera to convoy CameraAttachPoint"`

---

#### Step 5.6.2: Bind Camera Constraints to Convoy (Prototype Scope)

- [ ] **Analysis**

  - [ ] PROTOTYPE SCOPE: Basic attachment sufficient for Week 3
  - [ ] FUTURE: Spring arm length and pan boundaries based on convoy size
  - [ ] BACKLOG: Implement dynamic camera constraint system

- [ ] **Implementation (Week 3 Prototype)**

  - [ ] Verify camera attachment working
  - [ ] Use fixed spring arm length (from Task 3)
  - [ ] Pan constraints remain level-based (not convoy-based)
  - [ ] Document limitation in backlog (see end of file)

- [ ] **Testing After Step 5.6.2** ✅ CHECKPOINT
  - [ ] Camera follows convoy with fixed constraints
  - [ ] Pan and zoom work as expected
  - [ ] Attachment stable during movement

**COMMIT POINT 5.6.2**: N/A (prototype scope documented, no code changes)

---

### Step 5.7: Place BP_FC_OverworldConvoy in L_Overworld

#### Step 5.7.1: Add Convoy to Level and Configure

- [ ] **Analysis**

  - [ ] Convoy spawns at PlayerStart location
  - [ ] Members auto-spawn via Construction Script
  - [ ] Camera auto-attaches via BeginPlay

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open L_Overworld level
  - [ ] **Option A: Manual Placement**
    - [ ] Drag BP_FC_OverworldConvoy from Content Browser into viewport
    - [ ] Position at PlayerStart location (X=0, Y=0, Z=100)
  - [ ] **Option B: Default Pawn Class (Recommended if convoy needs controller possession)**
    - [ ] Window → World Settings
    - [ ] Game Mode → Default Pawn Class: BP_FC_OverworldConvoy
    - [ ] NOTE: This may conflict with controller not possessing pawn directly
    - [ ] For Week 3: Use Option A (manual placement, controller references convoy)
  - [ ] Save level

- [ ] **Testing After Step 5.7.1** ✅ CHECKPOINT
  - [ ] Convoy visible in level with 3 members
  - [ ] Members positioned correctly (leader in front, followers behind)
  - [ ] PIE: Convoy members spawn and attach
  - [ ] Camera attaches to convoy
  - [ ] Level saves without errors

**COMMIT POINT 5.7.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(convoy): Place BP_FC_OverworldConvoy in L_Overworld"`

---

### Step 5.8: Test Click-to-Move and POI Overlap

#### Step 5.8.1: Full Convoy Verification

- [ ] **Analysis**

  - [ ] Test click-to-move with convoy leader
  - [ ] Verify followers remain stationary (breadcrumb system not yet implemented)
  - [ ] Test POI overlap detection
  - [ ] Verify camera follows convoy

- [ ] **Test Sequence**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Verify convoy spawns with 3 members
  - [ ] Verify camera attached to convoy (top-down view follows convoy)
  - [ ] Press `P` to visualize NavMesh (green overlay)
  - [ ] **Test Click-to-Move**:
    - [ ] Left-click on ground (green NavMesh area)
    - [ ] Verify leader moves to clicked location
    - [ ] Check Output Log: "HandleClickMove: Moving convoy to [location]"
    - [ ] Verify camera follows leader smoothly
    - [ ] NOTE: Followers will NOT follow yet (breadcrumb system in next phase)
  - [ ] **Test POI Overlap**:
    - [ ] Left-click to move leader to a POI location
    - [ ] When leader capsule overlaps POI:
      - [ ] Check Output Log: "Convoy detected POI: [POI Name]"
      - [ ] On-screen message: "Convoy detected POI: [name]" (cyan text)
  - [ ] **Test Multiple Clicks**:
    - [ ] Click different locations rapidly
    - [ ] Verify leader updates path correctly
  - [ ] **Test Camera Pan/Zoom During Movement**:
    - [ ] While leader is moving, use WASD to pan camera
    - [ ] Verify camera controls still work (camera moves relative to convoy)
    - [ ] Use mouse wheel to zoom
    - [ ] Verify zoom works smoothly

- [ ] **Testing After Step 5.8.1** ✅ CHECKPOINT
  - [ ] Leader moves to clicked locations via NavMesh
  - [ ] Camera follows convoy smoothly
  - [ ] POI overlap detection works
  - [ ] Output logs confirm functionality
  - [ ] No "Accessed None" errors
  - [ ] Followers stationary (expected until breadcrumb system)

**COMMIT POINT 5.8.1**: `git add -A && git commit -m "test(convoy): Verify click-to-move and POI overlap detection"`

---

### Step 5.9: Document Follower Breadcrumb System as Backlog

- [ ] **Analysis**

  - [ ] Breadcrumb following system deferred to future sprint
  - [ ] Document requirements and architecture for later implementation
  - [ ] Create backlog item at end of file

- [ ] **Documentation** (see backlog section at end of file)
  - [ ] Backlog item created: "Convoy Follower Breadcrumb System"
  - [ ] Includes architecture notes and implementation steps

**COMMIT POINT 5.9**: N/A (backlog documentation at end of file)

---

### Task 5 Acceptance Criteria

- [ ] Left Mouse Button bound to IA_Interact in IMC_FC_TopDown
- [ ] BP_FC_ConvoyMember character created with mesh, collision, AI controller support, and POI overlap detection
- [ ] BP_FC_ConvoyAIController created with MoveTo method
- [ ] BP_FC_OverworldConvoy parent actor created with 3 child convoy members (leader + 2 followers)
- [ ] Convoy members spawn and attach correctly via Construction Script
- [ ] POI overlap aggregation implemented in BP_FC_OverworldConvoy
- [ ] AFCOverworldPlayerController implements click-to-move for convoy leader
- [ ] BP_OverworldCamera attached to convoy's CameraAttachPoint
- [ ] Camera follows convoy smoothly during movement
- [ ] Convoy placed in L_Overworld at PlayerStart
- [ ] Left-click moves convoy leader to location using NavMesh pathfinding
- [ ] POI overlap triggers on-screen message and log output
- [ ] Camera pan/zoom works while convoy moves
- [ ] No compilation errors or runtime crashes
- [ ] Follower movement deferred to backlog (breadcrumb system)

**Task 5 complete. Ready for Task 6 sub-tasks (POI Actor & Interaction Stub).**

---

## Updated Task 6: POI Actor & Right-Click Interaction Stub

**Purpose**: Create BP_OverworldPOI actor with right-click interaction that logs to console (stub for future implementation). POI overlap detection already implemented in convoy system.

---

### Step 6.1: Create Input Action for POI Interaction

#### Step 6.1.1: Create IA_InteractPOI Input Action

- [ ] **Analysis**

  - [ ] POI interaction uses right-click (separate from left-click move)
  - [ ] Boolean trigger (press detection)
  - [ ] Will be bound to Right Mouse Button in IMC_FC_TopDown

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/Input/Actions
/Game/FC/Input/`
  - [ ] Right-click → Input → Input Action
  - [ ] Name: `IA_InteractPOI`
  - [ ] Open IA_InteractPOI
  - [ ] Set Value Type: Digital (bool)
  - [ ] Save asset

- [ ] **Testing After Step 6.1.1** ✅ CHECKPOINT
  - [ ] Asset created at correct path
  - [ ] Value Type set to Digital (bool)
  - [ ] Asset saves without errors

**COMMIT POINT 6.1.1**: `git add Content/FC/Input/Actions/IA_InteractPOI.uasset && git commit -m "feat(overworld): Create IA_InteractPOI input action"`

---

#### Step 6.1.2: Add Right Mouse Button Binding to IMC_FC_TopDown

- [ ] **Analysis**

  - [ ] Right Mouse Button key name in Enhanced Input: "RightMouseButton"
  - [ ] No modifiers needed

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open `/Game/FC/Input/Contexts/IMC_FC_TopDown`
  - [ ] Add Mapping: IA_InteractPOI
  - [ ] Add Key: **Right Mouse Button**
    - [ ] No modifiers needed
  - [ ] Save IMC_FC_TopDown

- [ ] **Testing After Step 6.1.2** ✅ CHECKPOINT
  - [ ] Right Mouse Button bound to IA_InteractPOI
  - [ ] IMC_FC_TopDown now has 4 mappings (Pan, Zoom, Interact/ClickMove, InteractPOI)
  - [ ] Asset saves without errors

**COMMIT POINT 6.1.2**: `git add Content/FC/Input/IMC_FC_TopDown.uasset && git commit -m "feat(overworld): Add right mouse button binding for POI interaction in IMC_FC_TopDown"`

---

### Step 6.2: Create BP_OverworldPOI Actor Blueprint

#### Step 6.2.1: Create Actor Blueprint with Mesh and Collision

- [ ] **Analysis**

  - [ ] POI needs collision for mouse raycast detection AND convoy overlap
  - [ ] Static mesh for visual representation
  - [ ] Box or sphere collision component
  - [ ] Implements BPI_InteractablePOI interface (for convoy overlap detection)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Content Browser → `/Game/FC/World/Blueprints/Actors/`
  - [ ] Right-click → Blueprint Class → Actor
  - [ ] Name: `BP_OverworldPOI`
  - [ ] Open BP_OverworldPOI
  - [ ] Components Panel:
    - [ ] Root: Scene Component (rename to "POIRoot")
    - [ ] Add Child: Static Mesh Component (rename to "POIMesh")
      - [ ] Set Static Mesh: Choose placeholder (Cone, Sphere, or starter content mesh)
      - [ ] Set Scale: X=2, Y=2, Z=2 (visible from camera)
      - [ ] Set Material: Distinctive color (e.g., yellow/orange for POI)
    - [ ] Add Child: Box Component (rename to "InteractionBox")
      - [ ] Set Box Extent: X=150, Y=150, Z=100 (larger than mesh for easier clicking)
      - [ ] Enable **Generate Overlap Events**: True
      - [ ] Set Collision Preset: **Custom**
        - [ ] Collision Enabled: Query Only (No Physics Collision)
        - [ ] Object Type: WorldDynamic
        - [ ] Collision Responses: Overlap All (for convoy overlap detection)
        - [ ] Visibility Channel: **Block** (for mouse raycast)
  - [ ] Compile and save

- [ ] **Testing After Step 6.2.1** ✅ CHECKPOINT
  - [ ] Blueprint compiles without errors
  - [ ] Components hierarchy correct
  - [ ] InteractionBox set to Block Visibility and Overlap Pawn
  - [ ] Can place in level viewport (test, then remove)

**COMMIT POINT 6.2.1**: `git add Content/FC/World/Blueprints/Actors/BP_OverworldPOI.uasset && git commit -m "feat(overworld): Create BP_OverworldPOI actor with mesh and interaction collision"`

---

#### Step 6.2.2: Add Custom POI Name Property

- [ ] **Analysis**

  - [ ] Each POI should have a name for identification in logs and future UI
  - [ ] Editable per-instance property
  - [ ] String type

- [ ] **Implementation (BP_OverworldPOI Event Graph)**

  - [ ] Open BP_OverworldPOI
  - [ ] Variables Panel → Add Variable:
    - [ ] Name: `POIName`
    - [ ] Type: String
    - [ ] Instance Editable: ✅ Checked
    - [ ] Category: "FC|POI"
    - [ ] Default Value: "Unnamed POI"
    - [ ] Tooltip: "Display name for this Point of Interest"
  - [ ] Compile and save

- [ ] **Testing After Step 6.2.2** ✅ CHECKPOINT
  - [ ] POIName variable created
  - [ ] Instance Editable enabled
  - [ ] Default value set
  - [ ] Blueprint compiles without errors

**COMMIT POINT 6.2.2**: `git add Content/FC/World/Blueprints/Actors/BP_OverworldPOI.uasset && git commit -m "feat(overworld): Add POIName property to BP_OverworldPOI"`

---

### Step 6.3: Implement BPI_InteractablePOI Interface

#### Step 6.3.1: Create BPI_InteractablePOI Blueprint Interface

- [ ] **Analysis**

  - [ ] Interface allows convoy and controller to interact with POI-like actors
  - [ ] Provides GetPOIName() and OnPOIInteract() methods
  - [ ] Enables future POI types (cities, ruins, encounters, etc.)

- [ ] **Implementation (Unreal Editor - Blueprint Interface)**

  - [ ] Content Browser → `/Game/FC/Core/Interfaces/`
  - [ ] Right-click → Blueprints → Blueprint Interface
  - [ ] Name: `BPI_InteractablePOI`
  - [ ] Open BPI_InteractablePOI
  - [ ] Add Function: `OnPOIInteract`
    - [ ] No inputs/outputs (void function for stub)
  - [ ] Add Function: `GetPOIName`
    - [ ] Output: String (Return Value)
  - [ ] Compile and save

- [ ] **Testing After Step 6.3.1** ✅ CHECKPOINT
  - [ ] Interface created with 2 functions
  - [ ] Functions have correct signatures
  - [ ] Asset saves without errors

**COMMIT POINT 6.3.1**: `git add Content/FC/Core/Interfaces/BPI_InteractablePOI.uasset && git commit -m "feat(overworld): Create BPI_InteractablePOI interface"`

---

#### Step 6.3.2: Implement Interface in BP_OverworldPOI

- [ ] **Analysis**

  - [ ] BP_OverworldPOI implements BPI_InteractablePOI
  - [ ] GetPOIName returns POIName variable
  - [ ] OnPOIInteract displays stub message

- [ ] **Implementation (BP_OverworldPOI)**

  - [ ] Open BP_OverworldPOI
  - [ ] Class Settings → Interfaces → Add → BPI_InteractablePOI
  - [ ] Implement GetPOIName function:
    - [ ] Event Graph → Right-click → Add Event → Event GetPOIName
    - [ ] Connect POIName variable to Return Value
  - [ ] Implement OnPOIInteract function:
    - [ ] Event Graph → Right-click → Add Event → Event OnPOIInteract
    - [ ] Add Print String node:
      - [ ] In String: Append "POI Interaction Stub: " + POIName
      - [ ] Text Color: Yellow
      - [ ] Duration: 5.0
  - [ ] Compile and save

- [ ] **Testing After Step 6.3.2** ✅ CHECKPOINT
  - [ ] BP_OverworldPOI implements interface
  - [ ] GetPOIName returns POIName variable
  - [ ] OnPOIInteract has Print String stub
  - [ ] Blueprint compiles without errors

**COMMIT POINT 6.3.2**: `git add Content/FC/World/Blueprints/Actors/BP_OverworldPOI.uasset && git commit -m "feat(overworld): Implement BPI_InteractablePOI in BP_OverworldPOI"`

---

### Step 6.4: Implement POI Right-Click Interaction Handler

#### Step 6.4.1: Add POI Interaction Handler to AFCOverworldPlayerController

- [ ] **Analysis**

  - [ ] Controller needs to handle IA_InteractPOI input
  - [ ] On right-click: Raycast from mouse position, check if hit actor implements interface
  - [ ] If valid POI: Call OnPOIInteract() interface method

- [ ] **Implementation (FCOverworldPlayerController.h)**

  - [ ] Open `Source/FC/Core/FCOverworldPlayerController.h`
  - [ ] Add private members:

    ```cpp
    private:
        /** Input action for POI interaction */
        UPROPERTY(EditDefaultsOnly, Category = "FC|Input|Actions")
        TObjectPtr<UInputAction> InteractPOIAction;

        /** Handle POI interaction input */
        void HandleInteractPOI();
    ```

  - [ ] Save file

- [ ] **Implementation (FCOverworldPlayerController.cpp)**

  - [ ] Update SetupInputComponent():

    ```cpp
    void AFCOverworldPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        / Existing click-to-move binding...

        / Bind POI interaction action
        if (EnhancedInput && InteractPOIAction)
        {
            EnhancedInput->BindAction(InteractPOIAction, ETriggerEvent::Started, this, &AFCOverworldPlayerController::HandleInteractPOI);
            UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Bound InteractPOIAction"));
        }
    }
    ```

  - [ ] Implement HandleInteractPOI():

    ```cpp
    void AFCOverworldPlayerController::HandleInteractPOI()
    {
        / Get mouse cursor hit result
        FHitResult HitResult;
        bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

        if (!bHit)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleInteractPOI: No hit under cursor"));
            return;
        }

        / Check if hit actor implements POI interface
        AActor* HitActor = HitResult.GetActor();
        if (!HitActor)
        {
            UE_LOG(LogFCOverworldController, Warning, TEXT("HandleInteractPOI: No actor hit"));
            return;
        }

        / Check for interface (Blueprint interface check)
        if (HitActor->GetClass()->ImplementsInterface(UBPI_InteractablePOI::StaticClass()))
        {
            / Call interface method via Blueprint
            IBPI_InteractablePOI::Execute_OnPOIInteract(HitActor);

            / Get POI name for logging
            FString POIName = IBPI_InteractablePOI::Execute_GetPOIName(HitActor);
            UE_LOG(LogFCOverworldController, Log, TEXT("HandleInteractPOI: Interacted with POI '%s'"), *POIName);
        }
        else
        {
            UE_LOG(LogFCOverworldController, Log, TEXT("HandleInteractPOI: Hit actor '%s' is not a POI"), *HitActor->GetName());
        }
    }
    ```

  - [ ] Save file

- [ ] **Compilation**

  - [ ] Build solution in Visual Studio
  - [ ] Verify no compilation errors

- [ ] **Testing After Step 6.4.1** ✅ CHECKPOINT
  - [ ] Compilation succeeds
  - [ ] HandleInteractPOI() method added correctly
  - [ ] Interface call syntax correct
  - [ ] Can open Unreal Editor

**COMMIT POINT 6.4.1**: `git add Source/FC/Core/FCOverworldPlayerController.h Source/FC/Core/FCOverworldPlayerController.cpp && git commit -m "feat(overworld): Implement POI right-click interaction handler"`

---

#### Step 6.4.2: Configure InteractPOIAction in BP_FCOverworldPlayerController

- [ ] **Analysis**

  - [ ] InteractPOIAction property needs IA_InteractPOI assigned in Blueprint

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FCOverworldPlayerController
  - [ ] Class Defaults → FC | Input | Actions: - [ ] Set InteractPOIAction: `/Game/FC/Input/Actions
/Game/FC/Input/IA_InteractPOI`
  - [ ] Compile and save

- [ ] **Testing After Step 6.4.2** ✅ CHECKPOINT
  - [ ] InteractPOIAction assigned to IA_InteractPOI
  - [ ] Blueprint compiles without errors

**COMMIT POINT 6.4.2**: `git add Content/FC/Core/BP_FCOverworldPlayerController.uasset && git commit -m "feat(overworld): Assign IA_InteractPOI to InteractPOIAction"`

---

### Step 6.5: Place POI Actors and Test

#### Step 6.5.1: Add Multiple POI Instances to L_Overworld

- [ ] **Analysis**

  - [ ] Place 3-5 POI actors in different locations for testing
  - [ ] Set unique names for each POI instance
  - [ ] Position on ground (Z=0 or on terrain)

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open L_Overworld level
  - [ ] Drag BP_OverworldPOI from Content Browser into viewport
  - [ ] **POI Instance 1**:
    - [ ] Position: X=500, Y=0, Z=50
    - [ ] Details Panel → FC | POI → POIName: "Northern Village"
  - [ ] Drag another BP_OverworldPOI instance
  - [ ] **POI Instance 2**:
    - [ ] Position: X=-500, Y=500, Z=50
    - [ ] POIName: "Eastern Outpost"
  - [ ] Drag another BP_OverworldPOI instance
  - [ ] **POI Instance 3**:
    - [ ] Position: X=-500, Y=-500, Z=50
    - [ ] POIName: "Western Ruins"
  - [ ] Save level

- [ ] **Testing After Step 6.5.1** ✅ CHECKPOINT
  - [ ] Multiple POI actors visible in level
  - [ ] Each POI has unique POIName value
  - [ ] POIs positioned on ground
  - [ ] Level saves without errors

**COMMIT POINT 6.5.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(overworld): Place multiple BP_OverworldPOI instances in L_Overworld"`

---

#### Step 6.5.2: Full POI Interaction Verification

- [ ] **Analysis**

  - [ ] Test right-click interaction
  - [ ] Test convoy overlap detection
  - [ ] Verify both interaction methods work

- [ ] **Test Sequence**

  - [ ] Open L_Overworld in editor
  - [ ] PIE (Play In Editor)
  - [ ] Verify convoy and POIs visible
  - [ ] **Test Right-Click Interaction**:
    - [ ] Right-click on POI mesh (e.g., "Northern Village")
    - [ ] On-screen message: "POI Interaction Stub: Northern Village" (yellow, 5s)
    - [ ] Check Output Log: "HandleInteractPOI: Interacted with POI 'Northern Village'"
  - [ ] **Test Convoy Overlap Detection**:
    - [ ] Left-click to move convoy leader to POI location
    - [ ] When leader overlaps POI:
      - [ ] On-screen message: "Convoy detected POI: Northern Village" (cyan, 5s)
      - [ ] Check Output Log: "Convoy detected POI: Northern Village"
  - [ ] **Test Multiple POIs**:
    - [ ] Right-click on different POIs → Each shows unique name
    - [ ] Move convoy to different POIs → Overlap detection works for all
  - [ ] **Test Non-POI Right-Click**:
    - [ ] Right-click on ground (not POI) → No POI interaction message
    - [ ] Check log: "Hit actor is not a POI" or "No hit under cursor"

- [ ] **Testing After Step 6.5.2** ✅ CHECKPOINT
  - [ ] Right-click POI interaction works
  - [ ] Convoy overlap detection works
  - [ ] Both methods show correct POI names
  - [ ] No "Accessed None" errors

**COMMIT POINT 6.5.2**: `git add -A && git commit -m "test(overworld): Verify POI right-click and convoy overlap detection"`

---

### Task 6 Acceptance Criteria

- [ ] IA_InteractPOI input action created (Digital/Boolean)
- [ ] Right Mouse Button bound to IA_InteractPOI in IMC_FC_TopDown
- [ ] BP_OverworldPOI actor created with mesh, InteractionBox, and POIName property
- [ ] BPI_InteractablePOI interface created with OnPOIInteract() and GetPOIName() methods
- [ ] BP_OverworldPOI implements BPI_InteractablePOI interface
- [ ] OnPOIInteract() displays Print String stub message
- [ ] AFCOverworldPlayerController implements HandleInteractPOI() with raycast and interface check
- [ ] InteractPOIAction assigned to IA_InteractPOI in BP_FCOverworldPlayerController
- [ ] 3-5 POI instances placed in L_Overworld with unique names
- [ ] Right-click on POI shows on-screen message and logs interaction
- [ ] Convoy overlap with POI shows on-screen message and logs detection
- [ ] Both interaction methods work independently
- [ ] No compilation errors or runtime crashes

**Task 6 complete. Ready for Task 7-9 (transitions, pause, testing).**

---

## Backlog Items

### Backlog Item 1: Convoy Follower Breadcrumb System

**Priority**: Medium (Week 4 or 5)

**Description**: Implement breadcrumb trail system for convoy followers to follow the leader's path with realistic spacing and formation.

**Requirements**:

1. Leader leaves breadcrumb markers at regular intervals (e.g., every 100 units)
2. Followers navigate to breadcrumbs in sequence
3. Configurable spacing between convoy members (default: 150 units)
4. Followers maintain formation during turns and obstacles
5. Breadcrumbs clean up after all followers pass

**Architecture**:

- **BP_FC_ConvoyMember** (Leader):
  - Add Timer: Drop breadcrumb every 0.5 seconds or 100 units traveled
  - Breadcrumb data: FVector location + timestamp
  - Store in BP_FC_OverworldConvoy breadcrumb array
- **BP_FC_ConvoyMember** (Followers):
  - AI controller retrieves next breadcrumb from parent convoy
  - MoveTo using breadcrumb position
  - OnMoveCompleted: Request next breadcrumb
  - Skip breadcrumbs if too close (formation spacing logic)
- **BP_FC_OverworldConvoy**:
  - Breadcrumb queue (TArray<FVector>)
  - GetNextBreadcrumb(int FollowerIndex) method
  - CleanupOldBreadcrumbs() method (removes breadcrumbs passed by last follower)

**Implementation Steps**:

1. Add breadcrumb dropping to leader movement
2. Create GetNextBreadcrumb method in BP_FC_OverworldConvoy
3. Implement breadcrumb following in follower AI controllers
4. Add formation spacing logic (distance checks)
5. Implement breadcrumb cleanup system
6. Test with convoy moving in complex paths (turns, obstacles)

**Modularity Notes**:

- Breadcrumb system encapsulated in BP_FC_OverworldConvoy
- Follower AI controller logic separate from leader logic
- Configuration variables exposed for spacing, breadcrumb frequency

---

### Backlog Item 2: Dynamic Camera Constraint System

**Priority**: Low (Week 6+)

**Description**: Bind camera spring arm length and pan distance constraints to convoy size and state (moving vs. stationary).

**Requirements**:

1. Camera zoom limits based on convoy spread (how far apart members are)
2. Pan boundaries expand when convoy is spread out
3. Camera smoothly interpolates constraints during convoy movement
4. Configurable min/max values for zoom and pan

**Architecture**:

- **BP_FC_OverworldConvoy**:
  - CalculateConvoyBounds() method (returns bounding box of all members)
  - OnConvoyBoundsChanged event dispatcher
- **BP_OverworldCamera**:
  - Subscribe to OnConvoyBoundsChanged
  - Adjust spring arm length: Lerp(MinLength, MaxLength, ConvoySpread / MaxSpread)
  - Adjust pan boundaries: Bounds.Extents \* PanBoundaryMultiplier
  - Smooth interpolation (FInterpTo) for camera constraint changes

**Implementation Steps**:

1. Create CalculateConvoyBounds in BP_FC_OverworldConvoy (tick or timer-based)
2. Add OnConvoyBoundsChanged event dispatcher
3. Subscribe camera to event in BeginPlay
4. Implement dynamic spring arm length adjustment
5. Implement dynamic pan boundary adjustment
6. Add smoothing/interpolation for constraint changes
7. Test with convoy in various formations and states

**Modularity Notes**:

- Event-driven design (convoy broadcasts bounds, camera reacts)
- Camera logic independent of convoy implementation
- Configuration variables for min/max constraints and interpolation speed

---

### Backlog Item 3: Convoy Member Differentiation

**Priority**: Low (Week 5+)

**Description**: Visual and functional differentiation of convoy members (leader, guard, supply wagon, etc.) with unique meshes and properties.

**Requirements**:

1. Convoy member types: Leader, Guard, Supply Wagon, Scout
2. Each type has unique mesh and material
3. Different movement speeds and collision sizes per type
4. POI overlap detection may vary by type (e.g., scouts detect POIs from farther away)

**Architecture**:

- **EConvoyMemberType** enum: Leader, Guard, SupplyWagon, Scout
- **BP_FC_ConvoyMember**:
  - MemberType variable (EConvoyMemberType)
  - Struct for member properties (speed, collision size, POI detection radius)
  - ApplyMemberTypeSettings() method (called in BeginPlay)
- **BP_FC_OverworldConvoy**:
  - Convoy composition array: TArray<EConvoyMemberType> (e.g., [Leader, Guard, SupplyWagon])
  - Spawn members based on composition array

**Implementation Steps**:

1. Create EConvoyMemberType enum
2. Create FConvoyMemberProperties struct (speed, size, detection radius, mesh ref)
3. Add MemberType variable and properties map to BP_FC_ConvoyMember
4. Implement ApplyMemberTypeSettings method
5. Update BP_FC_OverworldConvoy Construction Script to spawn typed members
6. Create unique meshes/materials for each type
7. Test convoy with mixed member types

**Modularity Notes**:

- Data-driven design (properties in struct/map)
- Easy to add new member types (extend enum + add properties)
- Composition defined in convoy parent (flexible for different convoy types)

---

**End of Backlog Items**

---

**END OF 0003-TASKS_TEMP.MD**
