// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiDetailCustomization.h"
#include "T4CameraWorkSectionKeyDetailCustomization.h" // #58

#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/Common/Widgets/DropListView/ST4AnimSectionDropListWidget.h" // #39
#include "Products/Common/Widgets/DropListView/ST4ActionPointDropListWidget.h" // #57
#include "Products/Common/Widgets/DropListView/ST4StanceDropListWidget.h" // #73
#include "Products/Common/Widgets/DropListView/ST4ReactionDropListWidget.h" // #76
#include "Products/Common/Widgets/DropListView/ST4LayerTagDropListWidget.h" // #81

#include "Products/ContiEditor/Widgets/ST4CameraWorkSectionKeyListWidget.h" // #58

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"
#include "Products/ContiEditor/Helper/T4CameraWorkSectionKeyObject.h" // #58

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "Widgets/Layout/SScrollBox.h"

#include "Modules/ModuleManager.h"
#include "Widgets/Input/SButton.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailsView.h"
#include "IDetailGroup.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4ContiDetailCustomizationAction"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */

// #T4_ADD_ACTION_TAG_CONTI
void FT4ContiDetailCustomization::CustomizeBranchActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4BranchAction& InAction,
	uint32 InActionArrayIndex
) // #54
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder, 
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, Contition);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, ConditionName);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, ContiAsset);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, LoadingPolicy);
}

void FT4ContiDetailCustomization::CustomizeSpecialMoveActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4SpecialMoveAction& InAction,
	uint32 InActionArrayIndex
) // #54
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	CustomizeCommonActionDetails(
		InBuilder, 
		DetailCategoryBuilder,
		InHandle, 
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);
	
	/*
	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);
	TSharedPtr<IPropertyHandle> TurnTypeHandle = InHandle->GetChildHandle(TEXT("TurnType"), false);
	if (TurnTypeHandle.IsValid())
	{
		DetailCategoryBuilder.AddProperty(TurnTypeHandle);
	}
	TSharedPtr<IPropertyHandle> TargetYawAngleHandle = InHandle->GetChildHandle(TEXT("TargetYawAngle"), false);
	if (TargetYawAngleHandle.IsValid())
	{
		DetailCategoryBuilder.AddProperty(TargetYawAngleHandle); // #40
	}
	*/
}

void FT4ContiDetailCustomization::CustomizeAnimationActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4AnimationAction& InAction,
	uint32 InActionArrayIndex
)
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector(); // #39

	// #39 : FT4AnimationAction

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	TSharedPtr<IPropertyHandle> SectionNameHandle = InHandle->GetChildHandle(TEXT("SectionName"), false);
	TSharedPtr<ST4AnimSectionDropListWidget> AnimSectionDropListWidget = SNew(ST4AnimSectionDropListWidget, AnimSetAssetSelector, ET4EngineConstantType::SkillSection)
		.PropertyHandle(SectionNameHandle);
	AnimSectionDropListWidget->OnRefresh();

	if (SectionNameHandle.IsValid())
	{
		ActionPropertyGroup.AddWidgetRow()
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("AnimSequenceTitle", "SectionName"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			AnimSectionDropListWidget.ToSharedRef()
		];
	}
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendInTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendOutTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayRate);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, LoopCount);
}

void FT4ContiDetailCustomization::CustomizeParticleActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4ParticleAction& InAction,
	uint32 InActionArrayIndex
)
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	// #39 : FT4ParticleAction

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	// #54
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, AttachParent);

	// #57
	AddCustomActionPointProperty(ActionPropertyGroup, InHandle);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, ParticleAsset);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, LoadingPolicy);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, Scale); // #54
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayRate); // #54
}

void FT4ContiDetailCustomization::CustomizeDecalActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4DecalAction& InAction,
	uint32 InActionArrayIndex
) // #54
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, AttachParent);

	AddCustomActionPointProperty(ActionPropertyGroup, InHandle);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, DecalMaterial);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, LoadingPolicy);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, Scale);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, DecalSortOrder);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, DecalSize);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, FadeInTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, FadeOutTimeSec);
}

void FT4ContiDetailCustomization::CustomizeProjectileActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4ProjectileAction& InAction,
	uint32 InActionArrayIndex
) // #63
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	AddCustomActionPointProperty(ActionPropertyGroup, InHandle);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, CastingContiAsset);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, HeadContiAsset);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, EndContiAsset);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, LoadingPolicy);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, ThrowDelayTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, CastingStopDelayTimeSec);
}

void FT4ContiDetailCustomization::CustomizeReactionActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4ReactionAction& InAction,
	uint32 InActionArrayIndex
) // #76
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	AddCustomActionPointProperty(ActionPropertyGroup, InHandle);

	// #76
	TSharedPtr<IPropertyHandle> ReactionNameHandle = InHandle->GetChildHandle(TEXT("ReactionName"), false);
	if (ReactionNameHandle.IsValid())
	{
		TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57
		TSharedPtr<ST4ReactionDropListWidget> ReactionDropListWidget = SNew(ST4ReactionDropListWidget, ViewTargetSelector)
			.OnSelected(nullptr)
			.PropertyHandle(ReactionNameHandle);
		ReactionDropListWidget->OnRefresh();

		ActionPropertyGroup.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4ContiReactionNameSelectorTitle", "Reaction Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				ReactionDropListWidget.ToSharedRef()
			];
	}
}

