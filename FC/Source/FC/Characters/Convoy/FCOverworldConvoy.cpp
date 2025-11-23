// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Characters/Convoy/FCConvoyMember.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "AIController.h"
#include "Core/FCPlayerController.h"
#include "Core/FCFirstPersonCharacter.h"
#include "Interaction/FCInteractionComponent.h"
#include "FC.h"

DEFINE_LOG_CATEGORY_STATIC(LogFCOverworldConvoy, Log, All);

AFCOverworldConvoy::AFCOverworldConvoy()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsInteractingWithPOI = false;

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

	UE_LOG(LogFCOverworldConvoy, Log, TEXT("OverworldConvoy %s: Spawned %d convoy members"), *GetName(), ConvoyMembers.Num());
}

void AFCOverworldConvoy::StopAllMembers()
{
	UE_LOG(LogFCOverworldConvoy, Log, TEXT("Convoy %s: Stopping all members"), *GetName());
	
	for (AFCConvoyMember* Member : ConvoyMembers)
	{
		if (Member)
		{
			AAIController* AIController = Cast<AAIController>(Member->GetController());
			if (AIController)
			{
				AIController->StopMovement();
				UE_LOG(LogFCOverworldConvoy, Log, TEXT("  Stopped member: %s"), *Member->GetName());
			}
		}
	}
}

void AFCOverworldConvoy::HandlePOIOverlap(AActor* POIActor)
{
	if (!POIActor) return;
	
	// Check if already interacting - prevent multiple triggers
	if (bIsInteractingWithPOI)
	{
		UE_LOG(LogFCOverworldConvoy, Log, TEXT("Convoy %s: Already interacting with POI, ignoring overlap"), *GetName());
		return;
	}
	
	// Set interaction flag
	bIsInteractingWithPOI = true;
	
	// Stop all convoy members immediately
	StopAllMembers();
	
	// Get player controller's InteractionComponent for delegation
	AFCPlayerController* PC = Cast<AFCPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		AFCFirstPersonCharacter* FPCharacter = Cast<AFCFirstPersonCharacter>(PC->GetPawn());
		if (FPCharacter)
		{
			UFCInteractionComponent* InteractionComp = FPCharacter->GetInteractionComponent();
			if (InteractionComp)
			{
				UE_LOG(LogFCOverworldConvoy, Log, TEXT("Convoy %s: Delegating POI overlap to InteractionComponent"), *GetName());
				InteractionComp->NotifyPOIOverlap(POIActor);
				return;
			}
		}
	}
	
	UE_LOG(LogFCOverworldConvoy, Warning, TEXT("Convoy %s: Failed to delegate to InteractionComponent"), *GetName());
}
