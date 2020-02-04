// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Public/T4EngineTypes.h"
#include "Public/Asset/T4AssetLoader.h"

#include "Public/Action/T4ActionCodeCommandIncludes.h"

#include "Object/ActionNode/T4ActionNodeIncludes.h"
#include "Object/ActionNode/T4ActionNodeControl.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-US/Programming/Automation#enablingautomationtestplugins
 */

#if WITH_DEV_AUTOMATION_TESTS

#if WITH_EDITOR
static const FString T4CoreTestAssetPath = TEXT("/Game/T4Framework/Test/LoadTest/LargeTestAsset.LargeTestAsset");
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FT4AssetLoadTest, "T4Engine.AssetLoad.AsyncLoadTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FT4AssetLoadTest::RunTest(const FString& Parameters)
{
#if WITH_EDITOR
	FSoftObjectPath LoadAssetPath(T4CoreTestAssetPath);
	FT4AssetLoader* NewTestDefaultAssetLoader = new FT4AssetLoader;
	NewTestDefaultAssetLoader->Load(LoadAssetPath, false, TEXT("Test"));
	NewTestDefaultAssetLoader->Reset();
	delete NewTestDefaultAssetLoader;
#endif
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FT4ActionNodeControlCompositeTest, "T4Engine.Action.RootComposite", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FT4ActionNodeControlCompositeTest::RunTest(const FString& Parameters)
{
	FT4ActionNodeControl* NewActionRoot = new FT4ActionNodeControl();
	if (nullptr == NewActionRoot)
	{
		return false;
	}
	bool bResult = true;
	for (;;)
	{
#if 0
		const FT4ActionKey TestActionPrimaryKey(1213456, true);
		FT4AnimationAction NewParentAction;
		NewParentAction.ActionKey = TestActionPrimaryKey;
		NewParentAction.SectionName = TEXT("NormalAttack");
		FT4ActionNodeBase* NewActionNode = NewActionRoot->CreateNode(&NewParentAction, nullptr);
		if (nullptr == NewActionNode)
		{
			bResult = false;
			break;
		}
		IT4ActionNode* ParentActionNode = NewActionRoot->GetChildNodeByPrimary(TestActionPrimaryKey);
		if (nullptr == ParentActionNode)
		{
			bResult = false;
			break;
		}
		if (ParentActionNode != NewActionNode)
		{
			bResult = false;
			break;
		}
		FT4ParticleAction NewChildAction;
		NewChildAction.ActionKey = TestActionPrimaryKey;
		NewChildAction.ActionPoint = TEXT("ik_hand_r"); // TEXT("Root"); // #57 : BoneOrSocketName
		IT4ActionNode* ChildActionNode = ParentActionNode->AddChildNode(&NewChildAction, 0.0f);
		if (nullptr == ChildActionNode)
		{
			bResult = false;
			break;
		}
		bResult = NewActionRoot->DestroyNode(TestActionPrimaryKey, 0.0f, true);
		if (!bResult)
		{
			break;
		}
		if (0 != NewActionRoot->NumChildActions())
		{
			bResult = false;
			break;
		}
#endif
		break;
	}
	if (nullptr != NewActionRoot)
	{
		delete NewActionRoot;
		NewActionRoot = nullptr;
	}
	return bResult;
}

#endif //WITH_DEV_AUTOMATION_TESTS