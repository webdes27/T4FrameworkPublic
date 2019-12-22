// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldPreviewDetailCustomization.h"
#include "ST4WorldPreviewObjectWidget.h" // #85
#include "T4WorldPreviewLevelDetailObject.h" // #85

#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/WorldEditor/ViewModel/T4WorldViewModel.h"

#include "Products/WorldEditor/Widgets/ST4SubLevelListWidget.h" // #104
#include "Products/WorldEditor/Widgets/ST4LevelActorListWidget.h" // #104

#include "T4Asset/Classes/World/T4WorldAsset.h"

#include "Widgets/Layout/SScrollBox.h"

#include "Engine/LevelStreaming.h"
#include "Engine/WorldComposition.h"
#include "Engine/World.h"

#include "Modules/ModuleManager.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailsView.h"
#include "IDetailGroup.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4WorldPreviewDetailCustomization"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */

TSharedRef<IDetailCustomization> FT4WorldPreviewDetailCustomization::MakeInstance(
	TSharedRef<ST4WorldPreviewObjectWidget> InWorldLevelObjectWidget // #85
)
{
	TSharedPtr<FT4WorldViewModel> WorldViewModel = InWorldLevelObjectWidget->GetWorldViewModel();
	TSharedRef<FT4WorldPreviewDetailCustomization> NewDetailCustomization = MakeShared<FT4WorldPreviewDetailCustomization>(WorldViewModel);
	InWorldLevelObjectWidget->SetDetailCustomization(NewDetailCustomization);
	return NewDetailCustomization;
}

FT4WorldPreviewDetailCustomization::FT4WorldPreviewDetailCustomization(
	TSharedPtr<FT4WorldViewModel> InWorldViewModel
)	: ViewModelPtr(InWorldViewModel)
	, DetailLayoutRef(nullptr)
{
}

void FT4WorldPreviewDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}

	DetailLayoutRef = &InBuilder;

	UT4WorldPreviewLevelDetailObject* LevelDetailObject = ViewModelPtr->GetWorldLevelDetailObject();
	if (nullptr == LevelDetailObject)
	{
		return;
	}

	const bool bWorldCompositionEnabled = ViewModelPtr->IsWorldCompositionEnabled(); // #91

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, SubLevel);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, PackageName);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, ParentPackageName);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, LayerName);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, StreamingDistance);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, DistanceStreamingEnabled);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, Actors);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, BoundExtent);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, Position);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, AbsolutePosition);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, LODNums);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WorldPreviewLevelDetailObject, LODIndex);

	UWorld* PreviewWorld = ViewModelPtr->GetPreviewWorld();
	check(nullptr != PreviewWorld);

	{
		// #85 : Loaded Sub Levels
		static const FName ActiveLevelCategoryName = TEXT("Loaded Levels");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(ActiveLevelCategoryName);

		LoadedSubLevelListWidgetPtr = SNew(ST4SubLevelListWidget, PreviewWorld, ET4SubLevelListType::SubLevelList_Loaded)
			.OnMultiSelected(this, &FT4WorldPreviewDetailCustomization::HandleOnLoadedLevelMultiSelected)
			.OnDoubleClicked(nullptr);

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4WorldPreviewDetailLoadedSubLevelList", "Preview Loaded Levels"))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				LoadedSubLevelListWidgetPtr.ToSharedRef()
			];

		if (ViewModelPtr->IsWorldCompositionEnabled())
		{
			DetailCategoryBuilder
				.AddCustomRow(LOCTEXT("T4WorldPreviewDetailLoadedSubLevelButton", "LoadedSubLevelButton"))
				.WholeRowContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("T4WorldPreviewDetailUnloadLevelBtn", "Unload selected level from world"))
						.ToolTipText(LOCTEXT("T4WorldPreviewDetailUnloadLevelBtnBtn_Tooltip", "Unload selected level from world"))
						.OnClicked(this, &FT4WorldPreviewDetailCustomization::HandleOnUnloadSelelectedLevel)
					]
				];
		}

		IDetailGroup& LevelDetailGroup = DetailCategoryBuilder.AddGroup(FName("Level Details"), FText::FromString("Level Details"), false);

		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, SubLevel);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, PackageName);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, ParentPackageName);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, LayerName);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, StreamingDistance);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, DistanceStreamingEnabled);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, Actors);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, BoundExtent);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, Position);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, AbsolutePosition);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, LODNums);
		DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(LevelDetailGroup, LODIndex);
	}

	{
		// #85 : Level Actors
		static const FName SubLevelActorsCategoryName = TEXT("Loaded Level Actors");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(SubLevelActorsCategoryName);

		LevelActorListWidgetPtr = SNew(ST4LevelActorListWidget, PreviewWorld)
			.OnSelected(this, &FT4WorldPreviewDetailCustomization::HandleOnLevelActorSelected)
			.OnDoubleClicked(this, &FT4WorldPreviewDetailCustomization::HandleOnLevelActorDoubleClicked);

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4WorldPreviewDetailLoadedSubLevelList", "Loaded Level Actors"))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				LevelActorListWidgetPtr.ToSharedRef()
			];

		// #85 : Selected Actor
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);

		LevelActorDetailsViewPtr = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		LevelActorDetailsViewPtr->SetObject(nullptr);

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4WorldPreviewDetailCustomizationActorDetail", "ActorDetail"))
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
								LevelActorDetailsViewPtr.ToSharedRef()
							]
						]
					]
				]
			];
	}
	{
		// #104 : 첫번째 레벨, 첫번째 레벨 Actors 를 자동 선택하도록 처리한다.
		LoadedSubLevelListWidgetPtr->SetInitializeValue(PersistentLevelName);
		LoadedSubLevelListWidgetPtr->OnRefresh(true);
	}
}

