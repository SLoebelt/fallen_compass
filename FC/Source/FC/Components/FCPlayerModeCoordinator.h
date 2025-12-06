#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/FCGameStateManager.h"
#include "Core/FCPlayerModeTypes.h"
#include "Components/Data/FCPlayerModeProfileSet.h"
#include "FCPlayerModeCoordinator.generated.h"

class UFCGameStateManager;
class UFCPlayerModeProfileSet;
class AFCPlayerController;

DECLARE_LOG_CATEGORY_EXTERN(LogFCPlayerModeCoordinator, Log, All);

UCLASS(ClassGroup=(FC), meta=(BlueprintSpawnableComponent))
class FC_API UFCPlayerModeCoordinator : public UActorComponent
{
	GENERATED_BODY()

public:
	UFCPlayerModeCoordinator();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState);

	EFCPlayerMode MapStateToMode(EFCGameStateID State) const;
	void ApplyMode(EFCPlayerMode NewMode);

	UPROPERTY(EditDefaultsOnly, Category="FC|Mode")
	TObjectPtr<UFCPlayerModeProfileSet> ModeProfileSet = nullptr;

private:
	TWeakObjectPtr<UFCGameStateManager> CachedStateMgr;
	EFCPlayerMode CurrentMode = EFCPlayerMode::Static;
};
