#include "UFCGameInstance.h"
#include "Engine/Engine.h"
#include "Misc/ConfigCacheIni.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGame/FCSaveGame.h"
#include "FCPlayerController.h"
#include "FCFirstPersonCharacter.h"
#include "FCTransitionManager.h"
#include "Core/FCLevelManager.h"

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

bool UFCGameInstance::SaveGame(const FString& SlotName)
{
    UE_LOG(LogTemp, Log, TEXT("UFCGameInstance::SaveGame - Saving to slot: %s"), *SlotName);

    // Create save game object
    UFCSaveGame* SaveGameInstance = Cast<UFCSaveGame>(UGameplayStatics::CreateSaveGameObject(UFCSaveGame::StaticClass()));
    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create save game object"));
        return false;
    }

    // Fill save data
    SaveGameInstance->SaveSlotName = SlotName;
    SaveGameInstance->Timestamp = FDateTime::Now();
    SaveGameInstance->GameVersion = GetGameVersion();

    // Get current level name
    if (UWorld* World = GetWorld())
    {
        SaveGameInstance->CurrentLevelName = World->GetMapName();
    }

    // Get player data
    AFCPlayerController* PC = Cast<AFCPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (PC)
    {
        AFCFirstPersonCharacter* Character = Cast<AFCFirstPersonCharacter>(PC->GetPawn());
        if (Character)
        {
            SaveGameInstance->PlayerLocation = Character->GetActorLocation();
            SaveGameInstance->PlayerRotation = Character->GetActorRotation();
        }
    }

    // Save expedition data
    SaveGameInstance->CurrentExpeditionId = CurrentExpeditionId;
    SaveGameInstance->DiscoveredRegions = DiscoveredRegions;
    SaveGameInstance->ExpeditionsCounter = ExpeditionsCounter;

    // Save to disk
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully saved game to slot: %s"), *SlotName);
        MarkSessionSaved();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game to slot: %s"), *SlotName);
    }

    return bSuccess;
}

