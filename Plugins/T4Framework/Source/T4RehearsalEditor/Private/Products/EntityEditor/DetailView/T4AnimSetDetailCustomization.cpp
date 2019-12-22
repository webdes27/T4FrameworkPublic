// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AnimSetDetailCustomization.h"
#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/Common/Widgets/DropListView/ST4AnimSectionDropListWidget.h" // #39
#include "Products/Common/Widgets/DropListView/ST4BlendSpaceDropListWidget.h" // #38

#include "Products/Common/Widgets/ListView/ST4AnimationLayerListWidget.h" // #71

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "Products/EntityEditor/Utility/T4AssetAnimSetUtils.h" // #88

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"

#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/STextComboBox.h"

#include "ScopedTransaction.h" // #77

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "Misc/MessageDialog.h"
#include "EditorDirectories.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4AnimSetDetailCustomization"

static const FText T4AnimSetCustomDetailErrorTitleText = LOCTEXT("T4AnimSetCustomDetailsError", "AnimSet CustomDetail Error"); // #55

static const FText T4AnimSetCustomDetailErrorResultNotSetText = LOCTEXT("T4AnimSetCustomDetailsNotSetAnimSetError", "No Set AnimSetAsset"); // #55

static const FText T4AnimSetCustomDetailErrorResultNoSectionSelectedText = LOCTEXT("T4AnimSetCustomDetailsNoSectionSelectedError", "No SectionName Selected"); // #55
static const FText T4AnimSetCustomDetailErrorResultNoSetAnimSequenceText = LOCTEXT("T4AnimSetCustomDetailsNoSetAnimSequenceError", "No Set AnimSequence"); // #55

static const FText T4AnimSetCustomDetailErrorResultNoBlendSpaceSelectedText = LOCTEXT("T4AnimSetCustomDetailsNoBlendSpaceSelectedError", "No BlendSpace Name Selected"); // #55

static const FText T4AnimSetCustomDetailErrorResultFailedPlayText = LOCTEXT("T4AnimSetCustomDetailsFailedPlayError", "Failed to play animation"); // #55

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */
FT4AnimSetAnimationLayerHelper::FT4AnimSetAnimationLayerHelper(
	ET4AnimLayer InAnimationLayer,
	TSharedPtr<FT4EntityViewModel> InViewModelPtr
)
	: AnimationLayer(InAnimationLayer)
	, ViewModelPtr(InViewModelPtr)
{
}

FT4AnimSetAnimationLayerHelper::~FT4AnimSetAnimationLayerHelper()
{
}

bool FT4AnimSetAnimationLayerHelper::UpdatSection(FString& OutErrorMessage) // #95
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		OutErrorMessage = TEXT("No Set AnimSetAsset");
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	FName SelectedSectionName = GetNameSelected();
	if (SelectedSectionName == NAME_None)
	{
		OutErrorMessage = TEXT("No Section Name Selected");
		return false;
	}
	TSoftObjectPtr<UAnimSequence> SelectedAnimSequenceAsset = GetAssetSelected();
	if (SelectedAnimSequenceAsset.IsNull())
	{
		OutErrorMessage = TEXT("No Set AnimSequence");
		return false;
	}
	bool bResult = T4AssetUtil::AnimSetAddOrUpdateAnimSeqeunceInfo(
		AnimSetAsset,
		GetAnimMontageName(),
		SelectedSectionName,
		SelectedAnimSequenceAsset,
		OutErrorMessage
	); // #39
	if (!bResult)
	{
		return false;
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(GetEditorActionType());
	}
	return true;
}

bool FT4AnimSetAnimationLayerHelper::OnAdd()
{
	if (HasSection())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4AnimSetCustomDetailsAddAnimSequenceError", "AnimSection Name is Already exists"),
			&T4AnimSetCustomDetailErrorTitleText
		);
		return false;
	}
	FString ErrorMessage;
	bool bResult = UpdatSection(ErrorMessage); // #95
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4AnimSetCustomDetailErrorTitleText
		);
		return false;
	}
	return true;
}

bool FT4AnimSetAnimationLayerHelper::OnRemoveSectionByName()
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4AnimSetCustomDetailErrorResultNotSetText,
			&T4AnimSetCustomDetailErrorTitleText
		);
		return false;
	}
	FName SelectedSectionName = GetNameSelected();
	if (SelectedSectionName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4AnimSetCustomDetailErrorResultNoSectionSelectedText,
			&T4AnimSetCustomDetailErrorTitleText
		);
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	FString ErrorMessage;
	bool bResult = T4AssetUtil::AnimSetRemoveAnimSeqeunceInfo(
		AnimSetAsset,
		GetAnimMontageName(),
		SelectedSectionName,
		ErrorMessage
	); // #39
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4AnimSetCustomDetailErrorTitleText
		);
		return false;
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(GetEditorActionType());
	}
	return true;
}

bool FT4AnimSetAnimationLayerHelper::OnPlayOneShotBySelected()
{
	TSoftObjectPtr<UAnimSequence> AnimSequenceAsset = GetAssetSelected();
	if (AnimSequenceAsset.IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4AnimSetCustomDetailErrorResultNoSetAnimSequenceText,
			&T4AnimSetCustomDetailErrorTitleText
		);
		return false;
	}
	if (!ViewModelPtr->ClientPlayAnimSequence(AnimSequenceAsset.LoadSynchronous()))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4AnimSetCustomDetailErrorResultFailedPlayText,
			&T4AnimSetCustomDetailErrorTitleText
		);
		return false;
	}
	return true;
}

bool FT4AnimSetAnimationLayerHelper::OnSectionAssetChanged(const FAssetData& InAssetData) // #95
{
	FString ErrorMessage;
	bool bResult = UpdatSection(ErrorMessage); // #95
	if (!bResult)
	{
		return false;
	}
	return true;
}

