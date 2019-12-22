// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldAssetFactory.h"
#include "T4Asset/Classes/World/T4WorldAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #83
 */
UT4WorldAssetFactory::UT4WorldAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4WorldAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4WorldAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4WorldAsset::StaticClass()));
	return NewObject<UT4WorldAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}