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
	UE_LOG(LogFCUIManager, Log, TEXT("HandleNewLegacyClicked() called (stub)"));
}

void UFCUIManager::HandleContinueClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleContinueClicked() called (stub)"));
}

void UFCUIManager::HandleLoadSaveClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleLoadSaveClicked() called (stub)"));
}

void UFCUIManager::HandleOptionsClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleOptionsClicked() called (stub)"));
}

void UFCUIManager::HandleQuitClicked()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleQuitClicked() called (stub)"));
}

void UFCUIManager::HandleBackFromSaveSelector()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleBackFromSaveSelector() called (stub)"));
}

void UFCUIManager::HandleSaveSlotSelected(const FString& SlotName)
{
	UE_LOG(LogFCUIManager, Log, TEXT("HandleSaveSlotSelected(%s) called (stub)"), *SlotName);
}