bool FT4AnimSetAnimationLayerHelper::OnSectionNameSelected(const FName InName)
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset);
	T4AssetUtil::AnimSetSelectAnimSequenceInfoByName(AnimSetAsset, GetAnimMontageName(), InName);
	return true;
}

bool FT4AnimSetAnimationLayerHelper::OnSectionMoveUpItem(const FName InName)
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset);
	T4AssetUtil::AnimSetMoveUpAnimSequenceInfoByName(AnimSetAsset, GetAnimMontageName(), InName);
	return true;
}

bool FT4AnimSetAnimationLayerHelper::OnSectionMoveDownItem(const FName InName)
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset);
	T4AssetUtil::AnimSetMoveDownAnimSequenceInfoByName(AnimSetAsset, GetAnimMontageName(), InName);
	return true;
}

FName FT4AnimSetAnimationLayerHelper::GetNameSelected() const
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return NAME_None;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	const FT4AnimSetEditorTransientData& EditorTransientData = AnimSetAsset->EditorTransientData;
	if (ET4AnimLayer::AnimLayer_Skill == AnimationLayer)
	{
		return EditorTransientData.TransientSelectSkillSectionName;
	}
	else if (ET4AnimLayer::AnimLayer_Additive == AnimationLayer)
	{
		return EditorTransientData.TransientSelectAdditiveSectionName;
	}
	else if (ET4AnimLayer::AnimLayer_Default == AnimationLayer)
	{
		return EditorTransientData.TransientSelectDefaultSectionName;
	}
	return NAME_None;
}

TSoftObjectPtr<UAnimSequence> FT4AnimSetAnimationLayerHelper::GetAssetSelected() const
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return nullptr;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	const FT4AnimSetEditorTransientData& EditorTransientData = AnimSetAsset->EditorTransientData;
	if (ET4AnimLayer::AnimLayer_Skill == AnimationLayer)
	{
		return EditorTransientData.TransientSkillAnimSequenceAsset;
	}
	else if (ET4AnimLayer::AnimLayer_Additive == AnimationLayer)
	{
		return EditorTransientData.TransientAdditiveAnimSequenceAsset;
	}
	else if (ET4AnimLayer::AnimLayer_Default == AnimationLayer)
	{
		return EditorTransientData.TransientDefaultAnimSequenceAsset;
	}
	return nullptr;
}

bool FT4AnimSetAnimationLayerHelper::HasSection() const
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	const FT4AnimSetEditorTransientData& EditorTransientData = AnimSetAsset->EditorTransientData;
	if (ET4AnimLayer::AnimLayer_Skill == AnimationLayer)
	{
		FName SectionName = EditorTransientData.TransientSelectSkillSectionName;
		FT4AnimSequenceInfo* SelectedInfo = AnimSetAsset->SkillAnimSequenceArray.FindByKey(SectionName);
		return (nullptr != SelectedInfo) ? true : false;
	}
	else if (ET4AnimLayer::AnimLayer_Additive == AnimationLayer)
	{
		FName SectionName = EditorTransientData.TransientSelectAdditiveSectionName;
		FT4AnimSequenceInfo* SelectedInfo = AnimSetAsset->AdditiveAnimSequenceArray.FindByKey(SectionName);
		return (nullptr != SelectedInfo) ? true : false;
	}
	else if (ET4AnimLayer::AnimLayer_Default == AnimationLayer)
	{
		FName SectionName = EditorTransientData.TransientSelectDefaultSectionName;
		FT4AnimSequenceInfo* SelectedInfo = AnimSetAsset->DefaultAnimSequenceArray.FindByKey(SectionName);
		return (nullptr != SelectedInfo) ? true : false;
	}
	return false;
}

FName FT4AnimSetAnimationLayerHelper::GetAnimMontageName() const
{
	if (ET4AnimLayer::AnimLayer_Skill == AnimationLayer)
	{
		return T4AnimSetAnimMontageSkillName;
	}
	else if (ET4AnimLayer::AnimLayer_Additive == AnimationLayer)
	{
		return T4AnimSetAnimMontageAdditiveName;
	}
	else if (ET4AnimLayer::AnimLayer_Default == AnimationLayer)
	{
		return T4AnimSetAnimMontageDefaultName;
	}
	return NAME_None;
}

ET4EditorAction FT4AnimSetAnimationLayerHelper::GetEditorActionType() const
{
	if (ET4AnimLayer::AnimLayer_Skill == AnimationLayer)
	{
		return ET4EditorAction::ReloadAnimSetSkill;
	}
	else if (ET4AnimLayer::AnimLayer_Additive == AnimationLayer)
	{
		return ET4EditorAction::ReloadAnimSetAdditive;
	}
	else if (ET4AnimLayer::AnimLayer_Default == AnimationLayer)
	{
		return ET4EditorAction::ReloadAnimSetDefault;
	}
	return ET4EditorAction::None;
}

TSharedRef<IDetailCustomization> FT4AnimSetDetailCustomization::MakeInstance(
	TSharedPtr<FT4EntityViewModel> InEntityViewModel
)
{
	return MakeShared<FT4AnimSetDetailCustomization>(InEntityViewModel);
}

FT4AnimSetDetailCustomization::FT4AnimSetDetailCustomization(
	TSharedPtr<FT4EntityViewModel> InEntityViewModel
)	: ViewModel(InEntityViewModel)
	, DetailLayoutPtr(nullptr)
	, AnimationSkillLayerHelper(ET4AnimLayer::AnimLayer_Skill, InEntityViewModel) // #95
	, AnimationAdditiveLayerHelper(ET4AnimLayer::AnimLayer_Additive, InEntityViewModel) // #95
	, AnimationDefaultLayerHelper(ET4AnimLayer::AnimLayer_Default, InEntityViewModel) // #95
{
}

