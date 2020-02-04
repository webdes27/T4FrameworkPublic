// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/World/T4WorldZoneVolume.h"

#include "Public/T4EngineDefinitions.h" // #93

#include "Engine/Scene.h" // #98
#include "Engine/World.h" // #98

#include "T4EngineInternal.h"

/**
  * #92
 */
AT4WorldZoneVolume::AT4WorldZoneVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ZoneName(NAME_None)
	, ZoneType(ET4ZoneType::Static)
	, BlendPriority(0)
	, BlendInTimeSec(1.0f)
	, BlendOutTimeSec(1.0f)
#if WITH_EDITOR
	, DebugColor(FColor::White)
#endif
	, bEntered(false)
	, bBlendStart(false)
	, BlendTimeLeft(0.0f)
	, PostProcessingSettings(nullptr) // #98 : Gobal 이외에는 사용하지 않는다.
{
#if WITH_EDITOR
	DebugColor = DebugColor.WithAlpha(32);
#endif
}

void AT4WorldZoneVolume::BeginPlay()
{
	Super::BeginPlay();
	CheckGlobalPostProcessSettings(); // #104
}

void AT4WorldZoneVolume::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

void AT4WorldZoneVolume::PostLoad()
{
	Super::PostLoad();
	CheckGlobalPostProcessSettings(); // #104
}

// void DoPostProcessVolume(IInterface_PostProcessVolume* Volume, FVector ViewLocation, FSceneView* SceneView)
bool AT4WorldZoneVolume::EncompassesPoint(
	FVector Point, 
	float SphereRadius, 
	float* OutDistanceToPoint
) // #98
{
	if (IsGlobalZone())
	{
		*OutDistanceToPoint = -1.0f;
		return true;
	}
	bool bResult = AVolume::EncompassesPoint(Point, SphereRadius, OutDistanceToPoint); // #104 : APostPorcessVolume 에서 대체하며 누락된 처리!
	return bResult;
}

FPostProcessVolumeProperties AT4WorldZoneVolume::GetProperties() const // #98
{
	FPostProcessVolumeProperties Ret;
	Ret.bIsEnabled = IsGlobalZone();
	Ret.bIsUnbound = IsGlobalZone();
	Ret.BlendRadius = 6000.0f; // PostProcessing Default value
	Ret.BlendWeight = 1.0f;
	Ret.Priority = BlendPriority;
	Ret.Settings = PostProcessingSettings;
	return Ret;
}

void AT4WorldZoneVolume::PostRegisterAllComponents() // #98
{
	Super::PostRegisterAllComponents();

	TArray<IInterface_PostProcessVolume*>& VolumeArray = GetWorld()->PostProcessVolumes;

	const int32 NumVolumes = VolumeArray.Num();
	float TargetPriority = this->GetProperties().Priority;
	int32 InsertIndex = 0;
	// TODO: replace with binary search.
	for (; InsertIndex < NumVolumes; InsertIndex++)
	{
		IInterface_PostProcessVolume* CurrentVolume = VolumeArray[InsertIndex];
		float CurrentPriority = CurrentVolume->GetProperties().Priority;

		if (TargetPriority < CurrentPriority)
		{
			break;
		}
		if (CurrentVolume == this)
		{
			return;
		}
	}
	VolumeArray.Insert(this, InsertIndex);
}

void AT4WorldZoneVolume::PostUnregisterAllComponents() // #98
{
	Super::PostUnregisterAllComponents();
	if (GetWorld())
	{
		GetWorld()->PostProcessVolumes.RemoveSingle(this);
	}
}

void AT4WorldZoneVolume::Update(float InDeltaTime)
{
	if (!bBlendStart)
	{
		return;
	}
	BlendTimeLeft -= InDeltaTime;
	if (0.0 >= BlendTimeLeft)
	{
		bBlendStart = false;
	}
}

void AT4WorldZoneVolume::Enter()
{ 
	bEntered = true; 
	if (0.0f >= BlendInTimeSec)
	{
		return;
	}
	bBlendStart = true;
	BlendTimeLeft = BlendInTimeSec;
}

void AT4WorldZoneVolume::Leave() 
{ 
	bEntered = false;
	if (0.0f >= BlendOutTimeSec)
	{
		return;
	}
	bBlendStart = true;
	BlendTimeLeft = BlendOutTimeSec;
}

bool AT4WorldZoneVolume::IsGlobalZone() const 
{ 
	return (ZoneName == T4Const_GlobalWorldZoneName) ? true : false;
}

float AT4WorldZoneVolume::GetBlendWeight() const 
{
	if (IsGlobalZone())
	{
		return 1.0f;
	}
	if (!bBlendStart || 0.0f >= BlendTimeLeft)
	{
		return (bEntered) ? 1.0f : 0.0f;
	}
	if (bEntered)
	{
		if (0.0f >= BlendInTimeSec)
		{
			return 1.0f;
		}
		return FMath::Clamp(1.0f - (BlendTimeLeft / BlendInTimeSec), 0.0f, 1.0f); // blend in
	}
	if (0.0f >= BlendOutTimeSec)
	{
		return 0.0f;
	}
	return FMath::Clamp(BlendTimeLeft / BlendOutTimeSec, 0.0f, 1.0f); // blend out
}

void AT4WorldZoneVolume::CheckGlobalPostProcessSettings() // #104
{
	if (!IsGlobalZone())
	{
		if (nullptr != PostProcessingSettings) // #98
		{
			delete PostProcessingSettings;
			PostProcessingSettings = nullptr;
		}
		return;
	}
	if (nullptr != PostProcessingSettings)
	{
		return;
	}
	PostProcessingSettings = new FPostProcessSettings; // #98
	check(nullptr != PostProcessingSettings);
}

#if WITH_EDITOR
bool AT4WorldZoneVolume::IsSelectable() const
{
	return Super::IsSelectable();
}

void AT4WorldZoneVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	AVolume::PostEditChangeProperty(PropertyChangedEvent);
	CheckGlobalPostProcessSettings(); // #104
	OnPropertiesChanged().Broadcast();
}

bool AT4WorldZoneVolume::CanEditChange(const UProperty* InProperty) const
{
	return AVolume::CanEditChange(InProperty);
}

FColor AT4WorldZoneVolume::GetWireColor() const
{
	// #92 : WorldMap 에서 사용하는 Color 값인데 Alpha 를 없애서 리턴해준다. WireFrame 에 적합하도록 조정
	FColor ReturenColor = DebugColor;
	ReturenColor.WithAlpha(255);
	return ReturenColor;
}
#endif