#include "Components/FCPlayerModeCoordinator.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "FCPlayerController.h"
#include "Components/FCInputManager.h"
#include "Interaction/FCInteractionComponent.h"
#include "Input/FCInputConfig.h"

DEFINE_LOG_CATEGORY(LogFCPlayerModeCoordinator);

UFCPlayerModeCoordinator::UFCPlayerModeCoordinator()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFCPlayerModeCoordinator::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	UGameInstance* GI = PC->GetGameInstance();
	if (!GI) return;

	UFCGameStateManager* StateMgr = GI->GetSubsystem<UFCGameStateManager>();
	if (!StateMgr) return;

	CachedStateMgr = StateMgr;
	StateMgr->OnStateChanged.AddDynamic(this, &UFCPlayerModeCoordinator::OnGameStateChanged);

	const EFCGameStateID Current = StateMgr->GetCurrentState();

	FTimerDelegate D;
	D.BindUObject(this, &UFCPlayerModeCoordinator::OnGameStateChanged, EFCGameStateID::None, Current);
	GetWorld()->GetTimerManager().SetTimerForNextTick(D);

	UE_LOG(LogFCPlayerModeCoordinator, Log, TEXT("Initialized on %s"), *GetNameSafe(PC));
}

void UFCPlayerModeCoordinator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedStateMgr.IsValid())
	{
		CachedStateMgr->OnStateChanged.RemoveDynamic(this, &UFCPlayerModeCoordinator::OnGameStateChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void UFCPlayerModeCoordinator::OnGameStateChanged(EFCGameStateID OldState, EFCGameStateID NewState)
{
	UE_LOG(LogFCPlayerModeCoordinator, Log, TEXT("GameState: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	const EFCPlayerMode NewMode = MapStateToMode(NewState);
	ApplyMode(NewMode);
}

// Map high-level game states to player modes
// Add new GameStates or PlayerModes here (and in enum) as needed
EFCPlayerMode UFCPlayerModeCoordinator::MapStateToMode(EFCGameStateID State) const
{
	switch (State)
	{
		case EFCGameStateID::MainMenu:
			return EFCPlayerMode::MainMenu;

		case EFCGameStateID::Office_Exploration:
			return EFCPlayerMode::Office;

		case EFCGameStateID::Overworld_Travel:
		case EFCGameStateID::Combat_PlayerTurn:
		case EFCGameStateID::Combat_EnemyTurn:
			return EFCPlayerMode::Overworld;

		case EFCGameStateID::Camp_Local:
			return EFCPlayerMode::Camp;

		case EFCGameStateID::Paused:
		case EFCGameStateID::Loading:
		case EFCGameStateID::ExpeditionSummary:
		case EFCGameStateID::Office_TableView:
			return EFCPlayerMode::Static;

		default:
			UE_LOG(LogFCPlayerModeCoordinator, Warning, TEXT("Unhandled state %s -> Static"),
				*UEnum::GetValueAsString(State));
			return EFCPlayerMode::Static;
	}
}

static EFCInputMappingMode DeriveMappingMode(EFCPlayerMode Mode, EFCPlayerCameraMode Cam)
{
    switch (Cam)
    {
        case EFCPlayerCameraMode::FirstPerson: return EFCInputMappingMode::FirstPerson;
        case EFCPlayerCameraMode::TopDown:     return EFCInputMappingMode::TopDown;
        case EFCPlayerCameraMode::POIScene:    return EFCInputMappingMode::POIScene;
        case EFCPlayerCameraMode::TableView:
        case EFCPlayerCameraMode::SaveSlotView:
        case EFCPlayerCameraMode::MainMenu:    return EFCInputMappingMode::StaticScene;
        default:                               return EFCInputMappingMode::FirstPerson;
    }
}


void UFCPlayerModeCoordinator::ApplyMode(EFCPlayerMode NewMode)
{
	AFCPlayerController* PC = Cast<AFCPlayerController>(GetOwner());
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    const bool bIsReapply = (NewMode == CurrentMode);

	UE_LOG(LogFCPlayerModeCoordinator, Log, TEXT("ApplyMode: %s -> %s%s | ProfileSet=%s"),
		*UEnum::GetValueAsString(CurrentMode),
		*UEnum::GetValueAsString(NewMode),
		bIsReapply ? TEXT(" (reapply)") : TEXT(""),
		*GetNameSafe(ModeProfileSet));

	CurrentMode = NewMode;

	FPlayerModeProfile Profile;
	if (!GetProfileForMode(NewMode, Profile))
	{
		UE_LOG(LogFCPlayerModeCoordinator, Error, TEXT("ApplyMode: No valid profile for mode %s"), *UEnum::GetValueAsString(NewMode));
		return;
	}

	FString Problems;
    if (!ValidateProfile(Profile, Problems))
    {
        UE_LOG(LogFCPlayerModeCoordinator, Warning, TEXT("ApplyMode: Profile invalid for %s: %s"),
            *UEnum::GetValueAsString(NewMode), *Problems);
        // Still continue applying *safe* parts where possible.
    }
	else if (!Problems.IsEmpty())
    {
        UE_LOG(LogFCPlayerModeCoordinator, Verbose, TEXT("ApplyMode: Profile warnings for %s: %s"),
            *UEnum::GetValueAsString(NewMode), *Problems);
    }

	// 1) Camera
	const float BlendTime = (Profile.CameraMode == EFCPlayerCameraMode::MainMenu) ? 0.0f : 2.0f;
	PC->SetCameraModeLocal(Profile.CameraMode, BlendTime);

	// 2) Input config + mapping mode
	if (UFCInputManager* InputMgr = PC->FindComponentByClass<UFCInputManager>())
	{
		// Optional override (future-friendly). If unset, keep what BP assigned.
		if (!Profile.InputConfig.IsNull())
		{
			UFCInputConfig* LoadedConfig = Profile.InputConfig.LoadSynchronous();
			if (LoadedConfig)
			{
				if (InputMgr->GetInputConfig() != LoadedConfig)
				{
					InputMgr->SetInputConfig(LoadedConfig);
					UE_LOG(LogFCPlayerModeCoordinator, Log, TEXT("ApplyMode: InputConfig=%s"), *GetPathNameSafe(LoadedConfig));
				}
			}
			else
			{
				UE_LOG(LogFCPlayerModeCoordinator, Warning, TEXT("ApplyMode: Failed to load InputConfig for %s"),
					*UEnum::GetValueAsString(NewMode));
			}
		}

		const EFCInputMappingMode DesiredMapping = DeriveMappingMode(NewMode, Profile.CameraMode);
		if (InputMgr->GetCurrentMappingMode() != DesiredMapping)
		{
			InputMgr->SetInputMappingMode(DesiredMapping);
		}
	}
	else
	{
		UE_LOG(LogFCPlayerModeCoordinator, Warning, TEXT("ApplyMode: UFCInputManager missing on %s"), *GetNameSafe(PC));
	}

	// 3) Cursor + input mode (profile-driven)
	const bool bDesiredCursor = (Profile.CameraMode == EFCPlayerCameraMode::MainMenu) ? true : Profile.bShowMouseCursor;
	if (PC->bShowMouseCursor != bDesiredCursor)
	{
		PC->bShowMouseCursor = bDesiredCursor;
	}

	if (Profile.CameraMode == EFCPlayerCameraMode::MainMenu)
	{
		FInputModeUIOnly M;
		M.SetLockMouseToViewportBehavior(Profile.MouseLockMode);
		PC->SetInputMode(M);
	}
	else if (!bDesiredCursor)
	{
		FInputModeGameOnly M;
		PC->SetInputMode(M);
	}
	else
	{
		FInputModeGameAndUI M;
		M.SetHideCursorDuringCapture(false);
		M.SetLockMouseToViewportBehavior(Profile.MouseLockMode);
		PC->SetInputMode(M);
	}

	UE_LOG(LogFCPlayerModeCoordinator, Log, TEXT("ApplyMode: Cursor=%s Lock=%d Cam=%s"),
		bDesiredCursor ? TEXT("On") : TEXT("Off"),
		(int32)Profile.MouseLockMode,
		*UEnum::GetValueAsString(Profile.CameraMode));

	// 4) Interaction gating (no tick polling)
	if (UFCInteractionComponent* Interaction = PC->FindComponentByClass<UFCInteractionComponent>())
	{
		Interaction->SetFirstPersonFocusEnabled(Profile.CameraMode == EFCPlayerCameraMode::FirstPerson);
		// Later: Interaction->ApplyModeProfile(Profile); (once we add it)
	}
}

void UFCPlayerModeCoordinator::ReapplyCurrentMode()
{
    ApplyMode(CurrentMode);
}

bool UFCPlayerModeCoordinator::GetProfileForMode(EFCPlayerMode Mode, FPlayerModeProfile& OutProfile) const
{
    if (!ModeProfileSet)
    {
        UE_LOG(LogFCPlayerModeCoordinator, Warning, TEXT("GetProfileForMode: ModeProfileSet is null"));
        return false;
    }

    if (const FPlayerModeProfile* Found = ModeProfileSet->Profiles.Find(Mode))
    {
        OutProfile = *Found;
        return true;
    }

    UE_LOG(LogFCPlayerModeCoordinator, Warning,
        TEXT("GetProfileForMode: Missing profile for %s in %s"),
        *UEnum::GetValueAsString(Mode),
        *GetNameSafe(ModeProfileSet));

    return false;
}

bool UFCPlayerModeCoordinator::ValidateProfile(const FPlayerModeProfile& Profile, FString& OutProblems)
{
    bool bOk = true;

    if (Profile.InputConfig.IsNull())
    {
        OutProblems += TEXT("InputConfig not set (will use InputManager default if available). ");
    }

    if (const UEnum* CamEnum = StaticEnum<EFCPlayerCameraMode>())
    {
        if (!CamEnum->IsValidEnumValue((int64)Profile.CameraMode))
        {
            bOk = false;
            OutProblems += TEXT("CameraMode is invalid. ");
        }
    }

    return bOk;
}

