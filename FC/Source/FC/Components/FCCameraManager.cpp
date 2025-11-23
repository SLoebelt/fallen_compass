// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/FCCameraManager.h"
#include "Core/FCPlayerController.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogFCCameraManager);

UFCCameraManager::UFCCameraManager()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default configuration
	CurrentCameraMode = EFCPlayerCameraMode::FirstPerson;
	PreviousCameraMode = EFCPlayerCameraMode::FirstPerson;
	bIsTransitioning = false;
	DefaultBlendTime = 2.0f;
	DefaultBlendFunction = VTBlend_Cubic;
}

void UFCCameraManager::BeginPlay()
{
	Super::BeginPlay();

	// Store original view target (the player pawn)
	APlayerController* PC = GetPlayerController();
	if (PC)
	{
		// If we start at main menu, get the pawn instead of current view target
		OriginalViewTarget = PC->GetPawn();
		if (!OriginalViewTarget)
		{
			// Fallback to current view target if no pawn yet
			OriginalViewTarget = PC->GetViewTarget();
		}
		
		UE_LOG(LogFCCameraManager, Log, TEXT("BeginPlay: Original view target set to %s"), 
			*GetNameSafe(OriginalViewTarget));
	}
	else
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BeginPlay: PlayerController is null!"));
	}
}

// --- Camera Mode Management ---

void UFCCameraManager::SetCameraMode(EFCPlayerCameraMode NewMode)
{
	if (CurrentCameraMode != NewMode)
	{
		PreviousCameraMode = CurrentCameraMode;
		CurrentCameraMode = NewMode;

		UE_LOG(LogFCCameraManager, Log, TEXT("Camera mode transition: %s → %s"),
			*UEnum::GetValueAsString(PreviousCameraMode),
			*UEnum::GetValueAsString(CurrentCameraMode));
	}
}

// --- Camera Transition API ---

void UFCCameraManager::BlendToMenuCamera(float BlendTime)
{
	if (!MenuCamera)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToMenuCamera: MenuCamera is null!"));
		return;
	}

	float EffectiveBlendTime = GetEffectiveBlendTime(BlendTime);
	
	UE_LOG(LogFCCameraManager, Log, TEXT("Blending to menu camera (%.2fs)"), EffectiveBlendTime);
	
	BlendToTarget(MenuCamera, EffectiveBlendTime, DefaultBlendFunction);
	SetCameraMode(EFCPlayerCameraMode::MainMenu);
}

void UFCCameraManager::BlendToFirstPerson(float BlendTime)
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToFirstPerson: PlayerController is null!"));
		return;
	}

	// Update OriginalViewTarget to pawn if needed
	if (!OriginalViewTarget || !OriginalViewTarget->IsA<APawn>())
	{
		OriginalViewTarget = PC->GetPawn();
		UE_LOG(LogFCCameraManager, Warning, TEXT("BlendToFirstPerson: Updated OriginalViewTarget to pawn %s"), 
			*GetNameSafe(OriginalViewTarget));
	}

	if (!OriginalViewTarget)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToFirstPerson: OriginalViewTarget is null! Cannot blend to first person."));
		return;
	}

	float EffectiveBlendTime = GetEffectiveBlendTime(BlendTime);
	
	UE_LOG(LogFCCameraManager, Log, TEXT("Blending to first person (%.2fs) - Target: %s, Current ViewTarget: %s"), 
		EffectiveBlendTime, 
		*GetNameSafe(OriginalViewTarget),
		*GetNameSafe(PC->GetViewTarget()));
	
	BlendToTarget(OriginalViewTarget, EffectiveBlendTime, DefaultBlendFunction);
	SetCameraMode(EFCPlayerCameraMode::FirstPerson);

	// Cleanup any temporary cameras after blend completes
	if (EffectiveBlendTime > 0.0f)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FTimerDelegate CleanupDelegate = FTimerDelegate::CreateUObject(
				this, &UFCCameraManager::CleanupTableViewCamera);
			
			World->GetTimerManager().SetTimer(
				CameraCleanupTimerHandle,
				CleanupDelegate,
				EffectiveBlendTime + 0.1f,
				false);
		}
	}
	else
	{
		CleanupTableViewCamera();
	}
}

