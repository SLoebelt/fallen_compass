#include "CoreMinimal.h"
#include "Core/FCBaseGameMode.h"

#include "Core/FCLevelTransitionManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogFallenCompassGameMode);

void AFCBaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	const FString MapName = GetWorld() ? GetWorld()->GetMapName() : TEXT("Unknown");
	UE_LOG(LogFallenCompassGameMode, Log, TEXT("AFCBaseGameMode::BeginPlay | Class=%s | Map=%s"),
		*GetClass()->GetName(), *MapName);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UFCLevelTransitionManager* TransitionMgr = GI->GetSubsystem<UFCLevelTransitionManager>())
		{
			TransitionMgr->InitializeOnLevelStart();
		}
		else
		{
			UE_LOG(LogFallenCompassGameMode, Warning, TEXT("AFCBaseGameMode: UFCLevelTransitionManager subsystem missing"));
		}
	}
}
