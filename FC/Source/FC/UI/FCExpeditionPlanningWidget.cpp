// Copyright Slomotion Games. All Rights Reserved.

#include "UI/FCExpeditionPlanningWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/NamedSlot.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCGameStateManager.h"
#include "Core/FCLevelManager.h"
#include "Core/FCPlayerController.h"
#include "Core/FCLevelTransitionManager.h"
#include "Expedition/FCExpeditionManager.h"
#include "Kismet/GameplayStatics.h"

void UFCExpeditionPlanningWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Log, TEXT("UFCExpeditionPlanningWidget::NativeConstruct"));

    // Bind button click events
    if (StartExpeditionButton)
    {
        StartExpeditionButton->OnClicked.AddDynamic(this, &UFCExpeditionPlanningWidget::OnStartExpeditionClicked);
        UE_LOG(LogTemp, Log, TEXT("Bound StartExpeditionButton OnClicked"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("StartExpeditionButton is null - check widget binding"));
    }

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UFCExpeditionPlanningWidget::OnBackButtonClicked);
        UE_LOG(LogTemp, Log, TEXT("Bound BackButton OnClicked"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BackButton is null - check widget binding"));
    }

    // Initialize supplies and money display
    UpdateSuppliesDisplay();
}

void UFCExpeditionPlanningWidget::NativeDestruct()
{
    // Unbind button events
    if (StartExpeditionButton)
    {
        StartExpeditionButton->OnClicked.RemoveDynamic(this, &UFCExpeditionPlanningWidget::OnStartExpeditionClicked);
    }

    if (BackButton)
    {
        BackButton->OnClicked.RemoveDynamic(this, &UFCExpeditionPlanningWidget::OnBackButtonClicked);
    }

    Super::NativeDestruct();
}

void UFCExpeditionPlanningWidget::OnStartExpeditionClicked()
{
    UE_LOG(LogTemp, Log, TEXT("UFCExpeditionPlanningWidget::OnStartExpeditionClicked"));

    UWorld* World = GetWorld();
    if (!World)
    {
        const FString Reason = TEXT("Cannot start expedition: World is null");
        UE_LOG(LogTemp, Error, TEXT("OnStartExpeditionClicked: %s"), *Reason);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Reason);
        }
        return;
    }

    UFCGameInstance* GameInstance = Cast<UFCGameInstance>(World->GetGameInstance());
    if (!GameInstance)
    {
        const FString Reason = TEXT("Cannot start expedition: GameInstance is not UFCGameInstance");
        UE_LOG(LogTemp, Error, TEXT("OnStartExpeditionClicked: %s"), *Reason);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Reason);
        }
        return;
    }

    UFCExpeditionManager* ExpeditionManager = GameInstance->GetSubsystem<UFCExpeditionManager>();
    if (!ExpeditionManager)
    {
        const FString Reason = TEXT("Cannot start expedition: ExpeditionManager subsystem missing");
        UE_LOG(LogTemp, Error, TEXT("OnStartExpeditionClicked: %s"), *Reason);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Reason);
        }
        return;
    }

    // Reuse the shared affordability helper to validate the plan and
    // fetch the planned cost/current money in a single place.
    int32 PlannedCost = 0;
    int32 CurrentMoney = 0;
    if (!CanAffordCurrentExpedition(PlannedCost, CurrentMoney))
    {
        const FString Reason = FString::Printf(
            TEXT("Cannot start expedition: Plan invalid or insufficient funds (Cost=%d, Current=%d)"),
            PlannedCost,
            CurrentMoney);
        UE_LOG(LogTemp, Warning, TEXT("OnStartExpeditionClicked: %s"), *Reason);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Reason);
        }
        return;
    }

    // At this point we know the plan is valid and affordable; consume money.
    if (!GameInstance->ConsumeMoney(PlannedCost))
    {
        // This should be rare, but handle race conditions or external
        // money changes gracefully.
        CurrentMoney = GameInstance->GetMoney();
        const FString Reason = FString::Printf(
            TEXT("Cannot start expedition: ConsumeMoney failed (Cost=%d, Current=%d)"),
            PlannedCost,
            CurrentMoney);
        UE_LOG(LogTemp, Warning, TEXT("OnStartExpeditionClicked: %s"), *Reason);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, Reason);
        }
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("OnStartExpeditionClicked: Money consumed for expedition (Cost=%d, Remaining=%d)"),
        PlannedCost,
        GameInstance->GetMoney());

    UFCExpeditionData* CurrentExpedition = ExpeditionManager->GetCurrentExpedition();
    if (CurrentExpedition && CurrentExpedition->ExpeditionStatus == EFCExpeditionStatus::Planning)
    {
        CurrentExpedition->ExpeditionStatus = EFCExpeditionStatus::InProgress;
        ExpeditionManager->OnExpeditionStateChanged.Broadcast(CurrentExpedition);
    }

    // Delegate the actual state/level transition to UFCLevelTransitionManager so
    // that all complex flows live in one place.
    UFCLevelTransitionManager* TransitionManager = GameInstance->GetSubsystem<UFCLevelTransitionManager>();
    if (!TransitionManager)
    {
        const FString Reason = TEXT("Cannot start expedition: LevelTransitionManager subsystem missing");
        UE_LOG(LogTemp, Error, TEXT("OnStartExpeditionClicked: %s"), *Reason);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Reason);
        }
        return;
    }

    TransitionManager->StartExpeditionFromOfficeTableView();

    RemoveFromParent();
}

