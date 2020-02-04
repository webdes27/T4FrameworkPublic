// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "T4Engine.h"

/**
  * #22
 */
class AT4GameObject;
class FT4ActionAttachedNodeBase : public FT4ActionNodeBase
{
public:
	explicit FT4ActionAttachedNodeBase(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionAttachedNodeBase();

	// IT4ActionNode
	const FName GetActionPoint() const override { return ActionPoint; } // #63

public:
	void OnAttachToParent(USceneComponent* InComponent, bool bRegister); // #48
	void OnDetachFromParent(USceneComponent* InComponent, bool bUnregister); // #48

protected:
	void SetAttachInfo(
		ET4AttachParent InAttachParent,
		const bool bParentInheritPoint, // #76
		const FName& InActionPoint,
		const FSoftObjectPath& InAssetPath,
		ET4LoadingPolicy InLoadingPolicy
	);

	void SetRelativeTransform(
		const FVector& InLocalOffset, // #112
		const FRotator& InLocalRotation, // #112
		const FVector& InLocalScale // #54
	); // #112

	template <class T>
	T* NewComponentTemplate(UObject* InOuter, bool bInAutoActivate)
	{
		T* NewComponent = NewObject<T>(InOuter);
		NewComponent->bAutoActivate = bInAutoActivate;
		NewComponent->SetRelativeLocation(FVector::ZeroVector);
		return NewComponent;
	}

private:
	void SetOverrideParameters(USceneComponent* InComponent); // #112

	void AttachToComponent(
		USceneComponent* InParentComponent,
		USceneComponent* InComponent
	);
	void DetachFromComponent(USceneComponent* InComponent);

protected:
	bool bAttached;
	ET4AttachParent AttachParent; // #54
	FName ActionPoint;
	FSoftObjectPath AssetPath;
	ET4LoadingPolicy LoadingPolicy; // #56
	
	FVector RelativeOffset; // #112
	FRotator RelativeRotation; // #112
	FVector RelativeScale; // #54

	// #54
	bool bInheritLocation;
	bool bInheritRotation;
	bool bInheritScale;
	FAttachmentTransformRules AttachTransformRule;

	ET4ObjectType WorldObjectType; // #63
	FVector WorldInheritLocation;
	FRotator WorldInheritRotation;
	FVector WorldInheritScale;
	// #54
};
