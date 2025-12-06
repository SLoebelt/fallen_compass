// Copyright Slomotion Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FCSaveGame.generated.h"

/**
 * Basic save game data structure for Fallen Compass.
 * Contains essential game state for save/load functionality.
 */
UCLASS()
class FC_API UFCSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UFCSaveGame();

	/** Save slot name */
	UPROPERTY(VisibleAnywhere, Category = "Meta")
	FString SaveSlotName;

	/** Timestamp when the save was created */
	UPROPERTY(VisibleAnywhere, Category = "Meta")
	FDateTime Timestamp;

	/** Current level name */
	UPROPERTY(VisibleAnywhere, Category = "Level")
	FString CurrentLevelName;

	/** Player location in the level */
	UPROPERTY(VisibleAnywhere, Category = "Player")
	FVector PlayerLocation;

	/** Player rotation in the level */
	UPROPERTY(VisibleAnywhere, Category = "Player")
	FRotator PlayerRotation;

	/** Current expedition ID (placeholder for future use) */
	UPROPERTY(VisibleAnywhere, Category = "Expedition")
	FString CurrentExpeditionId;

	/** Discovered regions array */
	UPROPERTY(VisibleAnywhere, Category = "Expedition")
	TArray<FName> DiscoveredRegions;

	/** Expeditions counter */
	UPROPERTY(VisibleAnywhere, Category = "Meta")
	int32 ExpeditionsCounter;

	/** Game version when this save was created */
	UPROPERTY(VisibleAnywhere, Category = "Meta")
	FString GameVersion;
};
