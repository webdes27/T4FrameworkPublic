// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4CharacterEntityAssetFactory.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */

UT4CharacterEntityAssetFactory::UT4CharacterEntityAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4CharacterEntityAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4CharacterEntityAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4CharacterEntityAsset::StaticClass()));
	return NewObject<UT4CharacterEntityAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}