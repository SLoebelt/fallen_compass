// Copyright Epic Games, Inc. All Rights Reserved.

#include "FCSaveGame.h"

UFCSaveGame::UFCSaveGame()
{
	SaveSlotName = TEXT("Default");
	Timestamp = FDateTime::Now();
	CurrentLevelName = TEXT("L_Office");
	PlayerLocation = FVector::ZeroVector;
	PlayerRotation = FRotator::ZeroRotator;
	CurrentExpeditionId = TEXT("");
	DiscoveredRegions.Empty();
	ExpeditionsCounter = 1;
	GameVersion = TEXT("0.1.0-dev");
}