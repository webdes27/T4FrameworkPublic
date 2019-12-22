// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiAssetFactory.h"
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #24
 */
UT4ContiAssetFactory::UT4ContiAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4ContiAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4ContiAssetFactory::FactoryCreateNew(
	UClass* Class, 
	UObject* InParent, 
	FName Name, 
	EObjectFlags Flags, 
	UObject* Context, 
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4ContiAsset::StaticClass()));
	return NewObject<UT4ContiAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}