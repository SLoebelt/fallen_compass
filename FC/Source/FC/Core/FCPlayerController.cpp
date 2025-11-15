// Copyright Epic Games, Inc. All Rights Reserved.

#include "FCPlayerController.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"
#include "FCFirstPersonCharacter.h"
#include "Camera/CameraComponent.h"

DEFINE_LOG_CATEGORY(LogFallenCompassPlayerController);

AFCPlayerController::AFCPlayerController()
{
	CameraMode = EFCPlayerCameraMode::FirstPerson;
	bIsPauseMenuDisplayed = false;
	bShowMouseCursor = false;
	CurrentMappingMode = EFCInputMappingMode::FirstPerson;

	// Load FirstPerson mapping context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> FirstPersonContextFinder(TEXT("/Game/FC/Input/IMC_FC_FirstPerson"));
	if (FirstPersonContextFinder.Succeeded())
	{
		FirstPersonMappingContext = FirstPersonContextFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IMC_FC_FirstPerson"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IMC_FC_FirstPerson"));
	}

	// Load TopDown mapping context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> TopDownContextFinder(TEXT("/Game/FC/Input/IMC_FC_TopDown"));
	if (TopDownContextFinder.Succeeded())
	{
		TopDownMappingContext = TopDownContextFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IMC_FC_TopDown"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IMC_FC_TopDown"));
	}

	// Load Fight mapping context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> FightContextFinder(TEXT("/Game/FC/Input/IMC_FC_Fight"));
	if (FightContextFinder.Succeeded())
	{
		FightMappingContext = FightContextFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IMC_FC_Fight"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IMC_FC_Fight"));
	}

	// Load StaticScene mapping context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> StaticSceneContextFinder(TEXT("/Game/FC/Input/IMC_FC_StaticScene"));
	if (StaticSceneContextFinder.Succeeded())
	{
		StaticSceneMappingContext = StaticSceneContextFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IMC_FC_StaticScene"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IMC_FC_StaticScene"));
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionFinder(TEXT("/Game/FC/Input/IA_Interact"));
	if (InteractActionFinder.Succeeded())
	{
		InteractAction = InteractActionFinder.Object;
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_Interact."));
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> PauseActionFinder(TEXT("/Game/FC/Input/IA_Pause"));
	if (PauseActionFinder.Succeeded())
	{
		PauseAction = PauseActionFinder.Object;
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_Pause."));
	}
}

void AFCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	LogStateChange(TEXT("AFCPlayerController ready"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("FC controller active"));
	}
}

void AFCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Apply initial mapping context based on CurrentMappingMode
	SetInputMappingMode(CurrentMappingMode);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("EnhancedInputComponent missing on %s; bindings skipped."), *GetName());
		return;
	}

	if (InteractAction)
	{
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleInteractPressed);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("InteractAction not assigned on %s."), *GetName());
	}

	if (PauseAction)
	{
		EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AFCPlayerController::HandlePausePressed);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("PauseAction not assigned on %s."), *GetName());
	}
}

void AFCPlayerController::HandleInteractPressed()
{
	UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleInteractPressed | Camera=%s"), *StaticEnum<EFCPlayerCameraMode>()->GetNameStringByValue(static_cast<int64>(CameraMode)));

	if (CameraMode == EFCPlayerCameraMode::FirstPerson)
	{
		// Perform forward line trace to detect interactables
		AFCFirstPersonCharacter* FPCharacter = Cast<AFCFirstPersonCharacter>(GetPawn());
		if (FPCharacter)
		{
			UCameraComponent* CameraComp = FPCharacter->GetFirstPersonCamera();
			if (CameraComp)
			{
				FVector Start = CameraComp->GetComponentLocation();
				FVector End = Start + (CameraComp->GetForwardVector() * 1000.0f); // Trace 1000 units forward

				FHitResult HitResult;
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(GetPawn()); // Ignore the player character

				if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
				{
					UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Interact: Hit actor '%s' at distance %.1f units"),
						*HitResult.GetActor()->GetName(), HitResult.Distance);
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
							FString::Printf(TEXT("Interact: Hit %s (%.1f units)"), *HitResult.GetActor()->GetName(), HitResult.Distance));
					}
				}
				else
				{
					UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Interact: No interactable found"));
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Interact: No hit"));
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TODO: Route table-view interaction to board UI or exit handles."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Interact: Route table-view interaction to board UI"));
		}
	}
}

