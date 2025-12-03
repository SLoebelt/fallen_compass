#include "UI/FCExpeditionSummaryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/UFCGameInstance.h"
#include "Core/FCLevelManager.h"
#include "Core/FCGameStateManager.h"
#include "Core/FCLevelTransitionManager.h"

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
	CloseSummary();
}
void UFCExpeditionSummaryWidget::CloseSummary()
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

	if (UFCLevelTransitionManager* TransitionMgr = FCGameInstance->GetSubsystem<UFCLevelTransitionManager>())
	{
		TransitionMgr->CloseExpeditionSummaryAndReturnToOffice();
	}
}
