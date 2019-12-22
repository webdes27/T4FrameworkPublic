// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiActionSequence.h"
#include "T4ContiActionMovieScene.h" // #54
#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "T4RehearsalEditorInternal.h"

/**
  *
 */
UT4ContiActionSequence::UT4ContiActionSequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ContiMovieScene(nullptr)
{
}

void UT4ContiActionSequence::Initialize(
	FT4ContiViewModel* InViewModel, 
	UT4ContiActionMovieScene* InMovieScene
)
{
	ViewModel = InViewModel;
	ContiMovieScene = InMovieScene;
}

FT4ContiViewModel* UT4ContiActionSequence::GetViewModel() const
{
	checkf(ViewModel != nullptr, TEXT("T4RehearsalConti Sequence not initialized"));
	return ViewModel;
}

UT4ContiAsset* UT4ContiActionSequence::GetContiAsset() const
{
	if (nullptr == ViewModel)
	{
		return nullptr;
	}
	return ViewModel->GetContiAsset();
}

void UT4ContiActionSequence::BindPossessableObject(
	const FGuid& ObjectId,
	UObject& PossessedObject, 
	UObject* Context
)
{
}

bool UT4ContiActionSequence::CanPossessObject(UObject& Object, UObject* InPlaybackContext) const
{
	return false;
}

UMovieScene* UT4ContiActionSequence::GetMovieScene() const
{
	checkf(ContiMovieScene != nullptr, TEXT("T4RehearsalConti Sequence not initialized"));
	return Cast<UMovieScene>(ContiMovieScene);
}

UObject* UT4ContiActionSequence::GetParentObject(UObject* Object) const
{
	return nullptr;
}

void UT4ContiActionSequence::UnbindPossessableObjects(const FGuid& ObjectId)
{
}
