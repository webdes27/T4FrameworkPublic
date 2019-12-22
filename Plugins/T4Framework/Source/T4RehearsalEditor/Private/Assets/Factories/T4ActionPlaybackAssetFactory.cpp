// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionPlaybackAssetFactory.h"

#include "T4Engine/Classes/Playback/T4ActionPlaybackAsset.h" // #68

#include "T4RehearsalEditorInternal.h"

/**
  * #39
 */
UT4ActionPlaybackAssetFactory::UT4ActionPlaybackAssetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UT4ActionPlaybackAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UT4ActionPlaybackAssetFactory::FactoryCreateNew(
	UClass* Class, 
	UObject* InParent, 
	FName Name, 
	EObjectFlags Flags, 
	UObject* Context, 
	FFeedbackContext* Warn
)
{
	check(Class->IsChildOf(UT4ActionPlaybackAsset::StaticClass()));
	return NewObject<UT4ActionPlaybackAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}