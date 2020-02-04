// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4EnvironmentZoneComponent.h"

#include "T4EngineInternal.h"

/**
  * #99
 */
UT4EnvironmentZoneComponent::UT4EnvironmentZoneComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ZoneName(NAME_None)
	, ZoneType(ET4ZoneType::Default)
	, PlayRate(1.0f) // #102
	, bEntered(false)
	, bBlendStart(false)
	, BlendTimeLeft(0.0f)
	, InitializeOffsetTimeSec(0.0f) // #99
	, bLeaveOut(false) // #99 : 강제로 Leave 한다. Action 연계한 FadeOut 처리
	, bOverrideBlendTime(false) // #99
	, OverrideBlendInTimeSec(0.0f) // #99
	, OverrideBlendOutTimeSec(0.0f) // #99
{
}

void UT4EnvironmentZoneComponent::Reset()
{
	ZoneName = NAME_None;
	ZoneType = ET4ZoneType::Default;
	bEntered = false;
	bBlendStart = false;
	BlendTimeLeft = 0.0f;
	InitializeOffsetTimeSec = 0.0f;
	EntityAssetPtr.Reset();

	bLeaveOut = false; // #99 : 강제로 Leave 한다. Action 연계한 FadeOut 처리
	bOverrideBlendTime = false; // #99
	OverrideBlendInTimeSec = 0.0f; // #99
	OverrideBlendOutTimeSec = 0.0f; // #99
}

void UT4EnvironmentZoneComponent::Initialize(
	FName InZoneName,
	ET4ZoneType InZoneType,
	TWeakObjectPtr<const UT4ZoneEntityAsset> InZoneEntityAsset,
	float InOffsetTimeSec
)
{
	ZoneName = InZoneName;
	ZoneType = InZoneType;
	EntityAssetPtr = InZoneEntityAsset;

	// #99 : 일반적으로 Env 는 Blending 때문에 OffsetTime 이 0이 되어야 하나, 타이밍을 맞추기 위한 처리도 필요할 것이다. Conti Play/Pause
	InitializeOffsetTimeSec = InOffsetTimeSec;
}

void UT4EnvironmentZoneComponent::LeaveOut(float InOffsetTimeSec)
{
	// #99 : 강제로 Leave 한다. Action 연계한 FadeOut 처리
	bLeaveOut = true;
	Leave(InOffsetTimeSec);
}

void UT4EnvironmentZoneComponent::SetOverrideBlendTime(
	float InBlendInTimeSec, 
	float InBlendOutTimeSec
) // #99
{
	bOverrideBlendTime = true; // #99
	OverrideBlendInTimeSec = InBlendInTimeSec; // #99
	OverrideBlendOutTimeSec = InBlendOutTimeSec; // #99
}

void UT4EnvironmentZoneComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	const float ScaledDeltaTimeSec = DeltaTime * PlayRate; // #102
	Super::TickComponent(ScaledDeltaTimeSec, TickType, ThisTickFunction);

	// #99 : 일반적으로 Env 는 Blending 때문에 OffsetTime 이 0이 되어야 하나, 타이밍을 맞추기 위한 처리도 필요할 것이다. Conti Play/Pause
	if (0.0f < InitializeOffsetTimeSec)
	{
		InitializeOffsetTimeSec = FMath::Max(0.0f, InitializeOffsetTimeSec - ScaledDeltaTimeSec);
	}
	if (bBlendStart)
	{
		BlendTimeLeft -= ScaledDeltaTimeSec;
		if (0.0 >= BlendTimeLeft)
		{
			bBlendStart = false;
		}
	}
}

void UT4EnvironmentZoneComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UT4EnvironmentZoneComponent::TestEncompassesPoint(const FVector& InLocation) // #94
{
	if (!EntityAssetPtr.IsValid())
	{
		return false;
	}
	bool bInPoint = EncompassesPoint(InLocation);
	if (bEntered)
	{
		if (!bInPoint)
		{
			Leave(0.0f);
		}
	}
	else
	{
		if (bInPoint)
		{
			Enter();
		}
	}
	return (0.0f < GetBlendWeight()) ? true : false;
}

void UT4EnvironmentZoneComponent::GetEnvironmentZoneInfo(FT4EnvironmentZoneInfo& OutZoneInfo) // #94
{
	if (!EntityAssetPtr.IsValid())
	{
		return;
	}
	const FT4EntityZoneData& ZoneData = EntityAssetPtr->ZoneData;
	const FT4EntityZoneEnvironmentData& ZoneEnvironmentData = EntityAssetPtr->ZoneEnvironmentData;
	OutZoneInfo.ZoneName = ZoneName;
	OutZoneInfo.ZoneType = ZoneType;
	OutZoneInfo.BlendPriority = ZoneData.BlendPriority;
	OutZoneInfo.EnvironmentAsset = ZoneEnvironmentData.EnvironmentAsset.LoadSynchronous();
	OutZoneInfo.LayerBlendWeight = GetBlendWeight();
}

