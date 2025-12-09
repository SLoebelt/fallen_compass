// Copyright Slomotion Games. All Rights Reserved.

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
#include "Interaction/FCInteractionComponent.h"
#include "Interaction/IFCInteractablePOI.h"
#include "UFCGameInstance.h"
#include "FCTransitionManager.h"
#include "Core/FCLevelManager.h"
#include "Core/FCUIManager.h"
#include "Core/FCGameStateManager.h"
#include "Core/FCLevelTransitionManager.h"
#include "Components/FCInputManager.h"
#include "Interaction/FCTableInteractable.h"
#include "GameFramework/Character.h"
#include "Components/FCCameraManager.h"
#include "World/FCOverworldCamera.h"
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Characters/Convoy/FCConvoyMember.h"
#include "Characters/FC_ExplorerCharacter.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "Components/FCPlayerModeCoordinator.h"
#include "Core/FCUIBlockSubsystem.h"

DEFINE_LOG_CATEGORY(LogFallenCompassPlayerController);

AFCPlayerController::AFCPlayerController()
{
	// Create camera manager component
	CameraManager = CreateDefaultSubobject<UFCCameraManager>(TEXT("CameraManager"));

	// Create input manager component
	InputManager = CreateDefaultSubobject<UFCInputManager>(TEXT("InputManager"));

	// Create interaction component (handles POI interactions across all scenes)
	InteractionComponent = CreateDefaultSubobject<UFCInteractionComponent>(TEXT("InteractionComponent"));

	// Create player mode coordinator component
	PlayerModeCoordinator = CreateDefaultSubobject<UFCPlayerModeCoordinator>(TEXT("PlayerModeCoordinator"));

	bIsPauseMenuDisplayed = false;
	bShowMouseCursor = false;
	CurrentGameState = EFCGameState::MainMenu; // DEPRECATED: Use GameStateManager instead
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
		// For now we always restore the player position when a save
		// load is pending; otherwise, the Blueprint-level startup
		// calls InitializeMainMenu after setting the menu camera.
		GameInstance->RestorePlayerPosition();
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Controller ready, input mode will be set by game state transitions"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("BeginPlay: Failed to get GameInstance"));
	}

	if (!IsLocalController())
    {
        return;
    }

	if (CameraManager && CameraManager->GetCameraMode() == EFCPlayerCameraMode::POIScene)
	{
		if (!Cast<AFC_ExplorerCharacter>(GetPawn()))
		{
			UE_LOG(LogFallenCompassPlayerController, Error,
				TEXT("Camp/POI mode but controller is not possessing AFC_ExplorerCharacter. Check Camp GameMode / pawn placement."));
		}
	}

	// Optional: make sure config exists before applying mappings
    const UFCInputConfig* Config = InputManager ? InputManager->GetInputConfig() : nullptr;
    if (!Config)
    {
        UE_LOG(LogFallenCompassPlayerController, Error, TEXT("BeginPlay: InputConfig missing on InputManager"));
        return;
    }
}

const UFCInputConfig* AFCPlayerController::GetInputConfig() const
{
    return InputManager ? InputManager->GetInputConfig() : nullptr;
}

void AFCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("EnhancedInputComponent missing on %s; bindings skipped."), *GetName());
		return;
	}

	const UFCInputConfig* Config = InputManager ? InputManager->GetInputConfig() : nullptr;
    if (!Config)
    {
        UE_LOG(LogFallenCompassPlayerController, Error, TEXT("InputConfig missing (assign it on the InputManager component in BP_FC_PlayerController)."));
        return;
    }

    if (Config->InteractAction) EnhancedInput->BindAction(Config->InteractAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleInteractPressed);
    if (Config->QuickSaveAction) EnhancedInput->BindAction(Config->QuickSaveAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleQuickSavePressed);
    if (Config->QuickLoadAction) EnhancedInput->BindAction(Config->QuickLoadAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleQuickLoadPressed);
    if (Config->EscapeAction) EnhancedInput->BindAction(Config->EscapeAction, ETriggerEvent::Started, this, &AFCPlayerController::HandlePausePressed);
    if (Config->ClickAction) EnhancedInput->BindAction(Config->ClickAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleClick);

    if (Config->OverworldPanAction) EnhancedInput->BindAction(Config->OverworldPanAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleOverworldPan);
    if (Config->OverworldZoomAction) EnhancedInput->BindAction(Config->OverworldZoomAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleOverworldZoom);
    if (Config->ToggleOverworldMapAction) EnhancedInput->BindAction(Config->ToggleOverworldMapAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleToggleOverworldMap);
}

void AFCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APawn* PreviousPawn = GetPawn(); // After Super, this is the new pawn; we’ll log old via cached var below

    static int32 PossessCounter = 0;
    ++PossessCounter;

	if (!InPawn)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("OnPossess called with null pawn"));
		return;
	}

	UE_LOG(LogFallenCompassPlayerController, Log,
        TEXT("OnPossess #%d: Controller=%s NewPawn=%s Map=%s"),
        PossessCounter,
        *GetName(),
        InPawn ? *InPawn->GetName() : TEXT("nullptr"),
        *GetWorld()->GetMapName());
}

void AFCPlayerController::HandleToggleOverworldMap()
{
	UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleToggleOverworldMap called"));

	// Only meaningful while in Overworld-related camera mode/state, but
	// allowing it to toggle anywhere is harmless as long as GI/UI are valid.

	// If widget is already open, close it and restore input/cursor
	if (CurrentOverworldMapWidget)
	{
		if (UFCGameInstance* GI = GetGameInstance<UFCGameInstance>())
		{
			if (UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>())
			{
				if (UIManager->FocusedBlockingWidget == CurrentOverworldMapWidget)
				{
					UIManager->SetFocusedBlockingWidget(nullptr);
				}
			}
		}

		CurrentOverworldMapWidget->RemoveFromParent();
		CurrentOverworldMapWidget = nullptr;

		if (PlayerModeCoordinator)
		{
			PlayerModeCoordinator->ReapplyCurrentMode();
		}
		else
		{
			bShowMouseCursor = false;
			FInputModeGameOnly M;
			SetInputMode(M);
		}

		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandleToggleOverworldMap: Closed Overworld map widget"));
		return;
	}

	// Create and show the view-only Overworld map widget
	UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	if (!GI)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("HandleToggleOverworldMap: GameInstance is null"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("HandleToggleOverworldMap: World is null"));
		return;
	}

	// Use UIManager helper to create/show WBP_OverworldMap (view-only Overworld map HUD)
	UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
	if (!UIManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("HandleToggleOverworldMap: UIManager subsystem is null"));
		return;
	}

	UUserWidget* NewWidget = UIManager->ShowOverworldMapHUD(this);
	if (!NewWidget)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("HandleToggleOverworldMap: ShowOverworldMapHUD returned null"));
		return;
	}

	CurrentOverworldMapWidget = NewWidget;

	// Enable cursor and game+UI input when map is open
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandleToggleOverworldMap: Opened Overworld map widget"));
}