FT4AnimSetDetailCustomization::~FT4AnimSetDetailCustomization()
{
}

void FT4AnimSetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	DetailLayoutPtr = &InBuilder;

	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return;
	}

	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)

	USkeleton* Skeleton = nullptr;
	if (!AnimSetAsset->SkeletonAsset.IsNull())
	{
		Skeleton = AnimSetAsset->SkeletonAsset.LoadSynchronous(); // #39
	}

	{
		// WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, EditorTransientData);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientSelectSkillSectionName);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientSkillAnimSequenceAsset);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientSelectAdditiveSectionName);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientAdditiveAnimSequenceAsset);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientSelectDefaultSectionName);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientDefaultAnimSequenceAsset);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientSelectBlendSpaceName);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientBlendSpaceAsset);
	}

	CustomizeCommonAnimationDetails(InBuilder, AnimSetAsset);

	CustomizeBlendSpaceDetails(InBuilder, Skeleton); // #38
	CustomizeSkillAnimationDetails(InBuilder, Skeleton);
	CustomizeAdditiveAnimationDetails(InBuilder, Skeleton);
	CustomizeDefaultAnimationDetails(InBuilder, Skeleton); // #38

	{
		static const TCHAR* AnimSetEditorAttributeCategoryName = TEXT("Editor");
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, PreviewEntityAsset);
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(AnimSetEditorAttributeCategoryName);
		DetailCategoryBuilder.AddProperty(PreviewEntityAssetHandle);
	}
}

void FT4AnimSetDetailCustomization::CustomizeCommonAnimationDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4AnimSetAsset* InAnimSetAsset
)
{
	check(nullptr != InAnimSetAsset);

	static const TCHAR* AssetInfoCategoryName = TEXT("Common");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(AssetInfoCategoryName);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4AnimSetEntryBox", "T4AnimSetAsset"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4AnimSetTitle", "AnimSetAsset"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			SNew(SObjectPropertyEntryBox)
			.ObjectPath(FSoftObjectPath(InAnimSetAsset).ToString())
			.AllowedClass(UT4AnimSetAsset::StaticClass())
			.AllowClear(false)
			.ThumbnailPool(InBuilder.GetThumbnailPool())
		];

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4AnimSetSaveBtn", "UpdateAndSave"))
		.NameContent()
		[
			SNew(STextBlock)
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4AnimSetBuildAndSaveBtn", "Save to AnimSet"))
				.ToolTipText(LOCTEXT("T4AnimSetBuildAndSaveBtn_Tooltip", "Save to AnimSet"))
				.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnAnimSetSaveAll)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4AnimSetUpdateThumbnailBtn", "Update Thumbnail"))
				.ToolTipText(LOCTEXT("T4AnimSetUpdateThumbnailBtn_Tooltip", "Update Thumbnail"))
				.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnUpdateThumbnailAnimSetAsset)
			]
		];

	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, SkeletonAsset);
		DetailCategoryBuilder.AddProperty(SkeletonAssetHandle);

		SkeletonAssetHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4AnimSetDetailCustomization::HandleOnRefreshLayout)
		);
	}
}

