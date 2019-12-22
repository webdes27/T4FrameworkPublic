// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionAttachedNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionAttachedNode::FT4ActionAttachedNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNode(InControl, InKey)
	, bAttached(false)
	, AttachParent(ET4AttachParent::Default)
	, ActionPoint(NAME_None) // #57 : BoneOrSocketName
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

FT4ActionAttachedNode::~FT4ActionAttachedNode()
{
}

void FT4ActionAttachedNode::SetAttachInfo(
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

// #48
void FT4ActionAttachedNode::OnAttachToParent(USceneComponent* InComponent, bool bRegister)
{
	AT4GameObject* ParentGameObject = GetGameObject();
	check(nullptr != ParentGameObject);
	if (bRegister)
	{
		ParentGameObject->AddAttachedComponent(InComponent); // #48
	}
	if (ET4AttachParent::World == AttachParent) // #54
	{
		WorldInheritLocation = ParentGameObject->GetRootLocation(); // #63 : 기본 위치는 맞춰줘야 한다.
		if (bInheritLocation) // #54
		{
			ParentGameObject->GetSocketLocation(ActionPoint, WorldInheritLocation);
		}
		if (bInheritRotation) // #54
		{
			ParentGameObject->GetSocketRotation(
				ActionPoint, 
				ERelativeTransformSpace::RTS_World, 
				WorldInheritRotation
			);
		}
		if (bInheritScale)
		{
			ParentGameObject->GetSocketScale(
				ActionPoint,
				ERelativeTransformSpace::RTS_World,
				WorldInheritScale
			);
		}
		if (ET4ObjectType::None != WorldObjectType)
		{
			AT4GameObject* NewClientGameObject = NewClientObject(
				WorldObjectType,
				TEXT("FT4ActionAttachedNode"),
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

void FT4ActionAttachedNode::OnDetachFromParent(USceneComponent* InComponent, bool bUnregister)
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

template <class T>
T* FT4ActionAttachedNode::NewComponentTemplate(bool bInAutoActivate)
{
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	T* NewComponent = NewObject<T>(OwnerGameObject->GetPawn());
	NewComponent->bAutoActivate = bInAutoActivate;
	NewComponent->SetRelativeLocation(FVector::ZeroVector);
	return NewComponent;
}

void FT4ActionAttachedNode::AttachToComponent(
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

void FT4ActionAttachedNode::DetachFromComponent(USceneComponent* InComponent)
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
