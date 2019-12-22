// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldDetailCustomization.h"
#include "ST4WorldObjectWidget.h" // #90

#include "Products/WorldEditor/Widgets/ST4MapZoneListWidget.h" // #92
#include "Products/WorldEditor/Widgets/ST4SubLevelListWidget.h" // #104

#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"
#include "Products/Common/DetailView/T4EnvironmentDetailCustomization.h" // #94

#include "Products/Common/Widgets/DropListView/ST4MapZoneDropListWidget.h" // #92
#include "Products/Common/Widgets/ListView/ST4PointOfInterestListWidget.h" // #103

#include "Products/WorldEditor/ViewModel/T4WorldViewModel.h"

#include "Products/Common/Utility/T4AssetEnvironmentUtils.h" // #94
#include "Products/WorldEditor/Utility/T4AssetWorldUtils.h" // #90
#include "Products/T4RehearsalEditorUtils.h" // #104

#include "T4Asset/Classes/World/T4WorldAsset.h"
#include "T4Engine/Classes/World/T4MapZoneVolume.h" // #92

#include "AssetData.h"

#include "Engine/LevelActorContainer.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Engine/WorldComposition.h"
#include "Engine/Classes/GameFramework/Actor.h"

#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"

#include "Modules/ModuleManager.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "Misc/MessageDialog.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4WorldDetailCustomization"

static const FText T4WorldCustomDetailErrorResultNameIsAlreadyExistsText = LOCTEXT("T4WorldCustomDetailResultNameIsAlreadyExistsError", "MapZone Name is already exists"); // #92

static const FText T4WorldCustomDetailErrorTitleText = LOCTEXT("T4WorldCustomDetailsError", "World CustomDetail Error"); // #90

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */

TSharedRef<IDetailCustomization> FT4WorldDetailCustomization::MakeInstance(
	TSharedRef<ST4WorldObjectWidget> InWorldObjectWidget // #85
)
{
	TSharedPtr<FT4WorldViewModel> WorldViewModel = InWorldObjectWidget->GetWorldViewModel();
	TSharedRef<FT4WorldDetailCustomization> NewDetailCustomization = MakeShared<FT4WorldDetailCustomization>(WorldViewModel);
	InWorldObjectWidget->SetDetailCustomization(NewDetailCustomization);
	return NewDetailCustomization;
}

FT4WorldDetailCustomization::FT4WorldDetailCustomization(
	TSharedPtr<FT4WorldViewModel> InWorldViewModel
)	: ViewModelPtr(InWorldViewModel)
	, DetailLayoutPtr(nullptr)
{
	EnvironmentDetailPtr = MakeShareable(new FT4EnvironmentDetailCustomization(InWorldViewModel.Get())); // #94
}

