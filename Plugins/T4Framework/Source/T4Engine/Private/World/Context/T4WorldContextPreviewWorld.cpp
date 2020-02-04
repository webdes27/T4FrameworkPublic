// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldContextPreviewWorld.h"

#if WITH_EDITOR

#include "Preview/T4WorldPreviewGameInstance.h" // #79

#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #87

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "Engine/LevelStreaming.h"
#include "Engine/WorldComposition.h"
#include "GameFramework/PlayerController.h"

#include "T4EngineInternal.h"

/**
  * #87
 */
FT4WorldContextPreviewWorld::FT4WorldContextPreviewWorld(FT4WorldController* InWorldController)
	: WorldControllerRef(InWorldController)
	, WorldContext(nullptr)
	, GameInstance(nullptr)
{
	WorldContext = &GEngine->CreateNewWorldContext(T4EngineLayer::PreviewWorldType);

	UWorld* EmptyWorld = NewEmptyWorld();
	WorldContext->SetCurrentWorld(EmptyWorld);

	// #17 : bActorsInitialized 를 강제로 호출해 PlayerController 가 스폰될 수 있도록 처리
	//       단, 불필요한 레벨 저장이 일어나지는 않는지 체크 필요!
	EmptyWorld->InitializeActorsForPlay(FURL()); // bActorsInitialized check
}

FT4WorldContextPreviewWorld::~FT4WorldContextPreviewWorld()
{
	check(nullptr == GameInstance);
}

void FT4WorldContextPreviewWorld::Reset()
{
	if (nullptr != GameInstance) // #79
	{
		GameInstance->RemoveFromRoot();
		GameInstance = nullptr;
	}
	if (nullptr == WorldContext)
	{
		return;
	}
	UWorld* CurrentWorld = WorldContext->World();
	if (nullptr != CurrentWorld)
	{
		FlushAndCleanUpCurrentWorld();
		// #79 : WorldContext 삭제는 현재 DestroyWorldContext API만 제공하는데 
		//       이 처리는 WorldContext::GetWorld() 만 비교하기 때문에 Preview 에서 로드한 월드가 EditorWorld 와
		//       같을 경우 EditorWorldContext 를 삭제하는 문제가 있어 아래와 같이 종료 시점에 CurrentWorld 를
		//       변경해 삭제할 수 있도록 처리한다.
		UWorld* DummyWorld = NewEmptyWorld();
		WorldContext->SetCurrentWorld(DummyWorld);
		GEngine->DestroyWorldContext(DummyWorld);
		DummyWorld->DestroyWorld(false);
	}
	CurrentWorld->DestroyWorld(false);
	CurrentWorld = nullptr;
	WorldContext = nullptr;
}

void FT4WorldContextPreviewWorld::ProcessPre(float InDeltaTime) // #34 : OnWorldPreActorTick
{
	check(nullptr != WorldContext);
	UWorld* CurrentWorld = WorldContext->World();
	if (nullptr == CurrentWorld)
	{
		return;
	}
	// #86 : FT4RehearsalViewportClient::Tick 에서 World Tick 으로 진입
	//       UpdateLevelStreaming 은 FT4RehearsalViewportClient::Draw에서 불림
}

void FT4WorldContextPreviewWorld::ProcessPost(float InDeltaTime) // #34 : OnWorldPostActorTick
{
	UWorld* CurrentWorld = WorldContext->World();
	if (nullptr == CurrentWorld)
	{
		return;
	}
	// #86 : FT4RehearsalViewportClient::Tick 에서 World Tick 으로 진입
	//       UpdateLevelStreaming 은 FT4RehearsalViewportClient::Draw에서 불림
}

