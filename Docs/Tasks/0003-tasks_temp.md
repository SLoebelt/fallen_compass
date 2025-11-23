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

- [x] **Analysis**

  - [x] POI overlap aggregation already implemented in C++ AOverworldConvoy
  - [x] NotifyPOIOverlap() method callable from AConvoyMember
  - [x] OnConvoyPOIOverlap event dispatcher already exposed to Blueprint
  - [x] This step verifies functionality only

- [x] **Verification (Unreal Editor)**

  - [x] Open BP_FC_OverworldConvoy
  - [x] Verify NotifyPOIOverlap method visible in Blueprint (inherited from C++)
  - [x] Verify OnConvoyPOIOverlap event dispatcher in Event Graph (My Blueprint panel)
  - [x] No additional implementation needed

- [x] **Testing After Step 5.4.3** ✅ CHECKPOINT
  - [x] NotifyPOIOverlap method visible in Blueprint
  - [x] OnConvoyPOIOverlap event dispatcher accessible
  - [x] Blueprint compiles without errors

**COMMIT POINT 5.4.3**: N/A (functionality already in C++ base class)

---

### Step 5.5: Implement Click-to-Move for Convoy Leader

#### Step 5.5.1: Add Click-to-Move Handler to AFCPlayerController

- [x] **Analysis**

  - [x] Player controller detects left-click on terrain
  - [x] Raycasts from mouse position to world
  - [x] Sends move command to convoy leader's AI controller
  - [x] Uses existing HandleClick routing for TopDown mode

- [x] **Implementation (FCPlayerController.h)**

  - [x] Open `Source/FC/Core/FCPlayerController.h`
  - [x] Add forward declarations for AFCOverworldConvoy and AFCConvoyMember
  - [x] Add PossessedConvoy property (AFCOverworldConvoy\*)
  - [x] Add HandleOverworldClickMove() method declaration
  - [x] Save file

- [x] **Implementation (FCPlayerController.cpp)**

  - [x] Open `Source/FC/Core/FCPlayerController.cpp`
  - [x] Add include for FCConvoyMember.h
  - [x] Update BeginPlay to find convoy in level:

    ```cpp
    // Find convoy in level if we're in Overworld
    TArray<AActor*> FoundConvoys;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFCOverworldConvoy::StaticClass(), FoundConvoys);

    if (FoundConvoys.Num() > 0)
    {
        PossessedConvoy = Cast<AFCOverworldConvoy>(FoundConvoys[0]);
        UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Found convoy: %s"), *PossessedConvoy->GetName());
    }
    ```

  - [x] Update HandleClick to route TopDown clicks:
    ```cpp
    if (CurrentMode == EFCPlayerCameraMode::TopDown)
    {
        HandleOverworldClickMove();
    }
    ```
  - [x] Implement HandleOverworldClickMove:
    - [x] Raycast from cursor using GetHitResultUnderCursor
    - [x] Get convoy leader via PossessedConvoy->GetLeaderMember()
    - [x] Get leader's AI controller
    - [x] Project hit location to NavMesh
    - [x] Call AIController->MoveToLocation()
    - [x] Add logging and visual feedback
  - [x] Save file

- [x] **Compilation**

  - [x] Build solution in Visual Studio
  - [x] Verify no compilation errors
  - [x] Check includes and forward declarations

- [x] **Testing After Step 5.5.1** ✅ CHECKPOINT
  - [x] Compilation succeeds
  - [x] HandleOverworldClickMove method implemented
  - [x] Can open Unreal Editor
  - [x] PIE tested with convoy in Overworld level
  - [x] Left-click on ground moves leader to clicked location
  - [x] NavMesh projection works (only accepts valid paths)
  - [x] Visual feedback displays target location on screen (green text, 2s)
  - [x] Output log confirms: "HandleOverworldClickMove: Moving convoy to [location]"
  - [x] Camera follows leader smoothly during movement
  - [x] AI controller navigation working perfectly

**IMPLEMENTATION NOTES:**

