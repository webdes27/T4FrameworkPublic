// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "Products/Common/Widgets/ListView/ST4ListViewWidget.h"

/**
  * #104
 */
class UWorld;
class ULevel;
class ULevelStreaming;
class ST4LevelActorListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4LevelActorListWidget) {}
		SLATE_EVENT(FT4OnSelected, OnSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, UWorld* InWorld);

	ST4LevelActorListWidget();
	~ST4LevelActorListWidget();

	void RefreshWorld(UWorld* InWorld); // #104
	void SetSubLevelSelected(const TArray<FName>* InSubLevelNames); // #104

	const ULevelStreaming* GetLevelStreamingSelected() const { return LevelStreamingSelected; }

	AActor* GetActorSelected(FVector& OutLocation, FBox& OutBoundBox);

protected:
	void Reset();
	void UpdateLists() override;

private:
	UWorld* WorldRef;

	TArray<FName> LevelNames;
	TArray<const ULevel*> LevelRefs;
	const ULevelStreaming* LevelStreamingSelected;
};
