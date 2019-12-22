// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4SubLevelListWidget.h"

#include "Engine/WorldComposition.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4SubLevelListWidget"

/**
  * #104
 */
ST4SubLevelListWidget::ST4SubLevelListWidget()
	: WorldRef(nullptr)
	, SubLevelListType(ET4SubLevelListType::SubLevelList_Loaded)
{
	bShowSearchButton = true;
	SelectionMode = ESelectionMode::Multi; // #104
}

ST4SubLevelListWidget::~ST4SubLevelListWidget()
{
}

void ST4SubLevelListWidget::Construct(
	const FArguments& InArgs,
	UWorld* InWorld,
	ET4SubLevelListType InSubLevelListType
)
{
	// #39
	OnMultiSelected = InArgs._OnMultiSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	WorldRef = InWorld;
	SubLevelListType = InSubLevelListType;

	Create();
}

void ST4SubLevelListWidget::SetLoadedLevels(const TSet<FName>& InLoadedLevels)
{
	LoadedLevels.Empty();
	for (const FName PackageName : InLoadedLevels)
	{
		LoadedLevels.Add(PackageName);
	}
}

void ST4SubLevelListWidget::UpdateLists()
{
	if (nullptr == WorldRef)
	{
		return;
	}

	if (nullptr != WorldRef->PersistentLevel) // #91
	{
		FString DisplayString;
		if (ET4SubLevelListType::SubLevelList_Tile == SubLevelListType)
		{
			DisplayString = FString::Printf(
				TEXT("[Persistent] %s (Preview: Loaded / Editor: Loaded)"),
				*(WorldRef->PersistentLevel->GetFName().ToString()),
				WorldRef->PersistentLevel->Actors.Num()
			);
		}
		else
		{
			DisplayString = FString::Printf(
				TEXT("[Persistent] %s (Actors:%u)"),
				*(WorldRef->PersistentLevel->GetFName().ToString()),
				WorldRef->PersistentLevel->Actors.Num()
			);
		}
		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = *DisplayString;
		NewItem->ValueName = PersistentLevelName;
		NewItem->SortOrder = -1;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (NewItem->ValueName == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}

	int32 NumCount = 1;
	UWorldComposition* WorldComposition = WorldRef->WorldComposition;
	if (ET4SubLevelListType::SubLevelList_Tile == SubLevelListType)
	{
		if (nullptr != WorldComposition)
		{
			UWorldComposition::FTilesList& WorldTileList = WorldComposition->GetTilesList();
			for (const FWorldCompositionTile& Tile : WorldTileList)
			{
				const FString LevelPackageAssetName = FPackageName::GetLongPackageAssetName(
					Tile.PackageName.ToString()
				);
				bool bPreviewLoaded = LoadedLevels.Contains(Tile.PackageName);
				bool bEditorLoaded = false;
				ULevelStreaming* EditorLevelLoaded = WorldRef->GetLevelStreamingForPackageName(Tile.PackageName);
				if (nullptr != EditorLevelLoaded)
				{
					bEditorLoaded = (nullptr != EditorLevelLoaded->GetLoadedLevel()) ? true : false;
				}
				FString Status = TEXT("");
				if (bPreviewLoaded || bEditorLoaded)
				{
					Status = FString::Printf(
						TEXT(" (Preview: %s / Editor: %s)"),
						(bPreviewLoaded) ? TEXT("Loaded") : TEXT("Not Loaded"),
						(bEditorLoaded) ? TEXT("Loaded") : TEXT("Not Loaded")
					);
				}
				FString DisplayString = FString::Printf(
					TEXT("[%s] %s%s"),
					*(Tile.Info.Layer.Name),
					*LevelPackageAssetName,
					*Status
				);
				int32 ZOrder = NumCount++;
				TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
				NewItem->DisplayString = *DisplayString;
				NewItem->ValueName = Tile.PackageName;
				NewItem->SortOrder = NumCount++;
				NewItem->SortOrder += (!bPreviewLoaded) ? 10000 : 0;
				NewItem->SortOrder += (!bEditorLoaded) ? 1000 : 0;
				ItemList.Add(NewItem);
				if (InitializeValue == NAME_None)
				{
					ItemSelected = NewItem;
					InitializeValue = NewItem->ValueName;
				}
				else if (NewItem->ValueName == InitializeValue)
				{
					ItemSelected = NewItem;
				}
			}
		}
	}
	else
	{
		const TArray<ULevelStreaming*>& StreamingLevels = WorldRef->GetStreamingLevels();
		for (ULevelStreaming* LevelStreaming : StreamingLevels)
		{
			const ULevel* LoadedLevel = LevelStreaming->GetLoadedLevel();
			if (nullptr == LoadedLevel)
			{
				continue;
			}
			FString DisplayString;
			const FString LevelPackageAssetName = FPackageName::GetLongPackageAssetName(
				LevelStreaming->PackageNameToLoad.ToString()
			);
			int32 ZOrder = 1;
			if (nullptr != WorldComposition)
			{
				FWorldTileInfo WorldTileInfo = WorldComposition->GetTileInfo(
					LevelStreaming->GetWorldAssetPackageFName()
				);
				DisplayString = FString::Printf(
					TEXT("[%s] %s (%s, Actors:%u)"),
					*LevelPackageAssetName,
					*(LevelStreaming->GetFName().ToString()),
					*(WorldTileInfo.Layer.Name),
					LoadedLevel->Actors.Num()
				);
				ZOrder = WorldTileInfo.ZOrder;
			}
			else
			{
				DisplayString = FString::Printf(
					TEXT("[%s] %s (Actors:%u)"),
					*LevelPackageAssetName,
					*(LevelStreaming->GetFName().ToString()),
					LoadedLevel->Actors.Num()
				);
				ZOrder = NumCount++;
			}
			TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
			NewItem->DisplayString = *DisplayString;
			NewItem->ValueName = LevelStreaming->PackageNameToLoad;
			NewItem->SortOrder = ZOrder;
			ItemList.Add(NewItem);
			if (InitializeValue == NAME_None)
			{
				ItemSelected = NewItem;
				InitializeValue = NewItem->ValueName;
			}
			else if (NewItem->ValueName == InitializeValue)
			{
				ItemSelected = NewItem;
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE