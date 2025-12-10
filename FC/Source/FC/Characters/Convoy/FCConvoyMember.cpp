// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Characters/Convoy/FCConvoyMember.h"

#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Components/CapsuleComponent.h"
#include "Core/FCPlayerController.h"
#include "Interaction/FCInteractionComponent.h"
#include "Interaction/IFCInteractablePOI.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FC.h"

DEFINE_LOG_CATEGORY_STATIC(LogFCConvoyMember, Log, All);

AFCConvoyMember::AFCConvoyMember()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure capsule component
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		CapsuleComp->SetCapsuleHalfHeight(100.0f);
		CapsuleComp->SetCapsuleRadius(50.0f);
		CapsuleComp->SetGenerateOverlapEvents(true);
	}

	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->bConstrainToPlane = true;
        MoveComp->bSnapToPlaneAtStart = true;

        // Critical for unpossessed movement:
        MoveComp->bRunPhysicsWithNoController = true;

        MoveComp->MaxWalkSpeed = 300.0f; // tune
    }
}

void AFCConvoyMember::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsFollowingPath && PathPoints.IsValidIndex(CurrentPathIndex))
    {
        FVector Target = PathPoints[CurrentPathIndex];
        FVector ToTarget = Target - GetActorLocation();
        ToTarget.Z = 0;

        if (ToTarget.Size() < AcceptRadius)
        {
            CurrentPathIndex++;
            if (!PathPoints.IsValidIndex(CurrentPathIndex))
            {
                bIsFollowingPath = false;
                StopConvoyMovement();
            }
        }
        else
        {
            FVector Direction = ToTarget.GetSafeNormal();
            AddMovementInput(Direction, 1.0f, true); // bForce=true important!
        }
    }
    else if (bIsFollowingLeader && Leader.IsValid())
    {
        // Rotate offset by leader rotation so the formation turns with the leader
		const FVector DesiredPos =
			Leader->GetActorLocation() + Leader->GetActorRotation().RotateVector(FollowerOffset);

		FVector ToDesired = DesiredPos - GetActorLocation();
		ToDesired.Z = 0.0f;

		if (ToDesired.Size() > AcceptRadius)
		{
			AddMovementInput(ToDesired.GetSafeNormal(), 1.0f, /*bForce=*/true);
		}
    }
}

void AFCConvoyMember::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event to capsule component
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AFCConvoyMember::OnCapsuleBeginOverlap);
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Bound capsule overlap event"), *GetName());
	}
	else
	{
		UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: No capsule component found!"), *GetName());
	}
}

void AFCConvoyMember::OnCapsuleBeginOverlap(
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

	// Check if actor implements IFCInteractablePOI interface
	if (OtherActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
	{
		UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Detected overlap with POI %s"),
			*GetName(), *OtherActor->GetName());

		// Notify parent convoy to coordinate stop and interaction
		if (ParentConvoy)
		{
			ParentConvoy->HandlePOIOverlap(OtherActor);
		}
		else
		{
			UE_LOG(LogFCConvoyMember, Warning, TEXT("ConvoyMember %s: No parent convoy reference"), *GetName());
		}
	}
}

void AFCConvoyMember::SetParentConvoy(AFCOverworldConvoy* InConvoy)
{
	ParentConvoy = InConvoy;
	UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Parent convoy set to %s"),
		*GetName(), InConvoy ? *InConvoy->GetName() : TEXT("null"));
}

void AFCConvoyMember::MoveConvoyMemberToLocation(const FVector& TargetLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        StopConvoyMovement();
        return;
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        UE_LOG(LogFCConvoyMember, Warning, TEXT("MoveConvoyMemberToLocation: No NavSys."));
        StopConvoyMovement();
        return;
    }

    // Optional but recommended: project onto NavMesh to avoid invalid targets
    FNavLocation ProjectedTarget;
    if (!NavSys->ProjectPointToNavigation(TargetLocation, ProjectedTarget))
    {
        UE_LOG(LogFCConvoyMember, Warning, TEXT("MoveConvoyMemberToLocation: Target not on NavMesh."));
        StopConvoyMovement();
        return;
    }

    const FVector Start = GetActorLocation();
    UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(World, Start, ProjectedTarget.Location, this);

    if (!NavPath || NavPath->PathPoints.Num() == 0)
    {
        UE_LOG(LogFCConvoyMember, Warning, TEXT("MoveConvoyMemberToLocation: No valid path."));
        StopConvoyMovement();
        return;
    }

    FinalTarget = ProjectedTarget.Location;
    PathPoints = NavPath->PathPoints;
    CurrentPathIndex = 0;
    bIsFollowingPath = true;

    // If you want: stop follower mode when leader is pathing
    bIsFollowingLeader = false;
    Leader = nullptr;
}

void AFCConvoyMember::StopConvoyMovement()
{
    bIsFollowingPath = false;
    CurrentPathIndex = INDEX_NONE;
    PathPoints.Reset();
    FinalTarget = FVector::ZeroVector;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
    }
}

void AFCConvoyMember::StartFollowingLeader(AFCConvoyMember* InLeader, const FVector& InLocalOffset)
{
    Leader = InLeader;
    FollowerOffset = InLocalOffset;

    bIsFollowingLeader = Leader.IsValid();

    // Followers should not run their own nav path
    bIsFollowingPath = false;
    PathPoints.Reset();
    CurrentPathIndex = INDEX_NONE;

    UE_LOG(LogFCConvoyMember, Log, TEXT("ConvoyMember %s: Following leader %s Offset=%s"),
        *GetName(),
        Leader.IsValid() ? *Leader->GetName() : TEXT("null"),
        *FollowerOffset.ToString());
}

void AFCConvoyMember::StopFollowingLeader()
{
    bIsFollowingLeader = false;
    Leader = nullptr;
}
