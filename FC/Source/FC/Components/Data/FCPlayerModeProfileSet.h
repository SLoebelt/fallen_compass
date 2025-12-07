#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/Data/FCPlayerModeTypes.h"
#include "FCPlayerModeProfileSet.generated.h"

UCLASS(BlueprintType)
class FC_API UFCPlayerModeProfileSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
    TMap<EFCPlayerMode, FPlayerModeProfile> Profiles;
};
