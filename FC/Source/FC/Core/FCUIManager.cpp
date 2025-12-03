// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/FCUIManager.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCPlayerController.h"
#include "Interaction/FCInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogFCUIManager);

void UFCUIManager::ShowMainMenu()
{
	if (!MainMenuWidgetClass)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowMainMenu: MainMenuWidgetClass is null!"));
		return;
	}

	// Create widget if not already cached
	if (!MainMenuWidget)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(GetGameInstance(), MainMenuWidgetClass);
		if (!MainMenuWidget)
		{
			UE_LOG(LogFCUIManager, Error, TEXT("ShowMainMenu: Failed to create MainMenuWidget!"));
			return;
		}
		UE_LOG(LogFCUIManager, Log, TEXT("ShowMainMenu: Created MainMenuWidget"));
	}

	// Add to viewport
	MainMenuWidget->AddToViewport();
	UE_LOG(LogFCUIManager, Log, TEXT("ShowMainMenu: Widget added to viewport"));
}

void UFCUIManager::HideMainMenu()
{
	if (!MainMenuWidget)
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("HideMainMenu: MainMenuWidget is null, nothing to hide"));
		return;
	}

	MainMenuWidget->RemoveFromParent();
	UE_LOG(LogFCUIManager, Log, TEXT("HideMainMenu: Widget removed from viewport"));
}

void UFCUIManager::ShowSaveSlotSelector()
{
	if (!SaveSlotSelectorWidgetClass)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowSaveSlotSelector: SaveSlotSelectorWidgetClass is null!"));
		return;
	}

	// Create widget if not already cached
	if (!SaveSlotSelectorWidget)
	{
		SaveSlotSelectorWidget = CreateWidget<UUserWidget>(GetGameInstance(), SaveSlotSelectorWidgetClass);
		if (!SaveSlotSelectorWidget)
		{
			UE_LOG(LogFCUIManager, Error, TEXT("ShowSaveSlotSelector: Failed to create SaveSlotSelectorWidget!"));
			return;
		}
		UE_LOG(LogFCUIManager, Log, TEXT("ShowSaveSlotSelector: Created SaveSlotSelectorWidget"));
	}

	// Add to viewport
	SaveSlotSelectorWidget->AddToViewport();
	UE_LOG(LogFCUIManager, Log, TEXT("ShowSaveSlotSelector: Widget added to viewport"));
}

void UFCUIManager::HideSaveSlotSelector()
{
	if (!SaveSlotSelectorWidget)
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("HideSaveSlotSelector: SaveSlotSelectorWidget is null, nothing to hide"));
		return;
	}

	SaveSlotSelectorWidget->RemoveFromParent();
	UE_LOG(LogFCUIManager, Log, TEXT("HideSaveSlotSelector: Widget removed from viewport"));
}

void UFCUIManager::ShowPauseMenu()
{
	if (!PauseMenuWidgetClass)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowPauseMenu: PauseMenuWidgetClass is null!"));
		return;
	}

	// Create widget if not already cached
	if (!PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(GetGameInstance(), PauseMenuWidgetClass);
		if (!PauseMenuWidget)
		{
			UE_LOG(LogFCUIManager, Error, TEXT("ShowPauseMenu: Failed to create PauseMenuWidget!"));
			return;
		}
		UE_LOG(LogFCUIManager, Log, TEXT("ShowPauseMenu: Created PauseMenuWidget"));
	}

	// Add to viewport with high z-order (above gameplay UI)
	if (!PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->AddToViewport(100);
		UE_LOG(LogFCUIManager, Log, TEXT("ShowPauseMenu: Widget added to viewport"));

		// Conditionally enable engine pause based on current level
		// Office level (L_Office): NO engine pause (Enhanced Input needs to work)
		// Overworld level (L_Overworld): YES engine pause (stops convoy movement and physics)
		UWorld* World = GetWorld();
		bool bShouldUseEnginePause = false;
		
		if (World)
		{
			FString CurrentLevelName = World->GetMapName();
			// Remove PIE prefix if present (e.g., "UEDPIE_0_L_Overworld" -> "L_Overworld")
			CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
			
			// Enable engine pause only for Overworld level
			bShouldUseEnginePause = CurrentLevelName.Contains(TEXT("L_Overworld"));
			
			UE_LOG(LogFCUIManager, Log, TEXT("ShowPauseMenu: Current level is %s, engine pause %s"),
				*CurrentLevelName,
				bShouldUseEnginePause ? TEXT("ENABLED") : TEXT("DISABLED"));
		}

		// Set input mode and optionally pause engine
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC)
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
				InputMode.SetHideCursorDuringCapture(false);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;
				
				// Conditionally pause engine based on level
				if (bShouldUseEnginePause)
				{
					PC->SetPause(true);
					UE_LOG(LogFCUIManager, Log, TEXT("ShowPauseMenu: Engine paused (L_Overworld level)"));
				}
				
				UE_LOG(LogFCUIManager, Log, TEXT("ShowPauseMenu: Input mode set to GameAndUI, pause menu displayed"));
			}
		}
	}
}

