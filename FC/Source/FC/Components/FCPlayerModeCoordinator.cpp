#include "Components/FCPlayerModeCoordinator.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "FCPlayerController.h"

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

	// Phase 1: preserve existing behavior via controller helpers/router
	if (AFCPlayerController* FCPC = Cast<AFCPlayerController>(GetOwner()))
	{
		FCPC->ApplyPresentationForGameState(OldState, NewState);
	}
}

// Map high-level game states to player modes
// Add new GameStates or PlayerModes here (and in enum) as needed
EFCPlayerMode UFCPlayerModeCoordinator::MapStateToMode(EFCGameStateID State) const
{
	switch (State)
	{
		case EFCGameStateID::Office_Exploration:
		case EFCGameStateID::Office_TableView:
			return EFCPlayerMode::Office;

		case EFCGameStateID::Overworld_Travel:
		case EFCGameStateID::Combat_PlayerTurn:
		case EFCGameStateID::Combat_EnemyTurn:
			return EFCPlayerMode::Overworld;

		case EFCGameStateID::Camp_Local:
			return EFCPlayerMode::Camp;

		case EFCGameStateID::MainMenu:
		case EFCGameStateID::Paused:
		case EFCGameStateID::Loading:
		case EFCGameStateID::ExpeditionSummary:
			return EFCPlayerMode::Static;

		default:
			UE_LOG(LogFCPlayerModeCoordinator, Warning, TEXT("Unhandled state %s -> Static"),
				*UEnum::GetValueAsString(State));
			return EFCPlayerMode::Static;
	}
}

void UFCPlayerModeCoordinator::ApplyMode(EFCPlayerMode NewMode)
{
	if (NewMode == CurrentMode) return;

	UE_LOG(LogFCPlayerModeCoordinator, Log, TEXT("ApplyMode: %s -> %s | Profile=%s"),
		*UEnum::GetValueAsString(CurrentMode),
		*UEnum::GetValueAsString(NewMode),
		*GetNameSafe(ModeProfileSet));

	CurrentMode = NewMode;

	// IMPORTANT: In Phase 1, ApplyMode stays “high level”.
	// It should not do camera/input directly yet (that’s in the controller helper router above).
}
