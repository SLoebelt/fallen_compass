#include "UFCGameInstance.h"
#include "Engine/Engine.h"
#include "Misc/ConfigCacheIni.h"

void UFCGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogTemp, Log, TEXT("UFCGameInstance Init | StartupMap=%s OfficeMap=%s"),
        *StartupMap.ToString(),
        *OfficeMap.ToString());

    // Future hook: load persistent profile data before menus spawn.
}

void UFCGameInstance::Shutdown()
{
    UE_LOG(LogTemp, Log, TEXT("UFCGameInstance Shutdown | CurrentExpedition=%s Dirty=%d"),
        *CurrentExpeditionId,
        bIsSessionDirty ? 1 : 0);

    // Future hook: ensure outstanding meta saves flush before quitting.
    Super::Shutdown();
}

void UFCGameInstance::SetCurrentExpeditionId(const FString& InExpeditionId)
{
    if (CurrentExpeditionId == InExpeditionId)
    {
        return;
    }

    CurrentExpeditionId = InExpeditionId;
    bIsSessionDirty = true;

    UE_LOG(LogTemp, Log, TEXT("Expedition context switched to %s"), *CurrentExpeditionId);
    OnExpeditionContextChanged.Broadcast();
}

void UFCGameInstance::MarkSessionSaved()
{
    bIsSessionDirty = false;
}

FString UFCGameInstance::GetGameVersion() const
{
    // Return the game version - you can set this in Project Settings → Project → Description → Version
    FString ProjectVersion;
    if (GConfig->GetString(
            TEXT("/Script/EngineSettings.GeneralProjectSettings"),
            TEXT("ProjectVersion"),
            ProjectVersion,
            GGameIni))
    {
        UE_LOG(LogTemp, Log, TEXT("Project Version: %s"), *ProjectVersion);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ProjectVersion not found in config!"));
    }
    return ProjectVersion;
}