- Used existing HandleClick routing instead of separate ClickMoveAction
- Convoy finding happens in BeginPlay (logs warning if not found - expected in Office)
- NavMesh projection ensures valid pathfinding destinations
- Visual feedback shows target location on screen for 2 seconds
- Leader movement fully functional with AIController->MoveToLocation()

**COMMIT POINT 5.5.1**: `git add Source/FC/Core/FCPlayerController.h Source/FC/Core/FCPlayerController.cpp && git commit -m "feat(convoy): Implement click-to-move for convoy leader in player controller"`

---

#### Step 5.5.2: Verify GetLeaderMember Method (Already in C++) - OBSOLETE

- [x] **Analysis**

  - [x] GetLeaderMember() method already implemented in C++ AFCOverworldConvoy
  - [x] Marked as BlueprintCallable, accessible from both C++ and Blueprint
  - [x] Returns AFCConvoyMember\* (leader member reference)
  - [x] Used successfully in Step 5.5.1 implementation

- [x] **Verification (Unreal Editor)**

  - [x] GetLeaderMember method called in HandleOverworldClickMove()
  - [x] Returns valid AFCConvoyMember reference
  - [x] C++ implementation working correctly

- [x] **Testing After Step 5.5.2** ✅ CHECKPOINT
  - [x] GetLeaderMember method functional in C++ PlayerController
  - [x] Returns valid AFCConvoyMember reference
  - [x] No Blueprint verification needed

**OBSOLETE NOTE**: This step was marked as "verification only" and GetLeaderMember() was successfully used in Step 5.5.1 C++ implementation. No additional Blueprint testing required.

**COMMIT POINT 5.5.2**: N/A (functionality already in C++ base class, verified in 5.5.1)

---

#### Step 5.5.3: Configure ClickMoveAction in BP_FC_PlayerController - OBSOLETE

- [x] **Analysis**

  - [x] Original plan: Create separate ClickMoveAction property
  - [x] Actual implementation: Reused existing ClickAction and HandleClick routing
  - [x] HandleClick now routes TopDown clicks to HandleOverworldClickMove()
  - [x] Simpler architecture, fewer input actions to manage

- [x] **Implementation (C++ - Completed in 5.5.1)**

  - [x] No Blueprint configuration needed
  - [x] Existing ClickAction already bound to IA_Interact in IMC_FC_TopDown
  - [x] HandleClick checks camera mode and routes to appropriate handler
  - [x] TopDown mode → HandleOverworldClickMove()
  - [x] TableView/FirstPerson mode → HandleTableObjectClick()

- [x] **Testing After Step 5.5.3** ✅ CHECKPOINT
  - [x] Click routing working in PIE
  - [x] No separate ClickMoveAction property needed
  - [x] Existing input bindings sufficient

**OBSOLETE NOTE**: This step is no longer needed. Step 5.5.1 implementation reused the existing ClickAction and HandleClick routing instead of creating a separate ClickMoveAction property. This simplifies the input system and reduces redundancy.

**COMMIT POINT 5.5.3**: N/A (obsolete - functionality implemented in 5.5.1)

---

### Step 5.6: Integrate Camera with Convoy

#### Step 5.6.1: Attach BP_OverworldCamera to Convoy

- [x] **Analysis**

  - [x] Camera should follow convoy's CameraAttachPoint
  - [x] Spring arm and pan constraints bound to convoy center
  - [x] Camera remains smooth during movement

- [x] **Implementation (UFCCameraManager - Option C)**

  - [x] **Actual Implementation: C++ in UFCCameraManager::BlendToTopDown()**
    - [x] Added `GetCameraAttachPoint()` method to `AFCOverworldConvoy.h`
    - [x] Modified `UFCCameraManager::BlendToTopDown()` in `FCCameraManager.cpp`:
      - [x] Find convoy in level using `GetAllActorsOfClass` with name pattern matching
      - [x] Call convoy's `GetCameraAttachPoint()` via reflection
      - [x] Attach OverworldCamera to CameraAttachPoint using `AttachToComponent`
      - [x] Uses `SnapToTargetNotIncludingScale` attachment rule
      - [x] Logs success/warning messages for debugging
    - [x] Camera automatically attaches when transitioning to TopDown mode
    - [x] No Blueprint changes needed - pure C++ solution

