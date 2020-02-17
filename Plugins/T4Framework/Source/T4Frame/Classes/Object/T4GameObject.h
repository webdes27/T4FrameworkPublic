// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4Frame.h" // #25, #42
#include "T4GameObject.generated.h"

/**
  * #114 : BP 로 노출해서 게임 로직에서 사용한다.
 */
class UWorld;
class IT4GameFrame;
class IT4GameWorld;
UCLASS()
class T4FRAME_API UT4GameObject : public UObject, public IT4GameObject
{
	GENERATED_UCLASS_BODY()
		
	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	virtual ~UT4GameObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	// IT4GameObject
	ET4LayerType GetLayerType() const override { return LayerType; }
	const FT4ObjectID& GetObjectID() const override { return ObjectID; } // #114

public:
	void OnInitialize(ET4LayerType InLayerType, const FT4ObjectID& InObjectID);
	void OnFinalize();

	void OnProcess(float InDeltaTime);

	virtual bool IsPlayer() const { return false; }

	virtual bool IsServerObject() const { return false; }
	virtual bool IsClientObject() const { return false; }

protected:
	virtual void Initialize() {}
	virtual void Finalize() {}

	virtual void Process(float InDeltaTime) {}

	UWorld* GetWorld() const;
	IT4GameFrame* GetGameFrame() const;
	IT4GameWorld* GetGameWorld() const;

	IT4WorldActor* FindWorldActor(const FT4ActorID& InActorID) const;

private:
	ET4LayerType LayerType;
	FT4ObjectID ObjectID;
};
