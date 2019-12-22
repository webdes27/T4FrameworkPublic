// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiDetailCustomization.h"
#include "ST4ContiObjectWidget.h" // #58

#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/Common/Widgets/DropListView/ST4ActionPointDropListWidget.h" // #57
#include "Products/Common/Widgets/DropListView/ST4StanceDropListWidget.h" // #73
#include "Products/Common/Widgets/DropListView/ST4EntityDropListWidget.h" // #87
#include "Products/Common/Widgets/ListView/ST4PointOfInterestListWidget.h" // #100

#include "Products/ContiEditor/Widgets/ST4CameraWorkSectionKeyListWidget.h" // #58

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "Widgets/Input/SButton.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4ContiDetailCustomization"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */

TSharedRef<IDetailCustomization> FT4ContiDetailCustomization::MakeInstance(
	TSharedRef<ST4ContiObjectWidget> InContiObjectWidget // #58
)
{
	TSharedPtr<FT4ContiViewModel> ContiViewModel = InContiObjectWidget->GetContiViewModel();
	TSharedRef<FT4ContiDetailCustomization> NewDetailCustomization = MakeShared<FT4ContiDetailCustomization>(ContiViewModel);
	InContiObjectWidget->SetDetailCustomization(NewDetailCustomization);
	return NewDetailCustomization;
}

FT4ContiDetailCustomization::FT4ContiDetailCustomization(
	TSharedPtr<FT4ContiViewModel> InContiViewModel
)	: ViewModelPtr(InContiViewModel)
	, DetailLayoutPtr(nullptr)
{
}

void FT4ContiDetailCustomization::RefreshWidgets() // #58
{
	if (MapEntityDropListWidgetPtr.IsValid())
	{
		MapEntityDropListWidgetPtr->OnRefresh(); // #87
	}
	if (StanceDropListWidgetPtr.IsValid())
	{
		StanceDropListWidgetPtr->OnRefresh(); // #73
	}
	if (PointOfInterestListWidgetPtr.IsValid())
	{
		PointOfInterestListWidgetPtr->OnRefresh(false); // #100
	}
	for (TSharedPtr<ST4CameraWorkSectionKeyListWidget> CameraWorkSectionKeyWidgetPtr : CameraWorkSectionKeyWidgets) // #58
	{
		if (CameraWorkSectionKeyWidgetPtr.IsValid())
		{
			CameraWorkSectionKeyWidgetPtr->OnRefresh(false);
		}
	}
}

