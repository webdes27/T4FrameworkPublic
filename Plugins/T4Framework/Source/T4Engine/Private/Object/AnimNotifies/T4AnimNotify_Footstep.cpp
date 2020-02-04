// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/AnimNotifies/T4AnimNotify_Footstep.h"

#include "Object/Component/T4SkeletalMeshComponent.h"

#include "Public/T4Engine.h"
#include "Public/T4EngineAnimNotify.h" // #111

#include "T4EngineInternal.h"

/**
  * #111 : refer AnimNotify_PlayParticleEffect.h
 */
UT4AnimNotify_Footstep::UT4AnimNotify_Footstep()
	: Super()
	, FootstepType(ET4FootstepType::None)
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(192, 255, 99, 255);
#endif // WITH_EDITORONLY_DATA
}

void UT4AnimNotify_Footstep::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void UT4AnimNotify_Footstep::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UT4AnimNotify_Footstep::ValidateAssociatedAssets()
{
	static const FName NAME_AssetCheck("AssetCheck");
}
#endif

void UT4AnimNotify_Footstep::Notify(
	USkeletalMeshComponent* InMeshComp,
	UAnimSequenceBase* InAnimation
)
{
}

FString UT4AnimNotify_Footstep::GetNotifyName_Implementation() const
{
	return Super::GetNotifyName_Implementation();
}