void UFCCameraManager::BlendToTableObject(AActor* TableObject, float BlendTime)
{
	if (!TableObject)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTableObject: TableObject is null!"));
		return;
	}

	// Save current view target before transitioning (prevents camera jump when clicking map)
	APlayerController* PC = GetPlayerController();
	if (PC && !OriginalViewTarget)
	{
		OriginalViewTarget = PC->GetViewTarget();
		UE_LOG(LogFCCameraManager, Log, TEXT("BlendToTableObject: Saved OriginalViewTarget: %s"), 
			*GetNameSafe(OriginalViewTarget));
	}

	float EffectiveBlendTime = GetEffectiveBlendTime(BlendTime);

	// Find camera target point on table object
	TArray<USceneComponent*> SceneComponents;
	TableObject->GetComponents<USceneComponent>(SceneComponents);

	USceneComponent* CameraTargetPoint = nullptr;
	for (USceneComponent* Component : SceneComponents)
	{
		if (Component && Component->GetName().Contains(TEXT("CameraTarget")))
		{
			CameraTargetPoint = Component;
			break;
		}
	}

	if (!CameraTargetPoint)
	{
		UE_LOG(LogFCCameraManager, Warning, 
			TEXT("BlendToTableObject: No CameraTargetPoint found on %s, using root component"),
			*GetNameSafe(TableObject));
		CameraTargetPoint = TableObject->GetRootComponent();
	}

	if (!CameraTargetPoint)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTableObject: No valid component found!"));
		return;
	}

	// Spawn temporary camera at target point
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTableObject: World is null!"));
		return;
	}

	// Save current table view camera as previous (for widget close restoration)
	if (TableViewCamera)
	{
		// If we already have a table camera, save it as previous
		if (PreviousTableViewCamera)
		{
			PreviousTableViewCamera->Destroy();
		}
		PreviousTableViewCamera = TableViewCamera;
		TableViewCamera = nullptr;
		UE_LOG(LogFCCameraManager, Log, TEXT("BlendToTableObject: Saved previous table camera for restoration"));
	}

	// Spawn new camera
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TableViewCamera = World->SpawnActor<ACameraActor>(
		ACameraActor::StaticClass(),
		CameraTargetPoint->GetComponentLocation(),
		CameraTargetPoint->GetComponentRotation(),
		SpawnParams);

	if (!TableViewCamera)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTableObject: Failed to spawn camera!"));
		return;
	}

	UE_LOG(LogFCCameraManager, Log, TEXT("Blending to table object %s (%.2fs)"), 
		*GetNameSafe(TableObject), EffectiveBlendTime);

	BlendToTarget(TableViewCamera, EffectiveBlendTime, DefaultBlendFunction);
	SetCameraMode(EFCPlayerCameraMode::TableView);
}

void UFCCameraManager::BlendToTopDown(float BlendTime)
{
	// Find BP_OverworldCamera in level
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTopDown: World is null!"));
		return;
	}

	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass(World, ACameraActor::StaticClass(), FoundCameras);

	ACameraActor* OverworldCamera = nullptr;
	for (AActor* Actor : FoundCameras)
	{
		if (Actor->GetName().Contains(TEXT("OverworldCamera")))
		{
			OverworldCamera = Cast<ACameraActor>(Actor);
			break;
		}
	}

	if (!OverworldCamera)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTopDown: No OverworldCamera found in level! Place BP_OverworldCamera in L_Overworld."));
		return;
	}

	// Attach camera to convoy's CameraAttachPoint (Task 5.6.1)
	TArray<AActor*> FoundConvoys;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundConvoys);
	
	AActor* Convoy = nullptr;
	for (AActor* Actor : FoundConvoys)
	{
		if (Actor->GetClass()->GetName().Contains(TEXT("OverworldConvoy")))
		{
			Convoy = Actor;
			break;
		}
	}

	if (Convoy)
	{
		// Find CameraAttachPoint component via reflection
		UFunction* GetAttachPointFunc = Convoy->FindFunction(FName("GetCameraAttachPoint"));
		if (GetAttachPointFunc)
		{
			struct FGetCameraAttachPointParams
			{
				USceneComponent* ReturnValue;
			};
			FGetCameraAttachPointParams Params;
			Params.ReturnValue = nullptr;
			Convoy->ProcessEvent(GetAttachPointFunc, &Params);
			
			if (Params.ReturnValue)
			{
				// Attach camera to CameraAttachPoint
				OverworldCamera->AttachToComponent(Params.ReturnValue, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				UE_LOG(LogFCCameraManager, Log, TEXT("BlendToTopDown: Attached camera to convoy's CameraAttachPoint"));
			}
			else
			{
				UE_LOG(LogFCCameraManager, Warning, TEXT("BlendToTopDown: GetCameraAttachPoint returned null"));
			}
		}
		else
		{
			UE_LOG(LogFCCameraManager, Warning, TEXT("BlendToTopDown: Convoy has no GetCameraAttachPoint method"));
		}
	}
	else
	{
		UE_LOG(LogFCCameraManager, Log, TEXT("BlendToTopDown: No convoy found in level (expected in Office)"));
	}

	// Set PlayerPawn reference on camera for distance limiting
	APlayerController* PC = GetPlayerController();
	if (PC && PC->GetPawn())
	{
		// Call SetPlayerPawn on AFCOverworldCamera via reflection
		UFunction* SetPawnFunc = OverworldCamera->FindFunction(FName("SetPlayerPawn"));
		if (SetPawnFunc)
		{
			struct FSetPlayerPawnParams
			{
				APawn* NewPawn;
			};
			FSetPlayerPawnParams Params;
			Params.NewPawn = PC->GetPawn();
			OverworldCamera->ProcessEvent(SetPawnFunc, &Params);
			UE_LOG(LogFCCameraManager, Log, TEXT("BlendToTopDown: Set PlayerPawn reference on camera"));
		}
	}

	float EffectiveBlendTime = GetEffectiveBlendTime(BlendTime);
	UE_LOG(LogFCCameraManager, Log, TEXT("Blending to TopDown camera (%.2fs)"), EffectiveBlendTime);

	BlendToTarget(OverworldCamera, EffectiveBlendTime, DefaultBlendFunction);
	SetCameraMode(EFCPlayerCameraMode::TopDown);
}

