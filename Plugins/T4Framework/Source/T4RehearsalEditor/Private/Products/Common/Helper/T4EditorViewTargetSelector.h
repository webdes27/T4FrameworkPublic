// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  *
 */
class IT4GameObject;
class FT4EditorViewTargetSelector
	: public TSharedFromThis<FT4EditorViewTargetSelector>
{
public:
	FT4EditorViewTargetSelector();
	~FT4EditorViewTargetSelector();

	void Reset();

	bool IsNull() const { return (nullptr == ViewTarget) ? true : false; }

	void SetViewTarget(IT4GameObject* InViewTarget, bool bForceUpdate);
	IT4GameObject* GetViewTarget() const { return ViewTarget; }

	DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnViewTargetChanged, IT4GameObject*);

	FT4OnViewTargetChanged& GetOnViewTargetChanged()
	{
		return OnViewTargetChanged;
	}

private:
	IT4GameObject* ViewTarget;
	FT4OnViewTargetChanged OnViewTargetChanged;
};
