// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionStruct.h"
#include "Public/T4AssetDefinitions.h"
#include "Engine/Scene.h" // #100
#include "Camera/CameraShake.h" // #101
#include "T4ActionContiStructs.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG_CONTI

// ET4ActionType::Branch // #54
// ET4ActionType::SpecialMove
// ET4ActionType::Animation
// ET4ActionType::Particle
// ET4ActionType::Decal // #52
// ET4ActionType::Projectile // #63
// ET4ActionType::Reaction // #76
// ET4ActionType::LayerSet // #81
// ET4ActionType::TimeScale // #102
// ET4ActionType::CameraWork // #58
// ET4ActionType::CameraShake // #101
// ET4ActionType::PostProcess // #100
// ET4ActionType::Environment // #99

class UT4ContiAsset;

USTRUCT()
struct T4ASSET_API FT4ContiActionStruct : public FT4ActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeCommonActionDetails
	// #24 : Composite 일 경우 사용됨!
	// #65 : Property 추가시에는 FT4ActionCompositeData::CloneAndAddAction 에도 반영할 것! (FT4ActionCompositeData::CopyAction)
	UPROPERTY(VisibleAnywhere)
	int32 HeaderKey;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FName DisplayName;

	UPROPERTY(EditAnywhere)
	FColor DebugColorTint;
#endif

public:
	FT4ContiActionStruct()
		: FT4ActionStruct()
		, HeaderKey(INDEX_NONE) // #24
#if WITH_EDITORONLY_DATA
		, DisplayName(NAME_None)
		, DebugColorTint(FColor::Black)
#endif
	{
	}

	FT4ContiActionStruct(ET4ActionType InObjectAction)
		: FT4ActionStruct(InObjectAction)
		, HeaderKey(INDEX_NONE) // #24
#if WITH_EDITORONLY_DATA
		, DisplayName(NAME_None)
		, DebugColorTint(FColor::Black)
#endif
	{
	}

	virtual ~FT4ContiActionStruct() {}

	ET4ActionStructType GetActionStructType() const override { return ET4ActionStructType::Conti; } // #52

	virtual bool Validate(FString& OutMsg)
	{
		return true;
	}

	virtual FString ToString() const
	{
		return FString(TEXT("ContiActionStruct"));
	}

	virtual FString ToDisplayText()
	{
		return FString(TEXT("Untitled")); // #54
	}
};

// #54
USTRUCT()
struct T4ASSET_API FT4BranchAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeBranchActionDetails
	UPROPERTY(EditAnywhere)
	ET4BranchCondition Contition;

	UPROPERTY(EditAnywhere)
	FName ConditionName;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4ContiAsset> ContiAsset;

	UPROPERTY(EditAnywhere)
	ET4LoadingPolicy LoadingPolicy;

public:
	FT4BranchAction()
		: FT4ContiActionStruct(StaticActionType())
		, Contition(ET4BranchCondition::Default)
		, ConditionName(NAME_None)
		, LoadingPolicy(ET4LoadingPolicy::Default)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Branch; }

	FString ToString() const override
	{
		return FString(TEXT("BranchAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("Branch '%s'"), *(ContiAsset.GetAssetName())); // #54
	}
};

// #54
USTRUCT()
struct T4ASSET_API FT4SpecialMoveAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeSpecialMoveActionDetails

public:
	FT4SpecialMoveAction()
		: FT4ContiActionStruct(StaticActionType())
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::SpecialMove; }

	FString ToString() const override
	{
		return FString(TEXT("SpecialMoveAction"));
	}
};

USTRUCT()
struct T4ASSET_API FT4AnimationAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeAnimationActionDetails

	UPROPERTY(EditAnywhere)
	FName SectionName;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendInTimeSec;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendOutTimeSec;

	UPROPERTY(EditAnywhere)
	float PlayRate;

	UPROPERTY(EditAnywhere)
	int32 LoopCount;