void UFCCameraManager::RestorePreviousViewTarget(float BlendTime)
{
	// Restore to previous camera mode
	switch (PreviousCameraMode)
	{
	case EFCPlayerCameraMode::FirstPerson:
		BlendToFirstPerson(BlendTime);
		break;
	case EFCPlayerCameraMode::MainMenu:
		BlendToMenuCamera(BlendTime);
		break;
	default:
		UE_LOG(LogFCCameraManager, Warning, 
			TEXT("RestorePreviousViewTarget: Cannot restore to mode %s, defaulting to FirstPerson"),
			*UEnum::GetValueAsString(PreviousCameraMode));
		BlendToFirstPerson(BlendTime);
		break;
	}
}

void UFCCameraManager::RestorePreviousTableCamera(float BlendTime)
{
	if (!PreviousTableViewCamera)
	{
		UE_LOG(LogFCCameraManager, Warning, TEXT("RestorePreviousTableCamera: No previous table camera, returning to FirstPerson"));
		BlendToFirstPerson(BlendTime);
		return;
	}

	float EffectiveBlendTime = GetEffectiveBlendTime(BlendTime);
	
	UE_LOG(LogFCCameraManager, Log, TEXT("Restoring previous table camera (%.2fs)"), EffectiveBlendTime);
	
	// Blend back to previous table camera
	BlendToTarget(PreviousTableViewCamera, EffectiveBlendTime, DefaultBlendFunction);
	SetCameraMode(EFCPlayerCameraMode::TableView);
	
	// Swap cameras: previous becomes current, current is destroyed
	if (TableViewCamera)
	{
		TableViewCamera->Destroy();
	}
	TableViewCamera = PreviousTableViewCamera;
	PreviousTableViewCamera = nullptr;
}

void UFCCameraManager::SetMenuCamera(ACameraActor* InMenuCamera)
{
	MenuCamera = InMenuCamera;
	UE_LOG(LogFCCameraManager, Log, TEXT("MenuCamera set to %s"), *GetNameSafe(MenuCamera));
}

// --- Internal Helpers ---

float UFCCameraManager::GetEffectiveBlendTime(float BlendTime) const
{
	return (BlendTime < 0.0f) ? DefaultBlendTime : BlendTime;
}

void UFCCameraManager::BlendToTarget(AActor* Target, float BlendTime, EViewTargetBlendFunction BlendFunc)
{
	APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTarget: PlayerController is null!"));
		return;
	}

	if (!Target)
	{
		UE_LOG(LogFCCameraManager, Error, TEXT("BlendToTarget: Target is null!"));
		return;
	}

	bIsTransitioning = true;

	PC->SetViewTargetWithBlend(Target, BlendTime, BlendFunc);

	// Clear transition flag after blend completes
	if (BlendTime > 0.0f)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				CameraCleanupTimerHandle,
				[this]()
				{
					bIsTransitioning = false;
					UE_LOG(LogFCCameraManager, Verbose, TEXT("Camera transition complete"));
				},
				BlendTime,
				false);
		}
	}
	else
	{
		bIsTransitioning = false;
	}
}

void UFCCameraManager::CleanupTableViewCamera()
{
	if (TableViewCamera && IsValid(TableViewCamera))
	{
		UE_LOG(LogFCCameraManager, Log, TEXT("Cleaning up table view camera"));
		TableViewCamera->Destroy();
		TableViewCamera = nullptr;
	}
}

APlayerController* UFCCameraManager::GetPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}