void FT4ContiDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	DetailLayoutPtr = &InBuilder;

	UT4ContiAsset* ContiAsset = ViewModelPtr->GetContiAsset();
	if (nullptr == ContiAsset)
	{
		return;
	}

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ContiAsset, CompositeData); // all hided

	CustomizeEditorSetDetails(InBuilder);
	
	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ContiAsset, TotalPlayTimeSec);
		static const FName DefaultCategoryName = TEXT("Default"); // #56
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(DefaultCategoryName);
		DetailCategoryBuilder.AddProperty(TotalPlayTimeSecHandle);

		TotalPlayTimeSecHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4ContiDetailCustomization::HandleOnTotalPlayTimeSecChanged)
		); // #67
	}

	const TArray<FT4ContiActionInfo>& SelectedActionInfos = ViewModelPtr->GetSelectedActionInfos();
	if (0 == SelectedActionInfos.Num())
	{	
		return;
	}

	// #56 : Detail View 도 SortOrder 순으로 보여준다.
	struct FT4SortSelectedContiAction
	{
		uint32 SortOrder;
		int32 ArrayIndex;
	};
	TArray<FT4SortSelectedContiAction> SortedContiActionArray;
	int32 Idx = 0;
	for (const FT4ContiActionInfo& SelectedActionInfo : SelectedActionInfos)
	{
		FT4SortSelectedContiAction& NewInfo = SortedContiActionArray.AddDefaulted_GetRef();
		NewInfo.SortOrder = SelectedActionInfo.ActionSortOrder;
		NewInfo.ArrayIndex = Idx++;
	}
	SortedContiActionArray.Sort([](const FT4SortSelectedContiAction& A, const FT4SortSelectedContiAction&B)
	{
		return A.SortOrder < B.SortOrder;
	});

	const FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	for (const FT4SortSelectedContiAction& SortedContiActionInfo : SortedContiActionArray)
	{
		check(SortedContiActionInfo.ArrayIndex < SelectedActionInfos.Num());
		const FT4ContiActionInfo& SelectedActionInfo = SelectedActionInfos[SortedContiActionInfo.ArrayIndex];

		uint32 NumChildActions = 0;
		FName ActionPropertyName = NAME_None;

		switch (SelectedActionInfo.ActionType)
		{
#define DEFINE_CONTI_ACTION_MACRO(x)												\
			case ET4ActionType::##x:												\
				ActionPropertyName = FName(#x "Actions");							\
				NumChildActions = CompositeData.##x##Actions.Num();					\
				break;

			#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"
			default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("CustomizeDetails '%u' failed. no implementation."),
						uint32(SelectedActionInfo.ActionType)
					);
				}
				break;
		}
		if (0 == NumChildActions || ActionPropertyName == NAME_None)
		{
			continue;
		}
		TSharedPtr<IPropertyHandle> ActionsPropertyHandle = CompositeDataHandle->GetChildHandle(
			ActionPropertyName
		);
		if (!ActionsPropertyHandle.IsValid())
		{
			continue;
		}
		TSharedPtr<IPropertyHandleArray> ActionPropertyArrayHandle = ActionsPropertyHandle->AsArray();
		if (!ActionPropertyArrayHandle.IsValid())
		{
			continue;
		}
		uint32 NumItems = 0;
		FPropertyAccess::Result Result = ActionPropertyArrayHandle->GetNumElements(NumItems);
		if (FPropertyAccess::Success != Result || NumChildActions != NumItems)
		{
			continue;
		}
		check(SelectedActionInfo.ActionArrayIndex < (int32)NumItems);
		TSharedPtr<IPropertyHandle> ActionPropertyHandle = ActionsPropertyHandle->GetChildHandle(
			SelectedActionInfo.ActionArrayIndex
		);
		if (!ActionPropertyHandle.IsValid())
		{
			continue;
		}
		switch (SelectedActionInfo.ActionType)
		{
#define DEFINE_CONTI_ACTION_MACRO(x)														\
			case ET4ActionType::##x:														\
				Customize##x##ActionDetails(InBuilder, ActionPropertyHandle, CompositeData.##x##Actions[SelectedActionInfo.ActionArrayIndex], SelectedActionInfo.ActionArrayIndex); \
				break;

			#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"
			default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("CustomizeDetails '%u' failed. no implementation."),
						uint32(SelectedActionInfo.ActionType)
					);
				}
				break;
		}
	}
}

