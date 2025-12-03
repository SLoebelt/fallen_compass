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
#include "../Interaction/IFCInteractablePOI.h"
#include "UFCGameInstance.h"
#include "FCTransitionManager.h"
#include "Core/FCLevelManager.h"
#include "Core/FCUIManager.h"
#include "Core/FCGameStateManager.h"
#include "Components/FCInputManager.h"
#include "../Interaction/FCTableInteractable.h"
#include "GameFramework/Character.h"
#include "Components/FCCameraManager.h"
#include "World/FCOverworldCamera.h"
#include "Characters/Convoy/FCOverworldConvoy.h"
#include "Characters/Convoy/FCConvoyMember.h"
#include "AIController.h"
#include "NavigationSystem.h"

DEFINE_LOG_CATEGORY(LogFallenCompassPlayerController);

AFCPlayerController::AFCPlayerController()
{
	// Create camera manager component
	CameraManager = CreateDefaultSubobject<UFCCameraManager>(TEXT("CameraManager"));
	
	// Create input manager component
	InputManager = CreateDefaultSubobject<UFCInputManager>(TEXT("InputManager"));
	
	bIsPauseMenuDisplayed = false;
	bShowMouseCursor = false;
	CurrentGameState = EFCGameState::MainMenu; // DEPRECATED: Use GameStateManager instead

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

	// Week 3: Overworld input actions
	static ConstructorHelpers::FObjectFinder<UInputAction> OverworldPanActionFinder(TEXT("/Game/FC/Input/IA_OverworldPan"));
	if (OverworldPanActionFinder.Succeeded())
	{
		OverworldPanAction = OverworldPanActionFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_OverworldPan"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_OverworldPan."));
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> OverworldZoomActionFinder(TEXT("/Game/FC/Input/IA_OverworldZoom"));
	if (OverworldZoomActionFinder.Succeeded())
	{
		OverworldZoomAction = OverworldZoomActionFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_OverworldZoom"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_OverworldZoom."));
	}

	// Week 4: Overworld map toggle action (M key)
	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleOverworldMapActionFinder(TEXT("/Game/FC/Input/IA_ToggleOverworldMap"));
	if (ToggleOverworldMapActionFinder.Succeeded())
	{
		ToggleOverworldMapAction = ToggleOverworldMapActionFinder.Object;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Loaded IA_ToggleOverworldMap"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("Failed to load IA_ToggleOverworldMap."));
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

		// Subscribe to game state changes
		UFCGameStateManager* StateMgr = GameInstance->GetSubsystem<UFCGameStateManager>();
		if (StateMgr)
		{
			StateMgr->OnStateChanged.AddDynamic(this, &AFCPlayerController::OnGameStateChanged);
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Subscribed to GameStateManager.OnStateChanged"));
			
			// Log current state to verify subscription timing
			EFCGameStateID CurrentState = StateMgr->GetCurrentState();
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Current game state is %s"), 
				*UEnum::GetValueAsString(CurrentState));
			
			// If we're already in Overworld_Travel state, manually trigger the handler
			if (CurrentState == EFCGameStateID::Overworld_Travel)
			{
				UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("BeginPlay: Already in Overworld_Travel state, manually triggering camera/input switch"));
				OnGameStateChanged(EFCGameStateID::None, CurrentState);
			}
		}
		else
		{
			UE_LOG(LogFallenCompassPlayerController, Error, TEXT("BeginPlay: Failed to get GameStateManager subsystem"));
		}
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Error, TEXT("BeginPlay: Failed to get GameInstance"));
	}

	// Find convoy in level if we're in Overworld
	TArray<AActor*> FoundConvoys;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFCOverworldConvoy::StaticClass(), FoundConvoys);
	
	if (FoundConvoys.Num() > 0)
	{
		PossessedConvoy = Cast<AFCOverworldConvoy>(FoundConvoys[0]);
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: Found convoy: %s"), *PossessedConvoy->GetName());
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("BeginPlay: No convoy found in level (expected in Office)"));
	}

	// Note: MenuCamera reference will be set in Blueprint (BP_FC_PlayerController)
}

void AFCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Apply initial mapping context (default FirstPerson mode)
	SetInputMappingMode(EFCInputMappingMode::FirstPerson);

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
		EnhancedInput->BindAction(ClickAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleClick);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("ClickAction not assigned on %s."), *GetName());
	}

	// Week 3: Bind Overworld input actions
	if (OverworldPanAction)
	{
		EnhancedInput->BindAction(OverworldPanAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleOverworldPan);
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Bound IA_OverworldPan"));
	}
	
	if (OverworldZoomAction)
	{
		EnhancedInput->BindAction(OverworldZoomAction, ETriggerEvent::Triggered, this, &AFCPlayerController::HandleOverworldZoom);
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Bound IA_OverworldZoom"));
	}

	// Week 4: Bind Overworld map toggle
	if (ToggleOverworldMapAction)
	{
		EnhancedInput->BindAction(ToggleOverworldMapAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleToggleOverworldMap);
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("Bound IA_ToggleOverworldMap"));
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("ToggleOverworldMapAction not assigned on %s."), *GetName());
	}
}