void AFCPlayerController::HandleInteractPressed()
{
	if (!CanWorldInteract())
	{
		return;
	}

	EFCPlayerCameraMode CurrentMode = CameraManager ? CameraManager->GetCameraMode() : EFCPlayerCameraMode::FirstPerson;

	// Camp / POI local scenes: reuse Overworld POI interaction pattern but
	// treat it as a Camp-only interact. This avoids falling back into the
	// Office table/FirstPerson interaction flow while in Camp.
	if (CurrentMode == EFCPlayerCameraMode::POIScene)
	{
		// Mirror the Overworld TopDown POI handling: raycast under cursor,
		// look for IFCInteractablePOI, and show action selection or auto-trigger.
		FHitResult HitResult;
		const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		if (bHit && HitResult.GetActor())
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
			{
				UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandleInteractPressed(POIScene): POI detected - %s"), *HitActor->GetName());

				// Delegate to InteractionComponent (same as Overworld)
				if (InteractionComponent)
				{
					InteractionComponent->HandlePOIClick(HitActor);
					return;
				}
				else
				{
					UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleInteractPressed(POIScene): No InteractionComponent on controller"));
				}
			}
			else
			{
				UE_LOG(LogFallenCompassPlayerController, Verbose,
					TEXT("HandleInteractPressed(POIScene): Hit actor is not IFCInteractablePOI: %s"),
					*HitActor->GetName());
			}
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleInteractPressed(POIScene): No actor under cursor"));
		}

		// No valid POI/interact found in Camp; treat as benign no-op.
		return;
	}

	// Week 3: Route to Overworld POI interaction if in TopDown mode
	if (CurrentMode == EFCPlayerCameraMode::TopDown)
	{
		// Raycast to check if clicking on POI
		FHitResult HitResult;
		bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

		if (bHit && HitResult.GetActor())
		{
			AActor* HitActor = HitResult.GetActor();

			// Check if actor implements IFCInteractablePOI interface
			if (HitActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
			{
				UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandleInteractPressed: POI detected - %s"), *HitActor->GetName());

				// Delegate to InteractionComponent for POI handling
				if (InteractionComponent)
				{
					InteractionComponent->HandlePOIClick(HitActor);

					// After action selection, movement will be triggered by HandleClick
					// This maintains separation: Interact = select action, Click = move to location
					return;
				}
				else
				{
					UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleInteractPressed: No InteractionComponent on controller"));
				}
			}
		}

		// No POI under cursor; benign no-op in TopDown.
		return;
	}

	if (CurrentMode == EFCPlayerCameraMode::FirstPerson)
	{
		// Use the interaction component on the controller
		if (InteractionComponent)
		{
			// Let the InteractionComponent handle interaction (desk BP OnInteract, etc.).
			InteractionComponent->Interact();
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleInteractPressed: No InteractionComponent on controller"));
		}
	}
	else
	{
		// In table view or other modes, interaction is handled via click/ESC.
	}
}

void AFCPlayerController::HandlePausePressed()
{
	// Table view: ESC either closes an open table widget (object focus)
	// or exits back to Office_Exploration when no widget is open.
	if (GetCameraMode() == EFCPlayerCameraMode::TableView)
	{
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ESC pressed in TableView mode"));

		UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
		if (!GI)
		{
			return;
		}

		UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
		UFCLevelTransitionManager* LevelTransitionMgr = GI->GetSubsystem<UFCLevelTransitionManager>();

		// If any table widget is open, close it and remain in table view.
		if (UIManager && UIManager->IsTableWidgetOpen())
		{
			UIManager->CloseTableWidget();
			OfficeTableViewSubMode = EOfficeTableViewSubMode::Desk;
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandlePausePressed: Closed table widget, staying in TableView (desk)"));
			return;
		}

		// No widget open: exit table view back to Office_Exploration.
		if (LevelTransitionMgr)
		{
			LevelTransitionMgr->ExitOfficeTableView();
		}
		OfficeTableViewSubMode = EOfficeTableViewSubMode::None;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandlePausePressed: No table widget open, exiting TableView to Office_Exploration"));
		return;
	}

	// If in gameplay state (first-person, can move), ESC toggles pause menu
	// Use GameStateManager's state stack for proper pause/resume handling
	UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	if (!GI) return;

	UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
	UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();

	if (!StateMgr || !UIManager) return;

	// If we're in ExpeditionSummary, ESC should behave like closing the summary widget
	if (StateMgr->GetCurrentState() == EFCGameStateID::ExpeditionSummary)
	{
		UFCLevelTransitionManager* TransitionMgr = GI->GetSubsystem<UFCLevelTransitionManager>();
		if (TransitionMgr)
		{
			TransitionMgr->CloseExpeditionSummaryAndReturnToOffice();
		}
		return;
	}

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

	// Check if we're in a pausable state (Office_Exploration, Office_TableView, or Overworld_Travel)
	EFCGameStateID CurrentState = StateMgr->GetCurrentState();
	if (CurrentState == EFCGameStateID::Office_Exploration ||
	    CurrentState == EFCGameStateID::Office_TableView ||
	    CurrentState == EFCGameStateID::Overworld_Travel)
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
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Magenta, TEXT("Pause Menu Requested (TODO - Instantiate widget)"));
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
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Magenta, TEXT("Pause Menu Dismissed (TODO - Remove widget)"));
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

			// Mirror HandlePausePressed: if we resumed into Overworld_Travel,
			// restore cursor and GameAndUI input mode.
			if (StateMgr->GetCurrentState() == EFCGameStateID::Overworld_Travel)
			{
				bShowMouseCursor = true;
				FInputModeGameAndUI InputMode;
				InputMode.SetHideCursorDuringCapture(false);
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetInputMode(InputMode);
				UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ResumeGame: Resumed Overworld_Travel with cursor visible"));
			}
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
	if (!InputManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("SetInputMappingMode: InputManager is null!"));
		return;
	}

	InputManager->SetInputMappingMode(NewMode);
}

