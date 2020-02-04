// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ItemAnimControl.h"

#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ItemAnimControl::FT4ItemAnimControl(AT4GameObject* InGameObject)
	: FT4BaseAnimControl(InGameObject)
{
}

FT4ItemAnimControl::~FT4ItemAnimControl()
{
}

void FT4ItemAnimControl::Reset()
{
	// #38
}

void FT4ItemAnimControl::Advance(const FT4UpdateTime& InUpdateTime)
{
	check(bBegunPlay);
	check(OwnerObjectPtr.IsValid());

}