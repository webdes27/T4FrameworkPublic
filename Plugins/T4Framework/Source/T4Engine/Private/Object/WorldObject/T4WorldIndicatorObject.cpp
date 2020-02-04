// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldIndicatorObject.h"

#include "Object/Component/T4SceneComponent.h"
#include "Object/ActionNode/T4ActionNodeControl.h" // #20

#include "T4EngineInternal.h"

/** 
  * #54
 */
AT4WorldIndicatorObject::AT4WorldIndicatorObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SceneComponent(nullptr)
	, bAutoDestroy(false) // #68 : WorldObject 만 받는다.
{
	SceneComponent = CreateDefaultSubobject<UT4SceneComponent>(TEXT("T4SceneComponent"));
	RootComponent = SceneComponent;
}

AT4WorldIndicatorObject::~AT4WorldIndicatorObject()
{
}

void AT4WorldIndicatorObject::Reset()
{
}

void AT4WorldIndicatorObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AT4WorldIndicatorObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
{
}

void AT4WorldIndicatorObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
	if (!IsGhost() && bAutoDestroy) // #68 : WorldObject 만 받는다.
	{
		if (0 >= GetActionControl()->NumChildActions())
		{
			GetGameWorld()->GetContainer()->DestroyClientObject(GetObjectID());
		}
	}
}

USceneComponent* AT4WorldIndicatorObject::GetAttachParentComponent() // #54
{
	return Cast<USceneComponent>(SceneComponent);
}

bool AT4WorldIndicatorObject::ExecuteTeleportToAction(const FT4TeleportAction& InAction) // #117
{
	SetActorLocation(InAction.TargetLocation);
	return true;
}