void FT4AnimSetDetailCustomization::CustomizeBlendSpaceDetails(
	IDetailLayoutBuilder& InBuilder,
	USkeleton* InSkeleton
)
{
	// 순서 정렬
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, BlendSpaceAttributes);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, BlendSpaceArray);

	static const TCHAR* T4AnimSetBlendSpaceAnimationLayerCategoryName = TEXT("BlendSpace Layer");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4AnimSetBlendSpaceAnimationLayerCategoryName);

	// #38
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();

	BlendSpaceAnimationLayerListWidget = SNew(ST4AnimationLayerListWidget, AnimSetAssetSelector, ET4AnimLayer::AnimLayer_BlendSpace)
		.OnSelected(this, &FT4AnimSetDetailCustomization::HandleOnBlendSpaceNameSelected)
		.OnDoubleClicked(nullptr)
		.OnMoveUpItem(this, &FT4AnimSetDetailCustomization::HandleOnBlendSpaceSectionMoveUpItem)
		.OnMoveDownItem(this, &FT4AnimSetDetailCustomization::HandleOnBlendSpaceSectionMoveDownItem);
	
	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4AnimSetBlendSpaceList", "BlendSpaces"))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			BlendSpaceAnimationLayerListWidget.ToSharedRef()
		];

	{
		BlendSpaceAnimSectionDropListWidget = SNew(ST4BlendSpaceDropListWidget, AnimSetAssetSelector)
			.OnSelected(nullptr) // #95 : Name 변경은 지원하지 않는다.
			.PropertyHandle(TransientSelectBlendSpaceNameHandlePtr);
		BlendSpaceAnimSectionDropListWidget->OnRefresh();

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4AnimSetBlendSpaceNameSelector", "BlendSpace Name"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetBlendSpaceNameSelector_Title", "BlendSpace Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(200.0f)
			[
				BlendSpaceAnimSectionDropListWidget.ToSharedRef()
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4AnimSetBlendSpaceAssetSelector", "BlendSpace Asset"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetBlendSpaceAssetSelectorTitle", "BlendSpace Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientBlendSpaceAssetHandlePtr)
				.AllowedClass(UBlendSpaceBase::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				.OnObjectChanged(this, &FT4AnimSetDetailCustomization::HandleOnBlendSpaceAssetChanged) // #95
				.OnShouldFilterAsset(this, &FT4AnimSetDetailCustomization::HandleOnFilterSameSkeleton, InSkeleton)
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4AnimSetBlendSpacetLayerButtons", "BlendSpaceLayerButtons"))
			.NameContent()
			[
				SNew(STextBlock)
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetBlendSpaceNameSelectorBtn", "Add"))
					.ToolTipText(LOCTEXT("T4AnimSetBlendSpaceNameSelectorBtn_Tooltip", "Add to BlendSpaceInfo"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnAddBlendSpaceInfo)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetBlendSpaceNameSelectorRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4AnimSetBlendSpaceNameSelectorRemoveBtn_Tooltip", "Remove from BlendSpace Info"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnRemoveBlendSpaceInfoByName)
				]
			];

		DetailCategoryBuilder.AddProperty(BlendSpaceAttributesHandle);
	}

	BlendSpaceAnimationLayerListWidget->OnRefresh(true);
}

void FT4AnimSetDetailCustomization::CustomizeSkillAnimationDetails(
	IDetailLayoutBuilder& InBuilder,
	USkeleton* InSkeleton
)
{
	// 순서 정렬
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, bSkillAnimMontageAutoGen); // #69
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, SkillAnimMontageAsset);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, SkillAnimSequenceArray);

	const ET4EngineConstantType SkillSectionNameTableType = ET4EngineConstantType::SkillSection;
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();

	static const TCHAR* T4AnimSetSkillAnimationLayerCategoryName = TEXT("Skill Animation Layer");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4AnimSetSkillAnimationLayerCategoryName);

	DetailCategoryBuilder.AddProperty(bSkillAnimMontageAutoGenHandle); // #69
	DetailCategoryBuilder.AddProperty(SkillAnimMontageAssetHandle);

	IDetailGroup& AnimationLayerDetailGroup = DetailCategoryBuilder.AddGroup(FName("Skill Layer Details"), FText::FromString("Skill Layer Details"), false);

	SkillAnimationLayerListWidget = SNew(ST4AnimationLayerListWidget, AnimSetAssetSelector, ET4AnimLayer::AnimLayer_Skill)
		.OnSelected(this, &FT4AnimSetDetailCustomization::HandleOnSkillSectionNameSelected)
		.OnDoubleClicked(this, &FT4AnimSetDetailCustomization::HandleOnSkillSectionDoubleClicked)
		.OnMoveUpItem(this, &FT4AnimSetDetailCustomization::HandleOnSkillSectionMoveUpItem)
		.OnMoveDownItem(this, &FT4AnimSetDetailCustomization::HandleOnSkillSectionMoveDownItem);

	AnimationLayerDetailGroup
		.AddWidgetRow()
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			SkillAnimationLayerListWidget.ToSharedRef()
		];

	{
		SkillAnimSectionDropListWidget = SNew(ST4AnimSectionDropListWidget, AnimSetAssetSelector, SkillSectionNameTableType)
			.OnSelected(nullptr) // #95 : Name 변경은 지원하지 않는다.
			.PropertyHandle(TransientSelectSkillSectionNameHandlePtr);
		SkillAnimSectionDropListWidget->OnRefresh();

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetSkillAnimSectionNameSelector", "Section Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SkillAnimSectionDropListWidget.ToSharedRef()
			];

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetSkillAnimSequenceSelector", "AnimSequence Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientSkillAnimSequenceAssetHandlePtr)
				.AllowedClass(UAnimSequence::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				.OnObjectChanged(this, &FT4AnimSetDetailCustomization::HandleOnSkillSectionAssetChanged) // #95
				.OnShouldFilterAsset(this, &FT4AnimSetDetailCustomization::HandleOnFilterSameSkeleton, InSkeleton)
			];

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetSkillSectionSelectorBtn", "Add"))
					.ToolTipText(LOCTEXT("T4AnimSetSkillSectionSelectorBtn_Tooltip", "Add AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnAddSkillSection)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetSkillSectionSelectorRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4AnimSetSkillSectionSelectorRemoveBtn_Tooltip", "Remove Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnRemoveSkillSectionByName)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetSkillSectionSelectorTestPlayOneShotBtn", "Test Play OneShot"))
					.ToolTipText(LOCTEXT("T4AnimSetSkillSectionSelectorTestPlayOneShotBtn_Tooltip", "Play Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnTestPlayOneShotSelectedSkillSectionName)
				]
			];
	}

	SkillAnimationLayerListWidget->OnRefresh(true);
}

