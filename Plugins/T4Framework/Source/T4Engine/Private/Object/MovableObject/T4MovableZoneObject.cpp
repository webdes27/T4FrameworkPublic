// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4MovableZoneObject.h"

#include "Object/Component/T4EnvironmentZoneComponent.h" // #99

#include "Public/T4EngineStructs.h" // #94

#include "T4Asset/Classes/Entity/T4ZoneEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/World.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

#include "T4EngineInternal.h"

/**
  * #94
 */
AT4MovableZoneObject::AT4MovableZoneObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ZoneType(ET4ZoneType::Default)
{
	EnvironmentZoneComponent = CreateDefaultSubobject<UT4EnvironmentZoneComponent>(TEXT("T4EnvironmentZoneComponent"));
	{
		EnvironmentZoneComponent->PrimaryComponentTick.bCanEverTick = true;
	}
	RootComponent = EnvironmentZoneComponent;
}

AT4MovableZoneObject::~AT4MovableZoneObject()
{
	check(!EntityAssetPtr.IsValid());
}

void AT4MovableZoneObject::Reset()
{
	if (nullptr != EnvironmentZoneComponent) // #99
	{
		EnvironmentZoneComponent->Reset();
	}
	EntityAssetPtr.Reset();
}

void AT4MovableZoneObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AT4MovableZoneObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
{
#if WITH_EDITOR
	if (EntityAssetPtr.IsValid() && nullptr != EnvironmentZoneComponent)
	{
		UWorld* UnrealWorld = GetWorld();
		if (nullptr != UnrealWorld)
		{
			const FT4EntityZoneData& ZoneBrushData = EntityAssetPtr->ZoneData;
			const FVector CenterLocation = EnvironmentZoneComponent->GetComponentLocation();
			const FVector HalfHeight = FVector(0.0f, 0.0f, ZoneBrushData.HalfHeight);
			DrawDebugCylinder(
				UnrealWorld,
				CenterLocation + HalfHeight,
				CenterLocation - HalfHeight,
				ZoneBrushData.Radius,
				ZoneBrushData.DebugData.DebugSegments,
				ZoneBrushData.DebugData.DebugColor,
				false
			);
		}
	}
#endif
}

void AT4MovableZoneObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
}

bool AT4MovableZoneObject::Create(
	const FT4SpawnObjectAction* InAction
)
{
	check(!EntityAssetPtr.IsValid());
	EntityAssetPtr = T4AssetEntityManagerGet()->GetZoneEntity(InAction->EntityAssetPath);
	if (!EntityAssetPtr.IsValid())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("AT4MovableZoneObject : EntityAsset (%s) Not Found."),
			*(InAction->EntityAssetPath.ToString())
		);
		return false;
	}

	EntityKey.Type = InAction->EntityType; // #35
	EntityKey.Value = EntityAssetPtr->GetEntityKeyPath(); // #35

	ZoneType = ET4ZoneType::Dynamic;

	if (nullptr != EnvironmentZoneComponent) // #99
	{
		EnvironmentZoneComponent->Initialize(
			EntityAssetPtr->GetEntityKeyPath(), 
			ET4ZoneType::Dynamic,
			EntityAssetPtr,
			0.0f
		);
	}

	bool bResult = true;
	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("AT4MovableZoneObject : Create failed")
		);
		return false;
	}

	ApplyEntityAttributes();
	return bResult;
}

bool AT4MovableZoneObject::CheckAsyncLoading()
{
	return true;
}

void AT4MovableZoneObject::ApplyEntityAttributes()
{
	check(EntityAssetPtr.IsValid());
}

#if WITH_EDITOR
void AT4MovableZoneObject::RecreateAll() // #80
{

}

bool AT4MovableZoneObject::ExecuteEditorAction(const FT4EditorAction& InAction)
{
	// #37
	return true;
}
#endif