- [x] **Testing After Step 5.6.1** ✅ CHECKPOINT
  - [x] Camera attaches to convoy's CameraAttachPoint during TopDown mode transition
  - [x] Camera follows convoy smoothly when leader moves via click-to-move
  - [x] No jittering or offset issues observed
  - [x] Attachment logged in output: "Attached camera to convoy's CameraAttachPoint"

**IMPLEMENTATION NOTES**:

- Chose C++ implementation over Blueprint for consistency with existing camera system
- Attachment occurs in `BlendToTopDown()` to ensure camera is attached before view transition
- Uses reflection to call `GetCameraAttachPoint()` to avoid circular dependencies
- Gracefully handles missing convoy (logs message, continues without error)

**COMMIT POINT 5.6.1**: `git add Source/FC/Characters/Convoy/FCOverworldConvoy.h Source/FC/Components/FCCameraManager.cpp && git commit -m "feat(convoy): Attach camera to convoy CameraAttachPoint in BlendToTopDown"`

---

#### Step 5.6.2: Bind Camera Constraints to Convoy (Prototype Scope)

- [x] **Analysis**

  - [x] PROTOTYPE SCOPE: Basic attachment sufficient for Week 3
  - [x] FUTURE: Spring arm length and pan boundaries based on convoy size
  - [x] BACKLOG: Implement dynamic camera constraint system

- [x] **Implementation (Week 3 Prototype)**

  - [x] Verified camera attachment working in Step 5.6.1
  - [x] Using fixed spring arm length (from Task 3)
  - [x] Pan constraints remain level-based (not convoy-based)
  - [x] Limitation documented - future enhancement for dynamic constraints

- [x] **Testing After Step 5.6.2** ✅ CHECKPOINT
  - [x] Camera follows convoy with fixed constraints
  - [x] Pan and zoom work as expected via WASD and mouse wheel
  - [x] Attachment stable during movement

**PROTOTYPE SCOPE NOTE**: Dynamic camera constraints based on convoy size/position deferred to future tasks. Current implementation uses fixed spring arm and level-based pan boundaries, which is sufficient for Week 3 prototype.

**COMMIT POINT 5.6.2**: N/A (prototype scope documented, no code changes)

---

### Step 5.7: Place BP_FC_OverworldConvoy in L_Overworld

#### Step 5.7.1: Add Convoy to Level and Configure

- [x] **Analysis**

  - [x] Convoy spawns at PlayerStart location
  - [x] Members auto-spawn via Construction Script
  - [x] Camera auto-attaches via BeginPlay

- [x] **Implementation (Unreal Editor)**

  - [x] Open L_Overworld level
  - [x] **Option A: Manual Placement**
    - [x] Drag BP_FC_OverworldConvoy from Content Browser into viewport
    - [x] Position at PlayerStart location (X=0, Y=0, Z=100)
  - [x] **Option B: Default Pawn Class (Recommended if convoy needs controller possession)**
    - [x] Window → World Settings
    - [x] Game Mode → Default Pawn Class: BP_FC_OverworldConvoy
    - [x] NOTE: This may conflict with controller not possessing pawn directly
    - [x] For Week 3: Use Option A (manual placement, controller references convoy)
  - [x] Save level

- [x] **Testing After Step 5.7.1** ✅ CHECKPOINT
  - [x] Convoy visible in level with 3 members
  - [x] Members positioned correctly (leader in front, followers behind)
  - [x] PIE: Convoy members spawn and attach
  - [x] Camera attaches to convoy
  - [x] Level saves without errors

**COMMIT POINT 5.7.1**: `git add Content/FC/World/Levels/L_Overworld.umap && git commit -m "feat(convoy): Place BP_FC_OverworldConvoy in L_Overworld"`

---

### Step 5.8: Test Click-to-Move and Camera Following

#### Step 5.8.1: Full Convoy Verification

- [x] **Analysis**

  - [x] Test click-to-move with convoy leader
  - [x] Verify followers remain stationary (breadcrumb system not yet implemented)
  - [x] Test camera following convoy during movement
  - [x] Verify camera pan/zoom work while moving