void FT4ContiDetailCustomization::CustomizeLayerSetActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4LayerSetAction& InAction,
	uint32 InActionArrayIndex
) // #76
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	AddCustomActionPointProperty(ActionPropertyGroup, InHandle);

	TSharedPtr<IPropertyHandle> LayerTagNameHandle = InHandle->GetChildHandle(TEXT("LayerTagName"), false);

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57

	TSharedPtr<ST4LayerTagDropListWidget> LayerTagDropListWidget = SNew(ST4LayerTagDropListWidget, ET4LayerTagType::All, ViewTargetSelector)
		.PropertyHandle(LayerTagNameHandle);
	LayerTagDropListWidget->OnRefresh();

	ActionPropertyGroup.AddWidgetRow()
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4ContiLayerTagSelectorTitle", "LayerTag"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			LayerTagDropListWidget.ToSharedRef()
		];

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, LayerTagType);
}

void FT4ContiDetailCustomization::CustomizeTimeScaleActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4TimeScaleAction& InAction,
	uint32 InActionArrayIndex
) // #102
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayTarget);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendInCurve);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendInTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendOutCurve);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendOutTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, TimeScale);
}

void FT4ContiDetailCustomization::CustomizeCameraWorkActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4CameraWorkAction& InAction,
	uint32 InActionArrayIndex
) // #54
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayTarget);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendInCurve);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendInTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendOutCurve);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendOutTimeSec);

	UT4CameraWorkSectionKeyObject* CameraSectionKeyObject = ViewModelPtr->FindOrCreateCameraSectionKeyObject(
		InAction.HeaderKey,
		INDEX_NONE,
		false
	);
	check(nullptr != CameraSectionKeyObject);

	TSharedPtr<ST4CameraWorkSectionKeyListWidget> CameraSectionKeyListWidget = SNew(ST4CameraWorkSectionKeyListWidget, &InAction.SectionData)
		.OnSelectedByIndex(ST4CameraWorkSectionKeyListWidget::FT4OnDoubleClickedByIndex::CreateUObject(CameraSectionKeyObject, &UT4CameraWorkSectionKeyObject::HandleOnSectionKeySelected))
		.OnDoubleClickedByIndex(nullptr)
		.OnGetValueIndexSelcted(ST4CameraWorkSectionKeyListWidget::FT4OnGetValueIndexSelected::CreateUObject(CameraSectionKeyObject, &UT4CameraWorkSectionKeyObject::HandleOnGetValueIndexSelcted));

	ActionPropertyGroup
		.AddWidgetRow()
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4ContiCameraWorkSectionKeyListBoxTitle", "Section Keys"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			CameraSectionKeyListWidget.ToSharedRef()
		];

	CameraWorkSectionKeyWidgets.Add(CameraSectionKeyListWidget);

	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
		TSharedPtr<IDetailsView> CameraSectionKeyDetailsViewPtr = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

		CameraSectionKeyDetailsViewPtr->OnFinishedChangingProperties().AddUObject(
			CameraSectionKeyObject, 
			&UT4CameraWorkSectionKeyObject::HandleOnDetailPropertiesChanged
		);

		CameraSectionKeyDetailsViewPtr->RegisterInstancedCustomPropertyLayout(
			UT4CameraWorkSectionKeyObject::StaticClass(),
			FOnGetDetailCustomizationInstance::CreateStatic(
				&FT4CameraWorkSectionKeyDetailCustomization::MakeInstance,
				ViewModelPtr
			)
		);

		CameraSectionKeyDetailsViewPtr->SetObject(CameraSectionKeyObject);
		CameraSectionKeyObject->SetDetailView(CameraSectionKeyDetailsViewPtr.ToSharedRef());

		ActionPropertyGroup
			.AddWidgetRow()
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				.Padding(0.0f)
				[
					SNew(SBorder)
					//.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
					.Padding(0)
					[
						SNew(SBox)
						//.WidthOverride(175)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.MaxHeight(700)
							.Padding(4.0f)
							[
								CameraSectionKeyDetailsViewPtr.ToSharedRef()
							]
						]
					]
				]
			];
	}

	CameraSectionKeyListWidget->OnRefresh(true);
}

void FT4ContiDetailCustomization::CustomizeCameraShakeActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4CameraShakeAction& InAction,
	uint32 InActionArrayIndex
) // #101
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, true);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayTarget);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayScale);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlaySpace);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, UserDefinedPlaySpace);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, OscillationData);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, AnimData);
}

void FT4ContiDetailCustomization::CustomizePostProcessActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4PostProcessAction& InAction,
	uint32 InActionArrayIndex
) // #100
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, true);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayTarget);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendInCurve);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendInTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendOutCurve);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, BlendOutTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PostProcessSettings);
}

void FT4ContiDetailCustomization::CustomizeEnvironmentActionDetails(
	IDetailLayoutBuilder& InBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4EnvironmentAction& InAction,
	uint32 InActionArrayIndex
) // #99
{
	IDetailCategoryBuilder& DetailCategoryBuilder = GetCategoryBuilder(
		InBuilder,
		static_cast<const FT4ContiActionStruct*>(&InAction),
		InActionArrayIndex
	);

	TSharedPtr<SWidget> ActionHeaderWidget = GetActionPlayHeaderWidget(InAction.HeaderKey, false);
	DetailCategoryBuilder.HeaderContent(ActionHeaderWidget.ToSharedRef());

	CustomizeCommonActionDetails(
		InBuilder,
		DetailCategoryBuilder,
		InHandle,
		static_cast<const FT4ContiActionStruct*>(&InAction)
	);

	IDetailGroup& ActionPropertyGroup = DetailCategoryBuilder.AddGroup(
		FName("Detail Properties"),
		FText::FromString("Detail Properies"),
		false,
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, AttachParent);

	AddCustomActionPointProperty(ActionPropertyGroup, InHandle);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, ZoneEntityAsset);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, PlayTarget);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, bOverrideBlendTime);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, OverrideBlendInTimeSec);
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(ActionPropertyGroup, InHandle, OverrideBlendOutTimeSec);
}

#undef LOCTEXT_NAMESPACE
