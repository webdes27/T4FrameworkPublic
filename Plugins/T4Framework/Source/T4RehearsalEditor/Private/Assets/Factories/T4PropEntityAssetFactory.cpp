// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PropEntityAssetFactory.h"
#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
UT4PropEntityAssetFactory::UT4PropEntityAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4PropEntityAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4PropEntityAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4PropEntityAsset::StaticClass()));
	return NewObject<UT4PropEntityAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}