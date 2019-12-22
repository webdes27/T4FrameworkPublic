// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MovieSceneSequence.h"
#include "T4ContiActionSequence.generated.h"

/**
 * Movie scene sequence used by T4Rehearsal.
 */
class UMovieScene;
class UT4ContiActionMovieScene; // #54
class UT4ContiAsset;
class FT4ContiViewModel;

UCLASS(MinimalAPI)
class UT4ContiActionSequence : public UMovieSceneSequence
{
	GENERATED_UCLASS_BODY()

public:
	void Initialize(FT4ContiViewModel* InViewModel, UT4ContiActionMovieScene* InMovieScene);

	FT4ContiViewModel* GetViewModel() const;
	UT4ContiAsset* GetContiAsset() const;

	//~ UMovieSceneSequence interface
	virtual void BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context) override;
	virtual bool CanPossessObject(UObject& Object, UObject* InPlaybackContext) const override;
	virtual UMovieScene* GetMovieScene() const override;
	virtual UObject* GetParentObject(UObject* Object) const override;
	virtual void UnbindPossessableObjects(const FGuid& ObjectId) override;

private:
	/** Pointer to the movie scene that controls this sequence. */
	UPROPERTY()
	UT4ContiActionMovieScene* ContiMovieScene;

	/** The system view model which owns this T4Rehearsal sequence. */
	FT4ContiViewModel* ViewModel;
};
