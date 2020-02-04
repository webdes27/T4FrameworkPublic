// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionAttachedNodeBase.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionAttachedNodeBase::FT4ActionAttachedNodeBase(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
	, bAttached(false)
	, AttachParent(ET4AttachParent::Default)
	, ActionPoint(NAME_None) // #57 : BoneOrSocketName
	, LoadingPolicy(ET4LoadingPolicy::Default)
	, RelativeOffset(FVector::ZeroVector) // #112
	, RelativeRotation(FRotator::ZeroRotator) // #112
	, RelativeScale(FVector::OneVector) // #54
	, bInheritLocation(true) // #54
	, bInheritRotation(true) // #54
	, bInheritScale(true) // #54
	, AttachTransformRule(FAttachmentTransformRules::KeepRelativeTransform) // #54
	, WorldObjectType(ET4ObjectType::None) // #63
	, WorldInheritLocation(FVector::ZeroVector) // #54
	, WorldInheritRotation(FRotator::ZeroRotator) // #54
	, WorldInheritScale(FVector::OneVector) // #54
{
}

FT4ActionAttachedNodeBase::~FT4ActionAttachedNodeBase()
{
}

void FT4ActionAttachedNodeBase::SetAttachInfo(
	ET4AttachParent InAttachParent,
	const bool bParentInheritPoint, // #76
	const FName& InActionPoint,
	const FSoftObjectPath& InAssetPath,
	ET4LoadingPolicy InLoadingPolicy
)
{
	AttachParent = InAttachParent;
	if (ET4AttachParent::Default == AttachParent)
	{
		AttachParent = ET4AttachParent::Object;
	}
	if (bParentInheritPoint) // #63, #76
	{
		ActionPoint = GetParentNode()->GetActionPoint();
	}
	if (ActionPoint == NAME_None) // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...
	{
		ActionPoint = InActionPoint;
	}
	AssetPath = InAssetPath;
	LoadingPolicy = InLoadingPolicy; // #56
	if (ET4LoadingPolicy::Default == LoadingPolicy)
	{
		LoadingPolicy = ET4LoadingPolicy::Async;
	}
}

void FT4ActionAttachedNodeBase::SetRelativeTransform(
	const FVector& InLocalOffset, // #112
	const FRotator& InLocalRotation, // #112
	const FVector& InLocalScale // #54
) // #112
{
	RelativeOffset = InLocalOffset;
	RelativeRotation = InLocalRotation;
	RelativeScale = InLocalScale;
}

// #48
void FT4ActionAttachedNodeBase::OnAttachToParent(USceneComponent* InComponent, bool bRegister)
{
	{
		// #112
		if (!RelativeOffset.IsNearlyZero())
		{
			InComponent->SetRelativeLocation(RelativeOffset);
		}
		if (!RelativeRotation.IsNearlyZero())
		{
			InComponent->SetRelativeRotation(RelativeRotation);
		}
		if (FVector::OneVector != RelativeScale)
		{
			InComponent->SetRelativeScale3D(RelativeScale); // #54
		}
	}

	SetOverrideParameters(InComponent); // #112

	AT4GameObject* ParentGameObject = GetGameObject();
	check(nullptr != ParentGameObject);
	if (bRegister)
	{
		ParentGameObject->AddAttachedComponent(InComponent); // #48
	}
	if (ET4AttachParent::World == AttachParent) // #54
	{
		WorldInheritLocation = ParentGameObject->GetRootLocation(); // #63 : 기본 위치는 맞춰줘야 한다.
		{
			// Play 시점에 출력 위치를 현재까지의 Parent AttachPoint 를 기준으로 얻는다.
			if (bInheritLocation) // #54
			{
				bool bResult = ParentGameObject->GetSocketLocation(ActionPoint, WorldInheritLocation);
				if (!bResult)
				{
					// pass
				}
			}
			if (bInheritRotation) // #54
			{
				bool bResult = ParentGameObject->GetSocketRotation(
					ActionPoint,
					ERelativeTransformSpace::RTS_World,
					WorldInheritRotation
				);
				if (!bResult)
				{
					WorldInheritRotation = ParentGameObject->GetActorRotation();
				}
			}
			if (bInheritScale)
			{
				bool bResult = ParentGameObject->GetSocketScale(
					ActionPoint,
					ERelativeTransformSpace::RTS_World,
					WorldInheritScale
				);
				if (!bResult)
				{
					// pass
				}
			}
		}
		if (ET4ObjectType::None != WorldObjectType)
		{
			AT4GameObject* NewClientGameObject = NewClientObject(
				WorldObjectType,
				TEXT("FT4ActionAttachedNodeBase"),
				WorldInheritLocation,
				WorldInheritRotation,
				WorldInheritScale
			);
			if (nullptr == NewClientGameObject)
			{
				check(ET4LayerType::Server == ParentGameObject->GetLayerType()); // #63 : 서버는 사용할 필요가 없다!
			}
			else
			{
				ParentGameObject = NewClientGameObject; // todo : 월드에 던지면 AttachNode 가 알 필요 없도록 수정 필요
			}
		}
	}
	check(nullptr != ParentGameObject);
	USceneComponent* ParentComponent = ParentGameObject->GetAttachParentComponent();
	if (nullptr == ParentComponent)
	{
		return;
	}
	AttachToComponent(ParentComponent, InComponent);
}

void FT4ActionAttachedNodeBase::OnDetachFromParent(USceneComponent* InComponent, bool bUnregister)
{
	AT4GameObject* ParentGameObject = GetGameObject();
	check(nullptr != ParentGameObject);
	if (bUnregister)
	{
		ParentGameObject->RemoveAttachedComponent(InComponent); // #48
	}
	DetachFromComponent(InComponent);
	if (ET4AttachParent::World == AttachParent) // #54
	{
		DeleteClientObject();
	}
}
// #48

void FT4ActionAttachedNodeBase::SetOverrideParameters(USceneComponent* InComponent) // #112
{
	if (!ActionParameterPtr.IsValid())
	{
		return;
	}
	const FT4ActionOverrideParameters& OverrideParams = ActionParameterPtr->GetOverrideParams();
	if (ActionParameterPtr->CheckBits(ET4OverrideParamBits::ProjectileAttachTransformRuleBit)) // #112
	{
		AttachTransformRule.LocationRule = EAttachmentRule::KeepRelative;
		AttachTransformRule.RotationRule = EAttachmentRule::KeepWorld;
		AttachTransformRule.ScaleRule = EAttachmentRule::KeepWorld;
	}
	if (ActionParameterPtr->CheckBits(ET4OverrideParamBits::ActionPointBit)) // #112
	{
		ActionPoint = OverrideParams.ActionPoint;
	}
	if (ActionParameterPtr->CheckBits(ET4OverrideParamBits::LocalOrWorldLocationBit)) // #112
	{
		if (EAttachmentRule::KeepWorld == AttachTransformRule.LocationRule)
		{
			InComponent->AddWorldOffset(OverrideParams.LocalOrWorldLocation);
		}
		else if (EAttachmentRule::KeepRelative == AttachTransformRule.LocationRule)
		{
			InComponent->AddRelativeLocation(OverrideParams.LocalOrWorldLocation);
		}
		else
		{
			T4_LOG(Warning, TEXT("Unknown AttachTransform Location Rule"));
		}
	}
	if (ActionParameterPtr->CheckBits(ET4OverrideParamBits::LocalOrWorldRotationBit)) // #112
	{
		if (EAttachmentRule::KeepWorld == AttachTransformRule.RotationRule)
		{
			InComponent->AddWorldRotation(OverrideParams.LocalOrWorldRotation);
		}
		else if (EAttachmentRule::KeepRelative == AttachTransformRule.RotationRule)
		{
			InComponent->AddRelativeRotation(OverrideParams.LocalOrWorldRotation);
		}
		else
		{
			T4_LOG(Warning, TEXT("Unknown AttachTransform Rotation Rule"));
		}
	}
}

void FT4ActionAttachedNodeBase::AttachToComponent(
	USceneComponent* InParentComponent,
	USceneComponent* InComponent
)
{
	check(!bAttached);
	check(nullptr != InParentComponent);
	check(nullptr != InComponent);
	if (!bInheritLocation)
	{
		AttachTransformRule.LocationRule = EAttachmentRule::KeepWorld;
	}
	if (!bInheritRotation)
	{
		AttachTransformRule.RotationRule = EAttachmentRule::KeepWorld;
	}
	if (!bInheritScale)
	{
		AttachTransformRule.ScaleRule = EAttachmentRule::KeepWorld;
	}
	InComponent->AttachToComponent(InParentComponent, AttachTransformRule, ActionPoint);
	InComponent->RegisterComponent();
	bAttached = true;
}

void FT4ActionAttachedNodeBase::DetachFromComponent(USceneComponent* InComponent)
{
	if (!bAttached)
	{
		return;
	}

	check(nullptr != InComponent);
	InComponent->UnregisterComponent();
	InComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	bAttached = false;
}
