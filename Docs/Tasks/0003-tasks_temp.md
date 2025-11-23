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

### Step 6.3: Implement IFCInteractablePOI C++ Interface and POI Action System

#### Step 6.3.1: Create IFCInteractablePOI C++ Interface

- [x] **Analysis**

  - [x] C++ interface for POI interaction (matches existing IFCInteractable pattern)
  - [x] POIs can have multiple available actions (talk, ambush, enter, trade, harvest, observe)
  - [x] Interface provides methods to query available actions and execute selected action
  - [x] Enables different POI types with different action sets
  - [x] Integrates with existing UFCInteractionComponent system

- [x] **Architecture Requirements**

  - [x] **Action Selection Logic**:
    - [x] 0 actions → Right-click ignored
    - [x] 1 action → Right-click → convoy moves → overlap executes action automatically
    - [x] 2+ actions → Right-click → action selection widget → click action → convoy moves → overlap executes selected action
  - [x] **Overlap Triggers**:
    - [x] Intentional right-click movement triggers action on overlap
    - [x] Unintentional overlap (exploration, fleeing) also triggers actions
    - [x] If unintentional overlap + multiple actions → show action selection dialog
  - [x] **Enemy Encounters**:
    - [x] Enemies can chase convoy
    - [x] Enemy overlap triggers enemy's intended action (ambush)
  - [x] **Action Types** (extensible enum):
    - [x] Talk, Ambush, Enter, Trade, Harvest, Observe
    - [x] Easy to add new action types in future

