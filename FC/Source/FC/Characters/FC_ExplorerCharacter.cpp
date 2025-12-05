#include "Characters/FC_ExplorerCharacter.h"

#include "Logging/LogMacros.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogFCExplorerCharacter, Log, All);

AFC_ExplorerCharacter::AFC_ExplorerCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Controlled by Player, not AI by default
    AutoPossessPlayer = EAutoReceiveInput::Player0;
    AIControllerClass = nullptr; // Use default AIController

    // Enable tick for debug logging
	// TODO - disable in production builds
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
}


void AFC_ExplorerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	// Follow nav path if we have one (Camp/POI movement).
    if (bIsFollowingPath && PathPoints.Num() > 0 && CurrentPathIndex != INDEX_NONE)
    {
        if (CurrentPathIndex >= PathPoints.Num())
        {
            // Reached final point
            bIsFollowingPath = false;
            CurrentPathIndex = INDEX_NONE;
            UE_LOG(LogFCExplorerCharacter, Log, TEXT("Tick: Reached final path point"));
        }
        else
        {
            FVector CurrentTarget = PathPoints[CurrentPathIndex];
            FVector ToTarget = CurrentTarget - GetActorLocation();
            ToTarget.Z = 0.0f; // stay on Camp plane

            const float Distance = ToTarget.Size();
            if (Distance <= AcceptRadius)
            {
                ++CurrentPathIndex;
                if (CurrentPathIndex >= PathPoints.Num())
                {
                    bIsFollowingPath = false;
                    CurrentPathIndex = INDEX_NONE;
                    UE_LOG(LogFCExplorerCharacter, Log,
                        TEXT("Tick: Reached destination at %s"),
                        *CurrentTarget.ToString());
                }
            }
            else
            {
                const FVector Direction = ToTarget.GetSafeNormal();
                AddMovementInput(Direction, 1.0f);
            }
        }
    }
}


void AFC_ExplorerCharacter::MoveExplorerToLocation(const FVector& TargetLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogFCExplorerCharacter, Error, TEXT("MoveExplorerToLocation: World is null"));
        return;
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        UE_LOG(LogFCExplorerCharacter, Warning,
            TEXT("MoveExplorerToLocation: No navigation system available in map %s"),
            *World->GetMapName());
        return;
    }

    const FVector StartLocation = GetActorLocation();

    // Change NavPath variable type here:
    UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
        World,
        StartLocation,
        TargetLocation,
        this
    );

    if (!NavPath || !NavPath->IsValid() || NavPath->PathPoints.Num() == 0)
    {
        UE_LOG(LogFCExplorerCharacter, Warning,
            TEXT("MoveExplorerToLocation: No valid path from %s to %s"),
            *StartLocation.ToString(),
            *TargetLocation.ToString());
        bIsFollowingPath = false;
        PathPoints.Reset();
        CurrentPathIndex = INDEX_NONE;
        return;
    }

    PathPoints.Reset();
    PathPoints.Reserve(NavPath->PathPoints.Num());

    for (const FVector& Point : NavPath->PathPoints)
    {
        PathPoints.Add(Point);
    }

    CurrentPathIndex = 0;
    bIsFollowingPath = true;

    UE_LOG(LogFCExplorerCharacter, Log,
        TEXT("MoveExplorerToLocation: Path with %d points from %s to %s"),
        PathPoints.Num(),
        *StartLocation.ToString(),
        *TargetLocation.ToString());
}