void FT4WorldDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}

	DetailLayoutPtr = &InBuilder;

	UT4WorldAsset* WorldAsset = ViewModelPtr->GetWorldAsset();
	if (nullptr == WorldAsset)
	{
		return;
	}

	EditorTransientDataHandle = DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(UT4WorldAsset, EditorTransientData); // #90
	EditorTransientDataHandle->SetOnChildPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FT4WorldDetailCustomization::HandleOnMapZoneDetailChanged)
	); // #90, #92
	InBuilder.HideProperty(EditorTransientDataHandle);

	{
		// #103
		static const TCHAR* EditorCategoryName = TEXT("Editor");
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldAsset, TestAutomation);
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(EditorCategoryName);

		PointOfInterestListWidgetPtr = SNew(ST4PointOfInterestListWidget, &WorldAsset->TestAutomation)
			.OnSelectedByIndex(this, &FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestSelected)
			.OnDoubleClickedByIndex(this, &FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestGo);

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4WorldAutomationPointOfInterestList", "POI"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4WorldAutomationPointOfInterestListBoxTitle", "Point Of Interests"))
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
				.AddCustomRow(LOCTEXT("T4WorldAutomationPointOfInterestSelector", "WorldAutomationPointOfInterest"))
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
						.Text(LOCTEXT("T4WorldAutomationTeleportUpdateBtn", "Update POI"))
						.ToolTipText(LOCTEXT("T4WorldAutomationTeleportUpdateBtn_Tooltip", "Update Point Of Interest"))
						.OnClicked(this, &FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestUpdate)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("T4WorldAutomationTeleportAddBtn", "Add POI"))
						.ToolTipText(LOCTEXT("T4WorldAutomationTeleportAddBtn_Tooltip", "Add Point Of Interest"))
						.OnClicked(this, &FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestAdd)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("T4WorldAutomationTeleportRemoveBtn", "Remove POI"))
						.ToolTipText(LOCTEXT("T4WorldAutomationTeleportRemoveBtn_Tooltip", "Remove Selected POI"))
						.OnClicked(this, &FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestRemove)
					]
				];
		}

		PointOfInterestListWidgetPtr->OnRefresh(true);
	}

	{
		static const FName DefaultCategoryName = TEXT("Default");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(DefaultCategoryName);

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldAsset, MapEntityAsset);

		DetailCategoryBuilder.AddProperty(MapEntityAssetHandle);
		MapEntityAssetHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4WorldDetailCustomization::HandleOnMapEntityAssetChanged)
		);

	}

	if (ViewModelPtr->IsWorldCompositionEnabled())
	{
		UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor); // #104
		check(nullptr != EditorWorld);

		static const FName LoadedSubLevelsCategoryName = TEXT("Levels");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(LoadedSubLevelsCategoryName);

		LoadedSubLevelListWidgetPtr = SNew(ST4SubLevelListWidget, EditorWorld, ET4SubLevelListType::SubLevelList_Tile)
			.OnMultiSelected(this, &FT4WorldDetailCustomization::HandleOnLevelMultiSelected)
			.OnDoubleClicked(nullptr);

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4WorldDetailLoadedSubLevelList", "Levels"))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				LoadedSubLevelListWidgetPtr.ToSharedRef()
			];

		LoadedSubLevelListWidgetPtr->OnRefresh(false);

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4WorldDetailLoadedSubLevelSelector", "WorldDetailLoadedSubLevel"))
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
					.Text(LOCTEXT("T4WorldDetailPreviewSubLevelLoadBtn", "Load in Preview World"))
					.ToolTipText(LOCTEXT("T4WorldDetailPreviewSubLevelLoadBtn_Tooltip", "ALoadd Selected Level in a Preview World"))
					.OnClicked(this, &FT4WorldDetailCustomization::HandleOnLoadSubLevelInPreviewWorld)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4WorldDetailEditorSubLevelLoadBtn", "Load in Editor World"))
					.ToolTipText(LOCTEXT("T4WorldDetailEditorSubLevelLoadBtn_Tooltip", "Load Selected Level in a Editor World"))
					.OnClicked(this, &FT4WorldDetailCustomization::HandleOnLoadSubLevelInEditorWorld)
				]
			];
	}

	CustomizeWorldMapZoneDetails(InBuilder, WorldAsset); // #92
}

