// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Public/T4EngineTypes.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "T4Asset/Public/Entity/T4EntityKey.h"
#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-US/Programming/Automation#enablingautomationtestplugins
 */

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FT4ObjectIDTest, "T4Engine.Type.FT4ObjectID", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FT4ObjectIDTest::RunTest(const FString& Parameters)
{
	FT4ObjectID TestObjectID;

	TestFalse(FString(TEXT("Testing Default FT4ObjectID::IsValid()")), TestObjectID.IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FT4ActionKeyTest, "T4Engine.Type.FT4ActionKey", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FT4ActionKeyTest::RunTest(const FString& Parameters)
{
	FT4ActionKey TestActionDefaultKey;
	TestFalse(FString(TEXT("Testing Default FT4ActionKey::IsValid()")), TestActionDefaultKey.IsValid());

	FT4ActionKey TestActionPrimaryKey(12345, true);
	FT4ActionKey TestActionNormalKey(12345);
	TestTrue(FString(TEXT("Testing FT4ActionKey PrimaryKey(12345) != NromalKey(12345)")), TestActionPrimaryKey != TestActionNormalKey);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FT4EntityTest, "T4Engine.Type.FT4EntityKey", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FT4EntityTest::RunTest(const FString& Parameters)
{
	FT4EntityKey TestEntityKey;

	TestFalse(FString(TEXT("Testing Default FT4EntityKey::IsValid()")), TestEntityKey.IsValid());

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS