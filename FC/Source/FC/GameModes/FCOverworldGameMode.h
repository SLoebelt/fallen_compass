#pragma once

#include "CoreMinimal.h"
#include "Core/FCBaseGameMode.h"
#include "FCOverworldGameMode.generated.h"

class AFCOverworldConvoy;
class ACameraActor;

UCLASS()
class FC_API AFCOverworldGameMode : public AFCBaseGameMode
{
	GENERATED_BODY()

public:
	AFCOverworldGameMode();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditInstanceOnly, Category="FC|Overworld")
	TObjectPtr<AFCOverworldConvoy> DefaultConvoy = nullptr;

	UPROPERTY(EditInstanceOnly, Category="FC|Overworld")
	TObjectPtr<ACameraActor> OverworldCameraActor = nullptr;
};
