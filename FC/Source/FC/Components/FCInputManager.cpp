// Copyright Slomotion Games. All Rights Reserved.

#include "Components/FCInputManager.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Input/FCInputConfig.h"

DEFINE_LOG_CATEGORY(LogFCInputManager);

UFCInputManager::UFCInputManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentMappingMode = EFCInputMappingMode::FirstPerson;
}

void UFCInputManager::BeginPlay()
{
	Super::BeginPlay();

	// Validate owner is a PlayerController
    APlayerController* OwnerPC = Cast<APlayerController>(GetOwner());
    if (!OwnerPC)
    {
        UE_LOG(LogFCInputManager, Error, TEXT("UFCInputManager must be attached to a PlayerController! Owner: %s"),
            *GetNameSafe(GetOwner()));
        return;
    }

	    // Only run on local controllers (server controllers have no LocalPlayer)
    if (!OwnerPC->IsLocalController())
    {
        UE_LOG(LogFCInputManager, Verbose,
            TEXT("UFCInputManager::BeginPlay skipped on non-local controller %s"),
            *GetNameSafe(OwnerPC));
        return;
    }

    // Ensure InputConfig is assigned
    if (!InputConfig)
    {
        UE_LOG(LogFCInputManager, Error,
            TEXT("UFCInputManager on %s has no InputConfig assigned"),
            *GetNameSafe(this));
        return;
    }

	// Validate all input mapping contexts are assigned
	if (!InputConfig->FirstPersonMappingContext)
	{
		UE_LOG(LogFCInputManager, Warning, TEXT("FirstPersonMappingContext not assigned on %s"), *GetName());
	}
	if (!InputConfig->TopDownMappingContext)
	{
		UE_LOG(LogFCInputManager, Warning, TEXT("TopDownMappingContext not assigned on %s"), *GetName());
	}
	if (!InputConfig->FightMappingContext)
	{
		UE_LOG(LogFCInputManager, Warning, TEXT("FightMappingContext not assigned on %s"), *GetName());
	}
	if (!InputConfig->StaticSceneMappingContext)
	{
		UE_LOG(LogFCInputManager, Warning, TEXT("StaticSceneMappingContext not assigned on %s"), *GetName());
	}
	if (!InputConfig->POISceneMappingContext)
	{
		UE_LOG(LogFCInputManager, Warning, TEXT("POISceneMappingContext not assigned on %s"), *GetName());
	}

	UE_LOG(LogFCInputManager, Log, TEXT("UFCInputManager initialized on %s"), *GetNameSafe(OwnerPC));
}

UEnhancedInputLocalPlayerSubsystem* UFCInputManager::GetEnhancedInputSubsystem() const
{
	APlayerController* OwnerPC = Cast<APlayerController>(GetOwner());
	if (!OwnerPC)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = OwnerPC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogFCInputManager, Warning, TEXT("GetEnhancedInputSubsystem: No LocalPlayer found"));
		return nullptr;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!Subsystem)
	{
		UE_LOG(LogFCInputManager, Warning, TEXT("GetEnhancedInputSubsystem: EnhancedInputSubsystem not found"));
		return nullptr;
	}

	return Subsystem;
}

void UFCInputManager::SetInputMappingMode(EFCInputMappingMode NewMode)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem)
	{
		return;
	}

	// Determine which context to use
	UInputMappingContext* ContextToApply = nullptr;
	FString ModeName;

	switch (NewMode)
	{
		case EFCInputMappingMode::FirstPerson:
			ContextToApply = InputConfig->FirstPersonMappingContext;
			ModeName = TEXT("FirstPerson");
			break;
		case EFCInputMappingMode::TopDown:
			ContextToApply = InputConfig->TopDownMappingContext;
			ModeName = TEXT("TopDown");
			break;
		case EFCInputMappingMode::Fight:
			ContextToApply = InputConfig->FightMappingContext;
			ModeName = TEXT("Fight");
			break;
		case EFCInputMappingMode::StaticScene:
			ContextToApply = InputConfig->StaticSceneMappingContext;
			ModeName = TEXT("StaticScene");
			break;
		case EFCInputMappingMode::POIScene:
			ContextToApply = InputConfig->POISceneMappingContext;
			ModeName = TEXT("POIScene");
			break;
	}

	if (!ContextToApply)
	{
		UE_LOG(LogFCInputManager, Error, TEXT("SetInputMappingMode: %s context is null!"), *ModeName);
		return;
	}

	// Clear all existing mappings and apply new one
	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(ContextToApply, DefaultMappingPriority);

	CurrentMappingMode = NewMode;

	UE_LOG(LogFCInputManager, Log, TEXT("Input mapping switched to: %s"), *ModeName);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			FString::Printf(TEXT("Input Mode: %s"), *ModeName));
	}
}
