// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4CommonAssetStructs.generated.h"

/**
  * #100
 */
USTRUCT()
struct T4ASSET_API FT4EditorPointOfInterestData
{
	GENERATED_USTRUCT_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Name; // #103

	UPROPERTY(EditAnywhere, Category = Hide)
	FName MapEntityName;

	UPROPERTY(EditAnywhere, Category = Hide)
	float GameTimeHour;

	UPROPERTY(EditAnywhere, Category = Hide)
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, Category = Hide)
	FRotator SpawnRotation;
#endif

public:
	FT4EditorPointOfInterestData()
#if WITH_EDITORONLY_DATA
		: MapEntityName(NAME_None)
		, GameTimeHour(12.0f)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
#endif
	{
	}
};

USTRUCT()
struct T4ASSET_API FT4EditorTestAutomationData
{
	GENERATED_USTRUCT_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	TArray<FT4EditorPointOfInterestData> PointOfInterests;
#endif
};