void FT4AnimSetDetailCustomization::CustomizeAdditiveAnimationDetails(
	IDetailLayoutBuilder& InBuilder,
	USkeleton* InSkeleton
)
{
	// 순서 정렬
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, bAdditiveAnimMontageAutoGen); // #69
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, AdditiveAnimMontageAsset);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, AdditiveAnimSequenceArray);

	static const TCHAR* T4AnimSetAdditiveAnimationLayerCategoryName = TEXT("Additive Animation Layer");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4AnimSetAdditiveAnimationLayerCategoryName);

	const ET4EngineConstantType AdditiveSectionNameTableType = ET4EngineConstantType::AdditiveSection;
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();

	DetailCategoryBuilder.AddProperty(bAdditiveAnimMontageAutoGenHandle); // #69
	DetailCategoryBuilder.AddProperty(AdditiveAnimMontageAssetHandle);

	IDetailGroup& AnimationLayerDetailGroup = DetailCategoryBuilder.AddGroup(FName("Additive Layer Details"), FText::FromString("Additive Layer Details"), false);
	
	AdditiveAnimationLayerListWidget = SNew(ST4AnimationLayerListWidget, AnimSetAssetSelector, ET4AnimLayer::AnimLayer_Additive)
		.OnSelected(this, &FT4AnimSetDetailCustomization::HandleOnAdditiveSectionNameSelected)
		.OnDoubleClicked(this, &FT4AnimSetDetailCustomization::HandleOnAdditiveSectionDoubleClicked)
		.OnMoveUpItem(this, &FT4AnimSetDetailCustomization::HandleOnAdditiveSectionMoveUpItem)
		.OnMoveDownItem(this, &FT4AnimSetDetailCustomization::HandleOnAdditiveSectionMoveDownItem);

	AnimationLayerDetailGroup
		.AddWidgetRow()
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			AdditiveAnimationLayerListWidget.ToSharedRef()
		];

	{
		AdditiveAnimSectionDropListWidget = SNew(ST4AnimSectionDropListWidget, AnimSetAssetSelector, AdditiveSectionNameTableType)
			.OnSelected(nullptr) // #95 : Name 변경은 지원하지 않는다.
			.PropertyHandle(TransientSelectAdditiveSectionNameHandlePtr);
		AdditiveAnimSectionDropListWidget->OnRefresh();

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetAdditiveAnimSectionNameSelector", "Section Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				AdditiveAnimSectionDropListWidget.ToSharedRef()
			];

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetAdditiveAnimSequenceSelector", "AnimSequence Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientAdditiveAnimSequenceAssetHandlePtr)
				.AllowedClass(UAnimSequence::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				.OnObjectChanged(this, &FT4AnimSetDetailCustomization::HandleOnAdditiveSectionAssetChanged) // #95
				.OnShouldFilterAsset(this, &FT4AnimSetDetailCustomization::HandleOnFilterSameSkeleton, InSkeleton)
			];

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetDefaultSectionSelectorBtn", "Add"))
					.ToolTipText(LOCTEXT("T4AnimSetDefaultSectionSelectorBtn_Tooltip", "Add Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnAddAdditiveSection)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetDefaultSectionSelectorRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4AnimSetDefaultSectionSelectorRemoveBtn_Tooltip", "Remove Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnRemoveAdditiveSectionByName)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetDefaultSectionSelectorTestPlayOneShotBtn", "Test Play OneShot"))
					.ToolTipText(LOCTEXT("T4AnimSetDefaultSectionSelectorTestPlayOneShotBtn_Tooltip", "Play Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnTestPlayOneShotSelectedAdditiveSectionName)
				]
			];

	}

	AdditiveAnimationLayerListWidget->OnRefresh(true);
}

void FT4AnimSetDetailCustomization::CustomizeDefaultAnimationDetails(
	IDetailLayoutBuilder& InBuilder,
	USkeleton* InSkeleton
)
{
	// 순서 정렬
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, bDefaultAnimMontageAutoGen); // #69
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, DefaultAnimMontageAsset);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, DefaultAttributes);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4AnimSetAsset, DefaultAnimSequenceArray);

	static const TCHAR* T4AnimSetDefaultAnimationLayerCategoryName = TEXT("Default Animation Layer");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4AnimSetDefaultAnimationLayerCategoryName);

	const ET4EngineConstantType DefaultSectionNameTableType = ET4EngineConstantType::DefaultSection;
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();

	DetailCategoryBuilder.AddProperty(bDefaultAnimMontageAutoGenHandle); // #69
	DetailCategoryBuilder.AddProperty(DefaultAnimMontageAssetHandle);

	IDetailGroup& AnimationLayerDetailGroup = DetailCategoryBuilder.AddGroup(FName("Default Layer Details"), FText::FromString("Default Layer Details"), false);

	DefaultAnimationLayerListWidget = SNew(ST4AnimationLayerListWidget, AnimSetAssetSelector, ET4AnimLayer::AnimLayer_Default)
		.OnSelected(this, &FT4AnimSetDetailCustomization::HandleOnDefaultSectionNameSelected)
		.OnDoubleClicked(this, &FT4AnimSetDetailCustomization::HandleOnDefaultSectionDoubleClicked)
		.OnMoveUpItem(this, &FT4AnimSetDetailCustomization::HandleOnDefaultSectionMoveUpItem)
		.OnMoveDownItem(this, &FT4AnimSetDetailCustomization::HandleOnDefaultSectionMoveDownItem);

	AnimationLayerDetailGroup
		.AddWidgetRow()
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			DefaultAnimationLayerListWidget.ToSharedRef()
		];

	{
		DefaultAnimSectionDropListWidget = SNew(ST4AnimSectionDropListWidget, AnimSetAssetSelector, DefaultSectionNameTableType)
			.OnSelected(nullptr) // #95 : Name 변경은 지원하지 않는다.
			.PropertyHandle(TransientSelectDefaultSectionNameHandlePtr);
		DefaultAnimSectionDropListWidget->OnRefresh();

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetDefaultAnimSectionNameSelector", "Section Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				DefaultAnimSectionDropListWidget.ToSharedRef()
			];

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4AnimSetDefaultAnimSequenceSelector", "AnimSequence Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientDefaultAnimSequenceAssetHandlePtr)
				.AllowedClass(UAnimSequence::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				.OnObjectChanged(this, &FT4AnimSetDetailCustomization::HandleOnDefaultSectionAssetChanged) // #95
				.OnShouldFilterAsset(this, &FT4AnimSetDetailCustomization::HandleOnFilterSameSkeleton, InSkeleton)
			];

		AnimationLayerDetailGroup
			.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetDefaultSectionSelectorBtn", "Add"))
					.ToolTipText(LOCTEXT("T4AnimSetDefaultSectionSelectorBtn_Tooltip", "Add Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnAddDefaultSection)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetDefaultSectionSelectorRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4AnimSetDefaultSectionSelectorRemoveBtn_Tooltip", "Remove Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnRemoveDefaultSectionByName)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4AnimSetDefaultSectionSelectorTestPlayOneShotBtn", "Test Play OneShot"))
					.ToolTipText(LOCTEXT("T4AnimSetDefaultSectionSelectorTestPlayOneShotBtn_Tooltip", "Play Selected AnimSequence"))
					.OnClicked(this, &FT4AnimSetDetailCustomization::HandleOnTestPlayOneShotSelectedDefaultSectionName)
				]
			];

		DetailCategoryBuilder.AddProperty(DefaultAttributesHandle);
	}

	DefaultAnimationLayerListWidget->OnRefresh(true);
}