- [x] **Implementation (C++)**

  - [x] **Create Header File**: `Source/FC/Interaction/IFCInteractablePOI.h`

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "UObject/Interface.h"
    #include "IFCInteractablePOI.generated.h"

    /**
     * EFCPOIAction - Enum for POI action types
     * Extensible for future action types
     */
    UENUM(BlueprintType)
    enum class EFCPOIAction : uint8
    {
        Talk        UMETA(DisplayName = "Talk"),
        Ambush      UMETA(DisplayName = "Ambush"),
        Enter       UMETA(DisplayName = "Enter"),
        Trade       UMETA(DisplayName = "Trade"),
        Harvest     UMETA(DisplayName = "Harvest"),
        Observe     UMETA(DisplayName = "Observe")
    };

    /**
     * FFCPOIActionData - Struct containing action display information
     */
    USTRUCT(BlueprintType)
    struct FFCPOIActionData
    {
        GENERATED_BODY()

        /** Action type */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EFCPOIAction ActionType;

        /** Display text for action button (e.g., "Talk to Merchant") */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FText ActionText;

        /** Optional icon for action (future UI) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTexture2D* ActionIcon = nullptr;

        FFCPOIActionData()
            : ActionType(EFCPOIAction::Talk)
            , ActionText(FText::FromString(TEXT("Interact")))
        {}
    };

    // UInterface class (required by Unreal)
    UINTERFACE(MinimalAPI, Blueprintable)
    class UIFCInteractablePOI : public UInterface
    {
        GENERATED_BODY()
    };

    /**
     * IIFCInteractablePOI - Interface for overworld POI interaction
     * Provides action-based interaction system for POIs
     */
    class FC_API IIFCInteractablePOI
    {
        GENERATED_BODY()

    public:
        /**
         * Get all available actions for this POI
         * Called when right-clicking POI or on convoy overlap
         * @return Array of available actions with display data
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        TArray<FFCPOIActionData> GetAvailableActions() const;

        /**
         * Execute a specific action on this POI
         * Called when action is selected (or auto-executed if only one action)
         * @param Action - The action to execute
         * @param Interactor - The actor performing the action (typically convoy)
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        void ExecuteAction(EFCPOIAction Action, AActor* Interactor);

        /**
         * Get POI display name for UI and logging
         * @return Display name (e.g., "Village", "Ruins", "Enemy Camp")
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        FString GetPOIName() const;

        /**
         * Check if action can be executed (conditions, requirements)
         * Optional - default implementation returns true
         * @param Action - Action to check
         * @param Interactor - Actor attempting action
         * @return true if action can be executed
         */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|POI")
        bool CanExecuteAction(EFCPOIAction Action, AActor* Interactor) const;
    };
    ```

  - [x] **Create Source File**: `Source/FC/Interaction/IFCInteractablePOI.cpp`

    ```cpp
    #include "Interaction/IFCInteractablePOI.h"

    // Interface default implementations are in the header file
    // This file is intentionally minimal for interfaces
    ```

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.3.1** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] IFCInteractablePOI interface available in C++ and Blueprint
  - [x] EFCPOIAction enum visible in Blueprint

**COMMIT POINT 6.3.1**: `git add Source/FC/Interaction/IFCInteractablePOI.h Source/FC/Interaction/IFCInteractablePOI.cpp && git commit -m "feat(overworld): Create IFCInteractablePOI C++ interface with action system"`---

#### Step 6.3.2: Implement IFCInteractablePOI in AFCOverworldPOI

- [x] **Analysis**

  - [x] AFCOverworldPOI implements IFCInteractablePOI interface in C++
  - [x] Add available actions array (EditAnywhere for Blueprint configuration)
  - [x] Implement interface methods
  - [x] Blueprint children can override actions per POI type

- [x] **Implementation (C++)**

  - [x] **Update FCOverworldPOI.h**:

    ```cpp
    #pragma once

    #include "CoreMinimal.h"
    #include "GameFramework/Actor.h"
    #include "Interaction/IFCInteractablePOI.h"  // Add interface include
    #include "FCOverworldPOI.generated.h"

    // ... existing forward declarations ...

    /**
     * AFCOverworldPOI - Base class for overworld Points of Interest
     * Provides collision for mouse raycast detection and convoy overlap.
     * Blueprint children configure specific mesh, materials, and POI names.
     * Implements IFCInteractablePOI for action-based interaction
     */
    UCLASS()
    class FC_API AFCOverworldPOI : public AActor, public IIFCInteractablePOI
    {
        GENERATED_BODY()

    public:
        AFCOverworldPOI();

    protected:
        virtual void BeginPlay() override;

    private:
        // ... existing component properties ...

        /** Display name for this Point of Interest */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI", meta = (AllowPrivateAccess = "true"))
        FString POIName;

        /** Available actions for this POI (configured in Blueprint) */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FC|POI|Actions", meta = (AllowPrivateAccess = "true"))
        TArray<FFCPOIActionData> AvailableActions;

    public:
        // IFCInteractablePOI interface implementation
        virtual TArray<FFCPOIActionData> GetAvailableActions_Implementation() const override;
        virtual void ExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) override;
        virtual FString GetPOIName_Implementation() const override;
        virtual bool CanExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) const override;

        /** DEPRECATED: Old stub method - replaced by ExecuteAction() */
        UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "FC|POI")
        void OnPOIInteract();
    };
    ```

  - [x] **Update FCOverworldPOI.cpp**:

    ```cpp
    #include "World/FCOverworldPOI.h"
    // ... existing includes ...

    AFCOverworldPOI::AFCOverworldPOI()
    {
        // ... existing constructor code ...

        // Default POI name
        POIName = TEXT("Unnamed POI");

        // Default: no actions (must be configured in Blueprint)
        AvailableActions.Empty();
    }

    // ... existing BeginPlay ...

    // IFCInteractablePOI interface implementation
    TArray<FFCPOIActionData> AFCOverworldPOI::GetAvailableActions_Implementation() const
    {
        return AvailableActions;
    }

    void AFCOverworldPOI::ExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor)
    {
        // Stub implementation - log action
        UE_LOG(LogFCOverworldPOI, Log, TEXT("POI '%s': Executing action %s"),
            *POIName, *UEnum::GetValueAsString(Action));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
                FString::Printf(TEXT("POI '%s': Action '%s' executed (STUB)"),
                    *POIName, *UEnum::GetValueAsString(Action)));
        }

        // Future: Implement actual action logic (open dialog, start trade, etc.)
    }

    FString AFCOverworldPOI::GetPOIName_Implementation() const
    {
        return POIName;
    }

    bool AFCOverworldPOI::CanExecuteAction_Implementation(EFCPOIAction Action, AActor* Interactor) const
    {
        // Default: all actions allowed
        // Override in Blueprint for quest requirements, locked doors, etc.
        return true;
    }

    // Keep old OnPOIInteract for backward compatibility
    void AFCOverworldPOI::OnPOIInteract_Implementation()
    {
        UE_LOG(LogFCOverworldPOI, Warning, TEXT("POI '%s': OnPOIInteract is deprecated, use ExecuteAction()"), *POIName);
    }
    ```

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.3.2** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] AFCOverworldPOI implements IFCInteractablePOI
  - [x] Available Actions array visible in Blueprint Details panel

**COMMIT POINT 6.3.2**: `git add Source/FC/World/FCOverworldPOI.h Source/FC/World/FCOverworldPOI.cpp && git commit -m "feat(overworld): Implement IFCInteractablePOI in AFCOverworldPOI with action system"`

---

#### Step 6.3.3: Configure Actions in BP_FC_OverworldPOI

- [x] **Analysis**

  - [x] Configure available actions in Blueprint for testing
  - [x] Create test POI with single action (auto-execute on overlap)
  - [x] Create test POI with multiple actions (show selection dialog)

- [x] **Implementation (Unreal Editor)**

  - [x] Open BP_FC_OverworldPOI
  - [x] Class Defaults → FC | POI | Actions:
    - [x] Available Actions: Add 2 elements for testing
      - [x] Element 0:
        - [x] Action Type: Talk
        - [x] Action Text: "Talk to Merchant"
      - [x] Element 1:
        - [x] Action Type: Trade
        - [x] Action Text: "Open Trade Menu"
  - [x] Compile and save
  - [x] Create variant BP_FC_OverworldPOI_Village:
    - [x] Duplicate BP_FC_OverworldPOI
    - [x] Rename to BP_FC_OverworldPOI_Village
    - [x] POI Name: "Village"
    - [x] Available Actions: Keep 2 actions (Talk, Trade)
  - [x] Create variant BP_FC_OverworldPOI_Enemy:
    - [x] Duplicate BP_FC_OverworldPOI
    - [x] Rename to BP_FC_OverworldPOI_Enemy
    - [x] POI Name: "Enemy Camp"
    - [x] Available Actions: 1 action only
      - [x] Action Type: Ambush
      - [x] Action Text: "Ambush!"

- [x] **Testing After Step 6.3.3** ✅ CHECKPOINT
  - [x] BP_FC_OverworldPOI has 2 actions configured
  - [x] Variant Blueprints created with different action sets
  - [x] All Blueprints compile without errors

**COMMIT POINT 6.3.3**: `git add Content/FC/World/Blueprints/Actors/POI/BP_FC_OverworldPOI*.uasset && git commit -m "feat(overworld): Configure POI actions in Blueprint variants"`

---

### Step 6.4: Implement POI Interaction with Multi-Action Selection

#### Step 6.4.1: Create POI Action Selection Widget (Blueprint)

- [ ] **Analysis**

  - [ ] Widget displays available actions when POI has multiple options
  - [x] User clicks action button → stores selection → initiates convoy movement
  - [x] Widget shows when right-clicking multi-action POI or on unintentional overlap
  - [x] Blueprint implementation for UI flexibility

- [x] **Implementation (Unreal Editor - UMG Widget)**

  - [x] Content Browser → `/Game/FC/UI/Menus/ActionMenu`
  - [x] Right-click → User Interface → Widget Blueprint
  - [x] Name: `WBP_ActionSelection`
  - [x] Open WBP_ActionSelection
  - [x] **Widget Hierarchy**:
    - [x] Canvas Panel (root)
      - [x] Overlay (center screen with auto-size)
        - [x] Border (background panel)
          - [x] Vertical Box
            - [x] Text Block (header: "Select Action")
            - [x] Scroll Box (action list container)
  - [x] **Widget Variables**:
    - [x] `AvailableActions` (TArray<FFCPOIActionData>, Instance Editable)
    - [x] `SelectedAction` (EFCPOIAction)
    - [x] `TargetPOI` (AActor, instance ref)
    - [x] `OnActionSelected` (Event Dispatcher with EFCPOIAction parameter)
  - [x] **Graph: PopulateActions()**
    - [x] Input: AvailableActions array
    - [x] ForEach loop through actions:
      - [x] Create WBP_POIActionButton widget
      - [x] Set button text to ActionData.ActionText
      - [x] Bind button click to OnActionButtonClicked(ActionType)
      - [x] Add button to Scroll Box
  - [x] **Graph: OnActionButtonClicked(EFCPOIAction Action)**
    - [x] Set SelectedAction = Action
    - [x] Call OnActionSelected dispatcher
    - [x] Remove widget from viewport
  - [x] Compile and save

- [x] **Implementation (WBP_POIActionButton child widget)**

  - [x] Create Widget Blueprint: `WBP_POIActionButton`
  - [x] Hierarchy:
    - [x] Button (root)
      - [x] Text Block (action text)
  - [x] Variables:
    - [x] `ActionType` (EFCPOIAction)
    - [x] `ActionText` (Text)
    - [x] `OnClicked` (Event Dispatcher with EFCPOIAction parameter)
  - [x] Event OnClicked (button):
    - [x] Call OnClicked dispatcher with ActionType
  - [x] Compile and save

- [x] **Testing After Step 6.4.1** ✅ CHECKPOINT
  - [x] Widgets created without errors
  - [x] Action selection widget compiles
  - [x] Event dispatchers configured correctly

**COMMIT POINT 6.4.1**: `git add Content/FC/UI/Widgets/WBP_ActionSelection.uasset Content/FC/UI/Widgets/WBP_POIActionButton.uasset && git commit -m "feat(overworld): Create POI action selection widget"`

---

#### Step 6.4.2: Add POI Right-Click Handler with Component Architecture

- [x] **Analysis**

  - [x] Raycast on right-click to detect POI actors
  - [x] Query available actions via IFCInteractablePOI interface
  - [x] Action logic:
    - [x] 0 actions → ignore click (no interaction)
    - [x] 1 action → auto-execute immediately
    - [x] 2+ actions → show action selection widget via UIManager
  - [x] Store pending action and target POI for overlap execution
  - [x] **Architecture**: UFCInteractionComponent handles interaction logic, UFCUIManager manages widgets, PlayerController routes input

- [x] **Implementation (C++ - UFCInteractionComponent)**

  - [x] **Update FCInteractionComponent.h**:

    - [x] Add IFCInteractablePOI include
    - [x] Add PendingInteractionPOI, PendingInteractionAction, bHasPendingPOIInteraction properties
    - [x] Add HandlePOIClick(), OnPOIActionSelected(), NotifyPOIOverlap() method declarations
    - [x] Methods marked BlueprintCallable for widget integration

  - [x] **Update FCInteractionComponent.cpp**:
    - [x] Add IFCInteractablePOI and UFCUIManager includes
    - [x] Implement HandlePOIClick() with action count logic:
      - [x] Query IFCInteractablePOI::GetAvailableActions()
      - [x] 0 actions: ignore
      - [x] 1 action: auto-select and store as pending
      - [x] 2+ actions: request UIManager->ShowPOIActionSelection()
    - [x] Implement OnPOIActionSelected() callback to store user selection
    - [x] Implement NotifyPOIOverlap() with intentional/unintentional overlap handling:
      - [x] Intentional overlap (pending action): execute stored action, close widget
      - [x] Unintentional overlap: query actions, auto-execute if 1 action, show selection if 2+

- [x] **Implementation (C++ - UFCUIManager)**

  - [x] **Update FCUIManager.h**:

    - [x] Add IFCInteractablePOI include for FFCPOIActionData
    - [x] Add POIActionSelectionWidgetClass property
    - [x] Add CurrentPOIActionSelectionWidget instance property
    - [x] Add ShowPOIActionSelection(), ClosePOIActionSelection() method declarations
    - [x] Add getter methods: GetCurrentPOIActionSelectionWidget(), IsPOIActionSelectionOpen()

  - [x] **Update FCUIManager.cpp**:

    - [x] Implement ShowPOIActionSelection(Actions, Component):
      - [x] Validate POIActionSelectionWidgetClass configured
      - [x] Close existing widget if open
      - [x] Create widget instance, add to viewport
      - [x] Returns widget instance for Blueprint to populate actions
    - [x] Implement ClosePOIActionSelection() to remove widget and clear reference

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.4.2** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] UFCInteractionComponent has POI interaction methods
  - [x] UFCUIManager has widget lifecycle methods
  - [x] Architecture properly separates concerns (Component = logic, UIManager = widgets, Controller = routing)

**COMMIT POINT 6.4.2**: `git add Source/FC/Interaction/FCInteractionComponent.h Source/FC/Interaction/FCInteractionComponent.cpp Source/FC/Core/FCUIManager.h Source/FC/Core/FCUIManager.cpp && git commit -m "feat(overworld): Implement POI interaction with component architecture"`

---

#### Step 6.4.3: Update PlayerController to Delegate POI Interactions

- [x] **Analysis**

  - [x] PlayerController routes right-click to UFCInteractionComponent
  - [x] Remove bloated HandleInteractPOI implementation (196 lines)
  - [x] Delegation pattern: Controller→Component→UIManager
  - [x] Renamed MoveConvoyToPOI to MoveConvoyToLocation (takes FVector instead of AActor)

- [x] **Implementation (C++)**

  - [x] **Update FCPlayerController.h**:

    - [x] Remove POIActionSelectionWidgetClass property (now in UIManager)
    - [x] Remove POIActionSelectionWidget instance property
    - [x] Remove PendingInteractionPOI, PendingInteractionAction, bHasPendingInteraction properties (now in Component)
    - [x] Remove HandleInteractPOI(), OnPOIActionSelected() method declarations
    - [x] Remove NotifyPOIOverlap() method and getter inline methods (now handled by Component)
    - [x] Add MoveConvoyToLocation(FVector) declaration (convoy movement is controller responsibility)

  - [x] **Update FCPlayerController.cpp**:

    - [x] Update HandleClick() to call InteractionComponent->HandlePOIClick(HitActor) instead of HandleInteractPOI()
    - [x] Remove HandleInteractPOI() implementation (60 lines)
    - [x] Remove OnPOIActionSelected() implementation (22 lines)
    - [x] Remove NotifyPOIOverlap() implementation (67 lines)
    - [x] Remove POI interaction property initialization from constructor (4 lines)
    - [x] Replace MoveConvoyToPOI(AActor) with MoveConvoyToLocation(FVector) - simple delegation method

  - [x] Save files
  - [x] Compile C++ code

- [x] **Testing After Step 6.4.3** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] PlayerController reduced by ~180 lines (196 removed, 16 added for MoveConvoyToLocation)
  - [x] HandleClick delegates to InteractionComponent
  - [x] Architecture clean: Controller routes, Component handles logic

**COMMIT POINT 6.4.3**: `git add Source/FC/Core/FCPlayerController.h Source/FC/Core/FCPlayerController.cpp && git commit -m "refactor(overworld): Clean up PlayerController, delegate POI interaction to component"`

---

#### Step 6.4.4: Connect Convoy Overlap to Interaction Component

- [x] **Analysis**

  - [x] AFCConvoyMember already detects POI overlap (Step 6.2)
  - [x] Forward overlap event to InteractionComponent instead of parent convoy
  - [x] Add bIsInteractingWithPOI flag to convoy to prevent multiple members triggering same POI
  - [x] InteractionComponent clears convoy flag after interaction completes

- [x] **Implementation (C++)**

  - [x] **Update FCOverworldConvoy.h**:
    - [x] Add bool bIsInteractingWithPOI private member
    - [x] Add IsInteractingWithPOI() getter method
    - [x] Add SetInteractingWithPOI(bool) setter method
  - [x] **Update FCOverworldConvoy.cpp**:
    - [x] Initialize bIsInteractingWithPOI = false in constructor
    - [x] Check flag in NotifyPOIOverlap(), set to true if not already interacting
    - [x] Return early if already interacting (prevents multiple triggers)
  - [x] **Update FCConvoyMember.cpp** NotifyPOIOverlap():
    - [x] Add includes for FCPlayerController, FCFirstPersonCharacter, FCInteractionComponent
    - [x] Check ParentConvoy->IsInteractingWithPOI() - return early if true
    - [x] Get PlayerController → FirstPersonCharacter → InteractionComponent
    - [x] Call InteractionComponent->NotifyPOIOverlap(POIActor)
    - [x] Keep ParentConvoy->NotifyPOIOverlap() as fallback
  - [x] **Update FCInteractionComponent.cpp**:
    - [x] Add includes for FCPlayerController and FCOverworldConvoy
    - [x] After ExecuteAction() in NotifyPOIOverlap():
      - [x] Get PlayerController->GetPossessedConvoy()
      - [x] Call Convoy->SetInteractingWithPOI(false) to clear flag
    - [x] Clear flag after both intentional and auto-executed interactions
  - [x] **Update FCPlayerController.h**:
    - [x] Add public GetPossessedConvoy() getter method
  - [x] Save all files
  - [x] Compile C++ code

- [x] **Testing After Step 6.4.4** ✅ CHECKPOINT
  - [x] C++ code compiles without errors
  - [x] Convoy overlap notifies InteractionComponent
  - [x] Only first convoy member triggers POI interaction
  - [x] Convoy flag cleared after interaction completes

**COMMIT POINT 6.4.4**: `git add Source/FC/Characters/Convoy/FCConvoyMember.* Source/FC/Characters/Convoy/FCOverworldConvoy.* Source/FC/Interaction/FCInteractionComponent.cpp Source/FC/Core/FCPlayerController.h && git commit -m "feat(overworld): Add convoy interaction state to prevent multiple POI triggers"`

---

#### Step 6.4.5: Configure Widget Class in Game Instance

- [ ] **Analysis**

  - [ ] POIActionSelectionWidgetClass now configured in UFCGameInstance (via UIManager subsystem)
  - [ ] Set WBP_ActionSelection as widget class

- [ ] **Implementation (Unreal Editor)**

  - [ ] Open BP_FC_GameInstance
  - [ ] Class Defaults → FC | UI Manager:
    - [ ] POI Action Selection Widget Class: Select WBP_ActionSelection
  - [ ] Compile and save

- [ ] **Testing After Step 6.4.5** ✅ CHECKPOINT
  - [ ] Widget class reference set
  - [ ] Blueprint compiles

**COMMIT POINT 6.4.5**: `git add Content/FC/Core/Blueprints/BP_FC_GameInstance.uasset && git commit -m "feat(overworld): Configure POI action widget in game instance"`

---

#### Step 6.4.6: Wire Widget to Interaction Component

- [ ] **Analysis**

  - [ ] WBP_ActionSelection receives actions array from UFCUIManager
  - [ ] Widget populates action buttons dynamically
  - [ ] Widget's OnActionSelected dispatcher calls UFCInteractionComponent->OnPOIActionSelected()
  - [ ] Component reference passed from UFCUIManager::ShowPOIActionSelection()

- [ ] **Implementation (WBP_ActionSelection Blueprint)**

  - [ ] **Add Variables**:
    - [ ] `InteractionComponent` (UFCInteractionComponent\*, Instance Editable)
  - [ ] **Event Construct**:
    - [ ] Call PopulateActions(AvailableActions) to create button widgets
  - [ ] **Function: PopulateActions(Actions)**:
    - [ ] ForEach loop through Actions array:
      - [ ] Create WBP_POIActionButton instance
      - [ ] Set button's ActionType and ActionText from array element
      - [ ] Bind button's OnClicked → OnActionButtonClicked(ActionType)
      - [ ] Add button to Scroll Box
  - [ ] **Function: OnActionButtonClicked(EFCPOIAction Action)**:
    - [ ] Call InteractionComponent->OnPOIActionSelected(Action)
    - [ ] Remove widget from viewport
  - [ ] Compile and save

- [ ] **Testing After Step 6.4.6** ✅ CHECKPOINT
  - [ ] Widget binds to InteractionComponent
  - [ ] Action buttons call component method
  - [ ] Widget closes after selection

**COMMIT POINT 6.4.6**: `git add Content/FC/UI/Widgets/WBP_ActionSelection.uasset && git commit -m "feat(overworld): Wire action widget to interaction component"`

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