public:
	FT4AnimationAction()
		: FT4ContiActionStruct(StaticActionType())
		, SectionName(NAME_None)
		, BlendInTimeSec(T4AnimSetBlendTimeSec)
		, BlendOutTimeSec(T4AnimSetBlendTimeSec)
		, PlayRate(1.0f)
		, LoopCount(1)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Animation; }

	FString ToString() const override
	{
		return FString(TEXT("AnimationAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("Animation '%s'"), *(SectionName.ToString())); // #54
	}
};

class UParticleSystem;

USTRUCT()
struct T4ASSET_API FT4ParticleAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeParticleActionDetails
	UPROPERTY(EditAnywhere)
	ET4AttachParent AttachParent;

	UPROPERTY(EditAnywhere)
	bool bParentInheritPoint; // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...

	UPROPERTY(EditAnywhere)
	FName ActionPoint; // #57

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> ParticleAsset;

	UPROPERTY(EditAnywhere)
	ET4LoadingPolicy LoadingPolicy;

	UPROPERTY(EditAnywhere)
	FVector Scale; // #54

	UPROPERTY(EditAnywhere)
	float PlayRate;

public:
	FT4ParticleAction()
		: FT4ContiActionStruct(StaticActionType())
		, AttachParent(ET4AttachParent::Default) // #54
		, bParentInheritPoint(false) // #76
		, ActionPoint(T4ContiDefaultActionPontName)
		, LoadingPolicy(ET4LoadingPolicy::Default)
		, Scale(FVector::OneVector) // #54
		, PlayRate(1.0f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Particle; }

	FString ToString() const override
	{
		return FString(TEXT("ParticleAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("Particle '%s'"), *(ParticleAsset.GetAssetName())); // #54
	}
};

// #54
class UMaterialInterface;

USTRUCT()
struct T4ASSET_API FT4DecalAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeDecalActionDetails
	UPROPERTY(EditAnywhere)
	ET4AttachParent AttachParent;

	UPROPERTY(EditAnywhere)
	bool bParentInheritPoint; // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...

	UPROPERTY(EditAnywhere)
	FName ActionPoint;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(EditAnywhere)
	ET4LoadingPolicy LoadingPolicy;

	UPROPERTY(EditAnywhere)
	FVector Scale; // #54

	UPROPERTY(EditAnywhere)
	int32 DecalSortOrder;

	UPROPERTY(EditAnywhere)
	FVector DecalSize;

	UPROPERTY(EditAnywhere)
	float FadeInTimeSec;

	UPROPERTY(EditAnywhere)
	float FadeOutTimeSec;

public:
	FT4DecalAction()
		: FT4ContiActionStruct(StaticActionType())
		, AttachParent(ET4AttachParent::Default)
		, bParentInheritPoint(false) // #76
		, ActionPoint(T4ContiDefaultActionPontName)
		, LoadingPolicy(ET4LoadingPolicy::Default)
		, Scale(FVector::OneVector)
		, DecalSortOrder(0)
		, DecalSize(128.0f, 256.0f, 256.0f)
		, FadeInTimeSec(0.5f)
		, FadeOutTimeSec(0.5f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Decal; }

	FString ToString() const override
	{
		return FString(TEXT("DecalAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("Decal '%s'"), *(DecalMaterial.GetAssetName())); // #54
	}
};

// #63
USTRUCT()
struct T4ASSET_API FT4ProjectileAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeProjectileActionDetails
	UPROPERTY(EditAnywhere)
	FName ActionPoint; // 어딘가에 붙어야 할 경우. 예) 오른손...

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4ContiAsset> CastingContiAsset;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4ContiAsset> HeadContiAsset;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4ContiAsset> EndContiAsset;

	UPROPERTY(EditAnywhere)
	ET4LoadingPolicy LoadingPolicy;

	UPROPERTY(EditAnywhere)
	float ThrowDelayTimeSec; // Play 이후 ActionPoint 에서 떨어지는 시간!

	UPROPERTY(EditAnywhere)
	float CastingStopDelayTimeSec; // ThrowDelayTimeSec 이후 Casting Conti 가 삭제될 시간

public:
	FT4ProjectileAction()
		: FT4ContiActionStruct(StaticActionType())
		, ActionPoint(NAME_None)
		, LoadingPolicy(ET4LoadingPolicy::Default)
		, ThrowDelayTimeSec(0.0f)
		, CastingStopDelayTimeSec(0.2f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Projectile; }

	FString ToString() const override
	{
		return FString(TEXT("ProjectileAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("Projectile '%s'"), *(HeadContiAsset.GetAssetName()));
	}
};

// #76
USTRUCT()
struct T4ASSET_API FT4ReactionAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeReactionActionDetails
	UPROPERTY(EditAnywhere)
	FName ReactionName;

public:
	FT4ReactionAction()
		: FT4ContiActionStruct(StaticActionType())
		, ReactionName(NAME_None)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Reaction; }

	FString ToString() const override
	{
		return FString(TEXT("ReactionAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("Reaction '%s'"), *(ReactionName.ToString())); // #67
	}
};

// #81
USTRUCT()
struct T4ASSET_API FT4LayerSetAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeLayerSetActionDetails
	UPROPERTY(EditAnywhere)
	FName LayerTagName;

	UPROPERTY(EditAnywhere)
	ET4LayerTagType LayerTagType;

public:
	FT4LayerSetAction()
		: FT4ContiActionStruct(StaticActionType())
		, LayerTagName(NAME_None)
		, LayerTagType(ET4LayerTagType::All)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::LayerSet; }

	FString ToString() const override
	{
		return FString(TEXT("LayerSetAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("LayerSet '%s'"), *(LayerTagName.ToString()));
	}
};

// #102
USTRUCT()
struct T4ASSET_API FT4TimeScaleAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeTimeScaleActionDetails

	UPROPERTY(EditAnywhere)
	ET4PlayTarget PlayTarget;

	UPROPERTY(EditAnywhere)
	ET4BuiltInEasing BlendInCurve;

	UPROPERTY(EditAnywhere)
	float BlendInTimeSec;

	UPROPERTY(EditAnywhere)
	ET4BuiltInEasing BlendOutCurve;

	UPROPERTY(EditAnywhere)
	float BlendOutTimeSec;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.1", UIMin = "0.1", UIMax = "5"))
	float TimeScale;

public:
	FT4TimeScaleAction()
		: FT4ContiActionStruct(StaticActionType())
		, PlayTarget(ET4PlayTarget::Default)
		, BlendInCurve(ET4BuiltInEasing::Linear)
		, BlendInTimeSec(0.0f)
		, BlendOutCurve(ET4BuiltInEasing::Linear)
		, BlendOutTimeSec(0.0f)
		, TimeScale(1.0f)
	{
		LifecycleType = ET4LifecycleType::Duration; // Duration 만!, 시스템으로 제어 필요
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::TimeScale; }

	FString ToString() const override
	{
		return FString(TEXT("TimeScaleAction"));
	}

#if WITH_EDITOR
	void Reset()
	{
		PlayTarget = ET4PlayTarget::Default;
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("TimeScale 'x%.1f'"), TimeScale);
	}
#endif
};

// #58
USTRUCT()
struct T4ASSET_API FT4CameraWorkSectionKeyData
{
	GENERATED_USTRUCT_BODY()

public:
	// #58 : Property 수정시 UT4CameraWorkSectionKeyObject 에도 추가해줄 것!
	//       SaveCameraSectionKeyObject, UpdateCameraSectionKeyObject
	UPROPERTY(EditAnywhere)
	int32 ChannelKey; // Track Section 의 FFrameNumber 즉, FrameNumber 가 Unique Key 가 됨으로 저장해준다.

	UPROPERTY(EditAnywhere)
	float StartTimeSec; // FrameNumber 를 Sec 으로 변환

	UPROPERTY()
	float DelayTimeSec_DEPRECATED; // FrameNumber 를 Sec 으로 변환

	UPROPERTY(EditAnywhere)
	ET4BuiltInEasing EasingCurve; // #102 : Droplist 선택시 PropertyChanged event 가 와서 변경을 못하는 문제가 있어 하드코딩한 처리가 있음. 이름으로 검색!!

	UPROPERTY(EditAnywhere)
	FName LookAtPoint; // ActionPoint

	UPROPERTY(EditAnywhere)
	bool bInverse; // LookAtPoint Inverse

	UPROPERTY(EditAnywhere)
	FVector ViewDirection; // Local

	UPROPERTY(EditAnywhere)
	float Distance;

	UPROPERTY(EditAnywhere)
	float FOVDegree;

public:
	FT4CameraWorkSectionKeyData()
		: ChannelKey(INDEX_NONE)
		, StartTimeSec(0.0f)
		, EasingCurve(ET4BuiltInEasing::Linear)
		, LookAtPoint(NAME_None)
		, bInverse(false)
		, ViewDirection(FVector::BackwardVector)
		, Distance(100.0f)
		, FOVDegree(0.0f)
	{
	}
};

USTRUCT()
struct T4ASSET_API FT4CameraWorkSectionData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FT4CameraWorkSectionKeyData> KeyDatas;

public:
	FT4CameraWorkSectionData()
	{
	}
};

USTRUCT()
struct T4ASSET_API FT4CameraWorkAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeCameraWorkActionDetails

	UPROPERTY(EditAnywhere)
	ET4PlayTarget PlayTarget;

	UPROPERTY(EditAnywhere)
	ET4BuiltInEasing BlendInCurve;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendInTimeSec;

	UPROPERTY(EditAnywhere)
	ET4BuiltInEasing BlendOutCurve;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendOutTimeSec;

	UPROPERTY(EditAnywhere)
	FT4CameraWorkSectionData SectionData;

public:
	FT4CameraWorkAction()
		: FT4ContiActionStruct(StaticActionType())
		, PlayTarget(ET4PlayTarget::Default)
		, BlendInCurve(ET4BuiltInEasing::Linear)
		, BlendInTimeSec(0.0f)
		, BlendOutCurve(ET4BuiltInEasing::Linear)
		, BlendOutTimeSec(0.0f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::CameraWork; }

	FString ToString() const override
	{
		return FString(TEXT("CameraWorkAction"));
	}

#if WITH_EDITOR
	void Reset()
	{
		PlayTarget = ET4PlayTarget::Default;
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("")); // SectionKey 가 여려개 생성됨으로 출력을 제외!
	}
