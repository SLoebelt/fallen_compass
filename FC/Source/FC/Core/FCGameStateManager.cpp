// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#include "Core/FCGameStateManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogFCGameState);

void UFCGameStateManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize to None state
	CurrentState = EFCGameStateID::None;
	PreviousState = EFCGameStateID::None;

	// Initialize valid state transitions
	InitializeValidTransitions();

	UE_LOG(LogFCGameState, Log, TEXT("GameStateManager initialized"));
}

void UFCGameStateManager::InitializeValidTransitions()
{
	ValidTransitions.Empty();

	// MainMenu transitions
	ValidTransitions.Add(EFCGameStateID::MainMenu, {
		EFCGameStateID::Office_Exploration,
		EFCGameStateID::Loading,
		EFCGameStateID::Paused
	});

	// Office_Exploration transitions
	ValidTransitions.Add(EFCGameStateID::Office_Exploration, {
		EFCGameStateID::Office_TableView,
		EFCGameStateID::MainMenu,
		EFCGameStateID::Overworld_Travel,
		EFCGameStateID::Paused,
		EFCGameStateID::Loading
	});

	// Office_TableView transitions
	ValidTransitions.Add(EFCGameStateID::Office_TableView, {
		EFCGameStateID::Office_Exploration,
		EFCGameStateID::Paused
	});

	// Overworld_Travel transitions
	ValidTransitions.Add(EFCGameStateID::Overworld_Travel, {
		EFCGameStateID::Combat_PlayerTurn,
		EFCGameStateID::Office_Exploration,
		EFCGameStateID::Paused,
		EFCGameStateID::Loading
	});

	// Combat_PlayerTurn transitions
	ValidTransitions.Add(EFCGameStateID::Combat_PlayerTurn, {
		EFCGameStateID::Combat_EnemyTurn,
		EFCGameStateID::Overworld_Travel,
		EFCGameStateID::Paused
	});

	// Combat_EnemyTurn transitions
	ValidTransitions.Add(EFCGameStateID::Combat_EnemyTurn, {
		EFCGameStateID::Combat_PlayerTurn,
		EFCGameStateID::Overworld_Travel
	});

	// Paused transitions (can return to any gameplay state)
	ValidTransitions.Add(EFCGameStateID::Paused, {
		EFCGameStateID::MainMenu,
		EFCGameStateID::Office_Exploration,
		EFCGameStateID::Office_TableView,
		EFCGameStateID::Overworld_Travel,
		EFCGameStateID::Combat_PlayerTurn,
		EFCGameStateID::Combat_EnemyTurn,
		EFCGameStateID::Loading
	});

	// Loading transitions (can go to any state after loading)
	ValidTransitions.Add(EFCGameStateID::Loading, {
		EFCGameStateID::MainMenu,
		EFCGameStateID::Office_Exploration,
		EFCGameStateID::Overworld_Travel,
		EFCGameStateID::Combat_PlayerTurn
	});

	// None transitions (initial state, can go anywhere)
	ValidTransitions.Add(EFCGameStateID::None, {
		EFCGameStateID::MainMenu,
		EFCGameStateID::Office_Exploration,
		EFCGameStateID::Loading
	});

	UE_LOG(LogFCGameState, Log, TEXT("InitializeValidTransitions: Configured %d state transition rules"), 
		ValidTransitions.Num());
}

bool UFCGameStateManager::CanTransitionTo(EFCGameStateID NewState) const
{
	// Always allow transitioning to the same state (no-op)
	if (NewState == CurrentState)
	{
		return true;
	}

	// Check if there's a valid transitions array for current state
	const TArray<EFCGameStateID>* AllowedStates = ValidTransitions.Find(CurrentState);
	if (!AllowedStates)
	{
		UE_LOG(LogFCGameState, Warning, TEXT("CanTransitionTo: No transition rules defined for state %s"),
			*UEnum::GetValueAsString(CurrentState));
		return false;
	}

	// Check if NewState is in the allowed transitions
	return AllowedStates->Contains(NewState);
}

bool UFCGameStateManager::TransitionTo(EFCGameStateID NewState)
{
	// Check if already in target state
	if (NewState == CurrentState)
	{
		UE_LOG(LogFCGameState, Verbose, TEXT("TransitionTo: Already in state %s, ignoring"),
			*UEnum::GetValueAsString(NewState));
		return true;
	}

	// Validate transition
	if (!CanTransitionTo(NewState))
	{
		UE_LOG(LogFCGameState, Error, TEXT("TransitionTo: Invalid transition from %s to %s"),
			*UEnum::GetValueAsString(CurrentState),
			*UEnum::GetValueAsString(NewState));
		return false;
	}

	// Execute transition
	EFCGameStateID OldState = CurrentState;
	PreviousState = CurrentState;
	CurrentState = NewState;

	UE_LOG(LogFCGameState, Log, TEXT("State transition: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	// Broadcast state change event
	OnStateChanged.Broadcast(OldState, NewState);

	return true;
}

bool UFCGameStateManager::PushState(EFCGameStateID NewState)
{
	// Validate transition before pushing
	if (!CanTransitionTo(NewState))
	{
		UE_LOG(LogFCGameState, Error, TEXT("PushState: Invalid transition from %s to %s"),
			*UEnum::GetValueAsString(CurrentState),
			*UEnum::GetValueAsString(NewState));
		return false;
	}

	// Push current state onto stack
	StateStack.Add(CurrentState);
	
	UE_LOG(LogFCGameState, Log, TEXT("PushState: Pushed %s onto stack (depth now: %d)"),
		*UEnum::GetValueAsString(CurrentState),
		StateStack.Num());

	// Execute transition to new state
	EFCGameStateID OldState = CurrentState;
	PreviousState = CurrentState;
	CurrentState = NewState;

	UE_LOG(LogFCGameState, Log, TEXT("State transition (push): %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	// Broadcast state change event
	OnStateChanged.Broadcast(OldState, NewState);

	return true;
}

bool UFCGameStateManager::PopState()
{
	// Check if stack has states to pop
	if (StateStack.Num() == 0)
	{
		UE_LOG(LogFCGameState, Warning, TEXT("PopState: State stack is empty, cannot pop"));
		return false;
	}

	// Pop state from stack
	EFCGameStateID RestoredState = StateStack.Pop();

	UE_LOG(LogFCGameState, Log, TEXT("PopState: Popped state, restoring to %s (stack depth now: %d)"),
		*UEnum::GetValueAsString(RestoredState),
		StateStack.Num());

	// Transition to restored state (without validation - we trust the stack)
	EFCGameStateID OldState = CurrentState;
	PreviousState = CurrentState;
	CurrentState = RestoredState;

	UE_LOG(LogFCGameState, Log, TEXT("State transition (pop): %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(RestoredState));

	// Broadcast state change event
	OnStateChanged.Broadcast(OldState, RestoredState);

	return true;
}

EFCGameStateID UFCGameStateManager::GetStateAtDepth(int32 Depth) const
{
	// Handle negative indices (from top)
	if (Depth < 0)
	{
		Depth = StateStack.Num() + Depth;
	}

	// Validate depth
	if (Depth < 0 || Depth >= StateStack.Num())
	{
		UE_LOG(LogFCGameState, Warning, TEXT("GetStateAtDepth: Invalid depth %d (stack size: %d)"),
			Depth, StateStack.Num());
		return EFCGameStateID::None;
	}

	return StateStack[Depth];
}
