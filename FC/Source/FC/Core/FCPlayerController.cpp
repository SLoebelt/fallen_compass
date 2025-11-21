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
#include "UFCGameInstance.h"
#include "FCTransitionManager.h"
#include "Core/FCLevelManager.h"
#include "Core/FCUIManager.h"
#include "Core/FCGameStateManager.h"
#include "../Interaction/FCTableInteractable.h"
#include "GameFramework/Character.h"
#include "Components/FCCameraManager.h"

DEFINE_LOG_CATEGORY(LogFallenCompassPlayerController);

AFCPlayerController::AFCPlayerController()
{
	// Create camera manager component
	CameraManager = CreateDefaultSubobject<UFCCameraManager>(TEXT("CameraManager"));
	bIsPauseMenuDisplayed = false;
	bShowMouseCursor = false;
	CurrentMappingMode = EFCInputMappingMode::FirstPerson;
	CurrentGameState = EFCGameState::MainMenu; // DEPRECATED: Use GameStateManager instead

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

	static ConstructorHelpers::FObjectFinder<UInputAction> QuickSaveActionFinder(TEXT("/Game/FC/Input/IA_QuickSave"));
	if (QuickSaveActionFinder.Succeeded())
	{
		QuickSaveAction = QuickSaveActionFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_QuickSave"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_QuickSave."));
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> QuickLoadActionFinder(TEXT("/Game/FC/Input/IA_QuickLoad"));
	if (QuickLoadActionFinder.Succeeded())
	{
		QuickLoadAction = QuickLoadActionFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_QuickLoad"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_QuickLoad."));
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> EscapeActionFinder(TEXT("/Game/FC/Input/IA_Escape"));
	if (EscapeActionFinder.Succeeded())
	{
		EscapeAction = EscapeActionFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_Escape"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_Escape."));
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ClickActionFinder(TEXT("/Game/FC/Input/IA_Click"));
	if (ClickActionFinder.Succeeded())
	{
		ClickAction = ClickActionFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_Click"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_Click."));
	}
}

EFCPlayerCameraMode AFCPlayerController::GetCameraMode() const
{
	return CameraManager ? CameraManager->GetCameraMode() : EFCPlayerCameraMode::FirstPerson;
}

void AFCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	LogStateChange(TEXT("AFCPlayerController ready"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("FC controller active"));
	}

	// Check if we need to restore player position after loading
	UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->RestorePlayerPosition();
		
		// Note: L_Office serves as BOTH menu and gameplay location
		// Input mode will be set by InitializeMainMenu() or TransitionToGameplay()
		// Don't set input mode here to avoid race conditions
		
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Controller ready, input mode will be set by game state transitions"));
	}

	// Note: MenuCamera reference will be set in Blueprint (BP_FC_PlayerController)
}void AFCPlayerController::SetupInputComponent()
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

	if (QuickSaveAction)
	{
		EnhancedInput->BindAction(QuickSaveAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleQuickSavePressed);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("QuickSaveAction not assigned on %s."), *GetName());
	}

	if (QuickLoadAction)
	{
		EnhancedInput->BindAction(QuickLoadAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleQuickLoadPressed);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("QuickLoadAction not assigned on %s."), *GetName());
	}

	if (EscapeAction)
	{
		EnhancedInput->BindAction(EscapeAction, ETriggerEvent::Started, this, &AFCPlayerController::HandlePausePressed);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("EscapeAction not assigned on %s."), *GetName());
	}

	if (ClickAction)
	{
		EnhancedInput->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleTableObjectClick);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("ClickAction not assigned on %s."), *GetName());
	}
}

