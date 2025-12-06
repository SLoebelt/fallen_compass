#pragma once

#include "CoreMinimal.h"
#include "Core/FCBaseGameMode.h"
#include "FCOfficeGameMode.generated.h"

class ACameraActor;
class AActor;

UCLASS()
class FC_API AFCOfficeGameMode : public AFCBaseGameMode
{
	GENERATED_BODY()

public:
	AFCOfficeGameMode();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditInstanceOnly, Category="FC|Office")
	TObjectPtr<AActor> OfficeDeskActor = nullptr;

	UPROPERTY(EditInstanceOnly, Category="FC|Office")
	TObjectPtr<ACameraActor> MenuCameraActor = nullptr;
};
