// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AnimSetAssetFactory.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #39
 */
UT4AnimSetAssetFactory::UT4AnimSetAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4AnimSetAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4AnimSetAssetFactory::FactoryCreateNew(
	UClass* Class, 
	UObject* InParent, 
	FName Name, 
	EObjectFlags Flags, 
	UObject* Context, 
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4AnimSetAsset::StaticClass()));
	return NewObject<UT4AnimSetAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}