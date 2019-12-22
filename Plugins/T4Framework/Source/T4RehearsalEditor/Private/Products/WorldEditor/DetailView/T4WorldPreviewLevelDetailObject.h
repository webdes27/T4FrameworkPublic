// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4WorldPreviewLevelDetailObject.generated.h"

/**
  * #85
 */
UCLASS()
class UT4WorldPreviewLevelDetailObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// Tile long package name (readonly)	
	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	FName							SubLevel;

	// Tile long package name (readonly)	
	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	FName							PackageName;
	
	// Parent tile long package name	
	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	FName							ParentPackageName;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	FName							LayerName;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	float							StreamingDistance;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	bool							DistanceStreamingEnabled;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	int32							Actors;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	FVector2D						BoundExtent;

	// Tile position in the world, relative to parent 
	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	FIntVector						Position;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	FIntVector						AbsolutePosition;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	int32							LODNums;

	UPROPERTY(VisibleAnywhere, Category = "Level Details")
	int32							LODIndex;
};
