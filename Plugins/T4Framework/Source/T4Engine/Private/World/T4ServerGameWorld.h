// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if (WITH_EDITOR || WITH_SERVER_CODE)
#include "World/T4GameWorld.h"
#endif

/**
  *
  */
#if (WITH_EDITOR || WITH_SERVER_CODE)

class FT4ServerGameWorld : public FT4GameWorld
{
public:
	explicit FT4ServerGameWorld();
	virtual ~FT4ServerGameWorld();

	// IT4GameWorld
	virtual ET4WorldType GetType() const override { return ET4WorldType::Server; }

protected:
	virtual void Create() override;
	virtual void Reset() override;

	virtual void ProcessPre(float InDeltaTime);
	virtual void ProcessPost(float InDeltaTime);
};

#endif