void UFCUIManager::HidePauseMenu()
{
	if (!PauseMenuWidget)
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("HidePauseMenu: PauseMenuWidget is null, nothing to hide"));
		return;
	}

	if (PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
		UE_LOG(LogFCUIManager, Log, TEXT("HidePauseMenu: Widget removed from viewport"));

		// Conditionally disable engine pause if it was enabled
		// Check current level to determine if engine pause was used
		UWorld* World = GetWorld();
		bool bWasUsingEnginePause = false;
		
		if (World)
		{
			FString CurrentLevelName = World->GetMapName();
			// Remove PIE prefix if present
			CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
			
			// Engine pause was used if we're in Overworld level
			bWasUsingEnginePause = CurrentLevelName.Contains(TEXT("L_Overworld"));
			
			UE_LOG(LogFCUIManager, Log, TEXT("HidePauseMenu: Current level is %s, engine pause %s"),
				*CurrentLevelName,
				bWasUsingEnginePause ? TEXT("was ENABLED, unpausing") : TEXT("was DISABLED"));
		}

		// Restore input mode and optionally unpause engine
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC)
			{
				// Conditionally unpause engine if it was paused
				if (bWasUsingEnginePause)
				{
					PC->SetPause(false);
					UE_LOG(LogFCUIManager, Log, TEXT("HidePauseMenu: Engine unpaused (L_Overworld level)"));
				}
				
				PC->SetInputMode(FInputModeGameOnly());
				PC->bShowMouseCursor = false;
				UE_LOG(LogFCUIManager, Log, TEXT("HidePauseMenu: Input mode restored to game only, pause menu hidden"));
			}
		}
	}
}

void UFCUIManager::HandleNewLegacyClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleNewLegacyClicked: Transitioning to gameplay"));

	// Get player controller
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("HandleNewLegacyClicked: World is null!"));
		return;
	}

	AFCPlayerController* PC = Cast<AFCPlayerController>(World->GetFirstPlayerController());
	if (!PC)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("HandleNewLegacyClicked: PlayerController is null or not AFCPlayerController!"));
		return;
	}

	// Hide main menu
	HideMainMenu();

	// Transition to gameplay
	PC->TransitionToGameplay();
	
	UE_LOG(LogFCUIManager, Log, TEXT("HandleNewLegacyClicked: Transition complete"));
}

void UFCUIManager::HandleContinueClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleContinueClicked: Loading most recent save"));

	// Get game instance
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("HandleContinueClicked: GameInstance is null or not UFCGameInstance!"));
		return;
	}

	// Get most recent save slot
	FString SlotName = GI->GetMostRecentSave();
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("HandleContinueClicked: No recent save found!"));
		return;
	}

	// Hide main menu
	HideMainMenu();

	// Load the save
	GI->LoadGameAsync(SlotName);
	
	UE_LOG(LogFCUIManager, Log, TEXT("HandleContinueClicked: Loading save '%s'"), *SlotName);
}

