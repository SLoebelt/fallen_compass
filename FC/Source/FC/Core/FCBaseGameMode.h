
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FCBaseGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFallenCompassGameMode, Log, All);

UCLASS()
class FC_API AFCBaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
