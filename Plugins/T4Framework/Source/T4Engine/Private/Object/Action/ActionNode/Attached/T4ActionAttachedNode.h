// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/Action/ActionNode/T4ActionNode.h"
#include "T4Engine.h"

/**
  * #22
 */
class AT4GameObject;
class FT4ActionAttachedNode : public FT4ActionNode
{
public:
	explicit FT4ActionAttachedNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionAttachedNode();

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

	template <class T>
	T* NewComponentTemplate(bool bInAutoActivate);

private:
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