void AFCPlayerController::HandleInteractPressed()
{
	EFCPlayerCameraMode CurrentMode = CameraManager ? CameraManager->GetCameraMode() : EFCPlayerCameraMode::FirstPerson;
	UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleInteractPressed | Camera=%s"), *StaticEnum<EFCPlayerCameraMode>()->GetNameStringByValue(static_cast<int64>(CurrentMode)));

	if (CurrentMode == EFCPlayerCameraMode::FirstPerson)
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
	// If viewing a table object with widget, ESC closes widget and stays in TableView
	// If in TableView without widget, ESC returns to first-person
	if (GetCameraMode() == EFCPlayerCameraMode::TableView)
	{
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ESC pressed in TableView mode"));
		
		// Check if widget is open via UIManager
		UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
		UFCUIManager* UIManager = GI ? GI->GetSubsystem<UFCUIManager>() : nullptr;
		
		if (UIManager && UIManager->IsTableWidgetOpen())
		{
			CloseTableWidget();
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Closed table widget, staying in TableView"));
		}
		else
		{
			// No widget open - return to FirstPerson
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("No widget open, returning to FirstPerson"));
			SetCameraModeLocal(EFCPlayerCameraMode::FirstPerson, 2.0f);
		}
		return;
	}

	// If in gameplay state (first-person, can move), ESC toggles pause menu
	// Use GameStateManager's state stack for proper pause/resume handling
	UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	if (!GI) return;

	UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
	UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
	
	if (!StateMgr || !UIManager) return;

	// Check if we're currently paused
	if (StateMgr->GetCurrentState() == EFCGameStateID::Paused)
	{
		// Resume: Pop state from stack to return to previous state
		if (StateMgr->PopState())
		{
			UIManager->HidePauseMenu();
			bIsPauseMenuDisplayed = false;
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandlePausePressed: Resumed from pause, returned to %s"),
				*UEnum::GetValueAsString(StateMgr->GetCurrentState()));
		}
		return;
	}

	// Check if we're in a pausable state (Office_Exploration or Office_TableView)
	EFCGameStateID CurrentState = StateMgr->GetCurrentState();
	if (CurrentState == EFCGameStateID::Office_Exploration || 
	    CurrentState == EFCGameStateID::Office_TableView)
	{
		// Pause: Push current state onto stack and transition to Paused
		if (StateMgr->PushState(EFCGameStateID::Paused))
		{
			UIManager->ShowPauseMenu();
			bIsPauseMenuDisplayed = true;
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandlePausePressed: Paused from %s"),
				*UEnum::GetValueAsString(CurrentState));
		}
		return;
	}

	// If in main menu state or other non-pausable states, ESC does nothing (handled by UI widgets or state logic)
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandlePausePressed: ESC pressed in non-pausable state %s - no action"),
		*UEnum::GetValueAsString(StateMgr->GetCurrentState()));
}

void AFCPlayerController::ShowPauseMenuPlaceholder()
{
	bIsPauseMenuDisplayed = true;
	SetPause(true);
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ShowPauseMenuPlaceholder: Pause menu displayed, game paused."));
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
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HidePauseMenuPlaceholder: Pause menu hidden, game unpaused."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Magenta, TEXT("Pause Menu Dismissed (TODO: Remove widget)"));
	}
	LogStateChange(TEXT("Pause menu dismissed"));
}

void AFCPlayerController::ResumeGame()
{
	// Resume game by popping state from stack (same as ESC key)
	// This ensures Resume button and ESC key use the same code path
	UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	if (!GI)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("ResumeGame: GameInstance is null!"));
		return;
	}
	
	UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
	UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
	
	if (!StateMgr || !UIManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("ResumeGame: StateMgr or UIManager is null!"));
		return;
	}
	
	// Use the same logic as HandlePausePressed: PopState to resume
	if (StateMgr->GetCurrentState() == EFCGameStateID::Paused)
	{
		if (StateMgr->PopState())
		{
			UIManager->HidePauseMenu();
			bIsPauseMenuDisplayed = false;
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ResumeGame: Resumed from pause via PopState, returned to %s"),
				*UEnum::GetValueAsString(StateMgr->GetCurrentState()));
		}
	}
	
	LogStateChange(TEXT("Game resumed"));
}

void AFCPlayerController::SetFallenCompassCameraMode(EFCPlayerCameraMode NewMode)
{
	if (!CameraManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("SetFallenCompassCameraMode: CameraManager is null"));
		return;
	}

	if (CameraManager->GetCameraMode() == NewMode)
	{
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("SetFallenCompassCameraMode: Already in camera mode %d"), static_cast<int32>(NewMode));
		return;
	}
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("SetFallenCompassCameraMode: Changing camera mode from %d to %d"), static_cast<int32>(CameraManager->GetCameraMode()), static_cast<int32>(NewMode));

	SetCameraModeLocal(NewMode);
	LogStateChange(TEXT("Camera mode updated"));
}