void UFCUIManager::HandleLoadSaveClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleLoadSaveClicked: Opening save slot selector"));

	// Hide main menu
	HideMainMenu();

	// Show save slot selector
	ShowSaveSlotSelector();
	
	UE_LOG(LogFCUIManager, Log, TEXT("HandleLoadSaveClicked: Save slot selector shown"));
}

void UFCUIManager::HandleOptionsClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleOptionsClicked: TODO - Options menu not yet implemented"));
	// TODO: Implement options menu in future task
}

void UFCUIManager::HandleQuitClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleQuitClicked: Quitting game"));

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("HandleQuitClicked: World is null!"));
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("HandleQuitClicked: PlayerController is null!"));
		return;
	}

	UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
}

void UFCUIManager::HandleBackFromSaveSelector()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleBackFromSaveSelector: Returning to main menu"));

	// Hide save slot selector
	HideSaveSlotSelector();

	// Show main menu
	ShowMainMenu();
	
	UE_LOG(LogFCUIManager, Log, TEXT("HandleBackFromSaveSelector: Main menu shown"));
}

void UFCUIManager::HandleSaveSlotSelected(const FString& SlotName)
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleSaveSlotSelected: Loading save '%s'"), *SlotName);

	// Get game instance
	UFCGameInstance* GI = Cast<UFCGameInstance>(GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("HandleSaveSlotSelected: GameInstance is null or not UFCGameInstance!"));
		return;
	}

	// Hide save slot selector
	HideSaveSlotSelector();

	// Load the selected save
	GI->LoadGameAsync(SlotName);
	
	UE_LOG(LogFCUIManager, Log, TEXT("HandleSaveSlotSelected: Load initiated for '%s'"), *SlotName);
}

void UFCUIManager::ShowTableWidget(AActor* TableObject)
{
	if (!TableObject)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowTableWidget: TableObject is null!"));
		return;
	}

	// Get the widget class for this table object type
	TSubclassOf<AActor> TableClass = TableObject->GetClass();
	TSubclassOf<UUserWidget>* WidgetClassPtr = TableWidgetMap.Find(TableClass);
	
	if (!WidgetClassPtr || !(*WidgetClassPtr))
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowTableWidget: No widget class registered for %s"), *TableClass->GetName());
		return;
	}

	TSubclassOf<UUserWidget> WidgetClass = *WidgetClassPtr;

	// Close any existing table widget first
	if (CurrentTableWidget)
	{
		UE_LOG(LogFCUIManager, Log, TEXT("ShowTableWidget: Closing existing table widget"));
		CloseTableWidget();
	}

	// Create new table widget
	CurrentTableWidget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
	if (!CurrentTableWidget)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowTableWidget: Failed to create widget from class %s"), *WidgetClass->GetName());
		return;
	}

	// Add to viewport
	CurrentTableWidget->AddToViewport();
	UE_LOG(LogFCUIManager, Log, TEXT("ShowTableWidget: Created and displayed widget for %s"), *TableObject->GetName());
}

void UFCUIManager::CloseTableWidget()
{
	if (!CurrentTableWidget)
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("CloseTableWidget: No table widget currently open"));
		return;
	}

	// Remove from viewport
	CurrentTableWidget->RemoveFromParent();
	UE_LOG(LogFCUIManager, Log, TEXT("CloseTableWidget: Removed table widget from viewport"));

	// Clear reference
	CurrentTableWidget = nullptr;
}