void FT4WorldDetailCustomization::CustomizeWorldMapZoneDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4WorldAsset* InWorldAsset
) // #92
{
	check(nullptr != InWorldAsset);

	if (!ViewModelPtr.IsValid())
	{
		return;
	}

	if (InWorldAsset->MapEntityAsset.IsNull())
	{
		return;
	}

	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	check(nullptr != EditorWorld);

	static const FName MapZoneCategoryName = TEXT("Map Zone");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(MapZoneCategoryName);

	FT4WorldEditorTransientData& EditorTransientData = InWorldAsset->EditorTransientData;

	MapZoneListWidgetPtr = SNew(ST4MapZoneListWidget, EditorWorld)
		.OnSelected(this, &FT4WorldDetailCustomization::HandleOnMapZoneSelected)
		.OnDoubleClicked(nullptr)
		.InInitializeValue(EditorTransientData.TransientMapZoneName);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4WorldDetailMapZoneList", "MapZones"))
		.WholeRowContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			MapZoneListWidgetPtr.ToSharedRef()
		];

	{
		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4WorldDetailMapZoneButton", "MapZoneButton"))
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
					.Text(LOCTEXT("T4WorldDetailMapZoneRefreshBtn", "Refresh MapZones"))
					.ToolTipText(LOCTEXT("T4WorldDetailMapZoneRefreshBtn_Tooltip", "Refresh MapZones"))
					.OnClicked(this, &FT4WorldDetailCustomization::HandleOnRefreshMapZoneLists)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4WorldDetailMapZoneAddBtn", "New MapZone"))
					.ToolTipText(LOCTEXT("T4WorldDetailMapZoneAddBtn_Tooltip", "Add a New MapZone"))
					.OnClicked(this, &FT4WorldDetailCustomization::HandleOnAddNewMapZoneVolume)
				]
			];

		FName MapZoneSelected = MapZoneListWidgetPtr->GetItemValueSelected();
		{
			ViewModelPtr->SelectEditorWorldMapZone(MapZoneSelected);
			{
				AT4MapZoneVolume* MapZoneVolume = ViewModelPtr->GetMapZoneSelectedOnEditorWorld();
				T4AssetUtil::WorldSelectMapZoneVolumeByName(InWorldAsset, MapZoneVolume);
			}
		}

		IDetailGroup& MapZoneDetailGroup = DetailCategoryBuilder.AddGroup(FName("MapZone Details"), FText::FromString("MapZone Details"), false);

		TSharedPtr<IPropertyHandle> TransientMapZoneNameHandle = DEFINE_DETAIL_GET_CHILD_ROPERTY_MACRO(EditorTransientDataHandle, TransientMapZoneName);
		if (TransientMapZoneNameHandle.IsValid())
		{
			MapZoneDropListWidgetPtr = SNew(ST4MapZoneDropListWidget)
				.OnSelected(nullptr)
				.PropertyHandle(TransientMapZoneNameHandle); // #92
			MapZoneDropListWidgetPtr->OnRefresh();

			MapZoneDetailGroup
				.HeaderRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4WorldDetailMapZoneSelectorTitle", "MapZone Details"))
				]
			.ValueContent()
				.HAlign(HAlign_Fill)
				.MaxDesiredWidth(400.0f)
				[
					MapZoneDropListWidgetPtr.ToSharedRef()
				];
		}

		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MapZoneDetailGroup, EditorTransientDataHandle, TransientTransform); // #92
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MapZoneDetailGroup, EditorTransientDataHandle, TransientBlendPriority); // #92
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MapZoneDetailGroup, EditorTransientDataHandle, TransientBlendInTimeSec); // #92
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MapZoneDetailGroup, EditorTransientDataHandle, TransientBlendOutTimeSec); // #92
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MapZoneDetailGroup, EditorTransientDataHandle, TransientDebugColor); // #92

		CustomizeWorldMapZoneEnvironmentDetails(InBuilder, InWorldAsset);

	}

	MapZoneListWidgetPtr->OnRefresh(true);
}

void FT4WorldDetailCustomization::CustomizeWorldMapZoneEnvironmentDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4WorldAsset* InWorldAsset
) // #92
{
	check(nullptr != InWorldAsset);

	if (!ViewModelPtr.IsValid())
	{
		return;
	}

	if (InWorldAsset->MapEntityAsset.IsNull())
	{
		return;
	}

	static const FName MapZoneCategoryName = TEXT("Map Zone Environment");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(MapZoneCategoryName);

	TSharedRef<SWidget> EditorSettingsHeaderWidget =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "RoundButton")
			.ForegroundColor(FEditorStyle::GetSlateColor("DefaultForeground"))
			.ContentPadding(FMargin(2, 0))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Text(LOCTEXT("T4WorldDetailEnvironmentAssetSaveBtn", "Save the Modified Properties"))
			.ToolTipText(LOCTEXT("T4WorldDetailEnvironmentAssetSaveBtn_Tooltip", "Save the Modified Properties"))
			.OnClicked(this, &FT4WorldDetailCustomization::HandleOnEnvironmentAssetSave)
		];

	DetailCategoryBuilder.HeaderContent(EditorSettingsHeaderWidget);

	DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EditorTransientDataHandle, TransientEnvironmentAsset); // #92

	if (EnvironmentDetailPtr.IsValid())
	{
		EnvironmentDetailPtr->CustomizeEnvironmentDetails(
			DetailCategoryBuilder,
			InWorldAsset->EditorTransientData.TransientEnvironmentAsset.LoadSynchronous()
		); // #94
	}
}

void FT4WorldDetailCustomization::OnRefreshWorld() // #104
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	if (LoadedSubLevelListWidgetPtr.IsValid())
	{
		const TSet<FName>& LoadedLevels = ViewModelPtr->GetPreviewLoadedLevels();
		LoadedSubLevelListWidgetPtr->SetLoadedLevels(LoadedLevels);
		LoadedSubLevelListWidgetPtr->OnRefresh(false);
	}
}

void FT4WorldDetailCustomization::HandleOnLevelMultiSelected(
	const TArray<FName>& InSubLevelNames
) // #104
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	ViewModelPtr->SelectSubLevelOfWorldMap(InSubLevelNames);
}