- [x] **Test Sequence**

  - [x] Open L_Overworld in editor
  - [x] PIE (Play In Editor)
  - [x] Verify convoy spawns with 3 members
  - [x] Verify camera attached to convoy (top-down view follows convoy)
  - [x] Press `P` to visualize NavMesh (green overlay)
  - [x] **Test Click-to-Move**:
    - [x] Left-click on ground (green NavMesh area)
    - [x] Verify leader moves to clicked location
    - [x] Check Output Log: "Moving convoy to: [location]"
    - [x] Verify camera follows leader smoothly
    - [x] NOTE: Followers remain stationary (breadcrumb system not yet implemented)
  - [x] **Test Multiple Clicks**:
    - [x] Click different locations rapidly
    - [x] Verify leader updates path correctly
  - [x] **Test Camera Pan/Zoom During Movement**:
    - [x] While leader is moving, use WASD to pan camera
    - [x] Verify camera controls still work (camera moves relative to convoy)
    - [x] Use mouse wheel to zoom
    - [x] Verify zoom works smoothly

- [x] **Testing After Step 5.8.1** ✅ CHECKPOINT
  - [x] Leader moves to clicked locations via NavMesh
  - [x] Camera follows convoy smoothly (attached to CameraAttachPoint)
  - [x] Camera pan (WASD) and zoom (mouse wheel) work during movement
  - [x] Output logs confirm movement commands
  - [x] No "Accessed None" errors
  - [x] Followers stationary (expected - breadcrumb system deferred to backlog)

**TEST RESULTS**:

- Click-to-move fully functional with NavMesh pathfinding
- Camera attachment to convoy working perfectly
- Camera follows leader smoothly without jitter
- Pan and zoom controls responsive during movement
- No POI actors placed yet (deferred to Task 6)

**COMMIT POINT 5.8.1**: `git add -A && git commit -m "test(convoy): Verify click-to-move, camera following, and pan/zoom during movement"`

---

### Step 5.9: Document Follower Breadcrumb System as Backlog

- [x] **Analysis**

  - [x] Breadcrumb following system deferred to future sprint
  - [x] Document requirements and architecture for later implementation
  - [x] Create backlog item at end of file

- [x] **Documentation** (see backlog section at end of file)
  - [x] Backlog item created: "Convoy Follower Breadcrumb System" (Backlog Item 1, lines 1437-1457)
  - [x] Includes architecture notes and implementation steps
  - [x] Priority set to Medium (Week 4-5)

**COMMIT POINT 5.9**: N/A (backlog documentation already exists at end of file)

---

### Task 5 Acceptance Criteria

- [x] Left Mouse Button bound to IA_Interact in IMC_FC_TopDown
- [x] BP_FC_ConvoyMember character created with mesh, collision, AI controller support, and POI overlap detection
- [x] BP_FC_ConvoyAIController created with MoveTo method
- [x] BP_FC_OverworldConvoy parent actor created with 3 child convoy members (leader + 2 followers)
- [x] Convoy members spawn and attach correctly ~~via Construction Script~~ **via BeginPlay** (moved to avoid PIE destruction)
- [x] POI overlap aggregation implemented in BP_FC_OverworldConvoy (C++ base class)
- [x] AFCPlayerController implements click-to-move for convoy leader
- [x] ~~BP_OverworldCamera attached to convoy's CameraAttachPoint~~ **UFCCameraManager automatically attaches camera in BlendToTopDown()**
- [x] Camera follows convoy smoothly during movement
- [x] Convoy placed in L_Overworld at PlayerStart
- [x] Left-click moves convoy leader to location using NavMesh pathfinding
- [x] POI overlap triggers on-screen message and log output (tested with class name pattern matching)
- [x] Camera pan/zoom works while convoy moves
- [x] No compilation errors or runtime crashes
- [x] Follower movement deferred to backlog (breadcrumb system - Backlog Item 1, Week 4-5)

**IMPLEMENTATION NOTES**:

