#include "Characters/FC_ExplorerCharacter.h"

#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFCExplorerCharacter, Log, All);

AFC_ExplorerCharacter::AFC_ExplorerCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // NOTE: Changed from Epic's Top-Down template pattern.
    // We use an AI controller (like convoy movement) instead of player possession.
    // This allows SimpleMoveToLocation to work while maintaining static camp camera.
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AAIController::StaticClass();

    // Enable tick for debug logging
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Configure character movement for top-down style (no controller rotation influence).
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // Smooth rotation
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->bConstrainToPlane = true;
        MoveComp->bSnapToPlaneAtStart = true;
        MoveComp->MaxWalkSpeed = 300.0f; // Default walk speed
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void AFC_ExplorerCharacter::BeginPlay()
{
    Super::BeginPlay();

    AController* CurrentController = GetController();
    
    UE_LOG(LogFCExplorerCharacter, Log, TEXT("ExplorerCharacter BeginPlay:"));
    UE_LOG(LogFCExplorerCharacter, Log, TEXT("  ExplorerType: %s"), *UEnum::GetValueAsString(ExplorerType));
    UE_LOG(LogFCExplorerCharacter, Log, TEXT("  Controller: %s"), CurrentController ? *CurrentController->GetName() : TEXT("NONE"));
    UE_LOG(LogFCExplorerCharacter, Log, TEXT("  Note: Using AI controller (like convoy) - commanded by PlayerController"));

    // Initialize animation system
    InitializeAnimation();
}

void AFC_ExplorerCharacter::InitializeAnimation()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        UE_LOG(LogFCExplorerCharacter, Warning, TEXT("InitializeAnimation: No skeletal mesh component found!"));
        return;
    }

    // Ensure animation mode is set to use Animation Blueprint
    if (MeshComp->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
    {
        MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        UE_LOG(LogFCExplorerCharacter, Log, TEXT("InitializeAnimation: Set animation mode to AnimationBlueprint"));
    }

    // Verify AnimInstance is present
    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (AnimInst)
    {
        UE_LOG(LogFCExplorerCharacter, Log, TEXT("InitializeAnimation: AnimInstance active: %s"), 
            *AnimInst->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogFCExplorerCharacter, Warning, 
            TEXT("InitializeAnimation: No AnimInstance! Ensure Animation Class is set in Blueprint."));
    }
}

float AFC_ExplorerCharacter::GetMovementSpeed() const
{
    return GetVelocity().Size();
}

bool AFC_ExplorerCharacter::IsCharacterMoving() const
{
    return GetVelocity().SizeSquared() > 1.0f; // Moving if velocity > 1 unit/sec
}

void AFC_ExplorerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Debug: Log velocity and animation state every second
    static float DebugTimer = 0.0f;
    DebugTimer += DeltaTime;
    if (DebugTimer >= 1.0f)
    {
        DebugTimer = 0.0f;
        
        const FVector Velocity = GetVelocity();
        const float Speed = Velocity.Size();
        
        if (Speed > 0.01f)
        {
            UCharacterMovementComponent* MoveComp = GetCharacterMovement();
            if (MoveComp)
            {
                const FVector Acceleration = MoveComp->GetCurrentAcceleration();
                const float AccelSize = Acceleration.Size();
                
                UE_LOG(LogFCExplorerCharacter, Log, TEXT("Velocity: %.2f | Acceleration: %.2f"),
                    Speed, AccelSize);

                // Check ABP ShouldMove variable
                USkeletalMeshComponent* MeshComp = GetMesh();
                if (MeshComp)
                {
                    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
                    if (AnimInst)
                    {
                        FProperty* ShouldMoveProp = AnimInst->GetClass()->FindPropertyByName(FName("ShouldMove"));
                        if (ShouldMoveProp && ShouldMoveProp->IsA<FBoolProperty>())
                        {
                            bool ShouldMove = *ShouldMoveProp->ContainerPtrToValuePtr<bool>(AnimInst);
                            UE_LOG(LogFCExplorerCharacter, Log, TEXT("ABP ShouldMove: %s"), 
                                ShouldMove ? TEXT("TRUE") : TEXT("FALSE"));
                        }
                    }
                }
            }
        }
    }
}
