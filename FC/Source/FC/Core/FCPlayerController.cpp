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

	if (PauseAction)
	{
		EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AFCPlayerController::HandlePausePressed);
	}
	else
	{
		UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("PauseAction not assigned on %s."), *GetName());
	}

	if (QuickSaveAction)
	{
		EnhancedInput->BindAction(QuickSaveAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleQuickSavePressed);
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("QuickSaveAction bound (F6)"));
	}

	if (QuickLoadAction)
	{
		EnhancedInput->BindAction(QuickLoadAction, ETriggerEvent::Started, this, &AFCPlayerController::HandleQuickLoadPressed);
		UE_LOG(LogFallenCompassPlayerController, Log, TEXT("QuickLoadAction bound (F9)"));
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
		// Clean up any temp camera from table view
		if (bIsInTableView)
		{
			AActor* CurrentViewTarget = GetViewTarget();
			if (CurrentViewTarget && CurrentViewTarget->IsA<ACameraActor>())
			{
				// Destroy the temporary camera actor we spawned for table view
				CurrentViewTarget->Destroy();
			}
		}
		
		// Use the pawn's camera (handled by SetViewTargetWithBlend to Pawn)
		if (GetPawn())
		{
			SetViewTargetWithBlend(GetPawn(), BlendTime, VTBlend_Cubic);
			CameraMode = NewMode;
			bIsInTableView = false; // Exiting table view
			
			// Re-enable movement input
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(FirstPersonMappingContext, 0);
			}
			
			LogStateChange(TEXT("Camera switched to FirstPerson"));
			return;
		}
		break;	case EFCPlayerCameraMode::TableView:
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
						// Get the world transform of the CameraTargetPoint
						FVector TargetLocation = CameraTarget->GetComponentLocation();
						FRotator TargetRotation = CameraTarget->GetComponentRotation();
						
						// Debug: Log the rotation values
						UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TableView Debug: CameraTargetPoint Rotation = %s"), *TargetRotation.ToString());
						UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TableView Debug: CameraTargetPoint Location = %s"), *TargetLocation.ToString());
						
						// Spawn a temporary camera actor at the target point's transform
						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						
						ACameraActor* TableCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), TargetLocation, TargetRotation, SpawnParams);
						
						if (TableCamera)
						{
							// Debug: Verify spawned camera rotation
							UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TableView Debug: Spawned Camera Rotation = %s"), *TableCamera->GetActorRotation().ToString());
							
							// Smoothly blend to the table camera
							SetViewTargetWithBlend(TableCamera, BlendTime, VTBlend_Cubic);
							
							CameraMode = NewMode;
							bIsInTableView = true;
							
							// Disable movement input when in table view
							if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
							{
								Subsystem->RemoveMappingContext(FirstPersonMappingContext);
							}
							
							LogStateChange(TEXT("Camera switched to TableView"));
							UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TableView: Camera smoothly blending to CameraTargetPoint at location %s, rotation %s"), 
								*TargetLocation.ToString(), *TargetRotation.ToString());
							
							// Store reference to clean up later
							// Note: Camera will be cleaned up when we switch back to FirstPerson
							return;
						}
						else
						{
							UE_LOG(LogFallenCompassPlayerController, Error, TEXT("TableView: Failed to spawn camera actor"));
							return;
						}
					}
				}
			}
			
			UE_LOG(LogFallenCompassPlayerController, Warning, TEXT("TableView: Could not find BP_OfficeDesk or CameraTargetPoint"));
			return;
		}
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

	// Get UIManager and show main menu
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
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

	CurrentGameState = EFCGameState::Gameplay;

	// Get UIManager and hide main menu
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (GI)
	{
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

void AFCPlayerController::TestFadeOut()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TestFadeOut: Console command executed"));
	FadeScreenOut(1.5f, true);
}

void AFCPlayerController::TestFadeIn()
{
	UE_LOG(LogFallenCompassPlayerController, Log, TEXT("TestFadeIn: Console command executed"));
	FadeScreenIn(1.0f);
}