void UT4EnvironmentZoneComponent::Enter()
{
	if (!EntityAssetPtr.IsValid())
	{
		return;
	}
	const FT4EntityZoneData& ZoneData = EntityAssetPtr->ZoneData;
	bEntered = true;
	const float ApplyBlendInTimeSec = (bOverrideBlendTime) ? OverrideBlendInTimeSec : ZoneData.BlendInTimeSec; // #99
	if (0.0f >= ApplyBlendInTimeSec)
	{
		return;
	}
	if (0.0f < InitializeOffsetTimeSec && InitializeOffsetTimeSec >= ApplyBlendInTimeSec)
	{
		return; // #99 : 초기 단계에서 Offset Time 이 있을 경우 동기화를 위해 블랜딩을 고려하지 않도록 처리한다.
	}
	bBlendStart = true;
	BlendTimeLeft = ApplyBlendInTimeSec - InitializeOffsetTimeSec;
}

void UT4EnvironmentZoneComponent::Leave(float InOffsetTimeSec)
{
	if (!EntityAssetPtr.IsValid())
	{
		return;
	}
	const FT4EntityZoneData& ZoneData = EntityAssetPtr->ZoneData;
	bEntered = false;
	float ApplyBlendOutTimeSec = (bOverrideBlendTime) ? OverrideBlendOutTimeSec : ZoneData.BlendOutTimeSec; // #99
	ApplyBlendOutTimeSec = FMath::Max(0.0f, ApplyBlendOutTimeSec - InOffsetTimeSec);
	if (0.0f >= ApplyBlendOutTimeSec)
	{
		return;
	}
	bBlendStart = true;
	BlendTimeLeft = ApplyBlendOutTimeSec;
}

bool UT4EnvironmentZoneComponent::EncompassesPoint(const FVector& InLocation) // #94
{
	if (bLeaveOut)
	{
		return false; // #99 : 강제로 Leave 한다. Action 연계한 FadeOut 처리
	}
	if (!EntityAssetPtr.IsValid())
	{
		return false;
	}
	const FT4EntityZoneEnvironmentData& ZoneEnvironmentData = EntityAssetPtr->ZoneEnvironmentData;
	if (ZoneEnvironmentData.EnvironmentAsset.IsNull())
	{
		return false;
	}
	const FT4EntityZoneData& ZoneData = EntityAssetPtr->ZoneData;
	if (ET4EntityZoneBrushType::Cylinder != ZoneData.BrushType)
	{
		return false; // TODO
	}
	FVector CenterLocation = GetComponentLocation();
	if (InLocation.Z > (CenterLocation.Z + ZoneData.HalfHeight) ||
		InLocation.Z < (CenterLocation.Z - ZoneData.HalfHeight))
	{
		return false;
	}
	FVector2D DistanceToPoint = FVector2D(InLocation.X, InLocation.Y) - FVector2D(CenterLocation.X, CenterLocation.Y);
	if (FMath::Square(ZoneData.Radius) < DistanceToPoint.SizeSquared())
	{
		return false;
	}
	return true;
}

float UT4EnvironmentZoneComponent::GetBlendWeight()
{
	if (!EntityAssetPtr.IsValid())
	{
		return 0.0f;
	}
	const FT4EntityZoneData& ZoneData = EntityAssetPtr->ZoneData;
	if (!bBlendStart || 0.0f >= BlendTimeLeft)
	{
		return (bEntered) ? 1.0f : 0.0f;
	}
	if (bEntered)
	{
		const float ApplyBlendInTimeSec = (bOverrideBlendTime) ? OverrideBlendInTimeSec : ZoneData.BlendInTimeSec; // #99
		if (0.0f >= ApplyBlendInTimeSec)
		{
			return 1.0f;
		}
		return FMath::Clamp(1.0f - (BlendTimeLeft / ApplyBlendInTimeSec), 0.0f, 1.0f); // blend in
	}
	float ApplyBlendOutTimeSec = (bOverrideBlendTime) ? OverrideBlendOutTimeSec : ZoneData.BlendOutTimeSec; // #99
	if (0.0f >= ApplyBlendOutTimeSec)
	{
		return 0.0f;
	}
	return FMath::Clamp(BlendTimeLeft / ApplyBlendOutTimeSec, 0.0f, 1.0f); // blend out
}