FReply FT4WorldDetailCustomization::HandleOnLoadSubLevelInPreviewWorld() // #104
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!LoadedSubLevelListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	TArray<FName> ItemMultiSelected;
	if (!LoadedSubLevelListWidgetPtr->GetItemValueMultiSelected(ItemMultiSelected))
	{
		return FReply::Handled();
	}
	ViewModelPtr->LoadPreviewWorldSubLevel(ItemMultiSelected);
	return FReply::Handled();
}

FReply FT4WorldDetailCustomization::HandleOnLoadSubLevelInEditorWorld() // #104
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!LoadedSubLevelListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	TArray<FName> ItemMultiSelected;
	if (!LoadedSubLevelListWidgetPtr->GetItemValueMultiSelected(ItemMultiSelected))
	{
		return FReply::Handled();
	}
	ViewModelPtr->LoadEditorWorldSubLevel(ItemMultiSelected);
	return FReply::Handled();
}

void FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestSelected(int32 InSelectedIndex) // #103
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

void FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestGo(int32 InSelectedIndex) // #103
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

FReply FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestUpdate() // #103
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

FReply FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestAdd() // #103
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

FReply FT4WorldDetailCustomization::HandleOnAutomationPointOfInterestRemove() // #103
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

FReply FT4WorldDetailCustomization::HandleOnRefreshMapZoneLists() // #92
{
	if (MapZoneListWidgetPtr.IsValid())
	{
		MapZoneListWidgetPtr->OnRefresh(false);
	}
	FName MapZoneSelected = MapZoneListWidgetPtr->GetItemValueSelected();
	HandleOnMapZoneSelected(MapZoneSelected);
	return FReply::Handled();
}

FReply FT4WorldDetailCustomization::HandleOnAddNewMapZoneVolume() // #92
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4WorldAsset* WorldAsset = ViewModelPtr->GetWorldAsset();
	if (nullptr == WorldAsset)
	{
		return FReply::Handled();
	}
	const FT4WorldEditorTransientData& EditorTransientData = WorldAsset->EditorTransientData;
	const FName UniqueMapZoneName = MakeUniqueObjectName(
		GetTransientPackage(),
		AT4MapZoneVolume::StaticClass(),
		EditorTransientData.TransientMapZoneName
	);
	if (MapZoneListWidgetPtr.IsValid())
	{
		// 이름이 변경되었다면 기존 이름과 같은 존이 있는지 체크한다. 같은 이름을 허용하지 않는다!
		if (MapZoneListWidgetPtr->HasMapZoneName(UniqueMapZoneName))
		{
			FMessageDialog::Open(
				EAppMsgType::Ok,
				EAppReturnType::Ok,
				T4WorldCustomDetailErrorResultNameIsAlreadyExistsText,
				&T4WorldCustomDetailErrorTitleText
			);
			return FReply::Handled();
		}
	}
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	check(nullptr != EditorWorld);
	bool bResult = T4AssetUtil::WorldAddNewMapZoneVolume(WorldAsset, UniqueMapZoneName, EditorWorld);
	if (!bResult)
	{
		return FReply::Handled();
	}
	if (MapZoneListWidgetPtr.IsValid())
	{
		MapZoneListWidgetPtr->SetInitializeValue(UniqueMapZoneName);
		MapZoneListWidgetPtr->OnRefresh(false);
	}
	HandleOnMapZoneSelected(UniqueMapZoneName);
	ViewModelPtr->NotifyEditorWorldModified(); // #83, #92 : Editor / Preview World 가 틀릴 경우 화면에 노티!
	return FReply::Handled();
}

void FT4WorldDetailCustomization::HandleOnMapEntityAssetChanged()
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	ViewModelPtr->ChangeMapEntityAsset(); // #90
}