void AFCPlayerController::HandleToggleOverworldMap()
{
	UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleToggleOverworldMap called"));

	// Only meaningful while in Overworld-related camera mode/state, but
	// allowing it to toggle anywhere is harmless as long as GI/UI are valid.

	// If widget is already open, close it and restore input/cursor
	if (CurrentOverworldMapWidget)
	{
		CurrentOverworldMapWidget->RemoveFromParent();
		CurrentOverworldMapWidget = nullptr;

		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

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
	EFCPlayerCameraMode CurrentMode = CameraManager ? CameraManager->GetCameraMode() : EFCPlayerCameraMode::FirstPerson;
	UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleInteractPressed | Camera=%s"), *StaticEnum<EFCPlayerCameraMode>()->GetNameStringByValue(static_cast<int64>(CurrentMode)));

	// Week 3: Route to Overworld POI interaction if in TopDown mode
	if (CurrentMode == EFCPlayerCameraMode::TopDown)
	{
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("HandleInteractPressed: TopDown mode - checking for POI interaction"));
		
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
				
				// Get InteractionComponent from FirstPersonCharacter
				AFCFirstPersonCharacter* FPCharacter = Cast<AFCFirstPersonCharacter>(GetPawn());
				if (FPCharacter)
				{
					UFCInteractionComponent* InteractionComp = FPCharacter->GetInteractionComponent();
					if (InteractionComp)
					{
						// Delegate to InteractionComponent for POI handling
						InteractionComp->HandlePOIClick(HitActor);
						
						// After action selection, movement will be triggered by HandleClick
						// This maintains separation: Interact = select action, Click = move to location
						return;
					}
					else
					{
						UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleInteractPressed: No InteractionComponent on character"));
					}
				}
			}
		}
		
		UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleInteractPressed: No POI under cursor"));
		return;
	}

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
			SetInputMappingMode(EFCInputMappingMode::FirstPerson);
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
							SetInputMappingMode(EFCInputMappingMode::StaticScene);
							
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
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	// React to Overworld_Travel state entry
	if (NewState == EFCGameStateID::Overworld_Travel)
	{
		// Show mouse cursor for click-to-move
		bShowMouseCursor = true;
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: Mouse cursor enabled for Overworld"));

		// Switch to TopDown input mode
		if (InputManager)
		{
			InputManager->SetInputMappingMode(EFCInputMappingMode::TopDown);
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: Switched to TopDown input mode"));
		}

		// Blend to Overworld camera
		if (CameraManager)
		{
			CameraManager->BlendToTopDown(2.0f);
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: Blending to TopDown camera"));
		}

		// Note: Convoy pawn is controlled by its AIController, not possessed by PlayerController
		// PlayerController sends movement commands to the AIController via HandleOverworldClickMove()
	}
	// React to leaving Overworld_Travel state
	else if (OldState == EFCGameStateID::Overworld_Travel)
	{
		// Restore FirstPerson input mode when leaving Overworld
		if (InputManager && NewState == EFCGameStateID::Office_Exploration)
		{
			InputManager->SetInputMappingMode(EFCInputMappingMode::FirstPerson);
			CameraManager->BlendToFirstPerson(2.0f);
			UE_LOG(LogFallenCompassPlayerController, Log, TEXT("OnGameStateChanged: Returned to FirstPerson mode"));
		}
	}
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
	if (!CanProcessWorldInteraction())
	{
		UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("HandleTableObjectClick: UI is blocking world interaction"));
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
		UE_LOG(LogFallenCompassPlayerController, Verbose, TEXT("Cursor click - no actor hit"));
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

			// Set input mode to focus the currently open table widget so clicks
			// are captured by the UI instead of hitting world interactables.
			if (UUserWidget* TableWidget = UIManager->GetCurrentTableWidget())
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(TableWidget->TakeWidget());
				InputMode.SetHideCursorDuringCapture(false);
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
				SetInputMode(InputMode);
				bShowMouseCursor = true;
			}
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

void AFCPlayerController::HandleOverworldClickMove()
{
	// If a blocking UI widget (e.g., overworld map or table) is open,
	// ignore click-to-move so the map can be interacted with safely.
	if (!CanProcessWorldInteraction())
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
	if (!PossessedConvoy)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("HandleOverworldClickMove: No convoy reference"));
		return;
	}

	// Get leader member from convoy
	AFCConvoyMember* LeaderMember = PossessedConvoy->GetLeaderMember();
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

void AFCPlayerController::MoveConvoyToLocation(const FVector& TargetLocation)
{
	if (!PossessedConvoy)
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("MoveConvoyToLocation: No convoy possessed"));
		return;
	}

	// Get convoy leader
	AFCConvoyMember* LeaderMember = PossessedConvoy->GetLeaderMember();
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

