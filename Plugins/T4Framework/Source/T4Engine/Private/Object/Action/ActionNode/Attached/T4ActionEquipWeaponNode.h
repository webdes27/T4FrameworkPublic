// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionAttachedNode.h"
#include "Object/Component/T4StaticMeshComponent.h"
#include "Public/Asset/T4AssetLoader.h"

/**
  *
 */
// #22
class FT4ActionControl;
class FT4ActionEquipWeaponNode : public FT4ActionAttachedNode
{
public:
	explicit FT4ActionEquipWeaponNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionEquipWeaponNode();

	static FT4ActionEquipWeaponNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4EquipWeaponAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::EquipWeapon; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
	void Destroy() override;

	void AdvanceLoading(const FT4UpdateTime& InUpdateTime) override;
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	void StartLoading() override;
	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:;
	void CreateComponent();

	void ClearAsyncLoader(); // #80

private:
	FT4StaticMeshLoader MeshLoader;
	TArray<FT4MaterialLoader> OverrideMaterialLoaders; // #80
	TArray<FSoftObjectPath> OverrideMaterialPaths; // #80

	TWeakObjectPtr<UT4StaticMeshComponent> StaticMeshComponent;

#if WITH_EDITOR
	FT4EquipWeaponAction ActionCached;
#endif
};
