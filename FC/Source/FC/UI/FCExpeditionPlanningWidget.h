// Copyright Iron Anchor Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/FCBlockingWidgetBase.h"
#include "FCExpeditionPlanningWidget.generated.h"

class UCanvasPanel;
class UImage;
class UVerticalBox;
class UHorizontalBox;
class UBorder;
class UButton;
class UTextBlock;
class UNamedSlot;

/**
 * Main expedition planning widget shown when clicking the map table object.
 * Displays supplies, embedded world map (WBP_WorldMap), and expedition start/back buttons.
 * Week 2 Task 3.4.0: WBP_TableMap implementation in C++
 * Week 4 Task 4.2: Integrated with WBP_WorldMap for expedition planning
 */
UCLASS()
class FC_API UFCExpeditionPlanningWidget : public UFCBlockingWidgetBase
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    /** Button click handlers */
    UFUNCTION()
    void OnStartExpeditionClicked();

    UFUNCTION()
    void OnBackButtonClicked();

    /** Update supplies display from GameInstance */
    void UpdateSuppliesDisplay();

    /**
     * Shared affordability helper used by both C++ and Blueprint.
     * Evaluates whether the current expedition can be afforded based on
     * GameInstance money and the planned expedition cost.
     */
    UFUNCTION(BlueprintCallable, Category = "Expedition|Planning")
    bool CanAffordCurrentExpedition(int32& OutPlannedCost, int32& OutCurrentMoney) const;

public:
    // UI Components (exposed for Blueprint access if needed)
    
    /** Root canvas panel */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UCanvasPanel> RootCanvas;

    /** Background image (parchment texture) */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UImage> BackgroundImage;

    /** Main container for all UI elements */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UVerticalBox> MainContainer;

    /** Title text "Expedition Planning" */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> TitleText;

    /** Supplies label "Supplies: " */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> SuppliesLabel;

    /** Supplies value (bound to GameInstance) */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> SuppliesValue;

    /** Money label "Money: " */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> MoneyLabel;

    /** Money value (bound to GameInstance) */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> MoneyValue;

    /** Map container border */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UBorder> MapContainer;

    /** World map widget slot (for embedding WBP_WorldMap) */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UNamedSlot> WorldMapSlot;

    /** Start expedition button */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UButton> StartExpeditionButton;

    /** Start button text */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> StartButtonText;

    /** Back button */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UButton> BackButton;

    /** Back button text */
    UPROPERTY(BlueprintReadOnly, Category = "Expedition|UI", meta = (BindWidget))
    TObjectPtr<UTextBlock> BackButtonText;
};