void AFCPlayerController::LogStateChange(const FString& Context) const
{
	const UEnum* ModeEnum = StaticEnum<EFCPlayerCameraMode>();
	const FString ModeLabel = (CameraManager && ModeEnum) ? ModeEnum->GetNameStringByValue(static_cast<int64>(CameraManager->GetCameraMode())) : TEXT("Unknown");
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
	if (!CameraManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("SetCameraModeLocal: CameraManager is null"));
		return;
	}

	if (CameraManager->GetCameraMode() == NewMode)
	{
		return; // Already in this mode
	}

	// Delegate to CameraManager based on mode
	switch (NewMode)
	{
		case EFCPlayerCameraMode::MainMenu:
			CameraManager->BlendToMenuCamera(BlendTime);
			break;

		case EFCPlayerCameraMode::FirstPerson:
			CameraManager->BlendToFirstPerson(BlendTime);
			
			// Handle table view cleanup (input/cursor state)
			if (bIsInTableView)
			{
				bIsInTableView = false;
				
				// Disable cursor and mouse events
				bShowMouseCursor = false;
				bEnableClickEvents = false;
				bEnableMouseOverEvents = false;
				
				// Set input mode back to Game Only
				FInputModeGameOnly InputMode;
				SetInputMode(InputMode);
			}
			
			// Restore FirstPerson input mapping
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(FirstPersonMappingContext, 0);
			}
			break;

		case EFCPlayerCameraMode::TableView:
		case EFCPlayerCameraMode::SaveSlotView:
			// Find BP_OfficeDesk in the level to get CameraTargetPoint
			{
				TArray<AActor*> FoundDesks;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundDesks);
				
				for (AActor* Actor : FoundDesks)
				{
					if (Actor->GetName().Contains(TEXT("BP_OfficeDesk")))
					{
						// Find the CameraTargetPoint component
						USceneComponent* CameraTarget = nullptr;
						TArray<USceneComponent*> Components;
						Actor->GetComponents<USceneComponent>(Components);
						
						for (USceneComponent* Component : Components)
						{
							if (Component->GetName().Contains(TEXT("CameraTargetPoint")))
							{
								CameraTarget = Component;
								break;
							}
						}
						
						if (CameraTarget)
						{
							// Delegate table object camera to CameraManager
							CameraManager->BlendToTableObject(Actor, BlendTime);
							
							bIsInTableView = true;
							
							// Switch to StaticScene input mapping
							if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
							{
								Subsystem->ClearAllMappings();
								Subsystem->AddMappingContext(StaticSceneMappingContext, 0);
							}
							
							// Enable cursor and click events
							bShowMouseCursor = true;
							bEnableClickEvents = true;
							bEnableMouseOverEvents = true;
							
							// Set input mode to Game and UI
							FInputModeGameAndUI InputMode;
							InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
							InputMode.SetHideCursorDuringCapture(false);
							SetInputMode(InputMode);
							
							LogStateChange(FString::Printf(TEXT("Camera switched to mode %d"), static_cast<int32>(NewMode)));
							return;
						}
					}
				}
				
				UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TableView: Could not find BP_OfficeDesk or CameraTargetPoint"));
				return;
			}

		case EFCPlayerCameraMode::TopDown:
			CameraManager->BlendToTopDown(BlendTime);
			break;

		default:
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("SetCameraModeLocal: Unknown camera mode %d"), static_cast<int32>(NewMode));
			return;
	}

	LogStateChange(FString::Printf(TEXT("Camera switched to mode %d"), static_cast<int32>(NewMode)));
}

