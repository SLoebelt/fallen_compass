// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/FCUIManager.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCPlayerController.h"
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

		// Set input mode to allow both UI interaction and game input (ESC key)
		// NOTE: We do NOT call SetPause(true) because that prevents Enhanced Input actions from firing
		// The game state system (EFCGameStateID::Paused) provides the logical pause state
		// For gameplay that needs engine pause (e.g., Overworld 3D map), use CustomTimeDilation instead
		UWorld* World = GetWorld();
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

		// Restore input mode
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC)
			{
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
	if (!POIActionSelectionWidgetClass)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowPOIActionSelection: POIActionSelectionWidgetClass not configured!"));
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
	CurrentPOIActionSelectionWidget = CreateWidget<UUserWidget>(GetGameInstance(), POIActionSelectionWidgetClass);
	if (!CurrentPOIActionSelectionWidget)
	{
		UE_LOG(LogFCUIManager, Error, TEXT("ShowPOIActionSelection: Failed to create widget from class %s"), *POIActionSelectionWidgetClass->GetName());
		return nullptr;
	}

	// TODO: Set actions array on widget (requires Blueprint-exposed property)
	// Widget needs to populate action buttons from Actions array
	// Widget needs to bind OnActionSelected event to InteractionComponent->OnPOIActionSelected

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

	// Clear reference
	CurrentPOIActionSelectionWidget = nullptr;
}
