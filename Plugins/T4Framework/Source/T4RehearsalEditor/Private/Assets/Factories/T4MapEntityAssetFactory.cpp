// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4MapEntityAssetFactory.h"
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
UT4MapEntityAssetFactory::UT4MapEntityAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4MapEntityAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4MapEntityAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4MapEntityAsset::StaticClass()));
	return NewObject<UT4MapEntityAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}