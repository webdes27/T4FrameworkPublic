// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/T4EngineLayer.h"

#include "Engine/Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */

namespace T4EngineLayer
{
#if WITH_EDITOR
	static TSet<ET4LayerType> GUsingLayerSet; // #30
	static TMap<FName, ET4LayerType> GReservedMultiLayerMap; // #30
#endif

	FORCEINLINE bool IsMultiLayer(const EWorldType::Type InWorldType) // #30
	{
		if (PreviewWorldType != InWorldType && EWorldType::PIE != InWorldType)
		{
			return false;
		}
		return true;
	}

	bool Add(const FWorldContext* InWorldContext) // #30
	{
#if WITH_EDITOR
		check(nullptr != InWorldContext);
		if (IsServer(InWorldContext))
		{
			return false;
		}
		const EWorldType::Type CurrentWorldType = InWorldContext->WorldType;
		if (!IsMultiLayer(CurrentWorldType))
		{
			return false;
		}
		const FName& WorldContextName = InWorldContext->ContextHandle;
		if (GReservedMultiLayerMap.Contains(WorldContextName))
		{
			return true;
		}
		uint32 StartLayer = 0;
		uint32 EndLayer = 0;
		if (PreviewWorldType == CurrentWorldType)
		{
			StartLayer = uint32(ET4LayerType::Preview);
			EndLayer = uint32(ET4LayerType::PreviewMax);
		}
		else if (EWorldType::PIE == CurrentWorldType)
		{
			StartLayer = uint32(ET4LayerType::Client);
			EndLayer = uint32(ET4LayerType::ClientMax);
		}
		else
		{
			check(false); // WARN : IsMultiLayer check it!
		}
		ET4LayerType ReservedLayerType = ET4LayerType::Max;
		for (uint32 Layer = StartLayer; Layer < EndLayer; ++Layer)
		{
			ET4LayerType ConvLayerType = (ET4LayerType)Layer;
			if (!GUsingLayerSet.Contains(ConvLayerType))
			{
				ReservedLayerType = ConvLayerType;
				break;
			}
		}
		if (ET4LayerType::Max == ReservedLayerType)
		{
			return false; // WARN : 더이상 할당할 MultiSceneLyaer 가 없다. ET4LayerType Enum 을 추가할 것!
		}
		GUsingLayerSet.Add(ReservedLayerType);
		GReservedMultiLayerMap.Add(WorldContextName, ReservedLayerType);
#endif
		return true;
	}

	void Remove(const FName& InWorldContextName) // #30
	{
#if WITH_EDITOR
		if (!GReservedMultiLayerMap.Contains(InWorldContextName))
		{
			return;
		}
		ET4LayerType UsingLayerType = GReservedMultiLayerMap[InWorldContextName];
		check(GUsingLayerSet.Contains(UsingLayerType));
		GUsingLayerSet.Remove(UsingLayerType);
		GReservedMultiLayerMap.Remove(InWorldContextName);
#endif
	}

	ET4LayerType Get(const FWorldContext* InWorldContext)
	{
		check(nullptr != InWorldContext);
		if (IsServer(InWorldContext))
		{
			return ET4LayerType::Server;
		}
		ET4LayerType ReturnLayerType = ET4LayerType::Max;
#if WITH_EDITOR
		EWorldType::Type CurrentWorldType = InWorldContext->WorldType;
		if (IsMultiLayer(CurrentWorldType))
		{
			if (GReservedMultiLayerMap.Contains(InWorldContext->ContextHandle))
			{
				ReturnLayerType = GReservedMultiLayerMap[InWorldContext->ContextHandle];
				check(GUsingLayerSet.Contains(ReturnLayerType));
			}
			else
			{
				return ET4LayerType::Max; // WARN : 제한 초과? 여기가 호출되면 절대 안됨!!!
			}
		}
		else if (EWorldType::Editor == CurrentWorldType)
		{
			ReturnLayerType = ET4LayerType::LevelEditor; // #17
		}
		else
		{
			ReturnLayerType = ET4LayerType::Client;
		}
#else
		ReturnLayerType = ET4LayerType::Client;
#endif
		check(ET4LayerType::Max != ReturnLayerType); // WARN : 미지원 Layer??
		return ReturnLayerType;
	}

