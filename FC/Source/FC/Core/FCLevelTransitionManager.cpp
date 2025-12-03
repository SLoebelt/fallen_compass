#include "Core/FCLevelTransitionManager.h"

#include "Core/UFCGameInstance.h"
#include "Core/FCLevelManager.h"
#include "Core/FCGameStateManager.h"
#include "Core/FCUIManager.h"
#include "FCTransitionManager.h"
#include "Expedition/FCExpeditionManager.h" // adjust path if actual header differs
#include "Core/FCPlayerController.h"

void UFCLevelTransitionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogFCLevelTransitionManager, Log, TEXT("FCLevelTransitionManager: Initialized"));
}

UFCGameStateManager* UFCLevelTransitionManager::GetGameStateManager() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UFCGameStateManager>();
	}
	return nullptr;
}

UFCTransitionManager* UFCLevelTransitionManager::GetTransitionManager() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UFCTransitionManager>();
	}
	return nullptr;
}

UFCLevelManager* UFCLevelTransitionManager::GetLevelManager() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UFCLevelManager>();
	}
	return nullptr;
}

UFCUIManager* UFCLevelTransitionManager::GetUIManager() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UFCUIManager>();
	}
	return nullptr;
}

UFCExpeditionManager* UFCLevelTransitionManager::GetExpeditionManager() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UFCExpeditionManager>();
	}
	return nullptr;
}

void UFCLevelTransitionManager::StartExpeditionFromOfficeTableView()
{
	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCLevelManager* LevelMgr = GetLevelManager();
	UFCTransitionManager* TransitionMgr = GetTransitionManager();
	UFCExpeditionManager* ExpeditionMgr = GetExpeditionManager();

	if (!StateMgr || !LevelMgr || !TransitionMgr || !ExpeditionMgr)
	{
		UE_LOG(LogTemp, Error, TEXT("StartExpeditionFromOfficeTableView: Missing required subsystem (StateMgr=%p, LevelMgr=%p, TransitionMgr=%p, ExpeditionMgr=%p)"),
			StateMgr, LevelMgr, TransitionMgr, ExpeditionMgr);
		return;
	}

	// For Week 4 we assume we are in Office_TableView and that expedition
	// data has already been validated and money consumed by the caller.
	// Here we focus on state + level + fade.
	if (!StateMgr->TransitionViaLoading(EFCGameStateID::Overworld_Travel))
	{
		UE_LOG(LogTemp, Error, TEXT("StartExpeditionFromOfficeTableView: Failed to transition via Loading to Overworld_Travel"));
		return;
	}

	// Begin fade out (optional loading indicator can be enabled later).
	TransitionMgr->BeginFadeOut(1.0f, /*bShowLoading=*/false);

	// TODO: Make overworld level configurable from expedition data.
	const FName OverworldLevelName(TEXT("L_Overworld"));
	LevelMgr->LoadLevel(OverworldLevelName, /*bShowLoadingScreen*/ false);

	UE_LOG(LogTemp, Log, TEXT("StartExpeditionFromOfficeTableView: Loading overworld level %s"), *OverworldLevelName.ToString());
}

void UFCLevelTransitionManager::ReturnToMainMenuFromGameplay()
{
	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCTransitionManager* TransitionMgr = GetTransitionManager();
	UFCLevelManager* LevelMgr = GetLevelManager();
	UFCUIManager* UIManager = GetUIManager();

	if (!StateMgr || !TransitionMgr || !LevelMgr)
	{
		UE_LOG(LogTemp, Error, TEXT("ReturnToMainMenuFromGameplay: Missing required subsystem (StateMgr=%p, TransitionMgr=%p, LevelMgr=%p)"),
			StateMgr, TransitionMgr, LevelMgr);
		return;
	}

	// Ensure we are going through Loading en route to MainMenu.
	if (!StateMgr->TransitionViaLoading(EFCGameStateID::MainMenu))
	{
		UE_LOG(LogTemp, Error, TEXT("ReturnToMainMenuFromGameplay: Failed to transition via Loading to MainMenu"));
		return;
	}

	// Hide pause menu if it's open.
	if (UIManager)
	{
		UIManager->HidePauseMenu();
	}

	// Start fade out and reload L_Office; once Office is loaded the
	// unified startup hook (InitializeOnLevelStart) will see state
	// None/MainMenu + L_Office and call AFCPlayerController::InitializeMainMenu.
	TransitionMgr->BeginFadeOut(1.0f, /*bShowLoading=*/true);

	const FName OfficeLevelName(TEXT("L_Office"));
	LevelMgr->LoadLevel(OfficeLevelName, /*bShowLoadingScreen*/ true);

	UE_LOG(LogTemp, Log, TEXT("ReturnToMainMenuFromGameplay: Loading office level %s for main menu"), *OfficeLevelName.ToString());
}

