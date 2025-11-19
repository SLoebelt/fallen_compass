// Copyright Iron Anchor Interactive. All Rights Reserved.

#include "Expedition/FCExpeditionManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogFCExpedition);

void UFCExpeditionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogFCExpedition, Log, TEXT("UFCExpeditionManager initialized"));

	CurrentExpedition = nullptr;
}

void UFCExpeditionManager::Deinitialize()
{
	if (CurrentExpedition)
	{
		UE_LOG(LogFCExpedition, Warning, TEXT("Deinitialize called with active expedition: %s"),
			*CurrentExpedition->ExpeditionName);
	}

	Super::Deinitialize();
}

UFCExpeditionData* UFCExpeditionManager::StartNewExpedition(const FString& ExpeditionName, int32 AllocatedSupplies)
{
	if (CurrentExpedition)
	{
		UE_LOG(LogFCExpedition, Warning, TEXT("StartNewExpedition called while expedition already active: %s"),
			*CurrentExpedition->ExpeditionName);
		EndExpedition(false);
	}

	// Create new expedition data object (outer is this subsystem for GC tracking)
	CurrentExpedition = NewObject<UFCExpeditionData>(this);
	CurrentExpedition->ExpeditionName = ExpeditionName;
	CurrentExpedition->StartingSupplies = AllocatedSupplies;
	CurrentExpedition->StartDate = FString::Printf(TEXT("Day %d"), 1); // Placeholder
	CurrentExpedition->TargetRegion = TEXT("Unknown Region"); // Placeholder
	CurrentExpedition->ExpeditionStatus = EFCExpeditionStatus::InProgress;

	UE_LOG(LogFCExpedition, Log, TEXT("Started expedition: %s (Supplies: %d)"),
		*ExpeditionName, AllocatedSupplies);

	OnExpeditionStateChanged.Broadcast(CurrentExpedition);

	return CurrentExpedition;
}

void UFCExpeditionManager::EndExpedition(bool bSuccess)
{
	if (!CurrentExpedition)
	{
		UE_LOG(LogFCExpedition, Warning, TEXT("EndExpedition called with no active expedition"));
		return;
	}

	CurrentExpedition->ExpeditionStatus = bSuccess ? EFCExpeditionStatus::Completed : EFCExpeditionStatus::Failed;

	UE_LOG(LogFCExpedition, Log, TEXT("Ended expedition: %s (Success: %s)"),
		*CurrentExpedition->ExpeditionName,
		bSuccess ? TEXT("true") : TEXT("false"));

	OnExpeditionStateChanged.Broadcast(CurrentExpedition);

	// Clear current expedition (will be GC'd)
	CurrentExpedition = nullptr;
}

bool UFCExpeditionManager::IsExpeditionActive() const
{
	return CurrentExpedition != nullptr &&
		   CurrentExpedition->ExpeditionStatus == EFCExpeditionStatus::InProgress;
}
