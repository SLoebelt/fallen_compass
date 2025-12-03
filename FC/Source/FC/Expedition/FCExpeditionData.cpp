// Copyright Iron Anchor Interactive. All Rights Reserved.

#include "Expedition/FCExpeditionData.h"

UFCExpeditionData::UFCExpeditionData()
	: ExpeditionName(TEXT("Unnamed Expedition"))
	, StartDate(TEXT("Day 1"))
	, TargetRegion(TEXT("Unknown"))
	, StartingSupplies(0)
	, ExpeditionStatus(EFCExpeditionStatus::Planning)
	, SelectedGridId(INDEX_NONE)
	, SelectedStartGridId(INDEX_NONE)
	, SelectedStartSubId(INDEX_NONE)
	, PreviewTargetGridId(INDEX_NONE)
	, PreviewTargetSubId(INDEX_NONE)
	, PlannedMoneyCost(0)
	, PlannedRiskCost(0)
{
}
