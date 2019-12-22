// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldContextGame.h"

#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #87

#include "T4EngineInternal.h"

/**
  * #87
 */
FT4WorldContextGame::FT4WorldContextGame(FT4WorldController* InWorldController)
	: WorldControllerRef(InWorldController)
{
}

FT4WorldContextGame::~FT4WorldContextGame()
{
}

void FT4WorldContextGame::Reset()
{
}

void FT4WorldContextGame::ProcessPre(float InDeltaTime) // #34 : OnWorldPreActorTick
{

}

void FT4WorldContextGame::ProcessPost(float InDeltaTime) // #34 : OnWorldPostActorTick
{

}

bool FT4WorldContextGame::WorldTravel(
	const FSoftObjectPath& InAssetPath, 
	const FVector& InStartLocation
)
{
	UWorld* UnrealWorld = WorldControllerRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	const FString LongPackageName = InAssetPath.GetLongPackageName();
	GEngine->SetClientTravel(UnrealWorld, *LongPackageName, TRAVEL_Absolute);
	return true;
}