void AFCPlayerController::HandlePausePressed()
{
	if (CameraMode == EFCPlayerCameraMode::TableView)
	{
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ESC pressed while in table view. Returning to first-person instead of pause menu."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("ESC: Returning to first-person"));
		}
		ExitTableViewPlaceholder();
		return;
	}

	if (bIsPauseMenuDisplayed)
	{
		HidePauseMenuPlaceholder();
	}
	else
	{
		ShowPauseMenuPlaceholder();
	}
}

void AFCPlayerController::EnterTableViewPlaceholder()
{
	SetFallenCompassCameraMode(EFCPlayerCameraMode::TableView);
	UE_LOG(LogFallenCompassPlayerController, Display, TEXT("TODO: Blend to table-view camera and disable movement."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("Entering Table View (TODO: Blend camera)"));
	}
}

void AFCPlayerController::ExitTableViewPlaceholder()
{
	SetFallenCompassCameraMode(EFCPlayerCameraMode::FirstPerson);
	UE_LOG(LogFallenCompassPlayerController, Display, TEXT("TODO: Restore first-person camera and re-enable input."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("Exiting Table View (TODO: Restore camera)"));
	}
}

void AFCPlayerController::ShowPauseMenuPlaceholder()
{
	bIsPauseMenuDisplayed = true;
	SetPause(true);
	UE_LOG(LogFallenCompassPlayerController, Display, TEXT("TODO: Instantiate pause menu widget."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Magenta, TEXT("Pause Menu Requested (TODO: Instantiate widget)"));
	}
	LogStateChange(TEXT("Pause menu requested"));
}

void AFCPlayerController::HidePauseMenuPlaceholder()
{
	bIsPauseMenuDisplayed = false;
	SetPause(false);
	UE_LOG(LogFallenCompassPlayerController, Display, TEXT("TODO: Dismiss pause menu widget."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Magenta, TEXT("Pause Menu Dismissed (TODO: Remove widget)"));
	}
	LogStateChange(TEXT("Pause menu dismissed"));
}

void AFCPlayerController::SetFallenCompassCameraMode(EFCPlayerCameraMode NewMode)
{
	if (CameraMode == NewMode)
	{
		return;
	}

	CameraMode = NewMode;
	LogStateChange(TEXT("Camera mode updated"));
}

void AFCPlayerController::LogStateChange(const FString& Context) const
{
	const UEnum* ModeEnum = StaticEnum<EFCPlayerCameraMode>();
	const FString ModeLabel = ModeEnum ? ModeEnum->GetNameStringByValue(static_cast<int64>(CameraMode)) : TEXT("Unknown");
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("%s | CameraMode=%s | PauseMenu=%s"),
		*Context,
		*ModeLabel,
		bIsPauseMenuDisplayed ? TEXT("Shown") : TEXT("Hidden"));
	if (GEngine)
	{
		const FString DebugMsg = FString::Printf(TEXT("%s | CameraMode=%s | PauseMenu=%s"), *Context, *ModeLabel, bIsPauseMenuDisplayed ? TEXT("Shown") : TEXT("Hidden"));
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White, DebugMsg);
	}
}

void AFCPlayerController::SetInputMappingMode(EFCInputMappingMode NewMode)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("SetInputMappingMode: No LocalPlayer found"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!Subsystem)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("SetInputMappingMode: EnhancedInputSubsystem not found"));
		return;
	}

	// Determine which context to use
	UInputMappingContext* ContextToApply = nullptr;
	FString ModeName;

	switch (NewMode)
	{
		case EFCInputMappingMode::FirstPerson:
			ContextToApply = FirstPersonMappingContext;
			ModeName = TEXT("FirstPerson");
			break;
		case EFCInputMappingMode::TopDown:
			ContextToApply = TopDownMappingContext;
			ModeName = TEXT("TopDown");
			break;
		case EFCInputMappingMode::Fight:
			ContextToApply = FightMappingContext;
			ModeName = TEXT("Fight");
			break;
		case EFCInputMappingMode::StaticScene:
			ContextToApply = StaticSceneMappingContext;
			ModeName = TEXT("StaticScene");
			break;
	}

	if (!ContextToApply)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("SetInputMappingMode: %s context is null!"), *ModeName);
		return;
	}

	// Clear all existing mappings and apply new one
	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(ContextToApply, DefaultMappingPriority);

	CurrentMappingMode = NewMode;

	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Input mapping switched to: %s"), *ModeName);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			FString::Printf(TEXT("Input Mode: %s"), *ModeName));
	}
}
