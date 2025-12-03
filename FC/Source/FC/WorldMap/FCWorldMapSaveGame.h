// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FCWorldMapSaveGame.generated.h"

/**
 * SaveGame container for world map exploration data.
 * Stores reveal and terrain masks so the planning UI can persist progress.
 */
UCLASS()
class UFCWorldMapSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<uint8> RevealMask;

	UPROPERTY()
	TArray<uint8> LandMask;
};
