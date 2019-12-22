// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "T4Asset/Public/Action/T4ActionTypes.h"

#include "MovieSceneTrackEditor.h"

/**
  *
 */
class FT4ContiActionTrackEditor : public FMovieSceneTrackEditor
{
public:
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer);

public:
	FT4ContiActionTrackEditor(TSharedRef<ISequencer> InSequencer);

	//~ ISequencerTrackEditor interface
	TSharedRef<ISequencerSection> MakeSectionInterface(
		UMovieSceneSection& InSectionObject,
		UMovieSceneTrack& InTrack,
		FGuid InObjectBinding
	) override;

	bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	bool SupportsSequence(UMovieSceneSequence* InSequence) const override;

	const FSlateBrush* GetIconBrush() const override;

	void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	void BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track) override;

	TSharedPtr<SWidget> BuildOutlinerEditWidget(
		const FGuid& InObjectBinding, 
		UMovieSceneTrack* InTrack,
		const FBuildEditWidgetParams& InParams
	) override;

private:
	void OnViewModelChanged();

	TSharedRef<SWidget> HandleOnAddActionSubMenu();

	bool AddActionTrackExecute(ET4ActionType InActionType);
	bool HandleAddActionTrackCanExecute() const;

	// #T4_ADD_ACTION_TAG_CONTI
	void HandleAddBranchTrackExecute();
	void HandleAddSpecialMoveTrackExecute();
	void HandleAddAnimationTrackExecute();
	void HandleAddParticleTrackExecute();
	void HandleAddDecalTrackExecute(); // #54
	void HandleAddProjectileTrackExecute(); // #63
	void HandleAddReactionTrackExecute(); // #76
	void HandleAddLayerSetTrackExecute(); // #81
	void HandleAddTimeScaleTrackExecute(); // #102
	void HandleAddCameraWorkTrackExecute(); // #58
	void HandleAddCameraShakeTrackExecute(); // #101
	void HandleAddPostProcessTrackExecute(); // #100
	void HandleAddEnvironmentTrackExecute(); // #99
};