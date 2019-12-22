// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "Products/Common/Widgets/ListView/ST4ListViewWidget.h"

/**
  * #90
 */
class UWorld;
class ST4MapZoneListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4MapZoneListWidget) {}
		SLATE_EVENT(FT4OnSelected, OnSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
		SLATE_ARGUMENT(FName, InInitializeValue) // #92
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		UWorld* InWorld
	);

	ST4MapZoneListWidget();
	~ST4MapZoneListWidget();

	bool HasMapZoneName(FName InName) const; // #92

protected:
	void UpdateLists() override;

private:
	UWorld* WorldRef;
};