- **C++ base classes**: AFCConvoyMember, AFCOverworldConvoy with Blueprint children for visual config
- **Spawning**: Moved from OnConstruction to BeginPlay to survive PIE transition
- **Camera attachment**: Automatic in UFCCameraManager::BlendToTopDown() using reflection to call GetCameraAttachPoint()
- **Input routing**: Reused existing ClickAction binding, routed via HandleClick() based on camera mode
- **POI detection**: Capsule overlap with class name pattern matching (will use BPI_InteractablePOI in Task 6)

**Task 5 complete. Ready for Task 6 sub-tasks (POI Actor & Interaction Stub).**

---

## Updated Task 6: POI Actor & Right-Click Interaction Stub

**Purpose**: Create BP_OverworldPOI actor with right-click interaction that logs to console (stub for future implementation). POI overlap detection already implemented in convoy system.

---

### Step 6.1: Verify Input Action for POI Interaction

#### Step 6.1.1: Verify IA_Interact Input Action (OBSOLETE - Already Exists)

- [x] **Analysis**

  - [x] POI interaction uses right-click via existing IA_Interact action
  - [x] IA_Interact already exists at `/Game/FC/Input/IA_Interact`
  - [x] Currently not used for any other functionality in TopDown mode
  - [x] Will reuse instead of creating separate IA_InteractPOI

- [x] **Verification**

  - [x] IA_Interact exists and is Digital (bool) type
  - [x] Available for POI interaction binding

- [x] **Testing After Step 6.1.1** ✅ CHECKPOINT
  - [x] IA_Interact confirmed to exist
  - [x] No conflicts with other TopDown functionality

**OBSOLETE NOTE**: Step originally planned to create IA_InteractPOI, but we're reusing existing IA_Interact instead to simplify input system.

**COMMIT POINT 6.1.1**: N/A (using existing asset)

---

#### Step 6.1.2: Verify Right Mouse Button Binding in IMC_FC_TopDown (OBSOLETE - Already Bound)

- [x] **Analysis**

  - [x] Right Mouse Button already bound to IA_Interact in IMC_FC_TopDown
  - [x] No additional binding needed

- [x] **Verification**

  - [x] IMC_FC_TopDown already has IA_Interact mapped to Right Mouse Button
  - [x] Binding ready for POI interaction implementation

- [x] **Testing After Step 6.1.2** ✅ CHECKPOINT
  - [x] Right Mouse Button bound to IA_Interact in TopDown context
  - [x] IMC_FC_TopDown has required mappings (Pan, Zoom, ClickMove, Interact)
  - [x] No changes needed

**OBSOLETE NOTE**: Right Mouse Button already bound to IA_Interact in IMC_FC_TopDown mapping context. No additional configuration needed.

**COMMIT POINT 6.1.2**: N/A (binding already exists)

---

### Step 6.2: Create AFCOverworldPOI C++ Base Class

#### Step 6.2.1: Create AFCOverworldPOI C++ Class

- [x] **Analysis**

  - [x] C++ base class for all overworld POI actors (matches convoy architecture)
  - [x] Root component, static mesh, and collision box for raycast/overlap
  - [x] POI name property (FString, EditAnywhere, Instance Editable)
  - [x] Virtual OnPOIInteract() method for Blueprint extensibility
  - [x] Overlap detection setup in constructor
  - [x] Blueprint children configure mesh/materials per POI type

- [x] **Implementation (C++)**

  - [x] **Created Header File**: `Source/FC/World/FCOverworldPOI.h`

    - [x] Component properties: POIRoot, POIMesh, InteractionBox
    - [x] POIName property (EditAnywhere, BlueprintReadWrite)
    - [x] GetPOIName() accessor (BlueprintCallable)
    - [x] OnPOIInteract() virtual method (BlueprintNativeEvent)

  - [x] **Created Source File**: `Source/FC/World/FCOverworldPOI.cpp`

    - [x] Constructor creates component hierarchy
    - [x] POIMesh scaled 2x, no collision
    - [x] InteractionBox: 150x150x100 extent, QueryOnly, blocks Visibility, overlaps all
    - [x] Default POI name: "Unnamed POI"
    - [x] BeginPlay logs POI spawn with name and location
    - [x] OnPOIInteract_Implementation() shows yellow stub message (5s)

  - [x] Compiled successfully via Live Coding

