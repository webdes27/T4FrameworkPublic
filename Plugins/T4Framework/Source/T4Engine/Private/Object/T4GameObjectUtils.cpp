// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EngineInternal.h"

#include "T4GameObject.h"
#include "T4Engine.h"

namespace T4EnginetUtil
{
	IT4GameObject* TryCastGameObject(AActor* InActor)
	{
		AT4GameObject* GameObject = Cast<AT4GameObject>(InActor);
		if (nullptr != GameObject)
		{
			return static_cast<IT4GameObject*>(GameObject);
		}
		return nullptr;
	}
}