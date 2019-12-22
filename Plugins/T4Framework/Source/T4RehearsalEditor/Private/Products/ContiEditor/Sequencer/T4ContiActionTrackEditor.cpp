// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiActionTrackEditor.h"
#include "T4ContiActionSequence.h"
#include "T4ContiActionSequencerSection.h"

#include "MovieScene/T4ContiActionTrack.h"
#include "MovieScene/T4ContiActionSection.h"

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"
#include "T4RehearsalEditorStyle.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISequencerSection.h"
#include "MovieSceneTrack.h"
#include "CommonMovieSceneTools.h"
#include "Modules/ModuleManager.h"

#include "Widgets/SBoxPanel.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4ContiActionTrackEditor"

/**
  *
 */
class SContiTrackWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SContiTrackWidget) {}
		SLATE_ARGUMENT(UT4ContiActionTrack*, ContiTrack)
		SLATE_ARGUMENT(FT4ContiViewModel*, ContiViewModel)
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs)
	{
		ContiTrack = InArgs._ContiTrack;
		ContiViewModel = InArgs._ContiViewModel;

		{
			TSharedRef<SHorizontalBox> TrackBox = SNew(SHorizontalBox)
				// Enabled checkbox.
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(3, 0, 0, 0)
				[
					SNew(SCheckBox)
					.ToolTipText(LOCTEXT("T4ContiTrackEnabledTooltip", "Toggle whether or not this action is enabled."))
					.IsChecked(this, &SContiTrackWidget::GetEnabledCheckState)
					.OnCheckStateChanged(this, &SContiTrackWidget::OnEnabledCheckStateChanged)
					.Visibility(this, &SContiTrackWidget::GetEnableCheckboxVisibility)
				]
				// Isolate toggle
				+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(3, 0, 0, 0)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
						.HAlign(HAlign_Center)
						.ContentPadding(1)
						.ToolTipText(this, &SContiTrackWidget::GetToggleIsolateToolTip)
						.OnClicked(this, &SContiTrackWidget::OnToggleIsolateButtonClicked)
						.Visibility(this, &SContiTrackWidget::GetIsolateToggleVisibility)
						.Content()
						[
							SNew(SImage)
							.Image(FT4RehearsalEditorStyle::Get().GetBrush("T4RehearsalEditorStyle.ContiTrackActionIsolate_16x"))
						.ColorAndOpacity(this, &SContiTrackWidget::GetToggleIsolateImageColor)
						]
					];

			ChildSlot
				[
					TrackBox
				];
		}
	}

private:
	ECheckBoxState GetEnabledCheckState() const
	{
		return ContiTrack.IsValid() && ContiViewModel->IsActionInvisible(ContiTrack.Get()) 
			? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnEnabledCheckStateChanged(ECheckBoxState InCheckState)
	{
		if (ContiTrack.IsValid())
		{
			const FScopedTransaction Transaction(LOCTEXT("OnEnabledCheckStateChanged_Transaction", "Update a Editor Parameters"));
			ContiViewModel->SetActionInvisible(ContiTrack.Get(), InCheckState == ECheckBoxState::Checked); // #56
		}
	}

	FReply OnToggleIsolateButtonClicked()
	{
		if (ContiTrack.IsValid())
		{
			ContiViewModel->HandleOnToggleActionIsolation(ContiTrack.Get()); // #56
		}
		return FReply::Handled();
	}

	FText GetToggleIsolateToolTip() const
	{
		return ContiTrack.IsValid() && ContiViewModel->IsActionIsolate(ContiTrack.Get())
			? LOCTEXT("T4ContiTrackTurnOffActionIsolation", "Disable action isolation.")
			: LOCTEXT("T4ContiTrackIsolateThisAction", "Enable isolation for this action.");
	}

	FSlateColor GetToggleIsolateImageColor() const
	{
		return ContiTrack.IsValid() && ContiViewModel->IsActionIsolate(ContiTrack.Get())
			? FEditorStyle::GetSlateColor("SelectionColor")
			: FLinearColor::Gray;
	}

	EVisibility GetEnableCheckboxVisibility() const
	{
		return ContiTrack.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
	}

	EVisibility GetIsolateToggleVisibility() const
	{
		return ContiTrack.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
	}

private:
	TWeakObjectPtr<UT4ContiActionTrack> ContiTrack;
	FT4ContiViewModel* ContiViewModel;
	mutable TOptional<FText> TrackErrorIconToolTip;
};

