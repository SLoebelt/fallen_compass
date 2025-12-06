#pragma once

#include "CoreMinimal.h"
#include "Core/FCBaseGameMode.h"
#include "FCCampGameMode.generated.h"

class ACameraActor;
class AFC_ExplorerCharacter;

UCLASS()
class FC_API AFCCampGameMode : public AFCBaseGameMode
{
	GENERATED_BODY()

public:
	AFCCampGameMode();

protected:
	virtual void BeginPlay() override;

	/* virtual APawn* SpawnDefaultPawnAtTransform_Implementation(
		AController* NewPlayer,
		const FTransform& SpawnTransform
	) override; */

public:
	UPROPERTY(EditInstanceOnly, Category="FC|Camp")
	TObjectPtr<ACameraActor> CampCameraActor = nullptr;
};
