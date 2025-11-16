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