EFCInputMappingMode AFCPlayerController::GetCurrentMappingMode() const
{
	if (!InputManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("GetCurrentMappingMode: InputManager is null!"));
		return EFCInputMappingMode::FirstPerson;
	}

	return InputManager->GetCurrentMappingMode();
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
		return;
	}

	// Delegate to CameraManager based on mode
	switch (NewMode)
	{
		case EFCPlayerCameraMode::MainMenu:
			CameraManager->BlendToMenuCamera(BlendTime);
			break;

		case EFCPlayerCameraMode::FirstPerson:
			CameraManager->BlendToFirstPerson(BlendTime);

			bIsInTableView = false;
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

		case EFCPlayerCameraMode::POIScene:
		{
			if (!CameraManager)
			{
				UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("SetCameraModeLocal: CameraManager missing"));
				return;
			}

			// Profile/coordinator should decide cursor + input mode + mapping; this is camera-only.
			CameraManager->BlendToPOISceneCamera(POISceneCameraActor, BlendTime);
			break;
		}

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

	// TODO - remove deprecated CurrentGameState usage after week 3 cleanup
	CurrentGameState = EFCGameState::MainMenu; // DEPRECATED: Keep for backward compatibility (Week 3 cleanup)

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
		// TODO - Spawn AFCFirstPersonCharacter at designated start location (Task 5.4)
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TransitionToGameplay: No pawn found - need to spawn player"));
	}

	LogStateChange(TEXT("Transitioning to Gameplay"));
}

void AFCPlayerController::ReturnToMainMenu()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("ReturnToMainMenu: Starting fade and reload"));

	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("ReturnToMainMenu: GameInstance is null"));
		return;
	}

	UFCLevelTransitionManager* TransitionMgr = GI->GetSubsystem<UFCLevelTransitionManager>();
	if (!TransitionMgr)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("ReturnToMainMenu: LevelTransitionManager subsystem missing"));
		return;
	}

	TransitionMgr->ReturnToMainMenuFromGameplay();
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

void AFCPlayerController::HandleOverworldPan(const FInputActionValue& Value)
{
	// Forward input to Overworld camera (via CameraManager)
	if (!CameraManager || CameraManager->GetCameraMode() != EFCPlayerCameraMode::TopDown)
	{
		return;
	}

	// Find the Overworld camera and call HandlePan
	AActor* ViewTarget = GetViewTarget();
	AFCOverworldCamera* OverworldCamera = Cast<AFCOverworldCamera>(ViewTarget);
	if (OverworldCamera)
	{
		OverworldCamera->HandlePan(Value);
	}
}

void AFCPlayerController::HandleOverworldZoom(const FInputActionValue& Value)
{
	// Forward input to Overworld camera (via CameraManager)
	if (!CameraManager || CameraManager->GetCameraMode() != EFCPlayerCameraMode::TopDown)
	{
		return;
	}

	// Find the Overworld camera and call HandleZoom
	AActor* ViewTarget = GetViewTarget();
	AFCOverworldCamera* OverworldCamera = Cast<AFCOverworldCamera>(ViewTarget);
	if (OverworldCamera)
	{
		OverworldCamera->HandleZoom(Value);
	}
}

void AFCPlayerController::OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState)
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: State changed from %s to %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	// Additional handling can be added here if needed
	ApplyPresentationForGameState(OldState, NewState);
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

