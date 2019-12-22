// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/Action/T4ActionParameters.h"
#include "T4Frame/Public/T4FrameEditorGameplay.h"
#include "T4EditorGameplaySettingObject.generated.h"

/**
  * #60
 */
UCLASS()
class UT4EditorGameplaySettingObject : public UObject, public IT4EditorGameplayHandler
{
	GENERATED_UCLASS_BODY()

public:
	// UObject
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	// IT4EditorGameplayHandler
	bool IsSimulating() const override { return bSimulating; } // #102

	bool IsUsedGameplaySettings() const override { return bGameplaySettingsUsed; } // #104 : conti 에서만 true, world 에서는 false

	bool IsAISystemDisabled() const override { return (bSimulating) ? bAIDisabled : false; } // #102

	bool IsSandbackAttackable() const override { return (SandbackRole == ET4EditorPlayRole::Attacker) ? true : false; }
	bool IsSandbackOneHitDie() const override { return bSandbackOneHitDie; } // #76

	bool IsOverrideSkillData() const override { return bOverrideSkillData; } // #63
	bool IsOverrideEffectData() const override { return bOverrideEffectData; } // #68

	FName GetOverrideSkillDataNameID() const override { return SkillContentNameIDSelected; }
	FName GetOverrideEffectDataNameID() const override { return EffectContentNameIDSelected; }
	FName GetOverrideDieReactionNameID() const override { return DieReactionNameIDSelected; } // #76 : TODO : 사망 리엑션 값을 테이블에서 가져와야 함!

	const FT4EditorSkillDataInfo& GetOverrideSkillDataInfo() const override { return SkillDataInfo; }
	const FT4EditorEffectDataInfo& GetOverrideEffectDataInfo() const override { return EffectDataInfo; }
	const FSoftObjectPath& GetOverridePlayContiPath() const override;

public:
	void SetLayerType(ET4LayerType InLayerType) { LayerType = InLayerType; } // #60
	
	void SetSimulationEnabled(bool bInEnable) { bSimulating = bInEnable; } // #102
	void SetUseGameplaySettings(bool bInEnable) { bGameplaySettingsUsed = bInEnable; } // #104 : conti 에서만 true, world 에서는 false

	FT4ActionParameters& GetContiParameters() { return ContiParameter; }

	DECLARE_MULTICAST_DELEGATE(FT4OnEditorPlaySettingsChanged);
	FT4OnEditorPlaySettingsChanged& GetEditorPlaySettingsChanged()
	{
		return OnEditorPlaySettingsChanged;
	}

	class IT4EditorGameData* GetEditorGameData();

public:
	// #T4_ADD_EDITOR_PLAY_TAG

	UPROPERTY(EditAnywhere, Transient)
	FName WeaponContentNameIDSelected;

	UPROPERTY(EditAnywhere, Transient)
	FName CostumeContentNameIDSelected;

	UPROPERTY(EditAnywhere, Transient)
	FName NPCContentNameIDSelected;

	UPROPERTY(VisibleAnywhere, Transient)
	TSoftObjectPtr<UT4EntityAsset> NPCEntityAsset; // #76

	UPROPERTY(EditAnywhere, Transient)
	bool bAIDisabled;

	UPROPERTY(EditAnywhere, Transient)
	ET4EditorPlayRole SandbackRole; // #63

	UPROPERTY(EditAnywhere, Transient)
	bool bSandbackOneHitDie; // #76

	UPROPERTY(EditAnywhere, Transient)
	bool bOverrideSkillData; // #63 : 사용중인 Conti 를 덮어 씌울 것인가

	UPROPERTY(EditAnywhere, Transient)
	bool bOverrideEffectData; // #68 : 사용중인 Conti 를 덮어 씌울 것인가

	UPROPERTY(EditAnywhere, Transient)
	FName SkillContentNameIDSelected;

	UPROPERTY(EditAnywhere, Transient)
	FT4EditorSkillDataInfo SkillDataInfo;

	UPROPERTY(EditAnywhere, Transient)
	FName EffectContentNameIDSelected;

	UPROPERTY(EditAnywhere, Transient)
	FT4EditorEffectDataInfo EffectDataInfo;

	UPROPERTY(EditAnywhere, Transient)
	FName DieReactionNameIDSelected; // #76 : TODO : 사망 리엑션 값을 테이블에서 가져와야 함!

	TSoftObjectPtr<UT4ContiAsset> ThisContiAsset;

private:
	ET4LayerType LayerType; // #60
	bool bSimulating; // #102
	bool bGameplaySettingsUsed; // #104 : conti 에서만 true, world 에서는 false
	FT4ActionParameters ContiParameter;
	FT4OnEditorPlaySettingsChanged OnEditorPlaySettingsChanged;
};