void FT4ContiDetailCustomization::CustomizeEditorSetDetails(
	IDetailLayoutBuilder& InBuilder
)
{
	static const FName EditorCategoryName = TEXT("Editor");

	// #60 : 불필요한 값들은 감춘다.
	// #T4_ADD_EDITOR_PLAY_TAG
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ContiAsset, TestSettings);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ContiAsset, TestAutomation);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ContiAsset, PreviewEntityAsset);

	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(EditorCategoryName);
	{
		DetailCategoryBuilder.AddProperty(PreviewEntityAssetHandle);

		PreviewEntityAssetHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4ContiDetailCustomization::HandleOnSpawnPreviewEntity)
		); // #67
	}

	// #87
	TSharedPtr<IPropertyHandle> MapEntitySelectedHandle = TestSettingsHandle->GetChildHandle(TEXT("MapEntitySelected"), false);

	MapEntityDropListWidgetPtr = SNew(ST4EntityDropListWidget, ET4EntityType::Map)
		.OnSelected(this, &FT4ContiDetailCustomization::HandleOnMapEntitySelected)
		.PropertyHandle(MapEntitySelectedHandle);
	MapEntityDropListWidgetPtr->SetNoNameDescription(TEXT("[Virtual] Preview"));
	MapEntityDropListWidgetPtr->OnRefresh();

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4ContiPlayerMapEntitySelector", "ChangeWorld"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4ContiPlayerMapEntitySelectorTitle", "Change World"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			MapEntityDropListWidgetPtr.ToSharedRef()
		];

	// #73
	TSharedPtr<IPropertyHandle> StanceSelectedHandle = TestSettingsHandle->GetChildHandle(TEXT("StanceSelected"), false);

	TSet<FName> StanceNamelist;
	ViewModelPtr->ClientGetPlayerStanceList(StanceNamelist); // #73

	StanceDropListWidgetPtr = SNew(ST4StanceDropListWidget, StanceNamelist)
		.OnSelected(this, &FT4ContiDetailCustomization::HandleOnStanceSelected)
		.PropertyHandle(StanceSelectedHandle);
	StanceDropListWidgetPtr->OnRefresh();

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4ContiPlayerStanceSelector", "Stance"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4ContiPlayerStanceSelectorTitle", "Change Player Stance"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			StanceDropListWidgetPtr.ToSharedRef()
		];

	UT4ContiAsset* ContiAsset = ViewModelPtr->GetContiAsset();
	if (nullptr == ContiAsset)
	{
		return;
	}

	PointOfInterestListWidgetPtr = SNew(ST4PointOfInterestListWidget, &ContiAsset->TestAutomation)
		.OnSelectedByIndex(this, &FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestSelected)
		.OnDoubleClickedByIndex(this, &FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestGo);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4ContiAutomationPointOfInterestList", "POI"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4ContiAutomationPointOfInterestListBoxTitle", "Point Of Interests"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			PointOfInterestListWidgetPtr.ToSharedRef()
		];

	DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(TestAutomationHandle, TransientName); // #103

	{
		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4ContiAutomationPointOfInterestSelector", "ContiAutomationPointOfInterest"))
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
					.Text(LOCTEXT("T4ContiAutomationTeleportUpdateBtn", "Update POI"))
					.ToolTipText(LOCTEXT("T4ContiAutomationTeleportUpdateBtn_Tooltip", "Update Point Of Interest"))
					.OnClicked(this, &FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestUpdate)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4ContiAutomationTeleportAddBtn", "Add POI"))
					.ToolTipText(LOCTEXT("T4ContiAutomationTeleportAddBtn_Tooltip", "Add Point Of Interest"))
					.OnClicked(this, &FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestAdd)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4ContiAutomationTeleportRemoveBtn", "Remove POI"))
					.ToolTipText(LOCTEXT("T4ContiAutomationTeleportRemoveBtn_Tooltip", "Remove Selected POI"))
					.OnClicked(this, &FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestRemove)
				]
			];
	}

	PointOfInterestListWidgetPtr->OnRefresh(true);
}

void FT4ContiDetailCustomization::CustomizeCommonActionDetails(
	IDetailLayoutBuilder& InBuilder,
	IDetailCategoryBuilder& InDetailCategoryBuilder,
	TSharedPtr<IPropertyHandle> InHandle,
	const FT4ContiActionStruct* InAction
)
{
	// #39 : FT4ActionStruct
	IDetailGroup& CommonPropertyGroup = InDetailCategoryBuilder.AddGroup(
		FName("Common"), 
		FText::FromString("Common Properies"), 
		true, 
		true
	);

	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(CommonPropertyGroup, InHandle, DisplayName);
	DisplayNameHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FT4ContiDetailCustomization::HandleOnRefreshActionTrack, InAction->HeaderKey)
	);
	if (ET4ActionType::CameraWork == InAction->ActionType) // #56 : Duration 일 경우만 Detail 표시!
	{
		// #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
	}
	else
	{
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(CommonPropertyGroup, InHandle, LifecycleType);
		LifecycleTypeHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4ContiDetailCustomization::HandleOnRefreshActionTrack, InAction->HeaderKey)
		);
	}
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(CommonPropertyGroup, InHandle, StartTimeSec);
	StartTimeSecHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FT4ContiDetailCustomization::HandleOnRefreshActionTrack, InAction->HeaderKey)
	);
	if (ET4LifecycleType::Duration == InAction->LifecycleType) // #56 : Duration 일 경우만 Detail 표시!
	{
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(CommonPropertyGroup, InHandle, DurationSec);
		DurationSecHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4ContiDetailCustomization::HandleOnRefreshActionTrack, InAction->HeaderKey)
		);
	}
	DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(CommonPropertyGroup, InHandle, DebugColorTint);

	CommonPropertyGroup.AddWidgetRow()
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
				.Text(LOCTEXT("T4ContiCommonApplyDebugColorTintBtn", "Apply ColorTint"))
				.ToolTipText(LOCTEXT("T4ContiCommonApplyDebugColorTintBtn_Tooltip", "Apply to Debug ColorTint"))
				.OnClicked(this, &FT4ContiDetailCustomization::HandleOnRefreshActionTrackButton, InAction->HeaderKey)
			]
		];
}