bool FT4WorldContextPreviewWorld::WorldTravel(
	const FSoftObjectPath& InAssetPath, 
	const FVector& InStartLocation
)
{
	Reinitialize();

	const FString TravelURL = InAssetPath.GetLongPackageName();

	// #79 : Preview 에서는 다른 창에서 같은 월드를 열 수 있기 때문에 PIE 처리와 같이
	//       원본을 복제해서 World 를 생성하도록 처리해주기 위한 조치이다.
	CheckAndNewSourceWorldContext(*TravelURL);

	UWorld* EditorWorld = GWorld; // #79
	{
		{
			//GEngine->SetClientTravel(PreviewWorld, *CVS.InitializeURL, TRAVEL_Absolute);
			WorldContext->TravelURL = TravelURL;
			WorldContext->TravelType = TRAVEL_Absolute;
		}
		GEngine->TickWorldTravel(*WorldContext, 0.0f);
		UWorld* CurrentWorld = WorldContext->World();
		check(nullptr != CurrentWorld);
		UPackage* WorldPackage = CurrentWorld->GetOutermost();
		if (nullptr != WorldPackage)
		{
			if (!WorldPackage->HasAnyFlags(RF_Transient)) // #83 : 혹시라도 저장되지 않도록 방지한다.
			{
				WorldPackage->SetFlags(RF_Transient);
			}
			// Hack! 툴이 떠 있는 상황에서 PIE 실행후 종료시 UEditorEngine::EndPlayMap() 에서
			// PKG_PlayInEditor 가 켜진 Package 가 있을 경우 CriticalError 를 발생시킴으로 툴에서 로드한 월드는
			// 해당 플레그를 꺼준다. 현재까지는 체크 용도외에는 없는 듯하나, 문제가 발생할 수도 있을 것임!
			if (WorldPackage->HasAnyPackageFlags(PKG_PlayInEditor))
			{
				WorldPackage->ClearPackageFlags(PKG_PlayInEditor);
			}
		}
	}
	// Hack! TickWorldTravel 호출로 GIsPlayInEditorWorld = true 가 되어 PIE tick 에서 assert 조건이 됨으로 해당 옵션을 꺼준다.
	// GWorld = EditorWorld 와 같이 처리할 경우 PIE 실행상태에서 툴 실행시 assert 가 발생한다.
	RestoreEditorWorld(EditorWorld);

	if (!InStartLocation.IsNearlyZero())
	{
		// #87 : World Composition 을 사용하고, 시작 좌표가 있다면 강제로 로드한다.
		UWorld* CurrentWorld = WorldContext->World();
		check(nullptr != CurrentWorld);
		if (nullptr != CurrentWorld->WorldComposition)
		{
			CurrentWorld->WorldComposition->UpdateStreamingState(InStartLocation);
			CurrentWorld->FlushLevelStreaming();
		}
	}
	return true;
}

void FT4WorldContextPreviewWorld::SetPlayerController(APlayerController* InPlayerController) // #86, #87 : Only PreviewWorld
{
	// #86 : GameInstance 를 통한 PlayerContoller 처리를 하지 않음으로 임시로 LocalPlayer 를 설정해준다.
	// #87 : Rehearsal ViewModel 에 있던 처리를 이곳으로 옮김
	if (nullptr == InPlayerController)
	{
		return;
	}
	if (nullptr != GameInstance)
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(InPlayerController->Player);
		check(nullptr != LocalPlayer);
		GameInstance->SetLocalPlayer(LocalPlayer);
	}
}

void FT4WorldContextPreviewWorld::SetLevelStreamingFrozen(bool bInFrozen) // #86 : Only PreviewWorld
{
	check(nullptr != WorldContext);
	UWorld* CurrentWorld = WorldContext->World();
	if (nullptr == CurrentWorld)
	{
		return;
	}
	CurrentWorld->bIsLevelStreamingFrozen = bInFrozen; // 자동 레벨 스트리밍 flag 제어

	// #86 : World 의 UpdateStreamingState 를 제어하기 위한 옵션 처리
	if (nullptr != GameInstance)
	{
		GameInstance->SetLevelStreaming(!bInFrozen);
	}	
}

bool FT4WorldContextPreviewWorld::IsLevelStreamingFrozen() const // #86, #104
{
	check(nullptr != WorldContext);
	UWorld* CurrentWorld = WorldContext->World();
	if (nullptr == CurrentWorld)
	{
		return false;
	}
	return CurrentWorld->bIsLevelStreamingFrozen;
}

void FT4WorldContextPreviewWorld::Reinitialize()
{
	if (nullptr != GameInstance) // #86 : 월드가 바뀌면 GameInstance 도 새로 만들어준다.
	{
		GameInstance->RemoveFromRoot();
		GameInstance = nullptr;
	}
	if (nullptr == WorldContext)
	{
		WorldContext = &GEngine->CreateNewWorldContext(T4EngineLayer::PreviewWorldType);

		UWorld* EmptyWorld = NewEmptyWorld();
		WorldContext->SetCurrentWorld(EmptyWorld);
		// #17 : bActorsInitialized 를 강제로 호출해 PlayerController 가 스폰될 수 있도록 처리
		//       단, 불필요한 레벨 저장이 일어나지는 않는지 체크 필요!
		EmptyWorld->InitializeActorsForPlay(FURL()); // bActorsInitialized check
	}
	GameInstance = NewObject<UT4WorldPreviewGameInstance>(); // #79
	GameInstance->AddToRoot();
	GameInstance->SetWorldContext(WorldContext);
	WorldContext->OwningGameInstance = GameInstance; // #79
}