FReply FT4AnimSetDetailCustomization::HandleOnAnimSetSaveAll()
{
	FString ErrorMessage;
	if (!ViewModel->DoAnimSetUpdateAll(ErrorMessage))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4AnimSetCustomDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (!ViewModel->DoAnimSetSaveAll(ErrorMessage))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4AnimSetCustomDetailErrorTitleText
		);
	}
	if (ViewModel.IsValid())
	{
		ViewModel->ClientEditorAction(ET4EditorAction::ReloadObject);
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnUpdateThumbnailAnimSetAsset()
{
	FString ErrorMessage;
	if (!ViewModel->DoUpdateThumbnailAnimSetAsset(ErrorMessage))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4AnimSetCustomDetailErrorTitleText
		);
	}
	return FReply::Handled();
}

bool FT4AnimSetDetailCustomization::UpdateBlendSpaceBySelectedInfo(FString& OutErrorMessage)
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		OutErrorMessage = TEXT("No Set AnimSetAsset");
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	const FT4AnimSetEditorTransientData& EditorTransientData = AnimSetAsset->EditorTransientData;
	FName SelectedBlendSpaceName = EditorTransientData.TransientSelectBlendSpaceName;
	if (SelectedBlendSpaceName == NAME_None)
	{
		OutErrorMessage = TEXT("No BlendSpace Name Selected");
		return false;
	}
	if (EditorTransientData.TransientBlendSpaceAsset.IsNull())
	{
		OutErrorMessage = TEXT("No Set BlendSpace");
		return false;
	}
	bool bResult = T4AssetUtil::AnimSetAddOrUpdateBlendSpaceInfo(
		AnimSetAsset,
		SelectedBlendSpaceName,
		EditorTransientData.TransientBlendSpaceAsset,
		OutErrorMessage
	);
	if (!bResult)
	{
		return false;
	}
	if (ViewModel.IsValid())
	{
		ViewModel->ClientEditorAction(ET4EditorAction::ReloadAnimSetBlendSpace);
	}
	return true;
}

FReply FT4AnimSetDetailCustomization::HandleOnAddBlendSpaceInfo()
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4AnimSetCustomDetailErrorResultNotSetText,
			&T4AnimSetCustomDetailErrorTitleText
		);
		return FReply::Handled();
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	const FT4AnimSetEditorTransientData& EditorTransientData = AnimSetAsset->EditorTransientData;
	FName SelectedBlendSpaceName = EditorTransientData.TransientSelectBlendSpaceName;
	FT4BlendSpaceInfo* SelectedInfo = AnimSetAsset->BlendSpaceArray.FindByKey(SelectedBlendSpaceName);
	if (nullptr != SelectedInfo)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4AnimSetCustomDetailsAddBlendSpaceError", "BlendSpace Name is Already exists"),
			&T4AnimSetCustomDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnAddBlendSpaceInfo_Transaction", "Add to BlendSpace"));
	FString ErrorMessage;
	bool bResult = UpdateBlendSpaceBySelectedInfo(ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4AnimSetCustomDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (BlendSpaceAnimationLayerListWidget.IsValid())
	{
		BlendSpaceAnimationLayerListWidget->OnRefresh(false);
	}
	if (BlendSpaceAnimSectionDropListWidget.IsValid())
	{
		BlendSpaceAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnRemoveBlendSpaceInfoByName()
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4AnimSetCustomDetailErrorResultNotSetText,
			&T4AnimSetCustomDetailErrorTitleText
		);
		return FReply::Handled();
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset)
	const FT4AnimSetEditorTransientData& EditorTransientData = AnimSetAsset->EditorTransientData;
	FName SelectedBlendSpaceName = EditorTransientData.TransientSelectBlendSpaceName;
	if (SelectedBlendSpaceName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4AnimSetCustomDetailErrorResultNoBlendSpaceSelectedText,
			&T4AnimSetCustomDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnRemoveBlendSpaceInfoByName_Transaction", "Remove a BlendSpace"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::AnimSetRemoveBlendSpaceInfoByName(
		AnimSetAsset,
		SelectedBlendSpaceName,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4AnimSetCustomDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModel.IsValid())
	{
		ViewModel->ClientEditorAction(ET4EditorAction::ReloadAnimSetBlendSpace);
	}
	if (BlendSpaceAnimationLayerListWidget.IsValid())
	{
		BlendSpaceAnimationLayerListWidget->OnRefresh(false);
	}
	if (BlendSpaceAnimSectionDropListWidget.IsValid())
	{
		BlendSpaceAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

void FT4AnimSetDetailCustomization::HandleOnBlendSpaceAssetChanged(const FAssetData& InAssetData) // #95
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnBlendSpaceAssetChanged_Transaction", "Change a BlendSpace Name"));
	FString ErrorMessage;
	bool bResult = UpdateBlendSpaceBySelectedInfo(ErrorMessage);
	if (!bResult)
	{
		return;
	}
	if (BlendSpaceAnimationLayerListWidget.IsValid())
	{
		BlendSpaceAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnBlendSpaceNameSelected(const FName InName)
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset);
	const FScopedTransaction Transaction(LOCTEXT("HandleOnBlendSpaceNameSelected_Transaction", "Select a BlendSpace"));
	T4AssetUtil::AnimSetSelectBlendSpaceInfoByName(AnimSetAsset, InName);
	if (BlendSpaceAnimSectionDropListWidget.IsValid())
	{
		BlendSpaceAnimSectionDropListWidget->OnRefresh();
	}
}