void FT4ContiDetailCustomization::HandleOnSpawnPreviewEntity() // #67
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	ViewModelPtr->ReloadPlayerSpawn();
}

void FT4ContiDetailCustomization::HandleOnTotalPlayTimeSecChanged() // #100
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	ViewModelPtr->UpdateMovieScenePlaybackRange();
}

void FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestSelected(int32 InSelectedIndex) // #103
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	if (-1 == InSelectedIndex)
	{
		return;
	}
	ViewModelPtr->SelectPointOfInterest(InSelectedIndex);
}

void FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestGo(int32 InSelectedIndex)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	if (-1 == InSelectedIndex)
	{
		return;
	}
	ViewModelPtr->TravelPointOfInterest(InSelectedIndex);
}

FReply FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestUpdate() // #103
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!PointOfInterestListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	int32 POISelected = PointOfInterestListWidgetPtr->GetItemValueIndexSelected();
	if (-1 == POISelected)
	{
		return FReply::Handled();
	}
	ViewModelPtr->UpdatePointOfInterest(POISelected);
	PointOfInterestListWidgetPtr->OnRefresh(false);
	return FReply::Handled();
}

FReply FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestAdd()
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!PointOfInterestListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	int32 AddIndex = ViewModelPtr->AddPointOfInterest();
	PointOfInterestListWidgetPtr->SetInitializeIndex(AddIndex);
	PointOfInterestListWidgetPtr->OnRefresh(false);
	return FReply::Handled();
}

FReply FT4ContiDetailCustomization::HandleOnAutomationPointOfInterestRemove()
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!PointOfInterestListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	int32 ValueIndexSelected = PointOfInterestListWidgetPtr->GetItemValueIndexSelected();
	if (-1 == ValueIndexSelected)
	{
		return FReply::Handled();
	}
	ViewModelPtr->RemovePointOfInterest(ValueIndexSelected - 1);
	PointOfInterestListWidgetPtr->OnRefresh(false);
	return FReply::Handled();
}

void FT4ContiDetailCustomization::HandleOnRefreshActionTrack(int32 InActionHeaderKey) // #100
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	ViewModelPtr->RefreshMovieSceneActionTrack(InActionHeaderKey);
}

FReply FT4ContiDetailCustomization::HandleOnRefreshActionTrackButton(int32 InActionHeaderKey) // #100
{
	HandleOnRefreshActionTrack(InActionHeaderKey);
	return FReply::Handled();
}

void FT4ContiDetailCustomization::HandleOnStanceSelected(const FName InName) // #73
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	ViewModelPtr->ClientChangeStance(InName);
}

void FT4ContiDetailCustomization::HandleOnMapEntitySelected(const FName InName) // #87
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	bool bResult = false;
	if (InName == NAME_None)
	{
		bResult = ViewModelPtr->ClientWorldTravel(nullptr);
	}
	else
	{
		FT4EntityKey MapEntityKey(ET4EntityType::Map, InName);
		bResult = ViewModelPtr->ClientWorldTravel(MapEntityKey);
	}
	if (bResult)
	{
	}
}

FReply FT4ContiDetailCustomization::HandleOnActionPlay(int32 InActionHeaderKey) // #100
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	ViewModelPtr->PlayActionBy(InActionHeaderKey);
	return FReply::Handled();
}

FReply FT4ContiDetailCustomization::HandleOnActionPlayStop(int32 InActionHeaderKey) // #100
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	ViewModelPtr->PlayActionStopBy(InActionHeaderKey);
	return FReply::Handled();
}

FReply FT4ContiDetailCustomization::HandleOnActionReset(int32 InActionHeaderKey) // #100
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	ViewModelPtr->PlayActionResetBy(InActionHeaderKey);
	return FReply::Handled();
}

