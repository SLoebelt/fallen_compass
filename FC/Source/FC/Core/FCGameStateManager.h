// Copyright (c) 2024 @ Steffen Loebelt. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FCGameStateManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFCGameState, Log, All);

/**
 * EFCGameStateID
 * 
 * Defines all possible game states in Fallen Compass.
 * Used by UFCGameStateManager to track current state and enforce valid transitions.
 */
UENUM(BlueprintType)
enum class EFCGameStateID : uint8
{
	None              UMETA(DisplayName = "None"),
	MainMenu          UMETA(DisplayName = "Main Menu"),
	Office_Exploration UMETA(DisplayName = "Office Exploration"),
	Office_TableView  UMETA(DisplayName = "Office Table View"),
	Overworld_Travel  UMETA(DisplayName = "Overworld Travel"),
	Combat_PlayerTurn UMETA(DisplayName = "Combat - Player Turn"),
	Combat_EnemyTurn  UMETA(DisplayName = "Combat - Enemy Turn"),
	Paused            UMETA(DisplayName = "Paused"),
	Loading           UMETA(DisplayName = "Loading")
};

/**
 * UFCGameStateManager
 * 
 * Game Instance Subsystem that manages the explicit game state machine.
 * Tracks current and previous states, enforces valid state transitions,
 * and broadcasts state change events to interested systems.
 * 
 * Responsibilities:
 * - Track current game state (MainMenu, Office_Exploration, Combat, etc.)
 * - Validate and execute state transitions
 * - Maintain state history (current + previous)
 * - Optional: State stack for nested states (Pause, modals)
 * - Broadcast state change events
 * 
 * Usage:
 *   UFCGameStateManager* StateMgr = GetGameInstance()->GetSubsystem<UFCGameStateManager>();
 *   StateMgr->TransitionTo(EFCGameStateID::Office_Exploration);
 */
UCLASS()
class FC_API UFCGameStateManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Get the current game state */
	UFUNCTION(BlueprintCallable, Category = "FC|GameState")
	EFCGameStateID GetCurrentState() const { return CurrentState; }

	/** Get the previous game state */
	UFUNCTION(BlueprintCallable, Category = "FC|GameState")
	EFCGameStateID GetPreviousState() const { return PreviousState; }

	/** 
	 * Attempt to transition to a new state
	 * @param NewState The target state
	 * @return True if transition was successful, false if invalid transition
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|GameState")
	bool TransitionTo(EFCGameStateID NewState);

	/**
	 * Check if a transition to the given state is valid from current state
	 * @param NewState The target state to validate
	 * @return True if transition is allowed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "FC|GameState")
	bool CanTransitionTo(EFCGameStateID NewState) const;

	/** Delegate broadcast when state changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, EFCGameStateID, OldState, EFCGameStateID, NewState);
	
	UPROPERTY(BlueprintAssignable, Category = "FC|GameState")
	FOnStateChanged OnStateChanged;

private:
	/** Current game state */
	UPROPERTY()
	EFCGameStateID CurrentState;

	/** Previous game state (for state restoration) */
	UPROPERTY()
	EFCGameStateID PreviousState;

	/** 
	 * Valid state transitions map
	 * Key = Current State, Value = Array of allowed target states
	 */
	TMap<EFCGameStateID, TArray<EFCGameStateID>> ValidTransitions;

	/** Initialize the valid state transitions */
	void InitializeValidTransitions();
};