void FT4AnimSetDetailCustomization::HandleOnBlendSpaceSectionMoveUpItem(const FName InName)
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset);
	const FScopedTransaction Transaction(LOCTEXT("HandleOnBlendSpaceSectionMoveUpItem_Transaction", "Move Up a BlendSpace"));
	T4AssetUtil::AnimSetMoveUpBlendSpaceInfoByName(AnimSetAsset, InName);
	if (BlendSpaceAnimationLayerListWidget.IsValid())
	{
		BlendSpaceAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnBlendSpaceSectionMoveDownItem(const FName InName)
{
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModel->GetAnimSetAssetSelector();
	if (AnimSetAssetSelector->IsNull())
	{
		return;
	}
	UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
	check(nullptr != AnimSetAsset);
	const FScopedTransaction Transaction(LOCTEXT("HandleOnBlendSpaceSectionMoveDownItem_Transaction", "Move Down a BlendSpace"));
	T4AssetUtil::AnimSetMoveDownBlendSpaceInfoByName(AnimSetAsset, InName);
	if (BlendSpaceAnimationLayerListWidget.IsValid())
	{
		BlendSpaceAnimationLayerListWidget->OnRefresh(false);
	}
}

FReply FT4AnimSetDetailCustomization::HandleOnAddSkillSection()
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnAddSkillSection_Transaction", "Add to AnimSequnce"));
	FString ErrorMessage;
	bool bResult = AnimationSkillLayerHelper.OnAdd(); // #95
	if (!bResult)
	{
		return FReply::Handled();
	}
	if (SkillAnimationLayerListWidget.IsValid())
	{
		SkillAnimationLayerListWidget->OnRefresh(false);
	}
	if (SkillAnimSectionDropListWidget.IsValid())
	{
		SkillAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnRemoveSkillSectionByName()
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnRemoveSkillSectionByName_Transaction", "Remove a AnimSequnce"));
	bool bResult = AnimationSkillLayerHelper.OnRemoveSectionByName(); // #95
	if (!bResult)
	{
		return FReply::Handled();
	}
	if (SkillAnimationLayerListWidget.IsValid())
	{
		SkillAnimationLayerListWidget->OnRefresh(false);
	}
	if (SkillAnimSectionDropListWidget.IsValid())
	{
		SkillAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnTestPlayOneShotSelectedSkillSectionName()
{
	bool bResult = AnimationSkillLayerHelper.OnPlayOneShotBySelected(); // #95
	return FReply::Handled();
}

void FT4AnimSetDetailCustomization::HandleOnSkillSectionAssetChanged(const FAssetData& InAssetData) // #95
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnSkillSectionAssetChanged_Transaction", "Change a AnimSection Asset"));
	bool bResult = AnimationSkillLayerHelper.OnSectionAssetChanged(InAssetData); // #95
	if (!bResult)
	{
		return;
	}
	if (SkillAnimationLayerListWidget.IsValid())
	{
		SkillAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnSkillSectionNameSelected(const FName InName)
{
	bool bResult = AnimationSkillLayerHelper.OnSectionNameSelected(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (SkillAnimSectionDropListWidget.IsValid())
	{
		SkillAnimSectionDropListWidget->OnRefresh();
	}
}

void FT4AnimSetDetailCustomization::HandleOnSkillSectionDoubleClicked(const FName InName)
{
	AnimationSkillLayerHelper.OnPlayOneShotBySelected(); // #95
}

void FT4AnimSetDetailCustomization::HandleOnSkillSectionMoveUpItem(const FName InName)
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnSkillSectionMoveUpItem_Transaction", "Move Up a AnimSequence"));
	bool bResult = AnimationSkillLayerHelper.OnSectionMoveUpItem(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (SkillAnimationLayerListWidget.IsValid())
	{
		SkillAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnSkillSectionMoveDownItem(const FName InName)
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnSkillSectionMoveDownItem_Transaction", "Move Up a AnimSequence"));
	bool bResult = AnimationSkillLayerHelper.OnSectionMoveDownItem(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (SkillAnimationLayerListWidget.IsValid())
	{
		SkillAnimationLayerListWidget->OnRefresh(false);
	}
}

FReply FT4AnimSetDetailCustomization::HandleOnAddAdditiveSection()
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnAddAdditiveSection_Transaction", "Add to AnimSequnce"));
	FString ErrorMessage;
	bool bResult = AnimationAdditiveLayerHelper.OnAdd(); // #95
	if (!bResult)
	{
		return FReply::Handled();
	}
	if (AdditiveAnimationLayerListWidget.IsValid())
	{
		AdditiveAnimationLayerListWidget->OnRefresh(false);
	}
	if (AdditiveAnimSectionDropListWidget.IsValid())
	{
		AdditiveAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnRemoveAdditiveSectionByName()
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnRemoveAdditiveSectionByName_Transaction", "Remove a AnimSequnce"));
	bool bResult = AnimationAdditiveLayerHelper.OnRemoveSectionByName(); // #95
	if (!bResult)
	{
		return FReply::Handled();
	}
	if (AdditiveAnimationLayerListWidget.IsValid())
	{
		AdditiveAnimationLayerListWidget->OnRefresh(false);
	}
	if (AdditiveAnimSectionDropListWidget.IsValid())
	{
		AdditiveAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnTestPlayOneShotSelectedAdditiveSectionName()
{
	bool bResult = AnimationAdditiveLayerHelper.OnPlayOneShotBySelected(); // #95
	return FReply::Handled();
}

void FT4AnimSetDetailCustomization::HandleOnAdditiveSectionAssetChanged(const FAssetData& InAssetData) // #95
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnAdditiveSectionAssetChanged_Transaction", "Change a AnimSection Asset"));
	bool bResult = AnimationAdditiveLayerHelper.OnSectionAssetChanged(InAssetData); // #95
	if (!bResult)
	{
		return;
	}
	if (AdditiveAnimationLayerListWidget.IsValid())
	{
		AdditiveAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnAdditiveSectionNameSelected(const FName InName)
{
	bool bResult = AnimationAdditiveLayerHelper.OnSectionNameSelected(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (AdditiveAnimSectionDropListWidget.IsValid())
	{
		AdditiveAnimSectionDropListWidget->OnRefresh();
	}
}

void FT4AnimSetDetailCustomization::HandleOnAdditiveSectionDoubleClicked(const FName InName)
{
	AnimationAdditiveLayerHelper.OnPlayOneShotBySelected(); // #95
}

void FT4AnimSetDetailCustomization::HandleOnAdditiveSectionMoveUpItem(const FName InName)
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnAdditiveSectionMoveUpItem_Transaction", "Move Up a AnimSequence"));
	bool bResult = AnimationAdditiveLayerHelper.OnSectionMoveUpItem(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (AdditiveAnimationLayerListWidget.IsValid())
	{
		AdditiveAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnAdditiveSectionMoveDownItem(const FName InName)
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnAdditiveSectionMoveDownItem_Transaction", "Move Up a AnimSequence"));
	bool bResult = AnimationAdditiveLayerHelper.OnSectionMoveDownItem(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (AdditiveAnimationLayerListWidget.IsValid())
	{
		AdditiveAnimationLayerListWidget->OnRefresh(false);
	}
}

FReply FT4AnimSetDetailCustomization::HandleOnAddDefaultSection()
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnAddDefaultSection_Transaction", "Add to AnimSequnce"));
	FString ErrorMessage;
	bool bResult = AnimationDefaultLayerHelper.OnAdd(); // #95
	if (!bResult)
	{
		return FReply::Handled();
	}
	if (DefaultAnimationLayerListWidget.IsValid())
	{
		DefaultAnimationLayerListWidget->OnRefresh(false);
	}
	if (DefaultAnimSectionDropListWidget.IsValid())
	{
		DefaultAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnRemoveDefaultSectionByName()
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnRemoveDefaultSectionByName_Transaction", "Remove a AnimSequnce"));
	bool bResult = AnimationDefaultLayerHelper.OnRemoveSectionByName(); // #95
	if (!bResult)
	{
		return FReply::Handled();
	}
	if (DefaultAnimationLayerListWidget.IsValid())
	{
		DefaultAnimationLayerListWidget->OnRefresh(false);
	}
	if (DefaultAnimSectionDropListWidget.IsValid())
	{
		DefaultAnimSectionDropListWidget->OnRefresh();
	}
	return FReply::Handled();
}

FReply FT4AnimSetDetailCustomization::HandleOnTestPlayOneShotSelectedDefaultSectionName()
{
	bool bResult = AnimationDefaultLayerHelper.OnPlayOneShotBySelected(); // #95
	return FReply::Handled();
}

void FT4AnimSetDetailCustomization::HandleOnDefaultSectionAssetChanged(const FAssetData& InAssetData) // #95
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnDefaultSectionAssetChanged_Transaction", "Change a AnimSection Asset"));
	bool bResult = AnimationDefaultLayerHelper.OnSectionAssetChanged(InAssetData); // #95
	if (!bResult)
	{
		return;
	}
	if (DefaultAnimationLayerListWidget.IsValid())
	{
		DefaultAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnDefaultSectionNameSelected(const FName InName)
{
	bool bResult = AnimationDefaultLayerHelper.OnSectionNameSelected(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (DefaultAnimSectionDropListWidget.IsValid())
	{
		DefaultAnimSectionDropListWidget->OnRefresh();
	}
}

void FT4AnimSetDetailCustomization::HandleOnDefaultSectionDoubleClicked(const FName InName)
{
	AnimationDefaultLayerHelper.OnPlayOneShotBySelected(); // #95
}

void FT4AnimSetDetailCustomization::HandleOnDefaultSectionMoveUpItem(const FName InName)
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnDefaultSectionMoveUpItem_Transaction", "Move Up a AnimSequence"));
	bool bResult = AnimationDefaultLayerHelper.OnSectionMoveUpItem(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (DefaultAnimationLayerListWidget.IsValid())
	{
		DefaultAnimationLayerListWidget->OnRefresh(false);
	}
}

void FT4AnimSetDetailCustomization::HandleOnDefaultSectionMoveDownItem(const FName InName)
{
	const FScopedTransaction Transaction(LOCTEXT("HandleOnDefaultSectionMoveDownItem_Transaction", "Move Up a AnimSequence"));
	bool bResult = AnimationDefaultLayerHelper.OnSectionMoveDownItem(InName); // #95
	if (!bResult)
	{
		return;
	}
	if (DefaultAnimationLayerListWidget.IsValid())
	{
		DefaultAnimationLayerListWidget->OnRefresh(false);
	}
}

bool FT4AnimSetDetailCustomization::HandleOnFilterSameSkeleton(
	const FAssetData& InAssetData, 
	USkeleton* InSkeleton
) const
{
	FString SkeletonName;
	InAssetData.GetTagValue("Skeleton", SkeletonName);
	FAssetData SkeletonData(InSkeleton);
	return (SkeletonName != SkeletonData.GetExportTextName());
}

void FT4AnimSetDetailCustomization::HandleOnRefreshLayout()
{
	if (nullptr != DetailLayoutPtr)
	{
		DetailLayoutPtr->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
