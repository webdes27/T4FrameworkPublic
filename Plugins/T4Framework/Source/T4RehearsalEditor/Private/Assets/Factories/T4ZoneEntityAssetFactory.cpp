// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ZoneEntityAssetFactory.h"
#include "T4Asset/Classes/Entity/T4ZoneEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #94
 */
UT4ZoneEntityAssetFactory::UT4ZoneEntityAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4ZoneEntityAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4ZoneEntityAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4ZoneEntityAsset::StaticClass()));
	return NewObject<UT4ZoneEntityAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}