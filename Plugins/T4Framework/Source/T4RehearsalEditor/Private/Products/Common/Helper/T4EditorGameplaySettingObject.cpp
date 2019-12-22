// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EditorGameplaySettingObject.h"

#include "T4Frame/Public/T4Frame.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #60
 */
UT4EditorGameplaySettingObject::UT4EditorGameplaySettingObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	// #T4_ADD_EDITOR_PLAY_TAG
	, WeaponContentNameIDSelected(NAME_None)
	, CostumeContentNameIDSelected(NAME_None)
	, NPCContentNameIDSelected(NAME_None)
	, bAIDisabled(true)
	, SandbackRole(ET4EditorPlayRole::Defender)
	, bSandbackOneHitDie(false) // #76
	, bOverrideSkillData(true) // #63
	, bOverrideEffectData(true) // #68
	, SkillContentNameIDSelected(NAME_None)
	, EffectContentNameIDSelected(NAME_None)
	, DieReactionNameIDSelected(NAME_None) // #76
	, LayerType(ET4LayerType::Max)
	, bSimulating(false) // #102
	, bGameplaySettingsUsed(false) // #104 : conti 에서만 true, world 에서는 false
{
}

#if WITH_EDITOR
void UT4EditorGameplaySettingObject::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent
)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	GetEditorPlaySettingsChanged().Broadcast();
}
#endif

const FSoftObjectPath& UT4EditorGameplaySettingObject::GetOverridePlayContiPath() const
{ 
	return ThisContiAsset.ToSoftObjectPath();
}

IT4EditorGameData* UT4EditorGameplaySettingObject::GetEditorGameData()
{
	if (ET4LayerType::Max == LayerType)
	{
		return nullptr; // #79 : 아직 초기화 전...
	}
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4GameplayInstance* GameplayInstance = GameFrame->GetGameplayInstance();
	if (nullptr == GameplayInstance)
	{
		return nullptr;
	}
	return GameplayInstance->GetEditorGameData();
}