void FT4WorldDetailCustomization::HandleOnMapZoneDetailChanged() // #90, #92
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4WorldAsset* WorldAsset = ViewModelPtr->GetWorldAsset();
	if (nullptr == WorldAsset)
	{
		return;
	}
	AT4MapZoneVolume* EditorMapZoneVolume = ViewModelPtr->GetMapZoneSelectedOnEditorWorld();
	if (nullptr == EditorMapZoneVolume)
	{
		return;
	}

	bool bZoneNameChanged = false;
	const FT4WorldEditorTransientData& EditorTransientData = WorldAsset->EditorTransientData;
	if (EditorMapZoneVolume->ZoneName != EditorTransientData.TransientMapZoneName)
	{
		// 이름이 변경되었다면 기존 이름과 같은 존이 있는지 체크한다. 같은 이름을 허용하지 않는다!
		if (MapZoneListWidgetPtr.IsValid())
		{
			if (MapZoneListWidgetPtr->HasMapZoneName(EditorTransientData.TransientMapZoneName))
			{
				T4AssetUtil::WorldSelectMapZoneVolumeByName(WorldAsset, EditorMapZoneVolume); // 다시 돌려준다.
				FMessageDialog::Open(
					EAppMsgType::Ok,
					EAppReturnType::Ok,
					T4WorldCustomDetailErrorResultNameIsAlreadyExistsText,
					&T4WorldCustomDetailErrorTitleText
				);
				return;
			}
		}
		bZoneNameChanged = true;
	}

	if (bZoneNameChanged)
	{
		ViewModelPtr->SelectEditorWorldMapZone(NAME_None); // delegate 를 사용해서 null 로 무력화 후 재생성해준다.
	}

	bool bZoneEnvironmentAssetChanged = (EditorTransientData.TransientEnvironmentAsset != EditorMapZoneVolume->EnvironmentAsset) ? true : false;

	T4AssetUtil::WorldUpdateMapZoneVolume(WorldAsset, EditorMapZoneVolume); // 저장!

	if (bZoneNameChanged)
	{
		ViewModelPtr->SelectEditorWorldMapZone(EditorTransientData.TransientMapZoneName); // WARN : ZoneName 이 변경되었으니 업데이트를 해주어야 한다.
	}

	AT4MapZoneVolume* PreviewMapZoneVolume = ViewModelPtr->GetMapZoneSelectedOnPreviewWorld();
	if (nullptr != PreviewMapZoneVolume)
	{
		T4AssetUtil::WorldUpdateMapZoneVolume(WorldAsset, PreviewMapZoneVolume);
	}
	else
	{
		ViewModelPtr->NotifyEditorWorldModified(); // #83, #92 : Editor / Preview World 가 틀릴 경우 화면에 노티!
	}
	if (MapZoneListWidgetPtr.IsValid())
	{
		MapZoneListWidgetPtr->SetInitializeValue(EditorTransientData.TransientMapZoneName);
		MapZoneListWidgetPtr->OnRefresh(false);
	}
	if (MapZoneDropListWidgetPtr.IsValid())
	{
		MapZoneDropListWidgetPtr->OnRefresh();
	}
	if (EnvironmentDetailPtr.IsValid())
	{
		EnvironmentDetailPtr->OnApplyEnvironmentAsset(
			EditorTransientData.TransientEnvironmentAsset.LoadSynchronous(),
			bZoneEnvironmentAssetChanged
		); // #94
	}
}

void FT4WorldDetailCustomization::HandleOnMapZoneSelected(const FName InName) // #92
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4WorldAsset* WorldAsset = ViewModelPtr->GetWorldAsset();
	if (nullptr == WorldAsset)
	{
		return;
	}
	ViewModelPtr->SelectEditorWorldMapZone(InName);
	{
		AT4MapZoneVolume* MapZoneVolume = ViewModelPtr->GetMapZoneSelectedOnEditorWorld();
		T4AssetUtil::WorldSelectMapZoneVolumeByName(WorldAsset, MapZoneVolume);
	}
	if (MapZoneDropListWidgetPtr.IsValid())
	{
		MapZoneDropListWidgetPtr->OnRefresh();
	}
	if (EnvironmentDetailPtr.IsValid())
	{
		const FT4WorldEditorTransientData& EditorTransientData = WorldAsset->EditorTransientData;
		EnvironmentDetailPtr->OnApplyEnvironmentAsset(
			EditorTransientData.TransientEnvironmentAsset.LoadSynchronous(),
			true
		); // #94
	}
}

FReply FT4WorldDetailCustomization::HandleOnEnvironmentAssetSave() // #90
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4WorldAsset* WorldAsset = ViewModelPtr->GetWorldAsset();
	if (nullptr == WorldAsset)
	{
		return FReply::Handled();
	}
	const FT4WorldEditorTransientData& EditorTransientData = WorldAsset->EditorTransientData;
	if (!EditorTransientData.TransientEnvironmentAsset.IsValid())
	{
		return FReply::Handled();
	}
	UT4EnvironmentAsset* EnvironmentAsset = EditorTransientData.TransientEnvironmentAsset.LoadSynchronous();
	if (nullptr == EnvironmentAsset)
	{
		return FReply::Handled();
	}
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EnviromentAssetSave(EnvironmentAsset, ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4WorldCustomDetailErrorTitleText
		);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
