// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4EngineInternal.h"

#include "Object/T4GameObject.h"
#include "Public/T4Engine.h"

namespace T4EnginetInternal
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