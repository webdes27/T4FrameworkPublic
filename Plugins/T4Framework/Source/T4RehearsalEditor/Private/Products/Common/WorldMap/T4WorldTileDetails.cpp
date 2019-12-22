// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldTileDetails.h"

#include "UObject/UnrealType.h"


#define LOCTEXT_NAMESPACE "WorldMap"


/////////////////////////////////////////////////////
// FT4TileLODEntryDetails
FT4TileLODEntryDetails::FT4TileLODEntryDetails()
	// Initialize properties with default values from FWorldTileLODInfo
	: LODIndex(0)
	, Distance(FWorldTileLODInfo().RelativeStreamingDistance)
{
}

/////////////////////////////////////////////////////
// UT4WorldTileDetails
UT4WorldTileDetails::UT4WorldTileDetails(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LOD1.LODIndex = 0;
	LOD2.LODIndex = 1;
	LOD3.LODIndex = 2;
	LOD4.LODIndex = 3;
}

void UT4WorldTileDetails::SetInfo(const FWorldTileInfo& Info, ULevel* Level)
{
	ParentPackageName	= FName(*Info.ParentTilePackageName);
	Position			= Info.Position;
	AbsolutePosition	= Info.AbsolutePosition;
	Layer				= Info.Layer;
	Bounds				= Info.Bounds;
	ZOrder				= Info.ZOrder;
	bHideInTileView		= Info.bHideInTileView;

	// Sync LOD settings
	NumLOD				= Info.LODList.Num();
	FT4TileLODEntryDetails* LODEntries[WORLDTILE_LOD_MAX_INDEX] = {&LOD1, &LOD2, &LOD3, &LOD4};

	for (int32 i = 0; i < WORLDTILE_LOD_MAX_INDEX; ++i)
	{
		if (Info.LODList.IsValidIndex(i))
		{
			LODEntries[i]->Distance = Info.LODList[i].RelativeStreamingDistance;
			LODEntries[i]->SimplificationDetails = Level ? Level->LevelSimplification[i] : FLevelSimplificationDetails();
		}
		else
		{
			LODEntries[i]->Distance = FWorldTileLODInfo().RelativeStreamingDistance;
			LODEntries[i]->SimplificationDetails = FLevelSimplificationDetails();
		}
	}
}
	
FWorldTileInfo UT4WorldTileDetails::GetInfo() const
{
	FWorldTileInfo Info;
	
	Info.ParentTilePackageName	= ParentPackageName.ToString();
	Info.Position				= Position;
	Info.AbsolutePosition		= AbsolutePosition;
	Info.Layer					= Layer;
	Info.Bounds					= Bounds;
	Info.ZOrder					= ZOrder;
	Info.bHideInTileView		= bHideInTileView;

	// Sync LOD settings
	Info.LODList.SetNum(FMath::Clamp(NumLOD, 0, WORLDTILE_LOD_MAX_INDEX));
	const FT4TileLODEntryDetails* LODEntries[WORLDTILE_LOD_MAX_INDEX] = {&LOD1, &LOD2, &LOD3, &LOD4};
	for (int32 i = 0; i < Info.LODList.Num(); ++i)
	{
		Info.LODList[i].RelativeStreamingDistance = LODEntries[i]->Distance;
	}
	
	return Info;
}

void UT4WorldTileDetails::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, Position)
		|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, Position))
	{
		PositionChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, ParentPackageName))
	{
		ParentPackageNameChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, NumLOD))
	{
		LODSettingsChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD1)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD1))
	{
		LODSettingsChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD2)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD2))
	{
		LODSettingsChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD3)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD3))
	{
		LODSettingsChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD4)
			|| MemberPropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, LOD4))
	{
		LODSettingsChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, ZOrder))
	{
		ZOrderChangedEvent.Broadcast();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UT4WorldTileDetails, bHideInTileView))
	{
		HideInTileViewChangedEvent.Broadcast();
	}
}

void UT4WorldTileDetails::PostEditUndo()
{
	Super::PostEditUndo();
	
	PostUndoEvent.Broadcast();
}

#undef LOCTEXT_NAMESPACE