void UFCLevelTransitionManager::ReturnFromOverworldToOfficeWithSummary()
{
	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCTransitionManager* TransitionMgr = GetTransitionManager();
	UFCLevelManager* LevelMgr = GetLevelManager();
	UFCUIManager* UIManager = GetUIManager();
	UFCExpeditionManager* ExpeditionMgr = GetExpeditionManager();

	if (!StateMgr || !TransitionMgr || !LevelMgr || !UIManager || !ExpeditionMgr)
	{
		UE_LOG(LogTemp, Error, TEXT("ReturnFromOverworldToOfficeWithSummary: Missing required subsystem (StateMgr=%p, TransitionMgr=%p, LevelMgr=%p, UIManager=%p, ExpeditionMgr=%p)"),
			StateMgr, TransitionMgr, LevelMgr, UIManager, ExpeditionMgr);
		return;
	}

	// Allow being called either directly from Overworld_Travel or from
	// a paused state whose underlying state is Overworld_Travel.
	EFCGameStateID CurrentState = StateMgr->GetCurrentState();
	if (CurrentState == EFCGameStateID::Paused)
	{
		// Peek the state stack or rely on convention: we only expose this
		// action from the Overworld pause menu, so treat Paused as wrapping
		// Overworld_Travel.
		UE_LOG(LogTemp, Log, TEXT("ReturnFromOverworldToOfficeWithSummary: Invoked while Paused, assuming underlying Overworld_Travel state"));
	}
	else if (CurrentState != EFCGameStateID::Overworld_Travel)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnFromOverworldToOfficeWithSummary: Called from non-Overworld_Travel state %s"),
			*UEnum::GetValueAsString(CurrentState));
		return;
	}

	// Ensure main menu UI is hidden before we start the return flow.
	// The L_Office level blueprint always initializes the main menu on
	// load, but this path represents "returning from gameplay" where
	// we do not want the main menu widget to be visible.
	if (UIManager)
	{
		UIManager->HideMainMenu();
	}

	// Enter Loading with ExpeditionSummary as the intended target state.
	if (!StateMgr->TransitionViaLoading(EFCGameStateID::ExpeditionSummary))
	{
		UE_LOG(LogTemp, Error, TEXT("ReturnFromOverworldToOfficeWithSummary: Failed to transition via Loading to ExpeditionSummary"));
		return;
	}

	// Fade out and load the office level; once the level is loaded, a higher
	// level initialization step (e.g., Office BeginPlay) should call
	// TransitionTo(ExpeditionSummary) and ask UIManager to show the summary.
	TransitionMgr->BeginFadeOut(1.0f, /*bShowLoading=*/true);

	const FName OfficeLevelName(TEXT("L_Office"));
	LevelMgr->LoadLevel(OfficeLevelName, /*bShowLoadingScreen*/ true);

	UE_LOG(LogTemp, Log, TEXT("ReturnFromOverworldToOfficeWithSummary: Loading office level %s for expedition summary"), *OfficeLevelName.ToString());
}

void UFCLevelTransitionManager::InitializeLevelTransitionOnLevelStart()
{
	UWorld* World = GetWorld();
	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCLevelManager* LevelMgr = GetLevelManager();
	UFCUIManager* UIManager = GetUIManager();

	UE_LOG(LogFCLevelTransitionManager, Log, TEXT("InitializeLevelTransitionOnLevelStart: Entered (World=%p, StateMgr=%p, LevelMgr=%p)"), World, StateMgr, LevelMgr);

	if (!World || !StateMgr || !LevelMgr)
	{
		UE_LOG(LogFCLevelTransitionManager, Verbose, TEXT("InitializeLevelTransitionOnLevelStart: Missing subsystem or world (World=%p, StateMgr=%p, LevelMgr=%p)"), World, StateMgr, LevelMgr);
		return;
	}

	// Only react when coming out of a Loading state.
	if (StateMgr->GetCurrentState() != EFCGameStateID::Loading)
	{
		UE_LOG(LogFCLevelTransitionManager, Verbose, TEXT("InitializeLevelTransitionOnLevelStart: Current state is %s, not Loading"),
			*UEnum::GetValueAsString(StateMgr->GetCurrentState()));
		return;
	}

	// Normalize current level name via LevelManager so PIE prefixes are handled.
	const FName CurrentLevelName = LevelMgr->GetCurrentLevelName();
	UE_LOG(LogFCLevelTransitionManager, Log, TEXT("InitializeLevelTransitionOnLevelStart: LevelManager reports current level '%s'"), *CurrentLevelName.ToString());
	if (!CurrentLevelName.IsEqual(FName(TEXT("L_Office"))))
	{
		return;
	}

	// For now we only care about ExpeditionSummary as a pending target.
	const EFCGameStateID LoadingTarget = StateMgr->GetLoadingTargetState();
	if (LoadingTarget != EFCGameStateID::ExpeditionSummary)
	{
		UE_LOG(LogFCLevelTransitionManager, Verbose, TEXT("InitializeLevelTransitionOnLevelStart: Loading target is %s, not ExpeditionSummary"),
			*UEnum::GetValueAsString(LoadingTarget));
		return;
	}

	// Transition into ExpeditionSummary.
	if (!StateMgr->TransitionTo(EFCGameStateID::ExpeditionSummary))
	{
		UE_LOG(LogFCLevelTransitionManager, Error, TEXT("InitializeLevelTransitionOnLevelStart: Failed to transition to ExpeditionSummary after loading Office"));
		return;
	}

	// Ask UIManager to show the expedition summary widget if available.
	if (UIManager)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			// Helper will be implemented on UIManager side.
			UFunction* ShowSummaryFunc = UIManager->FindFunction(FName(TEXT("ShowExpeditionSummary")));
			if (ShowSummaryFunc)
			{
				struct FShowSummaryParams
				{
					APlayerController* OwningPlayer;
				};
				FShowSummaryParams Params;
				Params.OwningPlayer = PC;
				UIManager->ProcessEvent(ShowSummaryFunc, &Params);
			}
			else
			{
				UE_LOG(LogFCLevelTransitionManager, Warning, TEXT("InitializeLevelTransitionOnLevelStart: UFCUIManager::ShowExpeditionSummary not found (function missing)"));
			}
		}
	}
}