void UFCExpeditionPlanningWidget::OnBackButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Back button clicked"));

    // Get player controller and call CloseTableWidget
    APlayerController* PC = GetOwningPlayer();
    if (AFCPlayerController* FCPC = Cast<AFCPlayerController>(PC))
    {
        FCPC->CloseTableWidget();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get FCPlayerController in OnBackButtonClicked"));
    }
}

void UFCExpeditionPlanningWidget::UpdateSuppliesDisplay()
{
    if (!SuppliesValue)
    {
        UE_LOG(LogTemp, Warning, TEXT("SuppliesValue TextBlock is null - cannot update supplies display"));
        return;
    }

    // Get GameInstance and read CurrentSupplies
    UFCGameInstance* GameInstance = Cast<UFCGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        int32 CurrentSupplies = GameInstance->GetCurrentSupplies();
        FText SuppliesText = FText::AsNumber(CurrentSupplies);
        SuppliesValue->SetText(SuppliesText);

        UE_LOG(LogTemp, Log, TEXT("Updated supplies display: %d"), CurrentSupplies);

        // Update money display in the same pass if possible
        if (MoneyValue)
        {
            const int32 CurrentMoney = GameInstance->GetMoney();
            const FText MoneyText = FText::AsNumber(CurrentMoney);
            MoneyValue->SetText(MoneyText);

            UE_LOG(LogTemp, Log, TEXT("Updated money display: %d"), CurrentMoney);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MoneyValue TextBlock is null - cannot update money display"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get FCGameInstance in UpdateSuppliesDisplay"));
        SuppliesValue->SetText(FText::FromString(TEXT("???")));
    }
}

bool UFCExpeditionPlanningWidget::CanAffordCurrentExpedition(int32& OutPlannedCost, int32& OutCurrentMoney) const
{
    OutPlannedCost = 0;
    OutCurrentMoney = 0;

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    UFCGameInstance* GameInstance = Cast<UFCGameInstance>(World->GetGameInstance());
    if (!GameInstance)
    {
        return false;
    }

    UFCExpeditionManager* ExpeditionManager = GameInstance->GetSubsystem<UFCExpeditionManager>();
    if (!ExpeditionManager)
    {
        return false;
    }

    UFCExpeditionData* CurrentExpedition = ExpeditionManager->GetCurrentExpedition();
    if (!CurrentExpedition)
    {
        return false;
    }

    if (CurrentExpedition->PlannedRouteGlobalIds.Num() <= 0 || CurrentExpedition->PlannedMoneyCost <= 0)
    {
        return false;
    }

    OutPlannedCost = CurrentExpedition->PlannedMoneyCost;
    OutCurrentMoney = GameInstance->GetMoney();

    return OutCurrentMoney >= OutPlannedCost;
}
