// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WeaponEntityAssetFactory.h"
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
UT4WeaponEntityAssetFactory::UT4WeaponEntityAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4WeaponEntityAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4WeaponEntityAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4WeaponEntityAsset::StaticClass()));
	return NewObject<UT4WeaponEntityAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}