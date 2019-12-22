// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4CostumeEntityAssetFactory.h"
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
UT4CostumeEntityAssetFactory::UT4CostumeEntityAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4CostumeEntityAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4CostumeEntityAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4CostumeEntityAsset::StaticClass()));
	return NewObject<UT4CostumeEntityAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}