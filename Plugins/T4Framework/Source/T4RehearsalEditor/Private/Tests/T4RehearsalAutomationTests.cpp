// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "T4Asset/Public/T4AssetUtils.h" // #39

#include "T4RehearsalEditorInternal.h"

/**
  * https://docs.unrealengine.com/en-US/Programming/Automation#enablingautomationtestplugins
 */

#if WITH_DEV_AUTOMATION_TESTS

/*
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FT4ContiSaveAndLoadTest, "T4Engine.Conti.SaveAndLoad", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FT4ContiSaveAndLoadTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR
	const FString TestAssetName = TEXT("Conti_SaveAndLoad_Test");
	const FString TestAssetPath = TEXT("/EditorTests/T4Automation/Conti/");
	{
		UT4ContiAsset* NewContiAsset = Cast<UT4ContiAsset>(T4AssetUtil::NewAsset( // #39
			UT4ContiAsset::StaticClass(),
			TestAssetName,
			TestAssetPath
		));
		if (nullptr == NewContiAsset)
		{
			return false;
		}
		NewContiAsset->Modify();

		FT4RootAction& RootAction = NewContiAsset->RootAction;
		int32 ParentHeaderKey = INDEX_NONE;
		{
			FT4AnimationAction* NewAction = RootAction.AddChild<FT4AnimationAction>();
			NewAction->SectionName = TEXT("NormalAttack");

			ParentHeaderKey = NewAction->HeaderKey;
		}
		{
			FT4ParticleAction* NewAction = RootAction.AddChild<FT4ParticleAction>(
				ParentHeaderKey
			);
			NewAction->BoneOrSocketName = TEXT("ik_hand_r");
		}
		T4AssetUtil::SaveAsset(NewContiAsset, true); // #39
		NewContiAsset->MarkPendingKill();
	}
	{
		FString LoadContiAssetPath = FString::Printf(
			TEXT("RehearsalConti'%s%s.%s"),
			*TestAssetPath,
			*TestAssetName,
			*TestAssetName
		);
		FSoftObjectPath TestContiAssetPath(LoadContiAssetPath);
		UObject* LoadedObject = TestContiAssetPath.TryLoad();
		if (nullptr == LoadedObject)
		{
			return false;
		}
		UT4ContiAsset* RehearsalConti = Cast<UT4ContiAsset>(LoadedObject);
		if (nullptr == RehearsalConti)
		{
			return false;
		}
		FT4RootAction& RootAction = RehearsalConti->RootAction;
		if (2 != RootAction.HeaderInfoMap.Num())
		{
			return false;
		}
		const FT4ActionHeaderInfo& ParentHierarchyInfo = RootAction.HeaderInfoMap[0];
		if (ParentHierarchyInfo.ParentHeaderKey != INDEX_NONE)
		{
			return false;
		}
		if (ParentHierarchyInfo.ActionType != ET4ActionType::Animation)
		{
			return false;
		}
		const FT4ActionHeaderInfo& ChildHierarchyInfo = RootAction.HeaderInfoMap[1];
		if (ChildHierarchyInfo.ParentHeaderKey != 0)
		{
			return false;
		}
		if (ChildHierarchyInfo.ActionType != ET4ActionType::Particle)
		{
			return false;
		}
		if (0 >= RootAction.AnimationActions.Num())
		{
			return false;
		}
		FT4AnimationAction& AnimationAction = RootAction.AnimationActions[0];
		if (AnimationAction.SectionName != TEXT("NormalAttack"))
		{
			return false;
		}
		if (0 >= RootAction.ParticleActions.Num())
		{
			return false;
		}
		FT4ParticleAction& ParticleAction = RootAction.ParticleActions[0];
		if (ParticleAction.BoneOrSocketName != TEXT("ik_hand_r"))
		{
			return false;
		}
	}
#endif
	return true;
}
*/

#endif //WITH_DEV_AUTOMATION_TESTS