void AFCPlayerController::InitializeMainMenu()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("InitializeMainMenu: Setting up main menu state"));

	// Transition to MainMenu state via GameStateManager
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (GI)
	{
		UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
		if (StateMgr)
		{
			StateMgr->TransitionTo(EFCGameStateID::MainMenu);
		}
	}

	CurrentGameState = EFCGameState::MainMenu; // DEPRECATED: Keep for backward compatibility (Week 3 cleanup)

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

	// Get UIManager and show main menu (reuse GI from above)
	if (GI)
	{
		UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
		if (UIManager)
		{
			UIManager->ShowMainMenu();
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("InitializeMainMenu: UIManager showing main menu"));
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Error, TEXT("InitializeMainMenu: Failed to get UIManager subsystem"));
		}
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("InitializeMainMenu: Failed to get GameInstance"));
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

	// Transition to Office_Exploration state via GameStateManager
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (GI)
	{
		UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
		if (StateMgr)
		{
			StateMgr->TransitionTo(EFCGameStateID::Office_Exploration);
		}

		// Get UIManager and hide main menu
		UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
		if (UIManager)
		{
			UIManager->HideMainMenu();
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TransitionToGameplay: UIManager hiding main menu"));
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Error, TEXT("TransitionToGameplay: Failed to get UIManager subsystem"));
		}
	}

	CurrentGameState = EFCGameState::Gameplay; // DEPRECATED: Keep for backward compatibility (Week 3 cleanup)

	// Spawn player character if not already present
	if (!GetPawn())
	{
		// TODO: Spawn AFCFirstPersonCharacter at designated start location (Task 5.4)
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TransitionToGameplay: No pawn found - need to spawn player"));
	}

	// Blend camera to first-person
	SetCameraModeLocal(EFCPlayerCameraMode::FirstPerson, 2.0f);

	// Delay input restoration until after camera blend — use a UObject timer delegate so it's safe if controller is destroyed during level load
	FTimerHandle InputRestoreTimer;
	FTimerDelegate InputRestoreDel = FTimerDelegate::CreateUObject(this, &AFCPlayerController::RestoreInputAfterBlend);
	GetWorldTimerManager().SetTimer(InputRestoreTimer, InputRestoreDel, 2.0f, false); // Wait for camera blend to complete

	LogStateChange(TEXT("Transitioning to Gameplay"));
}

void AFCPlayerController::ReturnToMainMenu()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ReturnToMainMenu: Starting fade and reload"));

	// Transition to Loading state via GameStateManager
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (GI)
	{
		UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
		if (StateMgr)
		{
			StateMgr->TransitionTo(EFCGameStateID::Loading);
		}

		// Hide pause menu if it's open
		if (bIsPauseMenuDisplayed)
		{
			UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
			if (UIManager)
			{
				UIManager->HidePauseMenu();
				bIsPauseMenuDisplayed = false;
			}
		}
	}

	CurrentGameState = EFCGameState::Loading; // DEPRECATED: Keep for backward compatibility (Week 3 cleanup)

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

void AFCPlayerController::DevQuickSave()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("DevQuickSave: Saving to QuickSave slot"));

	UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("DevQuickSave: Failed to get GameInstance"));
		return;
	}

	bool bSuccess = GameInstance->SaveGame(TEXT("QuickSave"));
	if (bSuccess)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Quick Save successful"));
		}
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("DevQuickSave: Save successful"));
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Quick Save failed"));
		}
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("DevQuickSave: Save failed"));
	}
}

void AFCPlayerController::DevQuickLoad()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("DevQuickLoad: Loading from QuickSave slot"));

	UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("DevQuickLoad: Failed to get GameInstance"));
		return;
	}

	// Check if QuickSave exists
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("QuickSave"), 0))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("No Quick Save found"));
		}
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("DevQuickLoad: QuickSave slot does not exist"));
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Loading Quick Save..."));
	}

	GameInstance->LoadGameAsync(TEXT("QuickSave"));
}

void AFCPlayerController::RestoreInputAfterBlend()
{
	// Restore first-person input
	SetInputMappingMode(EFCInputMappingMode::FirstPerson);

	// Set input mode to game only
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("RestoreInputAfterBlend: Input restored after camera blend"));
}

void AFCPlayerController::RestorePlayerPositionDeferred()
{
	UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->RestorePlayerPosition();
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("RestorePlayerPositionDeferred: Player position restored after transition"));
	}
}

void AFCPlayerController::HandleQuickSavePressed()
{
	DevQuickSave();
}

void AFCPlayerController::HandleQuickLoadPressed()
{
	DevQuickLoad();
}

void AFCPlayerController::FadeScreenOut(float Duration, bool bShowLoading)
{
	UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("FadeScreenOut: Failed to get GameInstance"));
		return;
	}

	UFCTransitionManager* TransitionMgr = GameInstance->GetSubsystem<UFCTransitionManager>();
	if (!TransitionMgr)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("FadeScreenOut: Failed to get TransitionManager subsystem"));
		return;
	}

	TransitionMgr->BeginFadeOut(Duration, bShowLoading);
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("FadeScreenOut: Initiated fade (Duration: %.2fs, Loading: %s)"),
		Duration, bShowLoading ? TEXT("Yes") : TEXT("No"));
}

void AFCPlayerController::FadeScreenIn(float Duration)
{
	UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("FadeScreenIn: Failed to get GameInstance"));
		return;
	}

	UFCTransitionManager* TransitionMgr = GameInstance->GetSubsystem<UFCTransitionManager>();
	if (!TransitionMgr)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("FadeScreenIn: Failed to get TransitionManager subsystem"));
		return;
	}

	TransitionMgr->BeginFadeIn(Duration);
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("FadeScreenIn: Initiated fade (Duration: %.2fs)"), Duration);
}

void AFCPlayerController::HandleTableObjectClick()
{
	// Perform line trace from cursor position
	FHitResult HitResult;
	bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Cursor hit actor: %s"), *HitActor->GetName());

		// Check if actor implements IFCTableInteractable
		if (HitActor->Implements<UFCTableInteractable>())
		{
			// Check if interaction is allowed
			bool bCanInteract = IFCTableInteractable::Execute_CanInteract(HitActor);
			if (bCanInteract)
			{
				UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Table object clicked: %s"), *HitActor->GetName());
				OnTableObjectClicked(HitActor);
			}
			else
			{
				UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Table object cannot be interacted with: %s"), *HitActor->GetName());
			}
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Hit actor does not implement IFCTableInteractable: %s"), *HitActor->GetName());
		}
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("Cursor click - no actor hit"));
	}
}

void AFCPlayerController::OnTableObjectClicked(AActor* TableObject)
{
	if (!TableObject || !TableObject->Implements<UFCTableInteractable>())
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("OnTableObjectClicked: Invalid table object"));
		return;
	}

	// Get UIManager for widget management
	UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	UFCUIManager* UIManager = GI ? GI->GetSubsystem<UFCUIManager>() : nullptr;

	// Close current widget if one is open (switching between table objects)
	if (UIManager && UIManager->IsTableWidgetOpen())
	{
		UIManager->CloseTableWidget();
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Closed previous table widget"));
	}

	// Get camera target transform from table object
	FTransform CameraTargetTransform = IFCTableInteractable::Execute_GetCameraTargetTransform(TableObject);

	// Camera transitions now handled by CameraManager component
	// Delegate to CameraManager for camera spawning and blending
	if (CameraManager)
	{
		CameraManager->BlendToTableObject(TableObject, 2.0f);
	}

	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Blending camera to table object: %s"), *TableObject->GetName());

	// Show widget after camera blend completes (delay via timer)
	FTimerHandle ShowWidgetTimerHandle;
	GetWorldTimerManager().SetTimer(ShowWidgetTimerHandle, [this, TableObject, UIManager]()
	{
		if (UIManager)
		{
			UIManager->ShowTableWidget(TableObject);

			// Set input mode to GameAndUI so player controller can receive ESC key
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}, 2.0f, false);
}

void AFCPlayerController::CloseTableWidget()
{
	// Get UIManager for widget management
	UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	UFCUIManager* UIManager = GI ? GI->GetSubsystem<UFCUIManager>() : nullptr;

	// Remove current widget if one is displayed
	if (UIManager && UIManager->IsTableWidgetOpen())
	{
		UIManager->CloseTableWidget();
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Closed table widget"));

		// Restore input mode to GameAndUI (keep mouse cursor for clicking other table objects)
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		// Restore previous table camera (e.g., from Map back to OfficeDesk)
		if (CameraManager)
		{
			CameraManager->RestorePreviousTableCamera(2.0f);
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Widget closed, restoring previous table camera"));
		}
	}
	// Note: If no widget is open, ESC is handled by HandlePausePressed() which returns to FirstPerson
}
