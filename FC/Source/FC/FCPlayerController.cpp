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

DEFINE_LOG_CATEGORY(LogFallenCompassPlayerController);

AFCPlayerController::AFCPlayerController()
{
	CameraMode = EFCPlayerCameraMode::FirstPerson;
	bIsPauseMenuDisplayed = false;
	bShowMouseCursor = false;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextFinder(TEXT("/Game/FC/Input/IMC_FC_Default"));
	if (MappingContextFinder.Succeeded())
	{
		DefaultMappingContext = MappingContextFinder.Object;
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IMC_FC_Default mapping context."));
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

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, DefaultMappingPriority);
			}
			else
			{
				UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("DefaultMappingContext is null on %s."), *GetName());
			}
		}
	}

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
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TODO: Trace forward and interact with focused actor."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Interact: Trace forward and interact with focused actor"));
		}
		EnterTableViewPlaceholder();
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