void UFCLevelTransitionManager::InitializeOnLevelStart()
{
	// 4.7.4: Unified startup front-end. For now this simply
	// delegates to InitializeLevelTransitionOnLevelStart for
	// Loading->ExpeditionSummary flows, and additionally handles
	// Office startup and return-to-main-menu flows.

	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCLevelManager* LevelMgr = GetLevelManager();
	UWorld* World = GetWorld();

	if (!World || !StateMgr || !LevelMgr)
	{
		UE_LOG(LogFCLevelTransitionManager, Verbose, TEXT("InitializeOnLevelStart: Missing subsystem or world (World=%p, StateMgr=%p, LevelMgr=%p)"), World, StateMgr, LevelMgr);
		return;
	}

	const EFCGameStateID CurrentState = StateMgr->GetCurrentState();
	const EFCGameStateID LoadingTarget = StateMgr->GetLoadingTargetState();
	const FName CurrentLevelName = LevelMgr->GetCurrentLevelName();

	// Case 1: Office + fresh startup or return-to-main-menu -> ensure MainMenu state and show main menu.
	if (CurrentLevelName.IsEqual(FName(TEXT("L_Office"))))
	{
		bool bShouldInitMainMenu = false;

		// Fresh game start: None -> MainMenu.
		if (CurrentState == EFCGameStateID::None)
		{
			StateMgr->TransitionTo(EFCGameStateID::MainMenu);
			bShouldInitMainMenu = true;
		}
		// Back to main menu from gameplay: Loading(target=MainMenu) -> MainMenu.
		else if (CurrentState == EFCGameStateID::Loading && LoadingTarget == EFCGameStateID::MainMenu)
		{
			StateMgr->TransitionTo(EFCGameStateID::MainMenu);
			bShouldInitMainMenu = true;
		}
		// Already in MainMenu and reloading Office (e.g. hot reload).
		else if (CurrentState == EFCGameStateID::MainMenu)
		{
			bShouldInitMainMenu = true;
		}

		if (bShouldInitMainMenu)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			AFCPlayerController* FCPC = Cast<AFCPlayerController>(PC);
			if (FCPC)
			{
				UE_LOG(LogFCLevelTransitionManager, Log, TEXT("InitializeOnLevelStart: Office startup/main-menu detected (state=%s, target=%s), initializing main menu"),
					*UEnum::GetValueAsString(StateMgr->GetCurrentState()),
					*UEnum::GetValueAsString(LoadingTarget));
				FCPC->InitializeMainMenu();
			}
			else
			{
				UE_LOG(LogFCLevelTransitionManager, Warning, TEXT("InitializeOnLevelStart: First player controller is not AFCPlayerController; cannot initialize main menu"));
			}
			return;
		}
	}

	// Case 2: Loading->ExpeditionSummary or other loading-based flows.
	InitializeLevelTransitionOnLevelStart();
}

