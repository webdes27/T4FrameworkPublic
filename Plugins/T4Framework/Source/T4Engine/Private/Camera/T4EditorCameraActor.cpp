// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Camera/T4EditorCameraActor.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

#include "T4EngineInternal.h"

/**
  * #58
 */
AT4EditorCameraActor::AT4EditorCameraActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CameraKey(INDEX_NONE)
	, bEmulMode(false)
{
#if WITH_EDITOR
	PrimaryActorTick.bCanEverTick = true;
	if (nullptr != GetStaticMeshComponent())
	{
		GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
		GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	SetActorEnableCollision(false);
#endif
}

void AT4EditorCameraActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AT4EditorCameraActor::Initialize(uint32 InCameraKey, bool bInEmulMode)
{
	CameraKey = InCameraKey;
#if WITH_EDITOR
	bEmulMode = bInEmulMode;
	{
		UObject* StaticMesh = FSoftObjectPath(TEXT("/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM")).TryLoad();
		GetStaticMeshComponent()->SetStaticMesh(Cast<UStaticMesh>(StaticMesh));
	}
#endif
}