#endif
};

// #101
USTRUCT()
struct T4ASSET_API FT4CameraShakeOscillationData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendInTimeSec;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendOutTimeSec;

	UPROPERTY(EditAnywhere)
	FROscillator RotOscillation;

	UPROPERTY(EditAnywhere)
	FVOscillator LocOscillation;

	UPROPERTY(EditAnywhere)
	FFOscillator FOVOscillation;

public:
	FT4CameraShakeOscillationData()
		: BlendInTimeSec(0.0f)
		, BlendOutTimeSec(0.0f)
	{
	}
};

USTRUCT()
struct T4ASSET_API FT4CameraShakeAnimData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.001"))
	float AnimPlayRate;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float AnimScale;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float AnimBlendInTime;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float AnimBlendOutTime;

	UPROPERTY(EditAnywhere)
	uint32 bRandomAnimSegment : 1;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", editcondition = "bRandomAnimSegment"))
	float RandomAnimSegmentDuration;

	UPROPERTY(EditAnywhere)
	class UCameraAnim* CameraAnim;

public:
	FT4CameraShakeAnimData()
		: AnimPlayRate(1.0f)
		, AnimScale(0.0f)
		, AnimBlendInTime(0.0f)
		, AnimBlendOutTime(0.0f)
		, bRandomAnimSegment(0)
		, RandomAnimSegmentDuration(0.0f)
		, CameraAnim(nullptr)
	{
	}
};

