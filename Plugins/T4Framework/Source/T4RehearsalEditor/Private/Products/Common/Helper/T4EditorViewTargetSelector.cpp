// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EditorViewTargetSelector.h"

#include "T4Engine/Public/T4Engine.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EditorViewTargetSelector"

/**
  * 
 */
FT4EditorViewTargetSelector::FT4EditorViewTargetSelector()
	: ViewTarget(nullptr)
{
}

FT4EditorViewTargetSelector::~FT4EditorViewTargetSelector()
{
	Reset();
}

void FT4EditorViewTargetSelector::Reset()
{
	GetOnViewTargetChanged().Broadcast(nullptr);
	ViewTarget = nullptr;
}

void FT4EditorViewTargetSelector::SetViewTarget(
	IT4GameObject* InViewTarget,
	bool bForceUpdate
)
{
	if (!bForceUpdate && ViewTarget == InViewTarget)
	{
		return;
	}
	ViewTarget = InViewTarget;
	GetOnViewTargetChanged().Broadcast(InViewTarget);
}

#undef LOCTEXT_NAMESPACE