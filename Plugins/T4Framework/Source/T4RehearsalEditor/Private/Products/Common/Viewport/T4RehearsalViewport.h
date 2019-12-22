// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SlateFwd.h"
#include "UObject/GCObject.h"
#include "Widgets/Input/SSpinBox.h"
#include "SEditorViewport.h"

/**
  *
 */
class SSlider;
class SOverlay;
class IT4RehearsalViewModel;
class FT4RehearsalViewportClient;
class ST4RehearsalViewport : public SEditorViewport, public FGCObject
{
public:
	DECLARE_DELEGATE(FT4OnOverlayButtonCallback); // #83
	DECLARE_DELEGATE_TwoParams(FT4OnThumbnailCaptured, UObject*, UTexture2D*);

public:
	SLATE_BEGIN_ARGS(ST4RehearsalViewport)
		: _ViewModel(nullptr)
		, _OnThumbnailCaptured(nullptr)
		, _OnRefreshButtonClicked(nullptr) // #86
		, _OnSimulationButtonClicked(nullptr) // #86
		, _OnHotKeyJumpToPlay(nullptr) // #99
		, _OnHotKeyJumpToEnd(nullptr) // #99
		, _OnHotKeyTogglePlay(nullptr) // #99
		{}
		SLATE_ARGUMENT(IT4RehearsalViewModel*, ViewModel)
		SLATE_EVENT(FT4OnThumbnailCaptured, OnThumbnailCaptured)
		SLATE_EVENT(FT4OnOverlayButtonCallback, OnRefreshButtonClicked) // #86
		SLATE_EVENT(FT4OnOverlayButtonCallback, OnSimulationButtonClicked) // #86
		SLATE_EVENT(FT4OnOverlayButtonCallback, OnHotKeyJumpToPlay) // #99
		SLATE_EVENT(FT4OnOverlayButtonCallback, OnHotKeyJumpToEnd) // #99
		SLATE_EVENT(FT4OnOverlayButtonCallback, OnHotKeyTogglePlay) // #99
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	ST4RehearsalViewport();
	~ST4RehearsalViewport();

	void OnCleanup(); // #97

	TSharedPtr<FExtender> GetExtenders() const;

	void AddReferencedObjects(FReferenceCollector& Collector) override;

	void CreateThumbnail(UObject* InScreenShotOwner);

	void RefreshViewport();

	IT4RehearsalViewModel* GetViewModel() const { return ViewModelRef; }
	FT4RehearsalViewportClient* GetViewportClient() const { return ViewportClientPtr.Get(); } // #79

	bool IsVisible() const override; // #76

protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual void BindCommands() override;
	virtual void PopulateViewportOverlays(TSharedRef<SOverlay> Overlay) override;

	void OnScreenShotCaptured(UObject* InOwner, UTexture2D* InScreenShot);

	// #93
	TSharedRef<SWidget> HandleOnFillTimelapseScaleMenu(); 

	FText HandleOnGetTimelapseLabel() const;

	float HandleOnGetTimelapseScaleBoxValue() const;
	void HandleOnSetTimelapseScaleBoxValue(float NewValue);

	float HandleOnGetTimelapseSetHourBoxValue() const;
	void HandleOnSetTimelapseSetHourBoxValue(float NewValue);
	// ~#93

	// #83
	TSharedRef<SWidget> HandleOnFillCameraSpeedMenu();

	FText HandleOnGetCameraSpeedLabel() const;
	float HandleOnGetCamSpeedSliderPosition() const;
	void HandleOnSetCamSpeed(float NewValue);
	FText HandleOnGetCameraSpeedScalarLabel() const;
	float HandleOnGetCamSpeedScalarBoxValue() const;
	void HandleOnSetCamSpeedScalarBoxValue(float NewValue);
	// ~#83

	FReply HandleOnRefreshButtonClicked(); // #83
	FReply HandleOnSimulationButtonClicked(); // #83

	void HandleOnUseDefaultShowFlags(); // #94

	void HandleOnToggleShowCapsule(); // #76
	bool HandleOnIsToggleShowCapsuleChecked() const; // #76

	void HandleOnToggleAlwaysTick() { bAlwaysTickViewport = !bAlwaysTickViewport; } // #76
	bool HandleOnIsToggleAlwaysTickChecked() const { return bAlwaysTickViewport; } // #76

	void HandleOnJumpToPlay(); // #99
	void HandleOnJumpToEnd(); // #99
	void HandleOnTogglePlay(); // #99

private:
	TSharedPtr<FT4RehearsalViewportClient> ViewportClientPtr;

	IT4RehearsalViewModel* ViewModelRef;

	bool bAlwaysTickViewport; // #76

	FT4OnThumbnailCaptured OnThumbnailCaptured;
	FT4OnOverlayButtonCallback OnRefreshButtonClicked; // #86
	FT4OnOverlayButtonCallback OnSimulationButtonClicked; // #86

	FT4OnOverlayButtonCallback OnHotKeyJumpToPlay; // #99
	FT4OnOverlayButtonCallback OnHotKeyJumpToEnd; // #99
	FT4OnOverlayButtonCallback OnHotKeyTogglePlay; // #99

	// #83
	/** Reference to the camera slider used to display current camera speed */
	mutable TSharedPtr<SSlider> CamSpeedSlider;

	/** Reference to the camera spinbox used to display current camera speed scalar */
	mutable TSharedPtr<SSpinBox<float>> CamSpeedScalarBox;

	// #93
	mutable TSharedPtr<SSpinBox<float>> TimelapseScaleBox;
	mutable TSharedPtr<SSpinBox<float>> TimelapseSetHourBox;
};