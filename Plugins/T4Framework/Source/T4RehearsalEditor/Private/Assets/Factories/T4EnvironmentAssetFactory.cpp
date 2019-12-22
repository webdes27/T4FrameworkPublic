// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EnvironmentAssetFactory.h"
#include "T4Asset/Classes/World/T4EnvironmentAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #90
 */
UT4EnvironmentAssetFactory::UT4EnvironmentAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4EnvironmentAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4EnvironmentAssetFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4EnvironmentAsset::StaticClass()));
	return NewObject<UT4EnvironmentAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}