void AFCPlayerController::HandleClick(const FInputActionValue& Value)
{
	// Check camera mode to determine click behavior
	if (!CameraManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleClick: CameraManager is null"));
		return;
	}

	EFCPlayerCameraMode CurrentMode = CameraManager->GetCameraMode();

	// POI/local scene (e.g. Camp): command AI-controlled explorer to move (same pattern as convoy)
	if (CurrentMode == EFCPlayerCameraMode::POIScene)
	{
		if (!CanWorldClick())
		{
			return;
		}

		FHitResult HitResult;
		const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		if (!bHit)
		{
			return;
		}

		if (!Cast<AFC_ExplorerCharacter>(GetPawn()))
		{
			UE_LOG(LogFallenCompassPlayerController, Warning,
				TEXT("HandleClick(POIScene): Not possessing Explorer pawn; cannot move."));
			return;
		}

		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UIFCInteractablePOI::StaticClass()))
		{
			UE_LOG(LogFallenCompassPlayerController, Log,
				TEXT("HandleClick(POIScene): Clicked on IFCInteractablePOI - %s (queuing interaction)"),
				*HitActor->GetName());

			// Delegate to InteractionComponent (same as Overworld)
			if (InteractionComponent)
			{
				InteractionComponent->HandlePOIClick(HitActor);
			}
			return; // Don't also move if we clicked a POI
		}

		// Move explorer to clicked location
		MoveExplorerToLocation(HitResult.Location);
		return;
	}

	// TopDown mode: Click-to-move convoy (POI interaction is handled by HandleInteractPressed)
	if (CurrentMode == EFCPlayerCameraMode::TopDown)
	{
		HandleOverworldClickMove();
	}
	// TableView/FirstPerson mode: Interact with table objects
	else if (CurrentMode == EFCPlayerCameraMode::TableView || CurrentMode == EFCPlayerCameraMode::FirstPerson)
	{
		HandleTableObjectClick();
	}
}

void AFCPlayerController::HandleTableObjectClick()
{
	// If a blocking table/map widget is open, ignore world/table clicks entirely.
	if (!CanWorldClick())
	{
		return;
	}

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
		// No actor hit under cursor; benign no-op.
	}
}

bool AFCPlayerController::CanProcessWorldInteraction() const
{
	const UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	if (!GI)
	{
		return true; // No game instance; fall back to allowing interaction
	}

	const UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
	if (!UIManager)
	{
		return true;
	}

	// Block world interaction whenever a focused blocking widget is open
	// (e.g., expedition planning, overworld map). Fallback to legacy
	// behavior of treating table widgets as blocking if no focused
	// widget is configured.
	if (UIManager->IsFocusedWidgetOpen())
	{
		return false;
	}

	return !UIManager->IsTableWidgetOpen();
}

bool AFCPlayerController::CanWorldClick() const
{
    const UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
    if (!GI) return true;

    if (const UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
    {
        return Blocker->CanWorldClick();
    }

    // Safety fallback: keep old behavior until all widgets migrate
	// TODO - remove when migration is complete
    return CanProcessWorldInteraction();
}

bool AFCPlayerController::CanWorldInteract() const
{
    const UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
    if (!GI) return true;

    if (const UFCUIBlockSubsystem* Blocker = GI->GetSubsystem<UFCUIBlockSubsystem>())
    {
        return Blocker->CanWorldInteract();
    }

	// Safety fallback: keep old behavior until all widgets migrate
	// TODO - remove when migration is complete
    return CanProcessWorldInteraction();
}

void AFCPlayerController::OnTableObjectClicked(AActor* TableObject)
{
	if (!TableObject || !TableObject->Implements<UFCTableInteractable>())
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("OnTableObjectClicked: Invalid table object"));
		return;
	}

	UFCGameInstance* GI = GetGameInstance<UFCGameInstance>();
	if (!GI)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("OnTableObjectClicked: GameInstance is null"));
		return;
	}

	UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
	UFCUIManager* UIManager = GI->GetSubsystem<UFCUIManager>();
	if (!StateMgr || !UIManager)
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("OnTableObjectClicked: StateMgr or UIManager is null"));
		return;
	}

	EFCGameStateID CurrentState = StateMgr->GetCurrentState();
	// Allow clicks either directly from Office_Exploration (legacy path) or
	// from desk-level Office_TableView when sub-mode is Desk.
	const bool bFromOfficeExploration = (CurrentState == EFCGameStateID::Office_Exploration);
	const bool bFromDeskTableView = (CurrentState == EFCGameStateID::Office_TableView &&
		OfficeTableViewSubMode == EOfficeTableViewSubMode::Desk);

	if (!bFromOfficeExploration && !bFromDeskTableView)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning,
			TEXT("OnTableObjectClicked: Ignored because state is %s and sub-mode is %s (expected Office_Exploration or Office_TableView+Desk)"),
			*UEnum::GetValueAsString(CurrentState),
			*UEnum::GetValueAsString(OfficeTableViewSubMode));
		return;
	}

	// Close any existing table widget before opening a new one.
	if (UIManager->IsTableWidgetOpen())
	{
		UIManager->CloseTableWidget();
	}

	// Delegate high-level state change to LevelTransitionManager when coming
	// directly from Office_Exploration; from desk-level Office_TableView we
	// stay in the same global state and only adjust sub-mode/camera/UI.
	if (bFromOfficeExploration)
	{
		UFCLevelTransitionManager* LevelTransitionMgr = GI->GetSubsystem<UFCLevelTransitionManager>();
		if (LevelTransitionMgr)
		{
			LevelTransitionMgr->EnterOfficeTableView(TableObject);
		}
	}

	// Perform the table-view camera blend and open the table widget using
	// existing helpers so behaviour matches the pre-refactor flow.
	if (CameraManager)
	{
		CameraManager->BlendToTableObject(TableObject, 2.0f);
	}

	// Switch input/cursor to table interaction mode.
	SetInputMappingMode(EFCInputMappingMode::StaticScene);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// Finally, show the appropriate table widget for this object and mark
	// that we are now focused on a table object.
	UIManager->ShowTableWidget(TableObject);
	OfficeTableViewSubMode = EOfficeTableViewSubMode::Object;
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

