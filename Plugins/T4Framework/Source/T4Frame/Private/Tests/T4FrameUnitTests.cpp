// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Public/T4FrameNetwork.h"
#include "T4FrameInternal.h"

/**
  * https://docs.unrealengine.com/en-US/Programming/Automation#enablingautomationtestplugins
 */

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FT4NetIDTest, "T4Framework.Type.FT4NetID", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FT4NetIDTest::RunTest(const FString& Parameters)
{
	FT4NetID TestNetID;

	TestFalse(FString(TEXT("Testing Default FT4NetID::IsValid()")), TestNetID.IsValid());

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS