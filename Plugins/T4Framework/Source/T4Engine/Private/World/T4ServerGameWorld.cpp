// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ServerGameWorld.h"

#if (WITH_EDITOR || WITH_SERVER_CODE)
#include "Public/T4Engine.h"

#include "Engine/World.h"
#include "NavigationSystem.h"

#include "T4EngineInternal.h"
#endif

/**
  *
  */
#if (WITH_EDITOR || WITH_SERVER_CODE)
FT4ServerGameWorld::FT4ServerGameWorld()
{
}

FT4ServerGameWorld::~FT4ServerGameWorld()
{
}

void FT4ServerGameWorld::Create()
{
	// #34 : TODO 서버용 자료구조 추가!!
}

void FT4ServerGameWorld::Reset()
{
}

void FT4ServerGameWorld::ProcessPre(float InDeltaTime)
{
}

void FT4ServerGameWorld::ProcessPost(float InDeltaTime)
{
}

#endif