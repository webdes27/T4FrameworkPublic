// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4LevelActorListWidget.h"

#include "Engine/Classes/GameFramework/Actor.h"
#include "Engine/LevelStreaming.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4LevelActorListWidget"

/**
  * #90
 */
ST4LevelActorListWidget::ST4LevelActorListWidget()
	: WorldRef(nullptr)
	, LevelStreamingSelected(nullptr)
{
	bShowSearchButton = true;
	MaxHeight = 150; // #104
}

ST4LevelActorListWidget::~ST4LevelActorListWidget()
{
}

void ST4LevelActorListWidget::Construct(
	const FArguments& InArgs,
	UWorld* InWorld
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	WorldRef = InWorld;

	Create();
}

void ST4LevelActorListWidget::Reset()
{
	LevelNames.Empty();
	LevelRefs.Empty();
	LevelStreamingSelected = nullptr;
	InitializeValue = NAME_None;
	ItemSelected.Reset();
}

void ST4LevelActorListWidget::RefreshWorld(UWorld* InWorld) // #104
{
	WorldRef = InWorld;
	Reset();
}

void ST4LevelActorListWidget::SetSubLevelSelected(const TArray<FName>* InSubLevelNames) // #104
{
	Reset();
	if (nullptr == InSubLevelNames)
	{
		return;
	}
	if (InSubLevelNames->Contains(PersistentLevelName)) // #91
	{
		LevelNames.Add(PersistentLevelName);
		LevelRefs.Add(WorldRef->PersistentLevel);
	}
	const TArray<ULevelStreaming*>& StreamingLevels = WorldRef->GetStreamingLevels();
	for (ULevelStreaming* LevelStreaming : StreamingLevels)
	{
		const ULevel* LoadedLevel = LevelStreaming->GetLoadedLevel();
		if (nullptr != LoadedLevel)
		{
			if (InSubLevelNames->Contains(LevelStreaming->PackageNameToLoad))
			{
				LevelNames.Add(LevelStreaming->PackageNameToLoad);
				LevelRefs.Add(LoadedLevel);
				if (nullptr == LevelStreamingSelected)
				{
					LevelStreamingSelected = LevelStreaming;
				}
			}
		}
	}
}

void ST4LevelActorListWidget::UpdateLists()
{
	if (nullptr == WorldRef)
	{
		return;
	}
	if (0 >= LevelRefs.Num())
	{
		return;
	}
	int32 NumLevels = 0;
	int32 NumActors = 0;
	for (const ULevel* Level : LevelRefs)
	{
		const FString AssetName = FPackageName::GetLongPackageAssetName(
			LevelNames[NumLevels++].ToString()
		);
		for (auto ActorIt = Level->Actors.CreateConstIterator(); ActorIt; ++ActorIt)
		{
			const AActor* LevelActor = (*ActorIt);
			if (nullptr != LevelActor)
			{
				FString DisplayString = FString::Printf(
					TEXT("[%s] (%s) %s"),
					*AssetName,
					*LevelActor->GetClass()->GetName(),
					*(LevelActor->GetFName().ToString())
				);
				TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
				NewItem->DisplayString = *DisplayString;
				NewItem->ValueName = LevelActor->GetFName();
				NewItem->SortOrder = NumActors++;
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
}

AActor* ST4LevelActorListWidget::GetActorSelected(FVector& OutLocation, FBox& OutBoundBox)
{
	if (nullptr == WorldRef)
	{
		return nullptr;
	}
	if (!ItemSelected.IsValid())
	{
		return nullptr;
	}
	for (TActorIterator<AActor> It(WorldRef); It; ++It) // TODO : Optimizeing!!!
	{
		AActor* SpawnActor = *It;
		if (SpawnActor->GetFName() == ItemSelected->ValueName)
		{
			OutLocation = SpawnActor->GetActorLocation();
			OutBoundBox = SpawnActor->GetComponentsBoundingBox();
			return SpawnActor;
		}
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE