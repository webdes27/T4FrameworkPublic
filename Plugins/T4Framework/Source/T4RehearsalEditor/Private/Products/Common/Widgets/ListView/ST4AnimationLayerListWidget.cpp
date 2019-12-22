// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4AnimationLayerListWidget.h"

#include "Products/Common/Helper/T4EditorAnimSetAssetSelector.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "Animation/BlendSpace.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4AnimationLayerListWidget"

/**
  * #39
 */
ST4AnimationLayerListWidget::ST4AnimationLayerListWidget()
	: AnimationLayer(ET4AnimLayer::AnimLayer_Nums)
{
	bShowSearchAndRefreshButton = true;
	bShowUpAndDownButton = true;
}

ST4AnimationLayerListWidget::~ST4AnimationLayerListWidget()
{
	if (AnimSetAssetSelector.IsValid() && AnimSetSelectDelegate.IsValid())
	{
		AnimSetAssetSelector.Pin()->GetOnAnimSetChanged().Remove(AnimSetSelectDelegate);
		AnimSetAssetSelector.Reset();
	}
}

void ST4AnimationLayerListWidget::Construct(
	const FArguments& InArgs,
	TSharedRef<FT4EditorAnimSetAssetSelector> InAnimSetAssetSelector,
	ET4AnimLayer InAnimationLayer
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;

	OnMoveUpItem = InArgs._OnMoveUpItem;
	OnMoveDownItem = InArgs._OnMoveDownItem;

	AnimSetAssetSelector = InAnimSetAssetSelector;
	AnimationLayer = InAnimationLayer;

	if (AnimSetAssetSelector.IsValid())
	{
		AnimSetSelectDelegate = AnimSetAssetSelector.Pin()->GetOnAnimSetChanged().AddRaw(
			this,
			&ST4AnimationLayerListWidget::HandleOnAnimSetChanged
		);
	}
	
	check(ET4AnimLayer::AnimLayer_Nums != AnimationLayer);

	Create();
};

void ST4AnimationLayerListWidget::HandleOnAnimSetChanged(UT4AnimSetAsset* InAnimSetAsset)
{
	Refresh();
}

void ST4AnimationLayerListWidget::UpdateLists()
{
	if (!AnimSetAssetSelector.IsValid() || AnimSetAssetSelector.Pin()->IsNull())
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector.Pin()->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)

	if (ET4AnimLayer::AnimLayer_BlendSpace == AnimationLayer)
	{
		for (TArray<FT4BlendSpaceInfo>::TConstIterator It = AnimSetAsset->BlendSpaceArray.CreateConstIterator(); It; ++It)
		{
			FString DurationSecString = TEXT("");
			const FT4BlendSpaceInfo& BlendSpaceInfo = *It;
			const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
				ET4EngineConstantType::BlendSpace,
				BlendSpaceInfo.Name
			);
			FT4BlendSpaceInfo* SelectedInfo = AnimSetAsset->BlendSpaceArray.FindByKey(BlendSpaceInfo.Name);
			if (nullptr != SelectedInfo)
			{
				if (!SelectedInfo->BlendSpaceAsset.IsNull())
				{
					UBlendSpaceBase* BlendSpaceBase = SelectedInfo->BlendSpaceAsset.LoadSynchronous();
					check(nullptr != BlendSpaceBase);
					if (nullptr != Cast<UBlendSpace>(BlendSpaceBase))
					{
						DurationSecString = FString::Printf(
							TEXT(" (2D: %s x %s)"),
							*(BlendSpaceBase->GetBlendParameter(0).DisplayName),
							*(BlendSpaceBase->GetBlendParameter(1).DisplayName)
						);
					}
					else
					{
						DurationSecString = FString::Printf(
							TEXT(" (1D: %s)"),
							*(BlendSpaceBase->GetBlendParameter(0).DisplayName)
						);
					}
				}
			}
			TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
			NewItem->DisplayString = FString::Printf(
				TEXT("[%s] %s%s"),
				*BlendSpaceInfo.Name.ToString(),
				*ConstantData.Description,
				*DurationSecString
			);
			NewItem->ValueName = BlendSpaceInfo.Name;
			NewItem->SortOrder = ConstantData.SortOrder;
			ItemList.Add(NewItem);
			if (InitializeValue == NAME_None)
			{
				ItemSelected = NewItem;
				InitializeValue = NewItem->ValueName;
			}
			else if (BlendSpaceInfo.Name == InitializeValue)
			{
				ItemSelected = NewItem;
			}
		}
		return;
	}

	ET4EngineConstantType EngineNameTable = ET4EngineConstantType::Nums;
	const TArray<FT4AnimSequenceInfo>* SelectArray = nullptr;
	switch (AnimationLayer)
	{
		case AnimLayer_Skill:
			{
				EngineNameTable = ET4EngineConstantType::SkillSection;
				SelectArray = &AnimSetAsset->SkillAnimSequenceArray;
			}
			break;

		case AnimLayer_Additive:
			{
				EngineNameTable = ET4EngineConstantType::AdditiveSection;
				SelectArray = &AnimSetAsset->AdditiveAnimSequenceArray;
			}
			break;

		case AnimLayer_Default:
			{
				EngineNameTable = ET4EngineConstantType::DefaultSection;
				SelectArray = &AnimSetAsset->DefaultAnimSequenceArray;
			}
			break;

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("ST4AnimationLayerListWidget : Unknown AnimationLayer '%u'"),
					uint8(AnimationLayer)
				);
			}
			break;
	}

	check(nullptr != SelectArray);
	for (TArray<FT4AnimSequenceInfo>::TConstIterator It = SelectArray->CreateConstIterator(); It; ++It)
	{
		const FT4AnimSequenceInfo& AnimSequenceInfo = *It;
		FString DurationSecString = FString::Printf(TEXT(" (%.2f Sec)"), AnimSequenceInfo.DurationSec);

		const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
			EngineNameTable,
			AnimSequenceInfo.Name
		);

		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s%s"),
			*AnimSequenceInfo.Name.ToString(),
			*ConstantData.Description,
			*DurationSecString
		);
		NewItem->ValueName = AnimSequenceInfo.Name;
		NewItem->SortOrder = ConstantData.SortOrder;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (AnimSequenceInfo.Name == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE