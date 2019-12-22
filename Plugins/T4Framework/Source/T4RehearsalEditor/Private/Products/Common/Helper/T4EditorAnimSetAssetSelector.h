// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  *
 */
class UT4AnimSetAsset;
class FT4EditorAnimSetAssetSelector
	: public TSharedFromThis<FT4EditorAnimSetAssetSelector>
{
public:
	FT4EditorAnimSetAssetSelector();
	~FT4EditorAnimSetAssetSelector();

	void Reset();

	bool IsNull() const { return (nullptr == AnimSetAsset) ? true : false; }

	void SetAnimSetAsset(UT4AnimSetAsset* InAnimSetAsset, bool bForceUpdate);
	UT4AnimSetAsset* GetAnimSetAsset() const { return AnimSetAsset; }

	DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnAnimSetChanged, UT4AnimSetAsset*);

	FT4OnAnimSetChanged& GetOnAnimSetChanged()
	{
		return OnAnimSetChanged;
	}

private:
	UT4AnimSetAsset* AnimSetAsset;
	FT4OnAnimSetChanged OnAnimSetChanged;
};