USTRUCT()
struct T4ASSET_API FT4CameraShakeAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeShakeActionDetails

	// #101 : UCameraShake : Property

	UPROPERTY(EditAnywhere)
	ET4PlayTarget PlayTarget; // #100

	UPROPERTY(EditAnywhere)
	float PlayScale;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECameraAnimPlaySpace::Type> PlaySpace;

	UPROPERTY(EditAnywhere)
	FRotator UserDefinedPlaySpace;

	UPROPERTY(EditAnywhere)
	FT4CameraShakeOscillationData OscillationData;

	UPROPERTY(EditAnywhere)
	FT4CameraShakeAnimData AnimData;

public:
	FT4CameraShakeAction()
		: FT4ContiActionStruct(StaticActionType())
		, PlayTarget(ET4PlayTarget::Default)
		, PlayScale(1.0f)
		, PlaySpace(ECameraAnimPlaySpace::CameraLocal)
		, UserDefinedPlaySpace(ForceInitToZero)
	{
		LifecycleType = ET4LifecycleType::Duration; // Duration 만!, 시스템으로 제어 필요
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::CameraShake; }

	FString ToString() const override
	{
		return FString(TEXT("CameraShakeAction"));
	}

#if WITH_EDITOR
	void Reset()
	{
		PlayTarget = ET4PlayTarget::Default;
		PlayScale = 1.0f;
		PlaySpace = ECameraAnimPlaySpace::CameraLocal;
		UserDefinedPlaySpace = FRotator::ZeroRotator;
		OscillationData = FT4CameraShakeOscillationData();
		AnimData = FT4CameraShakeAnimData();
	}

	FString ToDisplayText() override
	{
		return FString::Printf(
			TEXT("CameraShake 'PlayTarget => %s'"), 
			(ET4PlayTarget::All == PlayTarget) ? TEXT("All") : TEXT("Player")
		);
	}
