#pragma once

#include "CoreMinimal.h"
#include "UI/FCBlockingWidgetBase.h"
#include "FCExpeditionSummaryWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * Base C++ class for the Expedition Summary screen.
 *
 * This widget is intended to be subclassed in UMG as WBP_ExpeditionSummary.
 * It inherits from UFCBlockingWidgetBase so it blocks world interaction
 * while visible. Blueprint should bind the "Return to Office" button's
 * OnClicked event to the exposed handler.
 */
UCLASS()
class FC_API UFCExpeditionSummaryWidget : public UFCBlockingWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** Optional summary text block showing basic expedition results. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* SummaryText;

	/** Button that returns the player to the office. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UButton* ReturnButton;

	/** Blueprint-callable handler to trigger the return flow.
	 *  Bind this to the WBP button's OnClicked in Blueprint if not using
	 *  the automatic binding from NativeConstruct.
	 */
		UFUNCTION(BlueprintCallable, Category = "Expedition|Summary")
		void CloseSummary();

	/** Internal helper used by NativeConstruct to bind the button if present. */
	UFUNCTION()
	void OnReturnButtonClickedInternal();
};