void UFCLevelTransitionManager::CloseExpeditionSummaryAndReturnToOffice()
{
	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCUIManager* UIManager = GetUIManager();
	if (!StateMgr)
	{
		UE_LOG(LogTemp, Error, TEXT("CloseExpeditionSummaryAndReturnToOffice: GameStateManager missing"));
		return;
	}

	// Ensure we are currently in ExpeditionSummary state.
	if (StateMgr->GetCurrentState() != EFCGameStateID::ExpeditionSummary)
	{
		UE_LOG(LogTemp, Warning, TEXT("CloseExpeditionSummaryAndReturnToOffice: Called from non-ExpeditionSummary state %s"),
			*UEnum::GetValueAsString(StateMgr->GetCurrentState()));
	}

	// Let UIManager close the summary widget if it tracks one.
	if (UIManager)
	{
		UIManager->CloseExpeditionSummary();
	}

	// Transition back to Office_TableView so the camera/input are aligned
	// with the office desk/tableview setup.
	StateMgr->TransitionTo(EFCGameStateID::Office_TableView);
}

void UFCLevelTransitionManager::RequestTransition(EFCGameStateID TargetState, FName TargetLevelName, bool bUseFade)
{
	// 4.7 stub: central orchestration point for future refactors.
	// For now this only logs and returns, so existing specialized
	// helpers (StartExpeditionFromOfficeTableView, ReturnToMainMenuFromGameplay,
	// etc.) remain the actual implementation.
	UE_LOG(LogFCLevelTransitionManager, Log,
		TEXT("RequestTransition (stub): TargetState=%s, TargetLevel=%s, bUseFade=%s"),
		*UEnum::GetValueAsString(TargetState),
		*TargetLevelName.ToString(),
		bUseFade ? TEXT("true") : TEXT("false"));
}

void UFCLevelTransitionManager::EnterOfficeTableView(AActor* TableActor)
{
	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCUIManager* UIManager = GetUIManager();
	UWorld* World = GetWorld();

	if (!StateMgr || !World)
	{
		UE_LOG(LogFCLevelTransitionManager, Error, TEXT("EnterOfficeTableView: Missing StateMgr or World (StateMgr=%p, World=%p)"), StateMgr, World);
		return;
	}

	// Only meaningful from Office_Exploration for now.
	if (StateMgr->GetCurrentState() != EFCGameStateID::Office_Exploration)
	{
		UE_LOG(LogFCLevelTransitionManager, Verbose,
			TEXT("EnterOfficeTableView: Ignored because current state is %s (expected Office_Exploration)"),
			*UEnum::GetValueAsString(StateMgr->GetCurrentState()));
		return;
	}

	// Transition state Office_Exploration -> Office_TableView.
	if (!StateMgr->TransitionTo(EFCGameStateID::Office_TableView))
	{
		UE_LOG(LogFCLevelTransitionManager, Error, TEXT("EnterOfficeTableView: Failed to transition to Office_TableView"));
		return;
	}

	// Let the PlayerController react to the new state for camera/input via
	// AFCPlayerController::OnGameStateChanged. We still allow the existing
	// camera blend + widget timing in AFCPlayerController to run for now so
	// behavior remains unchanged while we migrate.
	UE_LOG(LogFCLevelTransitionManager, Log,
		TEXT("EnterOfficeTableView: Transitioned to Office_TableView for TableActor=%s"),
		TableActor ? *TableActor->GetName() : TEXT("<null>"));
}

void UFCLevelTransitionManager::ExitOfficeTableView()
{
	UFCGameStateManager* StateMgr = GetGameStateManager();
	UFCUIManager* UIManager = GetUIManager();
	UWorld* World = GetWorld();

	if (!StateMgr || !World)
	{
		UE_LOG(LogFCLevelTransitionManager, Error, TEXT("ExitOfficeTableView: Missing StateMgr or World (StateMgr=%p, World=%p)"), StateMgr, World);
		return;
	}

	// Only valid when currently in Office_TableView.
	if (StateMgr->GetCurrentState() != EFCGameStateID::Office_TableView)
	{
		UE_LOG(LogFCLevelTransitionManager, Verbose,
			TEXT("ExitOfficeTableView: Ignored because current state is %s (expected Office_TableView)"),
			*UEnum::GetValueAsString(StateMgr->GetCurrentState()));
		return;
	}

	// Close any open table widget via UIManager.
	if (UIManager && UIManager->IsTableWidgetOpen())
	{
		UIManager->CloseTableWidget();
		UE_LOG(LogFCLevelTransitionManager, Log, TEXT("ExitOfficeTableView: Closed open table widget"));
	}

	// Transition state Office_TableView -> Office_Exploration.
	if (!StateMgr->TransitionTo(EFCGameStateID::Office_Exploration))
	{
		UE_LOG(LogFCLevelTransitionManager, Error, TEXT("ExitOfficeTableView: Failed to transition to Office_Exploration"));
		return;
	}

	UE_LOG(LogFCLevelTransitionManager, Log, TEXT("ExitOfficeTableView: Transitioned back to Office_Exploration"));
}