void FT4WorldPreviewDetailCustomization::OnRefreshWorld() // #85
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UWorld* PreviewWorld = ViewModelPtr->GetPreviewWorld();
	check(nullptr != PreviewWorld);
	if (LoadedSubLevelListWidgetPtr.IsValid())
	{
		LoadedSubLevelListWidgetPtr->RefreshWorld(PreviewWorld);
	}
	if (LevelActorListWidgetPtr.IsValid())
	{
		LevelActorListWidgetPtr->RefreshWorld(PreviewWorld);
	}
	UpdateLevelDetailObject(nullptr);
	LevelActorDetailsViewPtr->SetObject(nullptr);
	LevelActorListWidgetPtr->SetSubLevelSelected(nullptr);
	LoadedSubLevelListWidgetPtr->OnRefresh(true); // LevelDetailObject 도 자동 선택되도록 true 설정
}

void FT4WorldPreviewDetailCustomization::UpdateLevelDetailObject(
	const ULevelStreaming* InLevelStreaming
) // #85
{
	UWorld* PreviewWorld = ViewModelPtr->GetPreviewWorld();
	check(nullptr != PreviewWorld);

	UT4WorldPreviewLevelDetailObject* LevelDetailObject = ViewModelPtr->GetWorldLevelDetailObject();
	check(nullptr != LevelDetailObject);

	if (nullptr == InLevelStreaming) // #91
	{
		LevelDetailObject->PackageName = PersistentLevelName;
		LevelDetailObject->Actors = PreviewWorld->PersistentLevel->Actors.Num();

		LevelDetailObject->BoundExtent = FVector2D(0, 0);
		LevelDetailObject->SubLevel = NAME_None;
		LevelDetailObject->ParentPackageName = NAME_None;
		LevelDetailObject->Position = FIntVector(0);
		LevelDetailObject->LayerName = NAME_None;
		LevelDetailObject->AbsolutePosition = FIntVector(0);
		LevelDetailObject->LODNums = 0;
		LevelDetailObject->LODIndex = -1;
	}
	else
	{
		UWorldComposition* WorldComposition = PreviewWorld->WorldComposition;
		check(nullptr != WorldComposition);
		FWorldTileInfo WorldTileInfo = WorldComposition->GetTileInfo(InLevelStreaming->GetWorldAssetPackageFName());
		LevelDetailObject->SubLevel = *FPackageName::GetLongPackageAssetName(InLevelStreaming->PackageNameToLoad.ToString());
		LevelDetailObject->PackageName = InLevelStreaming->PackageNameToLoad;
		LevelDetailObject->ParentPackageName = *WorldTileInfo.ParentTilePackageName;
		const FVector BoundSize = WorldTileInfo.Bounds.GetSize();
		LevelDetailObject->BoundExtent = FVector2D(FMath::RoundToInt(BoundSize.X*0.5f), FMath::RoundToInt(BoundSize.Y*0.5f));
		LevelDetailObject->Position = WorldTileInfo.Position;
		LevelDetailObject->LayerName = *WorldTileInfo.Layer.Name;
		LevelDetailObject->StreamingDistance = WorldTileInfo.GetStreamingDistance(InLevelStreaming->GetLevelLODIndex());
		LevelDetailObject->DistanceStreamingEnabled = WorldTileInfo.Layer.DistanceStreamingEnabled;
		LevelDetailObject->Actors = 0;
		if (const ULevel* LoadedLevel = InLevelStreaming->GetLoadedLevel())
		{
			LevelDetailObject->Actors = LoadedLevel->Actors.Num();
		}
		LevelDetailObject->AbsolutePosition = WorldTileInfo.AbsolutePosition;
		LevelDetailObject->LODNums = InLevelStreaming->LODPackageNames.Num();
		LevelDetailObject->LODIndex = InLevelStreaming->GetLevelLODIndex();
	}
}

