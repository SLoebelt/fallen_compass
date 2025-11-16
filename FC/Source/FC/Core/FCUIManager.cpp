// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/FCUIManager.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogFCUIManager);

void UFCUIManager::ShowMainMenu()
{
	UE_LOG(LogFCUIManager, Log, TEXT("ShowMainMenu() called (stub)"));
}

void UFCUIManager::HideMainMenu()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HideMainMenu() called (stub)"));
}

void UFCUIManager::ShowSaveSlotSelector()
{
	UE_LOG(LogFCUIManager, Log, TEXT("ShowSaveSlotSelector() called (stub)"));
}

void UFCUIManager::HideSaveSlotSelector()
{
	UE_LOG(LogFCUIManager, Log, TEXT("HideSaveSlotSelector() called (stub)"));
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