void FT4WorldContextPreviewWorld::FlushAndCleanUpCurrentWorld() // #83
{
	// #83 
	check(nullptr != WorldContext);
	UWorld* CurrentWorld = WorldContext->World();
	if (nullptr == CurrentWorld)
	{
		return;
	}
	if (nullptr != CurrentWorld->WorldComposition)
	{
		// #83 : World Composition 을 사용할 경우는 LevelStreaming Flush 에서 Context 를 참조하기 때문에
		//       SetCurrentWorld 전 먼저 LevelStreaming 을 정리해준다.
		CurrentWorld->bIsLevelStreamingFrozen = false;
		CurrentWorld->SetShouldForceUnloadStreamingLevels(true);
		CurrentWorld->FlushLevelStreaming();

		// #83 : 서브레벨을 로드하였을 경우 CurrentWorld DestroyWorld 호출로는 GC 타이밍을 특정할 수 없기 때문에
		//       다음 레벨 로드시 LoadMap::VerifyLoadMapWorldCleanup 에서 Assert 조건이 됨 (GamePreview)
		//       서브레벨이 있을 경우에 한해 Presisitent Level 삭제시 SubLevel 의 World 를 찾아 DestroyWorld 를 강제로 호출하여 정리함.
		CurrentWorld->bIsLevelStreamingFrozen = false;
		CurrentWorld->FlushLevelStreaming();
		for (ULevelStreaming* StreamingLevel : CurrentWorld->GetStreamingLevels())
		{
			// refer ULevelStreaming::RequestLevel
			// #83 : ULevelStreaming 은 Load/Unload 처리가 동작하기 때문에 SubLevel world 의 패키지를 찾아 삭제해준다.
			//       참고로 Persistent World 생성시 PIE prefix 를 붙였기 때문에 SubLevel Package 에도 Prerix 가 있어 Editor World 의
			//       레벨과 중복되지 않는다. Unique
			const FName DesiredPackageName = *StreamingLevel->GetWorldAssetPackageName();
			UPackage* LevelPackage = (UPackage*)StaticFindObjectFast(
				UPackage::StaticClass(),
				nullptr,
				DesiredPackageName,
				0,
				0,
				RF_NoFlags,
				EInternalObjectFlags::PendingKill
			);
			if (nullptr != LevelPackage)
			{
				UWorld* LevelWorld = UWorld::FindWorldInPackage(LevelPackage);
				if (nullptr != LevelWorld)
				{
					LevelWorld->DestroyWorld(false);
				}
			}
		}
	}
}

void FT4WorldContextPreviewWorld::CheckAndNewSourceWorldContext(const TCHAR* InTravelURL)
{
	check(nullptr != WorldContext);
	
	// #79 : Preview 에서는 다른 창에서 같은 월드를 열 수 있기 때문에 PIE 처리와 같이
	//       원본을 복제해서 World 를 생성하도록 처리해주기 위한 조치이다.

	const int32 DefaultPreviewInstanceIndex = 1000000;
	static int32 GPreviewInstanceIndex = DefaultPreviewInstanceIndex;
	{
		WorldContext->PIEInstance = GPreviewInstanceIndex--;
		WorldContext->PIEPrefix = InTravelURL;
		if (10000 >= GPreviewInstanceIndex)
		{
			GPreviewInstanceIndex = DefaultPreviewInstanceIndex;
		}
	}
	FWorldContext& EditorContext = GEditor->GetEditorWorldContext();
	const FString EditorWorldPackageName = EditorContext.World()->GetOutermost()->GetName();
	if (InTravelURL == EditorWorldPackageName)
	{
		EditorContext.PIEPrefix = EditorWorldPackageName;
	}
	else
	{
		bool bEditorWorldLoaded = false;
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		for (const FWorldContext& Context : WorldContexts)
		{
			if (T4EngineLayer::PreviewWorldType == Context.WorldType && nullptr != Context.World())
			{
				const FString WorldPackageName = Context.World()->GetOutermost()->GetName();
				if (WorldPackageName == InTravelURL)
				{
					bEditorWorldLoaded = true;
					break;
				}
			}
		}
		if (!bEditorWorldLoaded)
		{
			UWorld* SaveWorld = GWorld; // #79
			{
				// #89 : 이 처리는 PreviewWorld 의 SubLevel 로드시 EditorWorld Package 를 먼저 로드해주는 처리와 사실상 같음 (FT4WorldPreviewViewModel::Tick(float DeltaTime))
				FString ErrorString;
				FWorldContext& NewEditorWorldContext = GEngine->CreateNewWorldContext(T4EngineLayer::PreviewWorldType);
				NewEditorWorldContext.OwningGameInstance = GameInstance; // #79
				EBrowseReturnVal::Type Result = GEngine->Browse(NewEditorWorldContext, FURL(InTravelURL), ErrorString);
				if (EBrowseReturnVal::Success != Result)
				{
					check(false);
				}
				NewEditorWorldContext.PIEPrefix = InTravelURL;
			}
			GWorld = SaveWorld;
		}
		// TODO : PIE 종료시 Preview 창이 열려있으면 크래시 발생. 확인 필요
	}
}

UWorld* FT4WorldContextPreviewWorld::NewEmptyWorld()
{
	UWorld* NewEmptyWorld = NewObject<UWorld>(GetTransientPackage(), NAME_None, RF_Transactional);
	NewEmptyWorld->WorldType = T4EngineLayer::PreviewWorldType;
	NewEmptyWorld->InitializeNewWorld(
		UWorld::InitializationValues()
		.AllowAudioPlayback(false)
		.CreatePhysicsScene(false)
		.RequiresHitProxies(false)
		.CreateNavigation(false)
		.CreateAISystem(false)
		.ShouldSimulatePhysics(false)
		.SetTransactional(true)
	);
	return NewEmptyWorld;
}

#endif