void AFCPlayerController::HandleOverworldClickMove()
{
	// If a blocking UI widget (e.g., overworld map or table) is open,
	// ignore click-to-move so the map can be interacted with safely.
	if (!CanWorldClick())
	{
		UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleOverworldClickMove: UI is blocking overworld movement"));
		return;
	}

	// Get mouse cursor hit result
	FHitResult HitResult;
	bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (!bHit)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleOverworldClickMove: No hit under cursor"));
		return;
	}

	// Get convoy reference
	if (!ActiveConvoy)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleOverworldClickMove: No convoy reference"));
		return;
	}

	// Get leader member from convoy
	AFCConvoyMember* LeaderMember = ActiveConvoy->GetLeaderMember();
	if (!LeaderMember)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleOverworldClickMove: No leader member found"));
		return;
	}

	// Get leader's AI controller
	AAIController* AIController = Cast<AAIController>(LeaderMember->GetController());
	if (!AIController)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleOverworldClickMove: Leader has no AI controller"));
		return;
	}

	// Project hit location to NavMesh
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		bool bFoundPath = NavSys->ProjectPointToNavigation(HitResult.Location, NavLocation);

		if (bFoundPath)
		{
			// Send move command to AI controller
			AIController->MoveToLocation(NavLocation.Location);
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandleOverworldClickMove: Moving convoy to %s"),
				*NavLocation.Location.ToString());

			// Visual feedback
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
					FString::Printf(TEXT("Moving to: %s"), *NavLocation.Location.ToString()));
			}
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleOverworldClickMove: Failed to project to NavMesh"));

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
					TEXT("Cannot move there - no valid path"));
			}
		}
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("HandleOverworldClickMove: NavigationSystem not found"));
	}
}

void AFCPlayerController::SetActiveConvoy(AFCOverworldConvoy* InConvoy)
{
	ActiveConvoy = InConvoy;

	// Avoid double-binding
    ActiveConvoy->OnConvoyPOIOverlap.RemoveAll(InteractionComponent);
    ActiveConvoy->OnConvoyPOIOverlap.AddDynamic(InteractionComponent, &UFCInteractionComponent::NotifyArrivedAtPOI);

    UE_LOG(LogFallenCompassPlayerController, Log,
        TEXT("SetActiveConvoy: Set active convoy to %s and bound OnConvoyPOIOverlap -> InteractionComponent"),
		*GetNameSafe(ActiveConvoy));
}