UUserWidget* UFCUIManager::ShowPOIActionSelection(const TArray<FFCPOIActionData>& Actions, UFCInteractionComponent* InteractionComponent)
{
	if (!ActionSelectionWidgetClass)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowPOIActionSelection: ActionSelectionWidgetClass not configured!"));
		return nullptr;
	}

	if (!InteractionComponent)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowPOIActionSelection: InteractionComponent is null!"));
		return nullptr;
	}

	if (Actions.Num() == 0)
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("ShowPOIActionSelection: No actions provided!"));
		return nullptr;
	}

	// Close any existing POI action selection widget first
	if (CurrentPOIActionSelectionWidget)
	{
		UE_LOG(LogFCUIManager, Log, TEXT("ShowPOIActionSelection: Closing existing POI action selection widget"));
		ClosePOIActionSelection();
	}

	// Create new POI action selection widget
	CurrentPOIActionSelectionWidget = CreateWidget<UUserWidget>(GetGameInstance(), ActionSelectionWidgetClass);
	if (!CurrentPOIActionSelectionWidget)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowPOIActionSelection: Failed to create widget from class %s"), *ActionSelectionWidgetClass->GetName());
		return nullptr;
	}

	// Store InteractionComponent reference for mediator pattern
	PendingInteractionComponent = InteractionComponent;

	// Call PopulateActions(TArray<FFCPOIActionData>) to initialize widget with actions
	// This function sets AvailableActions variable and creates action buttons
	UFunction* PopulateFunc = CurrentPOIActionSelectionWidget->FindFunction(FName("PopulateActions"));
	if (PopulateFunc)
	{
		// Prepare parameters for PopulateActions(TArray<FFCPOIActionData> Actions)
		// The function expects the Actions array as input
		struct FPopulateActionsParams
		{
			TArray<FFCPOIActionData> InActions;
		};
		
		FPopulateActionsParams Params;
		Params.InActions = Actions;
		
		CurrentPOIActionSelectionWidget->ProcessEvent(PopulateFunc, &Params);
		UE_LOG(LogFCUIManager, Log, TEXT("ShowPOIActionSelection: Called PopulateActions with %d actions"), Actions.Num());
	}
	else
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowPOIActionSelection: PopulateActions function not found on widget"));
	}

	// Add to viewport
	CurrentPOIActionSelectionWidget->AddToViewport();
	UE_LOG(LogFCUIManager, Log, TEXT("ShowPOIActionSelection: Created and displayed POI action selection widget with %d actions"), Actions.Num());

	return CurrentPOIActionSelectionWidget;
}

void UFCUIManager::ClosePOIActionSelection()
{
	if (!CurrentPOIActionSelectionWidget)
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("ClosePOIActionSelection: No POI action selection widget currently open"));
		return;
	}

	// Remove from viewport
	CurrentPOIActionSelectionWidget->RemoveFromParent();
	UE_LOG(LogFCUIManager, Log, TEXT("ClosePOIActionSelection: Removed POI action selection widget from viewport"));

	// Clear references
	CurrentPOIActionSelectionWidget = nullptr;
	PendingInteractionComponent = nullptr;
}

void UFCUIManager::HandlePOIActionSelected(EFCPOIAction Action)
{
	if (!PendingInteractionComponent)
	{
		UE_LOG(LogFCUIManager, Warning, TEXT("HandlePOIActionSelected: No pending interaction component!"));
		return;
	}

	UE_LOG(LogFCUIManager, Log, TEXT("HandlePOIActionSelected: Forwarding action %d to InteractionComponent"), (uint8)Action);

	// Forward action to InteractionComponent
	PendingInteractionComponent->OnPOIActionSelected(Action);

	// Clear reference
	PendingInteractionComponent = nullptr;

	// Close the widget
	ClosePOIActionSelection();
}

UUserWidget* UFCUIManager::ShowOverworldMapHUD(APlayerController* OwningPlayer)
{
	if (!OwningPlayer)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowOverworldMapHUD: OwningPlayer is null"));
		return nullptr;
	}

	// Expect widget class to be configured via BP_FC_GameInstance
	if (!OverworldMapHUDWidgetClass)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowOverworldMapHUD: OverworldMapHUDWidgetClass not configured on UIManager"));
		return nullptr;
	}

	UUserWidget* MapWidget = CreateWidget<UUserWidget>(OwningPlayer, OverworldMapHUDWidgetClass);
	if (!MapWidget)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowOverworldMapHUD: CreateWidget returned null"));
		return nullptr;
	}

	MapWidget->AddToViewport();
	FocusedBlockingWidget = MapWidget;
	UE_LOG(LogFCUIManager, Log, TEXT("ShowOverworldMapHUD: Created and added Overworld map HUD"));
	return MapWidget;
}
