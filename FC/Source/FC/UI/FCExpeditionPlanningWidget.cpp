// Copyright Iron Anchor Interactive. All Rights Reserved.

#include "UI/FCExpeditionPlanningWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCPlayerController.h"
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

    // Initialize supplies display
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
    UE_LOG(LogTemp, Log, TEXT("Start Expedition button clicked"));

    // Week 3 feature: Show "Coming Soon" message
    // For now, just log it
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1, 
            3.0f, 
            FColor::Yellow, 
            TEXT("Coming Soon - Overworld in Week 3")
        );
    }
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
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get FCGameInstance in UpdateSuppliesDisplay"));
        SuppliesValue->SetText(FText::FromString(TEXT("???")));
    }
}