void AFCPlayerController::MoveConvoyToLocation(const FVector& TargetLocation)
{
	if (!ActiveConvoy)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("MoveConvoyToLocation: No convoy possessed"));
		return;
	}

	// Get convoy leader
	AFCConvoyMember* LeaderMember = ActiveConvoy->GetLeaderMember();
	if (!LeaderMember)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("MoveConvoyToLocation: No leader member found"));
		return;
	}

	// Get AI controller
	AAIController* AIController = Cast<AAIController>(LeaderMember->GetController());
	if (!AIController)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("MoveConvoyToLocation: Leader has no AI controller"));
		return;
	}

	// Project to NavMesh and move
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		bool bFoundPath = NavSys->ProjectPointToNavigation(TargetLocation, NavLocation);

		if (bFoundPath)
		{
			AIController->MoveToLocation(NavLocation.Location);
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("MoveConvoyToLocation: Moving to %s"),
				*NavLocation.Location.ToString());
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("MoveConvoyToLocation: Failed to project to NavMesh"));
		}
	}
}

void AFCPlayerController::MoveExplorerToLocation(const FVector& WorldLocation)
{
    AFC_ExplorerCharacter* Explorer = Cast<AFC_ExplorerCharacter>(GetPawn());
    if (!Explorer)
    {
        UE_LOG(LogFallenCompassPlayerController, Warning,
            TEXT("MoveExplorerToLocation: Possessed pawn is not Explorer (Got: %s)"),
            GetPawn() ? *GetPawn()->GetName() : TEXT("nullptr"));
        return;
    }

    // Delegate actual pathfinding + movement to the Explorer pawn.
    Explorer->MoveExplorerToLocation(WorldLocation);

    UE_LOG(LogFallenCompassPlayerController, Log,
        TEXT("MoveExplorerToLocation: Requested move for Explorer %s to %s"),
        *Explorer->GetName(),
        *WorldLocation.ToString());
}

// If POISceneCameraActor is null, UFCCameraManager will auto-resolve by tag/name.
// TODO - Consider removing this setter and just setting the camera directly in CameraManager.
void AFCPlayerController::SetPOISceneCameraActor(ACameraActor* InPOICamera)
{
	POISceneCameraActor = InPOICamera;

	if (!POISceneCameraActor)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("SetPOISceneCameraActor: InPOICamera is null"));
		return;
	}

	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("SetPOISceneCameraActor: Registered POI scene camera %s"),
		*POISceneCameraActor->GetName());
}

void AFCPlayerController::BindOverworldConvoyDelegates()
{
    if (!InteractionComponent)
    {
        UE_LOG(LogFallenCompassPlayerController, Error,
            TEXT("BindOverworldConvoyDelegates: InteractionComponent is null on %s"), *GetName());
        return;
    }

    // Find convoy in THIS level (Overworld only)
    TArray<AActor*> FoundConvoys;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFCOverworldConvoy::StaticClass(), FoundConvoys);

    if (FoundConvoys.Num() == 0)
    {
        UE_LOG(LogFallenCompassPlayerController, Error,
            TEXT("BindOverworldConvoyDelegates: No OverworldConvoy found in map %s"), *GetWorld()->GetMapName());
        ActiveConvoy = nullptr;
        return;
    }

	AFCOverworldConvoy* FoundConvoy = Cast<AFCOverworldConvoy>(FoundConvoys[0]);
    SetActiveConvoy(FoundConvoy);
}

void AFCPlayerController::UnbindOverworldConvoyDelegates()
{
    if (ActiveConvoy && InteractionComponent)
    {
        ActiveConvoy->OnConvoyPOIOverlap.RemoveAll(InteractionComponent);
        UE_LOG(LogFallenCompassPlayerController, Log,
            TEXT("UnbindOverworldConvoyDelegates: Unbound convoy delegates from InteractionComponent"));
    }

    ActiveConvoy = nullptr;
}

void AFCPlayerController::ApplyPresentationForGameState(EFCGameStateID OldState, EFCGameStateID NewState)
{
	// State-specific extras (NOT camera/input/cursor)
    UnbindOverworldConvoyDelegates();

    if (NewState == EFCGameStateID::Overworld_Travel)
    {
        BindOverworldConvoyDelegates();
    }

    // If there are other non-profile side-effects, add them here.
}
