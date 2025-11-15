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
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Interaction/FCInteractionComponent.h"

DEFINE_LOG_CATEGORY(LogFallenCompassPlayerController);

AFCPlayerController::AFCPlayerController()
{
	CameraMode = EFCPlayerCameraMode::FirstPerson;
	bIsPauseMenuDisplayed = false;
	bShowMouseCursor = false;
	CurrentMappingMode = EFCInputMappingMode::FirstPerson;
	CurrentGameState = EFCGameState::MainMenu; // Start in MainMenu state

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
		// Use the interaction component on the character
		AFCFirstPersonCharacter* FPCharacter = Cast<AFCFirstPersonCharacter>(GetPawn());
		if (FPCharacter)
		{
			UFCInteractionComponent* InteractionComp = FPCharacter->GetInteractionComponent();
			if (InteractionComp)
			{
				InteractionComp->Interact();
			}
			else
			{
				UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleInteractPressed: No InteractionComponent on character"));
			}
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleInteractPressed: No pawn or not a FCFirstPersonCharacter"));
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

void AFCPlayerController::SetCameraModeLocal(EFCPlayerCameraMode NewMode, float BlendTime)
{
	if (CameraMode == NewMode)
	{
		return; // Already in this mode
	}

	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("SetCameraMode: Transitioning to mode %d with blend time %.2f"), static_cast<int32>(NewMode), BlendTime);

	ACameraActor* TargetCamera = nullptr;

	switch (NewMode)
	{
		case EFCPlayerCameraMode::MainMenu:
			TargetCamera = MenuCamera;
			break;

		case EFCPlayerCameraMode::FirstPerson:
			// Use the pawn's camera (handled by SetViewTargetWithBlend to Pawn)
			if (GetPawn())
			{
				SetViewTargetWithBlend(GetPawn(), BlendTime, VTBlend_Cubic);
				CameraMode = NewMode;
				LogStateChange(TEXT("Camera switched to FirstPerson"));
				return;
			}
			break;

		case EFCPlayerCameraMode::TableView:
		case EFCPlayerCameraMode::SaveSlotView:
			// TODO: Implement table view camera (Week 2)
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TableView/SaveSlotView camera not yet implemented"));
			return;
	}

	if (TargetCamera)
	{
		SetViewTargetWithBlend(TargetCamera, BlendTime, VTBlend_Cubic);
		CameraMode = NewMode;
		LogStateChange(FString::Printf(TEXT("Camera switched to mode %d"), static_cast<int32>(NewMode)));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("SetCameraModeLocal: Target camera is null for mode %d"), static_cast<int32>(NewMode));
	}
}

void AFCPlayerController::InitializeMainMenu()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("InitializeMainMenu: Setting up main menu state"));

	CurrentGameState = EFCGameState::MainMenu;

	// Clear all input contexts
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			Subsystem->ClearAllMappings();
		}
	}

	// Set input mode to UI only
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	// Set camera to MenuCamera
	SetCameraModeLocal(EFCPlayerCameraMode::MainMenu, 0.0f); // No blend on initial load

	// Spawn main menu widget
	if (MainMenuWidgetClass)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("InitializeMainMenu: Main menu widget spawned"));
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Error, TEXT("InitializeMainMenu: Failed to create main menu widget"));
		}
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("InitializeMainMenu: MainMenuWidgetClass is not set"));
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Main Menu State Initialized"));
	}

	LogStateChange(TEXT("Main Menu initialized"));
}

void AFCPlayerController::TransitionToGameplay()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TransitionToGameplay: Starting transition from MainMenu to Gameplay"));

	CurrentGameState = EFCGameState::Gameplay;

	// Remove main menu widget
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TransitionToGameplay: Main menu widget removed"));
	}

	// Spawn player character if not already present
	if (!GetPawn())
	{
		// TODO: Spawn AFCFirstPersonCharacter at designated start location (Task 5.4)
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TransitionToGameplay: No pawn found - need to spawn player"));
	}

	// Blend camera to first-person
	SetCameraModeLocal(EFCPlayerCameraMode::FirstPerson, 2.0f);

	// Delay input restoration until after camera blend
	FTimerHandle InputRestoreTimer;
	GetWorldTimerManager().SetTimer(InputRestoreTimer, [this]()
	{
		// Restore first-person input
		SetInputMappingMode(EFCInputMappingMode::FirstPerson);

		// Set input mode to game only
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;

		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TransitionToGameplay: Input restored"));
	}, 2.0f, false); // Wait for camera blend to complete

	LogStateChange(TEXT("Transitioning to Gameplay"));
}

void AFCPlayerController::ReturnToMainMenu()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ReturnToMainMenu: Starting fade and reload"));

	CurrentGameState = EFCGameState::Loading;

	// Fade to black
	PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 1.0f, FLinearColor::Black, false, true);

	// TODO: Play door open sound (Task 5.9)

	// Reload L_Office after fade completes
	FTimerHandle ReloadTimer;
	GetWorldTimerManager().SetTimer(ReloadTimer, [this]()
	{
		UGameplayStatics::OpenLevel(this, FName(TEXT("L_Office")));
	}, 1.0f, false);

	LogStateChange(TEXT("Returning to Main Menu"));
}

void AFCPlayerController::OnNewLegacyClicked()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnNewLegacyClicked: Starting new legacy"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("New Legacy - Transitioning to gameplay"));
	}

	TransitionToGameplay();
}

void AFCPlayerController::OnContinueClicked()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnContinueClicked: Continue not yet implemented"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Continue - Not yet implemented (Task 5.8)"));
	}

	// TODO: Load most recent save (Task 5.8)
}

void AFCPlayerController::OnLoadSaveClicked()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnLoadSaveClicked: Load save not yet implemented"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Load Save Game - Not yet implemented (Task 5.7)"));
	}

	// TODO: Show save slot selector (Task 5.7)
}

void AFCPlayerController::OnOptionsClicked()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnOptionsClicked: Options not yet implemented"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Options - Not yet implemented"));
	}

	// TODO: Show options menu (future task)
}

void AFCPlayerController::OnQuitClicked()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnQuitClicked: Quitting game"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Quitting game..."));
	}

	// TODO: Optional auto-save before quit (Task 5.10)

	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
