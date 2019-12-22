// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldPreviewLevelDetailObject.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #85
 */
UT4WorldPreviewLevelDetailObject::UT4WorldPreviewLevelDetailObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SubLevel(NAME_None)
	, PackageName(NAME_None)
	, ParentPackageName(NAME_None)
	, LayerName(NAME_None)
	, StreamingDistance(0.0f)
	, DistanceStreamingEnabled(true)
	, Actors(0)
	, BoundExtent(FVector2D::ZeroVector)
	, Position(FIntVector::ZeroValue)
	, AbsolutePosition(FIntVector::ZeroValue)
	, LODNums(0)
	, LODIndex(0)
{
}