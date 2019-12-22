// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "ST4ListViewWidget.h"

/**
  * #90
 */
class UT4EnvironmentAsset;
class ST4TimeTagListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4TimeTagListWidget) {}
		SLATE_EVENT(FT4OnSelected, OnSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		const UT4EnvironmentAsset* InEnvironmentAsset
	);

	ST4TimeTagListWidget();
	~ST4TimeTagListWidget();

	void SetEnvironmentAsset(const UT4EnvironmentAsset* InEnvironmentAsset); // #94

protected:
	void UpdateLists() override;

private:
	TWeakObjectPtr<const UT4EnvironmentAsset> EnvironmentAssetPtr;
};
