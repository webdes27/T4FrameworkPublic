// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4CommonAssetStructs.generated.h"

/**
  * #100
 */
USTRUCT()
struct T4ASSET_API FT4EditorPointOfInterest
{
	GENERATED_USTRUCT_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Name; // #103

	UPROPERTY(EditAnywhere, Category = Editor)
	FName MapEntityName;

	UPROPERTY(EditAnywhere, Category = Editor)
	float GameTimeHour;

	UPROPERTY(EditAnywhere, Category = Editor)
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, Category = Editor)
	FRotator SpawnRotation;
#endif

public:
	FT4EditorPointOfInterest()
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
struct T4ASSET_API FT4EditorTestAutomation
{
	GENERATED_USTRUCT_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	TArray<FT4EditorPointOfInterest> PointOfInterests;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "POI Name"))
	FString TransientName; // #103
#endif
};
