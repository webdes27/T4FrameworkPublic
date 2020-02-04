// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldDefaultObject.h"

#include "Object/Component/T4SceneComponent.h"
#include "Object/ActionNode/T4ActionNodeControl.h" // #20

#include "T4EngineInternal.h"

/** 
  * #54
 */
AT4WorldDefaultObject::AT4WorldDefaultObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SceneComponent(nullptr)
	, bAutoDestroy(false) // #68 : WorldObject 만 받는다.
{
	SceneComponent = CreateDefaultSubobject<UT4SceneComponent>(TEXT("T4SceneComponent"));
	RootComponent = SceneComponent;
}

AT4WorldDefaultObject::~AT4WorldDefaultObject()
{
}

void AT4WorldDefaultObject::Reset()
{
}

void AT4WorldDefaultObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AT4WorldDefaultObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
{
}

void AT4WorldDefaultObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
	if (!IsGhost() && bAutoDestroy) // #68 : WorldObject 만 받는다.
	{
		if (0 >= GetActionControl()->NumChildActions())
		{
			GetGameWorld()->GetContainer()->DestroyClientObject(GetObjectID());
		}
	}
}

USceneComponent* AT4WorldDefaultObject::GetAttachParentComponent() // #54
{
	return Cast<USceneComponent>(SceneComponent);
}