TSharedRef<ISequencerTrackEditor> FT4ContiActionTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FT4ContiActionTrackEditor(InSequencer));
}

FT4ContiActionTrackEditor::FT4ContiActionTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
	UT4ContiActionSequence* ContiSequence = Cast<UT4ContiActionSequence>(GetMovieSceneSequence());
	if (nullptr != ContiSequence)
	{
		FT4ContiViewModel* ContiViewModel = ContiSequence->GetViewModel();
		ContiViewModel->GetOnViewModelChanged().AddRaw(
			this,
			&FT4ContiActionTrackEditor::OnViewModelChanged
		);
	}
}

TSharedRef<ISequencerSection> FT4ContiActionTrackEditor::MakeSectionInterface(
	UMovieSceneSection& InSectionObject,
	UMovieSceneTrack& InTrack,
	FGuid InObjectBinding
)
{
	UT4ContiActionTrack* ContiTrack = CastChecked<UT4ContiActionTrack>(&InTrack);
	check(nullptr != ContiTrack);
	if (ET4LifecycleType::Duration == ContiTrack->GetLifecycleType())
	{
		return MakeShareable(new FT4ContiActionSequencerSection(InSectionObject));
	}
	return MakeShareable(new FT4ContiSequencerKeySection(InSectionObject));
}

bool FT4ContiActionTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UT4ContiActionTrack::StaticClass();
}

bool FT4ContiActionTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return (InSequence != nullptr) && (InSequence->GetClass()->GetName() == TEXT("T4ContiActionSequence"));
}

const FSlateBrush* FT4ContiActionTrackEditor::GetIconBrush() const
{
	return FEditorStyle::GetBrush("Sequencer.Tracks.Fade");
}

void FT4ContiActionTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddWrapperSubMenu(
		LOCTEXT("T4ContiTrackWidgetAddAction", "Add Action"),
		LOCTEXT("T4ContiTrackWidgetAddAction_Tooltip", "Add a new action"),
		FOnGetContent::CreateSP(this, &FT4ContiActionTrackEditor::HandleOnAddActionSubMenu),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x")
	);
}