#endif
};

// #100
USTRUCT()
struct T4ASSET_API FT4PostProcessAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizePostProcessActionDetails
	UPROPERTY(EditAnywhere)
	ET4PlayTarget PlayTarget; // #100

	UPROPERTY(EditAnywhere)
	ET4BuiltInEasing BlendInCurve;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendInTimeSec;

	UPROPERTY(EditAnywhere)
	ET4BuiltInEasing BlendOutCurve;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BlendOutTimeSec;

	UPROPERTY(EditAnywhere)
	FPostProcessSettings PostProcessSettings; // #98 에서는 Zone 처리,

public:
	FT4PostProcessAction()
		: FT4ContiActionStruct(StaticActionType())
		, PlayTarget(ET4PlayTarget::Default)
		, BlendInCurve(ET4BuiltInEasing::Linear)
		, BlendInTimeSec(0.0f)
		, BlendOutCurve(ET4BuiltInEasing::Linear)
		, BlendOutTimeSec(0.0f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::PostProcess; }

	FString ToString() const override
	{
		return FString(TEXT("PostProcessAction"));
	}

#if WITH_EDITOR
	void Reset()
	{
		PlayTarget = ET4PlayTarget::Default;
		BlendInTimeSec = 0.0f;
		BlendOutTimeSec = 0.0f;
		PostProcessSettings.SetBaseValues();
	}

	FString ToDisplayText() override
	{
		return FString::Printf(
			TEXT("PostProcess 'PlayTarget => %s'"), 
			(ET4PlayTarget::All == PlayTarget) ? TEXT("All") : TEXT("Player")
		);
	}
#endif
};

// #99
class UT4ZoneEntityAsset;
USTRUCT()
struct T4ASSET_API FT4EnvironmentAction : public FT4ContiActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeEnvironmentActionDetails
	UPROPERTY(EditAnywhere)
	ET4AttachParent AttachParent;

	UPROPERTY(EditAnywhere)
	bool bParentInheritPoint; // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...

	UPROPERTY(EditAnywhere)
	FName ActionPoint;

	UPROPERTY(EditAnywhere)
	ET4PlayTarget PlayTarget; // #100

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4ZoneEntityAsset> ZoneEntityAsset;

	UPROPERTY(EditAnywhere)
	bool bOverrideBlendTime;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideBlendTime", ClampMin = "0.0"))
	float OverrideBlendInTimeSec;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bOverrideBlendTime", ClampMin = "0.0"))
	float OverrideBlendOutTimeSec;

public:
	FT4EnvironmentAction()
		: FT4ContiActionStruct(StaticActionType())
		, AttachParent(ET4AttachParent::Default)
		, bParentInheritPoint(false) // #76
		, ActionPoint(T4ContiDefaultActionPontName)
		, PlayTarget(ET4PlayTarget::Default)
		, bOverrideBlendTime(false)
		, OverrideBlendInTimeSec(1.0f)
		, OverrideBlendOutTimeSec(1.0f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Environment; }

	FString ToString() const override
	{
		return FString(TEXT("EnvironmentAction"));
	}

	FString ToDisplayText() override
	{
		return FString::Printf(TEXT("Environment '%s'"), *(ZoneEntityAsset.GetAssetName()));
	}
};
