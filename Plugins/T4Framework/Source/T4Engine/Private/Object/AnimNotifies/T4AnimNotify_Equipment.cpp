// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/AnimNotifies/T4AnimNotify_Equipment.h"

#include "Object/Component/T4SkeletalMeshComponent.h"

#include "Public/T4Engine.h"
#include "Public/T4EngineAnimNotify.h" // #111

#include "Animation/AnimSequenceBase.h"

#include "T4EngineInternal.h"

/**
  * #111 : refer AnimNotify_PlayParticleEffect.h
 */
UT4AnimNotify_Equipment::UT4AnimNotify_Equipment()
	: Super()
	, EquipmentType(ET4EquipmentType::None)
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(196, 142, 255, 255);
#endif // WITH_EDITORONLY_DATA
}

void UT4AnimNotify_Equipment::Notify(
	USkeletalMeshComponent* InMeshComp, 
	UAnimSequenceBase* InAnimation
)
{
	UT4SkeletalMeshComponent* SkeletalMeshComponent = Cast<UT4SkeletalMeshComponent>(InMeshComp);
	if (nullptr == SkeletalMeshComponent)
	{
		return;
	}
	ET4LayerType LayerType = SkeletalMeshComponent->GetLayerType();
	if (ET4LayerType::Max == LayerType)
	{
		return;
	}
	const FT4ObjectID& OwnerObjectID = SkeletalMeshComponent->GetOwnerObjectID();
	if (!OwnerObjectID.IsValid())
	{
		return;
	}
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	if (nullptr == GameWorld)
	{
		return;
	}
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	IT4GameObject* OwnerObject = WorldContainer->FindGameObject(OwnerObjectID);
	if (nullptr == OwnerObject)
	{
		return;
	}
	FT4AnimNotifyEquipment NewMessage;
	NewMessage.EquipmentType = EquipmentType;
	NewMessage.SameStanceName = OwnerObject->GetStanceName();
#if WITH_EDITOR
	NewMessage.DebugSting = InAnimation->GetFName().ToString();
#endif
	OwnerObject->OnAnimNotifyMessage(&NewMessage);
}

FString UT4AnimNotify_Equipment::GetNotifyName_Implementation() const
{
	return Super::GetNotifyName_Implementation();
}

#if WITH_EDITOR
void UT4AnimNotify_Equipment::ValidateAssociatedAssets()
{
	static const FName NAME_AssetCheck("AssetCheck");
}
#endif