void UFCGameInstance::LoadGameAsync(const FString& SlotName)
{
    UE_LOG(LogTemp, Log, TEXT("UFCGameInstance::LoadGameAsync - Loading from slot: %s"), *SlotName);

    // Check if save exists
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("Save slot does not exist: %s"), *SlotName);
        OnGameLoaded.Broadcast(false);
        return;
    }

    // Load save game object
    UFCSaveGame* LoadGameInstance = Cast<UFCSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!LoadGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load save game object from slot: %s"), *SlotName);
        OnGameLoaded.Broadcast(false);
        return;
    }

    // Apply loaded data
    CurrentExpeditionId = LoadGameInstance->CurrentExpeditionId;
    DiscoveredRegions = LoadGameInstance->DiscoveredRegions;
    ExpeditionsCounter = LoadGameInstance->ExpeditionsCounter;

    // Cache the save data for position restoration after level load
    PendingLoadData = LoadGameInstance;

    // Get level manager and transition manager
    UFCLevelManager* LevelMgr = GetSubsystem<UFCLevelManager>();
    UFCTransitionManager* TransitionMgr = GetSubsystem<UFCTransitionManager>();
    
    if (!LevelMgr)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadGameAsync: LevelManager subsystem not found"));
        OnGameLoaded.Broadcast(false);
        return;
    }

    // Use LevelManager to normalize level names and check if same level
    FName CurrentLevelName = LevelMgr->GetCurrentLevelName();
    FName TargetLevelFName = LevelMgr->NormalizeLevelName(FName(*LoadGameInstance->CurrentLevelName));
    bool bIsSameLevel = (CurrentLevelName == TargetLevelFName);

    if (!bIsSameLevel && !TargetLevelFName.IsNone())
    {
        // Cross-level load - use fade transition
        UE_LOG(LogTemp, Log, TEXT("Loading different level: %s (cross-level fade transition)"), *TargetLevelFName.ToString());
        
        if (TransitionMgr)
        {
            // Fade out with loading indicator
            TransitionMgr->BeginFadeOut(1.0f, true);
            
            // Store target level for deferred load
            PendingLevelLoad = TargetLevelFName;
            
            // Use timer to load level after fade completes (1.2s gives time for 1.0s fade)
            GetWorld()->GetTimerManager().SetTimer(
                LevelLoadTimerHandle,
                [this, TargetLevelFName]()
                {
                    UGameplayStatics::OpenLevel(GetWorld(), TargetLevelFName);
                },
                1.2f,
                false
            );
        }
        else
        {
            // Fallback: immediate load
            UGameplayStatics::OpenLevel(GetWorld(), TargetLevelFName);
        }
    }
    else
    {
        // Same level - position will be restored with smooth camera blend
        UE_LOG(LogTemp, Log, TEXT("Same level (%s), position will be restored with camera blend"), *CurrentLevelName.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully loaded game from slot: %s"), *SlotName);
    OnGameLoaded.Broadcast(true);
}

TArray<FString> UFCGameInstance::GetAvailableSaveSlots()
{
    TArray<FString> SaveSlots;

    // Check for auto saves
    for (int32 i = 1; i <= 3; ++i)
    {
        FString SlotName = FString::Printf(TEXT("AutoSave_%03d"), i);
        if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
        {
            SaveSlots.Add(SlotName);
        }
    }

    // Check for quick save
    if (UGameplayStatics::DoesSaveGameExist(TEXT("QuickSave"), 0))
    {
        SaveSlots.Add(TEXT("QuickSave"));
    }

    // Check for manual saves (up to 10 for now)
    for (int32 i = 1; i <= 10; ++i)
    {
        FString SlotName = FString::Printf(TEXT("Manual_%03d"), i);
        if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
        {
            SaveSlots.Add(SlotName);
        }
    }

    return SaveSlots;
}

FString UFCGameInstance::GetMostRecentSave()
{
    TArray<FString> SaveSlots = GetAvailableSaveSlots();
    if (SaveSlots.Num() == 0)
    {
        return TEXT("");
    }

    FString MostRecentSlot;
    FDateTime MostRecentTime = FDateTime::MinValue();

    for (const FString& SlotName : SaveSlots)
    {
        UFCSaveGame* SaveGame = Cast<UFCSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
        if (SaveGame && SaveGame->Timestamp > MostRecentTime)
        {
            MostRecentTime = SaveGame->Timestamp;
            MostRecentSlot = SlotName;
        }
    }

    return MostRecentSlot;
}

void UFCGameInstance::RestorePlayerPosition()
{
    if (!PendingLoadData)
    {
        UE_LOG(LogTemp, Warning, TEXT("RestorePlayerPosition: No pending load data"));
        return;
    }

    AFCPlayerController* PC = Cast<AFCPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("RestorePlayerPosition: Failed to get PlayerController"));
        return;
    }

    AFCFirstPersonCharacter* Character = Cast<AFCFirstPersonCharacter>(PC->GetPawn());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("RestorePlayerPosition: No character found, spawning may be needed"));
        return;
    }

    // Restore position and rotation
    Character->SetActorLocation(PendingLoadData->PlayerLocation);
    Character->SetActorRotation(PendingLoadData->PlayerRotation);

    // Also update controller rotation for proper camera orientation
    PC->SetControlRotation(PendingLoadData->PlayerRotation);

    UE_LOG(LogTemp, Log, TEXT("RestorePlayerPosition: Restored to %s"), *PendingLoadData->PlayerLocation.ToString());

    // Update level manager with current level (after position restoration)
    UFCLevelManager* LevelMgr = GetSubsystem<UFCLevelManager>();
    if (LevelMgr)
    {
        FString RawMapName = GetWorld()->GetMapName();
        LevelMgr->UpdateCurrentLevel(FName(*RawMapName));
    }

    // Clear pending data
    PendingLoadData = nullptr;
}
