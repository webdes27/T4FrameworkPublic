// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "AssetData.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */
class UAnimSequence;
class FT4EntityViewModel;
class FT4AnimSetDetailCustomization;
class FT4AnimSetAnimationLayerHelper // #95
{
public:
	explicit FT4AnimSetAnimationLayerHelper(ET4AnimLayer InAnimationLayer, TSharedPtr<FT4EntityViewModel> InViewModelPtr);
	virtual ~FT4AnimSetAnimationLayerHelper();

	bool UpdatSection(FString& OutErrorMessage); // #95

	bool OnAdd();
	bool OnRemoveSectionByName();
	bool OnPlayOneShotBySelected();

	bool OnSectionNameChanged(const FName InName);
	bool OnSectionAssetChanged(const FAssetData& InAssetData); // #95
	bool OnSectionNameSelected(const FName InName);
	bool OnSectionMoveUpItem(const FName InName);
	bool OnSectionMoveDownItem(const FName InName);

private:
	FName GetNameSelected() const;
	TSoftObjectPtr<UAnimSequence> GetAssetSelected() const;
	bool HasSection() const;
	FName GetAnimMontageName() const;
	ET4EditorAction GetEditorActionType() const;

private:
	ET4AnimLayer AnimationLayer;
	TSharedPtr<FT4EntityViewModel> ViewModelPtr;
};

class SEditableTextBox;
class USkeleton;
class IPropertyHandle;
class ST4AnimSectionDropListWidget;
class ST4AnimationLayerListWidget;
class ST4BlendSpaceDropListWidget;
class UT4AnimSetAsset;
class FT4AnimSetDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TSharedPtr<FT4EntityViewModel> InEntityViewModel);
	
	FT4AnimSetDetailCustomization(TSharedPtr<FT4EntityViewModel> InEntityViewModel);
	~FT4AnimSetDetailCustomization();

	void CustomizeDetails(IDetailLayoutBuilder& InBuilder);

protected:
	void CustomizeCommonAnimationDetails(IDetailLayoutBuilder& InBuilder, UT4AnimSetAsset* InAnimSetAsset);

	void CustomizeBlendSpaceDetails(IDetailLayoutBuilder& InBuilder, USkeleton* InSkeleton); // #38
	void CustomizeSkillAnimationDetails(IDetailLayoutBuilder& InBuilder, USkeleton* InSkeleton);
	void CustomizeAdditiveAnimationDetails(IDetailLayoutBuilder& InBuilder, USkeleton* InSkeleton);
	void CustomizeDefaultAnimationDetails(IDetailLayoutBuilder& InBuilder, USkeleton* InSkeleton); // #38

	FReply HandleOnAnimSetSaveAll();
	FReply HandleOnUpdateThumbnailAnimSetAsset();

	bool UpdateBlendSpaceBySelectedInfo(FString& OutErrorMessage); // #95

	FReply HandleOnAddBlendSpaceInfo();
	FReply HandleOnRemoveBlendSpaceInfoByName();
	void HandleOnBlendSpaceAssetChanged(const FAssetData& InAssetData); // #95
	void HandleOnBlendSpaceNameSelected(const FName InName);
	void HandleOnBlendSpaceSectionMoveUpItem(const FName InName);
	void HandleOnBlendSpaceSectionMoveDownItem(const FName InName);

	FReply HandleOnAddSkillSection();
	FReply HandleOnRemoveSkillSectionByName();
	FReply HandleOnTestPlayOneShotSelectedSkillSectionName();
	void HandleOnSkillSectionAssetChanged(const FAssetData& InAssetData); // #95
	void HandleOnSkillSectionNameSelected(const FName InName);
	void HandleOnSkillSectionDoubleClicked(const FName InName);
	void HandleOnSkillSectionMoveUpItem(const FName InName);
	void HandleOnSkillSectionMoveDownItem(const FName InName);

	FReply HandleOnAddAdditiveSection();
	FReply HandleOnRemoveAdditiveSectionByName();
	FReply HandleOnTestPlayOneShotSelectedAdditiveSectionName();
	void HandleOnAdditiveSectionAssetChanged(const FAssetData& InAssetData); // #95
	void HandleOnAdditiveSectionNameSelected(const FName InName);
	void HandleOnAdditiveSectionDoubleClicked(const FName InName);
	void HandleOnAdditiveSectionMoveUpItem(const FName InName);
	void HandleOnAdditiveSectionMoveDownItem(const FName InName);

	FReply HandleOnAddDefaultSection();
	FReply HandleOnRemoveDefaultSectionByName();
	FReply HandleOnTestPlayOneShotSelectedDefaultSectionName();
	void HandleOnDefaultSectionAssetChanged(const FAssetData& InAssetData); // #95
	void HandleOnDefaultSectionNameSelected(const FName InName);
	void HandleOnDefaultSectionDoubleClicked(const FName InName);
	void HandleOnDefaultSectionMoveUpItem(const FName InName);
	void HandleOnDefaultSectionMoveDownItem(const FName InName);

	bool HandleOnFilterSameSkeleton(const FAssetData& InAssetData, USkeleton* InSkeleton) const;

	void HandleOnRefreshLayout();

public:
	TSharedPtr<FT4EntityViewModel> ViewModel;
	IDetailLayoutBuilder* DetailLayoutPtr;

	TSharedPtr<ST4AnimationLayerListWidget> BlendSpaceAnimationLayerListWidget;
	TSharedPtr<ST4BlendSpaceDropListWidget> BlendSpaceAnimSectionDropListWidget;

	TSharedPtr<ST4AnimationLayerListWidget> SkillAnimationLayerListWidget;
	TSharedPtr<ST4AnimSectionDropListWidget> SkillAnimSectionDropListWidget;

	TSharedPtr<ST4AnimationLayerListWidget> AdditiveAnimationLayerListWidget;
	TSharedPtr<ST4AnimSectionDropListWidget> AdditiveAnimSectionDropListWidget;

	TSharedPtr<ST4AnimationLayerListWidget> DefaultAnimationLayerListWidget;
	TSharedPtr<ST4AnimSectionDropListWidget> DefaultAnimSectionDropListWidget;

	TSharedPtr<IPropertyHandle> TransientSelectSkillSectionNameHandlePtr;
	TSharedPtr<IPropertyHandle> TransientSkillAnimSequenceAssetHandlePtr;
	TSharedPtr<IPropertyHandle> TransientSelectAdditiveSectionNameHandlePtr;
	TSharedPtr<IPropertyHandle> TransientAdditiveAnimSequenceAssetHandlePtr;
	TSharedPtr<IPropertyHandle> TransientSelectDefaultSectionNameHandlePtr; // #38
	TSharedPtr<IPropertyHandle> TransientDefaultAnimSequenceAssetHandlePtr; // #38
	TSharedPtr<IPropertyHandle> TransientSelectBlendSpaceNameHandlePtr;
	TSharedPtr<IPropertyHandle> TransientBlendSpaceAssetHandlePtr;

	FT4AnimSetAnimationLayerHelper AnimationSkillLayerHelper; // #95
	FT4AnimSetAnimationLayerHelper AnimationAdditiveLayerHelper; // #95
	FT4AnimSetAnimationLayerHelper AnimationDefaultLayerHelper; // #95
};