void FT4ContiDetailCustomization::AddCustomActionPointProperty(
	IDetailGroup& InActionDetailGroup,
	TSharedPtr<IPropertyHandle> InHandle
)
{
	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57
	TSharedPtr<IPropertyHandle> ActionPointHandle = InHandle->GetChildHandle(TEXT("ActionPoint"), false);
	TSharedPtr<ST4ActionPointDropListWidget> ActionPointDropListWidget = SNew(ST4ActionPointDropListWidget, ViewTargetSelector)
		.PropertyHandle(ActionPointHandle);
	ActionPointDropListWidget->OnRefresh();

	if (ActionPointHandle.IsValid())
	{
		InActionDetailGroup.AddWidgetRow()
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("ActionPointTitle", "ActionPoint"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			ActionPointDropListWidget.ToSharedRef()
		];
	}
}

IDetailCategoryBuilder& FT4ContiDetailCustomization::GetCategoryBuilder(
	IDetailLayoutBuilder& InBuilder,
	const FT4ContiActionStruct* InAction,
	uint32 InActionArrayIndex
)
{
	FString CategoryName = FString::Printf(
		TEXT("%s (%u) : %s"),
		*InAction->ToString(),
		InActionArrayIndex,
		*InAction->DisplayName.ToString()
	);
	return InBuilder.EditCategory(*CategoryName);
}

TSharedPtr<SWidget> FT4ContiDetailCustomization::GetActionPlayHeaderWidget(
	int32 InActionHeaderKey,
	bool bShowResetButton
) // #100
{
	if (!bShowResetButton)
	{
		TSharedRef<SWidget> WitoutResetHeaderWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("T4ContiActionControl", "Action "))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "RoundButton")
					.ForegroundColor(FEditorStyle::GetSlateColor("DefaultForeground"))
					.ContentPadding(FMargin(2, 0))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Text(LOCTEXT("T4ContiActionPlayBtn", "Play"))
					.ToolTipText(LOCTEXT("T4ContiActionPlayBtn_Tooltip", "Test play"))
					.OnClicked(this, &FT4ContiDetailCustomization::HandleOnActionPlay, InActionHeaderKey)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "RoundButton")
					.ForegroundColor(FEditorStyle::GetSlateColor("DefaultForeground"))
					.ContentPadding(FMargin(2, 0))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Text(LOCTEXT("T4ContiActionPlayStopBtn", "Stop"))
					.ToolTipText(LOCTEXT("T4ContiActionPlayStopBtn_Tooltip", "Stop"))
					.OnClicked(this, &FT4ContiDetailCustomization::HandleOnActionPlayStop, InActionHeaderKey)
				]
			];

		return WitoutResetHeaderWidget;
	}

	TSharedRef<SWidget> FullHeaderWidget =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4ContiActionControl", "Action "))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "RoundButton")
				.ForegroundColor(FEditorStyle::GetSlateColor("DefaultForeground"))
				.ContentPadding(FMargin(2, 0))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text(LOCTEXT("T4ContiActionPlayBtn", "Play"))
				.ToolTipText(LOCTEXT("T4ContiActionPlayBtn_Tooltip", "Test play"))
				.OnClicked(this, &FT4ContiDetailCustomization::HandleOnActionPlay, InActionHeaderKey)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "RoundButton")
				.ForegroundColor(FEditorStyle::GetSlateColor("DefaultForeground"))
				.ContentPadding(FMargin(2, 0))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text(LOCTEXT("T4ContiActionPlayStopBtn", "Stop"))
				.ToolTipText(LOCTEXT("T4ContiActionPlayStopBtn_Tooltip", "Stop"))
				.OnClicked(this, &FT4ContiDetailCustomization::HandleOnActionPlayStop, InActionHeaderKey)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "RoundButton")
				.ForegroundColor(FEditorStyle::GetSlateColor("DefaultForeground"))
				.ContentPadding(FMargin(2, 0))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text(LOCTEXT("T4ContiActionResetBtn", "Reset"))
				.ToolTipText(LOCTEXT("T4ContiActionResetBtn_Tooltip", "Reset"))
				.OnClicked(this, &FT4ContiDetailCustomization::HandleOnActionReset, InActionHeaderKey)
			]
		];

	return FullHeaderWidget;
}

#undef LOCTEXT_NAMESPACE