- [x] **Testing After Step 6.2.1** ✅ CHECKPOINT
  - [x] C++ code compiled without errors
  - [x] AFCOverworldPOI visible in Unreal Editor Content Browser
  - [x] Can create Blueprint child class from AFCOverworldPOI

**COMMIT POINT 6.2.1**: `git add Source/FC/World/FCOverworldPOI.h Source/FC/World/FCOverworldPOI.cpp && git commit -m "feat(overworld): Create AFCOverworldPOI C++ base class"`

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Actor.h"
    #include "FCOverworldPOI.generated.h"

    class USceneComponent;
    class UStaticMeshComponent;
    class UBoxComponent;

    DECLARE_LOG_CATEGORY_EXTERN(LogFCOverworldPOI, Log, All);

    /**
     * AFCOverworldPOI - Base class for overworld Points of Interest
     *
     * Provides collision for mouse raycast detection and convoy overlap.
     * Blueprint children configure specific mesh, materials, and POI names.
     */
    UCLASS()
    class FC_API AFCOverworldPOI : public AActor
    {
        GENERATED_BODY()

    public:
        AFCOverworldPOI();

    protected:
        virtual void BeginPlay() override;

    private:
        /** Root component for POI hierarchy */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        USceneComponent* POIRoot;

        /** Static mesh for visual representation */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        UStaticMeshComponent* POIMesh;

        /** Collision box for mouse raycast and convoy overlap detection */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        UBoxComponent* InteractionBox;

        /** Display name for this Point of Interest */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        FString POIName;

    public:
        /** Get POI display name */
        UFUNCTION(BlueprintCallable, Category = "FC|POI")
        FString GetPOIName() const { return POIName; }

        /** Handle POI interaction (stub for Task 6 - will log to console) */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|POI")
        void OnPOIInteract();
    };
    ```

- [ ] **Create Source File**: `Source/FC/World/FCOverworldPOI.cpp`

  ```cpp
  #include "World/FCOverworldPOI.h"
  #include "Components/SceneComponent.h"
  #include "Components/StaticMeshComponent.h"
  #include "Components/BoxComponent.h"

  DEFINE_LOG_CATEGORY(LogFCOverworldPOI);

  AFCOverworldPOI::AFCOverworldPOI()
  {
      PrimaryActorTick.bCanEverTick = false;

      // Create component hierarchy
      POIRoot = CreateDefaultSubobject<USceneComponent>(TEXT("POIRoot"));
      SetRootComponent(POIRoot);

      // Create static mesh component
      POIMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("POIMesh"));
      POIMesh->SetupAttachment(POIRoot);
      POIMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
      POIMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

      // Create interaction box for raycast and overlap
      InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
      InteractionBox->SetupAttachment(POIRoot);
      InteractionBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
      InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
      InteractionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
      InteractionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
      InteractionBox->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
      InteractionBox->SetGenerateOverlapEvents(true);

      // Default POI name
      POIName = TEXT("Unnamed POI");
  }

  void AFCOverworldPOI::BeginPlay()
  {
      Super::BeginPlay();

      UE_LOG(LogFCOverworldPOI, Log, TEXT("POI '%s' spawned at %s"),
          *POIName, *GetActorLocation().ToString());
  }

  void AFCOverworldPOI::OnPOIInteract_Implementation()
  {
      // Stub implementation - logs to console
      UE_LOG(LogFCOverworldPOI, Log, TEXT("POI Interaction: %s"), *POIName);

      if (GEngine)
      {
          GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
              FString::Printf(TEXT("POI Interaction Stub: %s"), *POIName));
      }
  }
  ```

- [x] Save files
- [x] Compile C++ code (Live Coding or full build)

- [x] **Testing After Step 6.2.1** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] AFCOverworldPOI visible in Unreal Editor Content Browser
  - [x] Can create Blueprint child class from AFCOverworldPOI

**COMMIT POINT 6.2.1**: `git add Source/FC/World/FCOverworldPOI.h Source/FC/World/FCOverworldPOI.cpp && git commit -m "feat(overworld): Create AFCOverworldPOI C++ base class"`

---

#### Step 6.2.2: Create BP_FC_OverworldPOI Blueprint Child Class

- [x] **Analysis**

  - [x] Blueprint child of AFCOverworldPOI for visual configuration
  - [x] Configure mesh, materials, and default POI name
  - [x] Can be placed in level and name edited per-instance

- [x] **Implementation (Unreal Editor)**

  - [x] Created Blueprint at `/Game/FC/World/Blueprints/Actors/POI/BP_FC_OverworldPOI`
  - [x] Parent class: AFCOverworldPOI
  - [x] Components Panel:
    - [x] POIMesh component configured with static mesh
    - [x] Material assigned (visible from top-down camera)
    - [x] Component hierarchy inherited from C++ base class
  - [x] Class Defaults:
    - [x] POI Name configured (instance editable in Details panel)
  - [x] Compiled and saved

- [x] **Testing After Step 6.2.2** ✅ CHECKPOINT
  - [x] Blueprint compiles without errors
  - [x] Components hierarchy inherited from C++ base class
  - [x] POIMesh has mesh and material assigned
  - [x] Placed in L_Overworld level successfully
  - [x] POI Name editable per-instance in Details panel
  - [x] Convoy detects POI overlap (logs "Convoy detected POI: [name]")

**IMPLEMENTATION NOTE**: Convoy overlap detection already working via AFCConvoyMember capsule overlap with InteractionBox. Class name pattern matching successfully detects POI actors.

**COMMIT POINT 6.2.2**: `git add Content/FC/World/Blueprints/Actors/POI/BP_FC_OverworldPOI.uasset && git commit -m "feat(overworld): Create BP_FC_OverworldPOI Blueprint child with mesh config and test in L_Overworld"`

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

#### Step 6.4.1: Add POI Interaction Handler to AFCPlayerController

- [ ] **Analysis**

  - [ ] Controller needs to handle IA_InteractPOI input
  - [ ] On right-click: Raycast from mouse position, check if hit actor implements interface
  - [ ] If valid POI: Call OnPOIInteract() interface method

- [ ] **Implementation (FCPlayerController.h)**

  - [ ] Open `Source/FC/Core/FCPlayerController.h`
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

- [ ] **Implementation (FCPlayerController.cpp)**

  - [ ] Update SetupInputComponent():

    ```cpp
    void AFCPlayerController::SetupInputComponent()
    {
        Super::SetupInputComponent();

        / Existing click-to-move binding...

        / Bind POI interaction action
        if (EnhancedInput && InteractPOIAction)
        {
            EnhancedInput->BindAction(InteractPOIAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleInteractPOI);
            UE_LOG(LogFCOverworldController, Log, TEXT("SetupInputComponent: Bound InteractPOIAction"));
        }
    }
    ```

  - [ ] Implement HandleInteractPOI():

    ```cpp
    void AFCPlayerController::HandleInteractPOI()
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

**COMMIT POINT 6.4.1**: `git add Source/FC/Core/FCPlayerController.h Source/FC/Core/FCPlayerController.cpp && git commit -m "feat(overworld): Implement POI right-click interaction handler"`

---

#### Step 6.4.2: Configure InteractPOIAction in BP_FC_PlayerController

- [ ] **Analysis**

  - [ ] InteractPOIAction property needs IA_InteractPOI assigned in Blueprint

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FC_PlayerController
  - [ ] Class Defaults → FC | Input | Actions: - [ ] Set InteractPOIAction: `/Game/FC/Input/Actions
/Game/FC/Input/IA_InteractPOI`
  - [ ] Compile and save

- [ ] **Testing After Step 6.4.2** ✅ CHECKPOINT
  - [ ] InteractPOIAction assigned to IA_InteractPOI
  - [ ] Blueprint compiles without errors

**COMMIT POINT 6.4.2**: `git add Content/FC/Core/BP_FC_PlayerController.uasset && git commit -m "feat(overworld): Assign IA_InteractPOI to InteractPOIAction"`

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
- [ ] AFCPlayerController implements HandleInteractPOI() with raycast and interface check
- [ ] InteractPOIAction assigned to IA_InteractPOI in BP_FC_PlayerController
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