TSharedRef<SWidget> FT4ContiActionTrackEditor::HandleOnAddActionSubMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	// #T4_ADD_ACTION_TAG_CONTI
	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddBranchAction", "Branch"),
		LOCTEXT("T4ContiTrackAddBranchAction_Tooltip", "Adds a new action track that controls the Branch of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddBranchTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #54

#if 0

	MenuBuilder.AddMenuEntry(
		LOCTEXT("Add SpecialMove ActionTrack", "SpecialMove"),
		LOCTEXT("Add SpecialMove ActionTrackTooltip", "Adds a new action track that controls the SpecialMove of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddSpecialMoveTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	);

#endif

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddAnimationAction", "Animation"),
		LOCTEXT("T4ContiTrackAddAnimationAction_Tooltip", "Adds a new action track that controls the Animation of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddAnimationTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddParticleAction", "Particle"),
		LOCTEXT("T4ContiTrackAddParticleAction_Tooltip", "Adds a new action track that controls the Particle of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddParticleTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddDecalAction", "Decal"),
		LOCTEXT("T4ContiTrackAddDecalAction_Tooltip", "Adds a new action track that controls the Decal of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddDecalTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #54

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddProjectileAction", "Projectile"),
		LOCTEXT("T4ContiTrackAddProjectileAction_Tooltip", "Adds a new action track that controls the Projectile of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddProjectileTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #63

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddReactionAction", "Reaction"),
		LOCTEXT("T4ContiTrackAddReactionAction_Tooltip", "Adds a new action track that controls the Reaction of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddReactionTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #76

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddLayerSetAction", "LayerSet"),
		LOCTEXT("T4ContiTrackAddLayerSetAction_Tooltip", "Adds a new action track that controls the LayerSet of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddLayerSetTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #81

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddTimeScaleAction", "TimeScale"),
		LOCTEXT("T4ContiTrackAddTimeScaleAction_Tooltip", "Adds a new action track that controls the TimeScale of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddTimeScaleTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #102

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddCameraWorkAction", "CameraWork"),
		LOCTEXT("T4ContiTrackAddCameraWorkAction_Tooltip", "Adds a new action track that controls the CameraWork of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddCameraWorkTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #58

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddCameraShakeAction", "CameraShake"),
		LOCTEXT("T4ContiTrackAddCameraShakeAction_Tooltip", "Adds a new action track that controls the CameraShake of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddCameraShakeTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #100

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddPostProcessAction", "PostProcess"),
		LOCTEXT("T4ContiTrackAddPostProcessAction_Tooltip", "Adds a new action track that controls the PostProcess of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddPostProcessTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #100

	MenuBuilder.AddMenuEntry(
		LOCTEXT("T4ContiTrackAddEnvironmentAction", "Environment"),
		LOCTEXT("T4ContiTrackAddEnvironmentAction_Tooltip", "Adds a new action track that controls the Environment of the sequence."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Fade"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddEnvironmentTrackExecute),
			FCanExecuteAction::CreateRaw(this, &FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute)
		)
	); // #99

	return MenuBuilder.MakeWidget();
}

void FT4ContiActionTrackEditor::BuildTrackContextMenu(
	FMenuBuilder& MenuBuilder, 
	UMovieSceneTrack* Track
)
{
	UT4ContiActionTrack* ContiActionTrack = CastChecked<UT4ContiActionTrack>(Track);
	if (nullptr != ContiActionTrack)
	{
		MenuBuilder.BeginSection("T4ContiActionTrackEditor", LOCTEXT("T4ContiTrackContextMenuSectionName", "T4RehearsalConti Editor"));
		{
			// #56
			UT4ContiActionSequence* ContiSequence = Cast<UT4ContiActionSequence>(GetMovieSceneSequence());
			check(nullptr != ContiSequence);
			FT4ContiViewModel* ContiViewModel = ContiSequence->GetViewModel();
			if (ET4ActionType::CameraWork == ContiActionTrack->GetActionType())
			{
				// #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
			}
			else if (ET4LifecycleType::Duration == ContiActionTrack->GetLifecycleType())
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("T4ContiTrackChangeActionSectionRangeToKey", "Change Lifecycle => Auto"),
					LOCTEXT("T4ContiTrackChangeActionSectionRangeToKey_Tooltip", "Change all of the selected actions to key section"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(ContiViewModel, &FT4ContiViewModel::HandleOnChangeActionSectionKey, ContiActionTrack))
				);
			}
			else
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("T4ContiTrackChangeActionSectionKeyToRange", "Change Lifecycle => Duration"),
					LOCTEXT("T4ContiTrackChangeActionSectionKeyToRange_Tootip", "Change all of the selected actions to ragne section"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(ContiViewModel, &FT4ContiViewModel::HandleOnChangeActionSectionRange, ContiActionTrack))
				);
			}
			
			MenuBuilder.AddMenuEntry(
				ContiViewModel->IsActionIsolate(ContiActionTrack)
					? LOCTEXT("T4ContiTrackRemoveFromIsolation", "Remove this from isolation")
					: LOCTEXT("T4ContiTrackAddToIsolation", "Add this to isolation"),
				ContiViewModel->IsActionIsolate(ContiActionTrack)
					? LOCTEXT("T4ContiTrackRemoveFromIsolation_NoChangeOthers", "Remove this action from isolation, without changing other actions")
					: LOCTEXT("T4ContiTrackAddToIsolation_NoChangeOthers", "Add this action to isolation, without changing other actions"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(ContiViewModel, &FT4ContiViewModel::HandleOnToggleActionIsolation, ContiActionTrack))
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("T4ContiTrackIsolateSelected", "Isolate all selected"),
				LOCTEXT("T4ContiTrackIsolateSelectedToolTip", "Add all of the selected actions to isloation"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(ContiViewModel, &FT4ContiViewModel::HandleOnIsolateAllSelectedActions))
			);
		}
		MenuBuilder.EndSection();
	}
}

TSharedPtr<SWidget> FT4ContiActionTrackEditor::BuildOutlinerEditWidget(
	const FGuid& ObjectBinding, 
	UMovieSceneTrack* Track, 
	const FBuildEditWidgetParams& Params
)
{
	UT4ContiActionSequence* ContiSequence = Cast<UT4ContiActionSequence>(GetMovieSceneSequence());
	check(nullptr != ContiSequence);
	FT4ContiViewModel* ContiViewModel = ContiSequence->GetViewModel();
	return SNew(SContiTrackWidget)
		.ContiTrack(CastChecked<UT4ContiActionTrack>(Track))
		.ContiViewModel(ContiViewModel);
}

void FT4ContiActionTrackEditor::OnViewModelChanged()
{
}

bool FT4ContiActionTrackEditor::AddActionTrackExecute(
	ET4ActionType InActionType
)
{
	UMovieScene* MovieScene = GetFocusedMovieScene();
	if (nullptr == MovieScene)
	{
		return false;
	}
	if (MovieScene->IsReadOnly())
	{
		return false;
	}
	UT4ContiActionSequence* ContiSequence = Cast<UT4ContiActionSequence>(GetMovieSceneSequence());
	check(nullptr != ContiSequence);
	const FScopedTransaction Transaction(LOCTEXT("AddActionTrackExecute_Transaction", "Add to NewAction"));
	FT4ContiViewModel* ContiViewModel = ContiSequence->GetViewModel();
	bool bResult = ContiViewModel->AddNewMovieSceneActionTrack(InActionType, nullptr, NAME_None);
	if (!bResult)
	{
		return false;
	}
	return true;
}

bool FT4ContiActionTrackEditor::HandleAddActionTrackCanExecute() const
{
	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();
	return (nullptr != FocusedMovieScene);
}

// #T4_ADD_ACTION_TAG_CONTI
void FT4ContiActionTrackEditor::HandleAddBranchTrackExecute() // #54
{
	AddActionTrackExecute(ET4ActionType::Branch);
}

void FT4ContiActionTrackEditor::HandleAddSpecialMoveTrackExecute() // #54
{
	AddActionTrackExecute(ET4ActionType::SpecialMove);
}

void FT4ContiActionTrackEditor::HandleAddAnimationTrackExecute()
{
	AddActionTrackExecute(ET4ActionType::Animation);
}

void FT4ContiActionTrackEditor::HandleAddParticleTrackExecute()
{
	AddActionTrackExecute(ET4ActionType::Particle);
}

void FT4ContiActionTrackEditor::HandleAddDecalTrackExecute() // #54
{
	AddActionTrackExecute(ET4ActionType::Decal);
}

void FT4ContiActionTrackEditor::HandleAddProjectileTrackExecute() // #63
{
	AddActionTrackExecute(ET4ActionType::Projectile);
}

void FT4ContiActionTrackEditor::HandleAddReactionTrackExecute() // #76
{
	AddActionTrackExecute(ET4ActionType::Reaction);
}

void FT4ContiActionTrackEditor::HandleAddLayerSetTrackExecute() // #81
{
	AddActionTrackExecute(ET4ActionType::LayerSet);
}

void FT4ContiActionTrackEditor::HandleAddTimeScaleTrackExecute() // #102
{
	AddActionTrackExecute(ET4ActionType::TimeScale);
}

void FT4ContiActionTrackEditor::HandleAddCameraWorkTrackExecute() // #54
{
	AddActionTrackExecute(ET4ActionType::CameraWork);
}

void FT4ContiActionTrackEditor::HandleAddCameraShakeTrackExecute() // #101
{
	AddActionTrackExecute(ET4ActionType::CameraShake);
}

void FT4ContiActionTrackEditor::HandleAddPostProcessTrackExecute() // #100
{
	AddActionTrackExecute(ET4ActionType::PostProcess);
}

void FT4ContiActionTrackEditor::HandleAddEnvironmentTrackExecute() // #99
{
	AddActionTrackExecute(ET4ActionType::Environment);
}

#undef LOCTEXT_NAMESPACE