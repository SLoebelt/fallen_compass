#include "UI/FCExpeditionSummaryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCLevelManager.h"
#include "Core/FCGameStateManager.h"

void UFCExpeditionSummaryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReturnButton)
	{
		ReturnButton->OnClicked.Clear();
		ReturnButton->OnClicked.AddDynamic(this, &UFCExpeditionSummaryWidget::OnReturnButtonClickedInternal);
	}
}

void UFCExpeditionSummaryWidget::OnReturnButtonClickedInternal()
{
	HandleReturnToOfficeClicked();
}

void UFCExpeditionSummaryWidget::HandleReturnToOfficeClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UFCGameInstance* FCGameInstance = Cast<UFCGameInstance>(World->GetGameInstance());
	if (!FCGameInstance)
	{
		return;
	}

	// Game state transition to Office_Exploration
	if (UFCGameStateManager* GameStateManager = FCGameInstance->GetSubsystem<UFCGameStateManager>())
	{
		GameStateManager->TransitionTo(EFCGameStateID::Office_Exploration);
	}

	// Level transition back to L_Office (can be data-driven later)
	if (UFCLevelManager* LevelManager = FCGameInstance->GetSubsystem<UFCLevelManager>())
	{
		LevelManager->LoadLevel(FName(TEXT("L_Office")), true);
	}

	RemoveFromParent();
}
