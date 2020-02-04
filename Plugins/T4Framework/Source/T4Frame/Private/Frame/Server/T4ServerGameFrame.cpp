// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ServerGameFrame.h"

#if (WITH_EDITOR || WITH_SERVER_CODE)
#include "T4Engine/Public/T4Engine.h"

#include "AIController.h"

#include "T4FrameInternal.h"
#endif

/**
  *
 */
#if (WITH_EDITOR || WITH_SERVER_CODE)
FT4ServerGameFrame::FT4ServerGameFrame()
	: UniqueNetIDIncr(0) // #15
	, UniqueObjectIDIncr(0)
{
}

FT4ServerGameFrame::~FT4ServerGameFrame()
{
}

bool FT4ServerGameFrame::Initialize()
{
	return true;
}

void FT4ServerGameFrame::Finalize()
{
}

void FT4ServerGameFrame::ResetPre()
{
	for (TMap<FT4NetID, IT4GameAIController*>::TConstIterator It = GameAIControllerRefs.CreateConstIterator(); It; ++It)
	{
		IT4GameAIController* GameAIController = It.Value();
		check(nullptr != GameAIController);
		// #68 : GameplayAIControllerFactory 에서 AddToRoot 를 한 상황임으로 여기서는 Root 에서만 빼주고
		//       GameObject (APawn) 소멸시 AController 가 함께 소멸되도록 처리해준다.
		//       참고로 여기는 강제 종료시 호출됨. 현재 T4GameBuiltin 에서는 따로 컨테이너가 없기 때문. 좋은 방법이 있으면 변경!
		// #104 : T4GameBuiltin 모듈에서 Spawn 시켜놓고 소멸 되지 않도록 AddToRoot 를 했는데,
		//        현재 명시적인 Leave 처리가 없어서 여기서 강제로 RemoveFromRoot 를 해주고 있음. 정리 필요
		check(nullptr != GameAIController->GetAIController());
		GameAIController->GetAIController()->RemoveFromRoot();
	}
}

void FT4ServerGameFrame::ResetPost()
{
	UniqueNetIDIncr = 0;
	UniqueObjectIDIncr = 0;
	GameAIControllerRefs.Empty(); // #31
}

void FT4ServerGameFrame::StartPlay()
{
}

void FT4ServerGameFrame::ProcessPre(float InDeltaTime)
{
	for (TMap<FT4NetID, IT4GameAIController*>::TConstIterator It = GameAIControllerRefs.CreateConstIterator(); It; ++It)
	{
		// TODO : NPC AI Proc
	}
}

void FT4ServerGameFrame::ProcessPost(float InDeltaTime)
{
}

FT4ObjectID FT4ServerGameFrame::GenerateObjectIDForServer()
{
	UniqueObjectIDIncr++;
	return UniqueObjectIDIncr;
}

bool FT4ServerGameFrame::RegisterGameAIController(
	const FT4NetID& InUniqueID, 
	IT4GameAIController* InController
)
{
	// #31
	if (GameAIControllerRefs.Contains(InUniqueID))
	{
		T4_LOG(
			Error,
			TEXT("Already added. %s"),
			*(InUniqueID.ToString())
		);
		return false;
	}
	GameAIControllerRefs.Add(InUniqueID, InController);
	return true;
}

void FT4ServerGameFrame::UnregisterGameAIController(const FT4NetID& InUniqueID)
{
	// #31
	if (!GameAIControllerRefs.Contains(InUniqueID))
	{
		T4_LOG(
			Error,
			TEXT("Added contgroller not found. UniqueID = '%u'"),
			*(InUniqueID.ToString())
		);
		return;
	}
	{
		// #68 : GameplayAIControllerFactory 에서 AddToRoot 가 되었음으로 삭제시에는 Root 에서 빼준다.
		// #104 : T4GameBuiltin 모듈에서 Spawn 시켜놓고 소멸 되지 않도록 AddToRoot 를 했는데,
		//        현재 명시적인 Leave 처리가 없어서 여기서 강제로 RemoveFromRoot 를 해주고 있음. 정리 필요
		IT4GameAIController* GameAIController = GameAIControllerRefs[InUniqueID];
		check(nullptr != GameAIController);
		check(nullptr != GameAIController->GetAIController());
		GameAIController->GetAIController()->RemoveFromRoot();
	}
	GameAIControllerRefs.Remove(InUniqueID);
}

IT4GameAIController* FT4ServerGameFrame::FindGameAIController(const FT4NetID& InUniqueID) const
{
	// #31
	if (!GameAIControllerRefs.Contains(InUniqueID))
	{
		return nullptr;
	}
	return GameAIControllerRefs[InUniqueID];
}

#endif