	ET4LayerType Get(const UWorld* InWorld)
	{
		check(nullptr != InWorld);
		const FWorldContext* WorldContext = nullptr;
#if 1
		// #79 : GetWorldContextFromWorld 를 사용하면 안된다!
		//       EditorWorld 와 GamePreview World 가 같을 수 있기 때문...
		const EWorldType::Type WorldType = InWorld->WorldType;
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		for (const FWorldContext& Context : WorldContexts)
		{
			if (WorldType == Context.WorldType && Context.World() == InWorld)
			{
				WorldContext = &Context;
				break;
			}
		}
#else
		WorldContext = GetWorldContextFromWorld(InWorld);
#endif
		if (nullptr == WorldContext)
		{
			return ET4LayerType::Max;
		}
		if (IsServer(WorldContext))
		{
			return ET4LayerType::Server;
		}
		return Get(WorldContext);
	}

	bool IsServer(const FWorldContext* InWorldContext) // #15
	{
		UWorld* UnrealWorld = InWorldContext->World();
		check(nullptr != UnrealWorld);
		const ENetMode CurrNetMode = UnrealWorld->GetNetMode();
		return (ENetMode::NM_DedicatedServer == CurrNetMode) ? true : false;
	}

	ET4LayerType FromString(const FString& InLayerString)
	{
		ET4LayerType LayerType = ET4LayerType::Max;
		if (InLayerString.Contains(TEXT("Client")))
		{
			LayerType = ET4LayerType::Client;
#if WITH_EDITOR
			if (6 != InLayerString.Len()) // hack!! 6 == Client
			{
				FString LayerString = InLayerString;
				LayerString.RemoveFromStart(TEXT("Client"));
				int32 LayerIndex = FCString::Atoi(*LayerString);
				if (0 < LayerIndex)
				{
					LayerType = ET4LayerType(uint32(LayerType) + LayerIndex);
				}
			}
#endif
		}
		else if (InLayerString.Contains(TEXT("Server")))
		{
			LayerType = ET4LayerType::Server;
		}
#if WITH_EDITOR
		else if (InLayerString.Contains(TEXT("LevelEditor")))
		{
			LayerType = ET4LayerType::LevelEditor;
		}
		else if (InLayerString.Contains(TEXT("Preview")))
		{
			LayerType = ET4LayerType::Preview;
			if (7 != InLayerString.Len()) // hack!! 7 == Preview
			{
				FString LayerString = InLayerString;
				LayerString.RemoveFromStart(TEXT("Preview"));
				int32 LayerIndex = FCString::Atoi(*LayerString);
				if (0 < LayerIndex)
				{
					LayerType = ET4LayerType(uint32(LayerType) + LayerIndex);
				}
			}
		}
#endif
		return LayerType;
	}

	const FString ToString(ET4LayerType InLayerType)
	{
		FString ReturnLayerString = TEXT("");
		if (T4EngineLayer::IsClient(InLayerType))
		{
			if (ET4LayerType::Client == InLayerType)
			{
				ReturnLayerString = TEXT("Client");
			}
			else
			{
#if WITH_EDITOR
				ReturnLayerString = FString::Printf(
					TEXT("Client%u"),
					uint32(InLayerType) - uint32(ET4LayerType::Client)
				);
#endif
			}
		}
		else if (ET4LayerType::Server == InLayerType)
		{
			ReturnLayerString = TEXT("Server");
		}
#if WITH_EDITOR
		else if (ET4LayerType::LevelEditor == InLayerType)
		{
			ReturnLayerString = TEXT("LevelEditor");
		}
		else if (T4EngineLayer::IsPreview(InLayerType))
		{
			if (ET4LayerType::Preview == InLayerType)
			{
				ReturnLayerString = TEXT("Preview");
			}
			else
			{
#if WITH_EDITOR
				ReturnLayerString = FString::Printf(
					TEXT("Preview%u"), 
					uint32(InLayerType) - uint32(ET4LayerType::Preview)
				);
#endif
			}
		}
#endif
		return ReturnLayerString;
	}
}