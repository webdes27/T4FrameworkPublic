// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EditorAnimSetAssetSelector.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EditorAnimSetAssetSelector"

/**
  * 
 */
FT4EditorAnimSetAssetSelector::FT4EditorAnimSetAssetSelector()
	: AnimSetAsset(nullptr)
{
}

FT4EditorAnimSetAssetSelector::~FT4EditorAnimSetAssetSelector()
{
	Reset();
}

void FT4EditorAnimSetAssetSelector::Reset()
{
	GetOnAnimSetChanged().Broadcast(nullptr);
	AnimSetAsset = nullptr;
}

void FT4EditorAnimSetAssetSelector::SetAnimSetAsset(
	UT4AnimSetAsset* InAnimSetAsset, 
	bool bForceUpdate
)
{
	if (!bForceUpdate && AnimSetAsset == InAnimSetAsset)
	{
		return;
	}
	AnimSetAsset = InAnimSetAsset;
	GetOnAnimSetChanged().Broadcast(InAnimSetAsset);
}

#undef LOCTEXT_NAMESPACE