void FT4WorldPreviewDetailCustomization::HandleOnRefreshLayout()
{
	if (nullptr != DetailLayoutRef)
	{
		DetailLayoutRef->ForceRefreshDetails();
	}
}

FReply FT4WorldPreviewDetailCustomization::HandleOnUnloadSelelectedLevel() // #104
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
	ViewModelPtr->UnloadPreviewWorldSubLevel(ItemMultiSelected);
	return FReply::Handled();
}

void FT4WorldPreviewDetailCustomization::HandleOnLoadedLevelMultiSelected(
	const TArray<FName>& InSubLevelNames
)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	if (!LoadedSubLevelListWidgetPtr.IsValid())
	{
		return;
	}
	if (LevelActorListWidgetPtr.IsValid())
	{
		LevelActorListWidgetPtr->SetSubLevelSelected(&InSubLevelNames);
		const ULevelStreaming* LevelStreamingSelected = LevelActorListWidgetPtr->GetLevelStreamingSelected();
		UpdateLevelDetailObject(LevelStreamingSelected);
		LevelActorListWidgetPtr->OnRefresh(true);
	}
	ViewModelPtr->SelectSubLevelOfWorldMap(InSubLevelNames);
}

void FT4WorldPreviewDetailCustomization::HandleOnLevelActorSelected(
	const FName InName
) // #85
{
	if (!LevelActorListWidgetPtr.IsValid())
	{
		return;
	}
	FVector Location;
	FBox BoundBox;
	AActor* FoundSpawnActor = LevelActorListWidgetPtr->GetActorSelected(Location, BoundBox);
	if (nullptr != FoundSpawnActor)
	{
		ViewModelPtr->SetPreviewWorldActorSelected(Location, BoundBox);
		LevelActorDetailsViewPtr->SetObject((UObject*)FoundSpawnActor);
	}
	else
	{
		LevelActorDetailsViewPtr->SetObject(nullptr);
	}
}

void FT4WorldPreviewDetailCustomization::HandleOnLevelActorDoubleClicked(
	const FName InName
) // #85
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	FVector Location;
	FBox BoundBox;
	AActor* FoundSpawnActor = LevelActorListWidgetPtr->GetActorSelected(Location, BoundBox);
	if (nullptr != FoundSpawnActor)
	{
		ViewModelPtr->SetPreviewWorldUpdateCamera(Location, BoundBox, true);
		LevelActorDetailsViewPtr->SetObject((UObject*)FoundSpawnActor);
	}
	else
	{
		LevelActorDetailsViewPtr->SetObject(nullptr);
	}
}

#undef LOCTEXT_NAMESPACE
