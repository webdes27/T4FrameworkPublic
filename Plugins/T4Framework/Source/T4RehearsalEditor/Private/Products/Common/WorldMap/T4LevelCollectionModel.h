// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Framework/Commands/UICommandList.h"
#include "Engine/World.h"
#include "TickableEditorObject.h"
#include "Misc/IFilter.h"
#include "T4LevelModel.h"

#include "Misc/FilterCollection.h"

class FMenuBuilder;
class IDetailsView;
class UEditorEngine;
class UMaterialInterface;
class UT4WorldAsset; // #84

typedef IFilter< const FT4LevelModel* >				T4LevelFilter;
typedef TFilterCollection< const FT4LevelModel* >	T4LevelFilterCollection;

/**
 * Interface for non-UI presentation logic for a world
 */
class FT4LevelCollectionModel
	: public TSharedFromThis<FT4LevelCollectionModel>	
	, public FTickableEditorObject
{
public:
	DECLARE_EVENT_OneParam( FT4LevelCollectionModel, FOnNewItemAdded, TSharedPtr<FT4LevelModel>);
	DECLARE_EVENT( FT4LevelCollectionModel, FSimpleEvent );


public:
	FT4LevelCollectionModel();
	virtual ~FT4LevelCollectionModel();

	/** FTickableEditorObject interface */
	virtual void Tick( float DeltaTime ) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
	/** FTickableEditorObject interface */
	
	/**	@return	Whether level collection is read only now */
	bool IsReadOnly() const;
	
	/**	@return	Whether level collection is in PIE/SIE mode */
	virtual bool IsSimulating() const; // #86 : virtual => override

	/**	@return	Current simulation world */
	virtual UWorld* GetSimulationWorld() const; // #86 : virtual => override

	/**	@return	Current editor world */
	UWorld* GetWorld(bool bEvenIfPendingKill = false) const { return CurrentWorld.Get(bEvenIfPendingKill); }

	/** @return	Whether current world has world origin rebasing enabled */
	bool IsOriginRebasingEnabled() const;

	/** Current world size  */
	FIntPoint GetWorldSize() const { return WorldSize; }
	
	/**	@return	Root list of levels in hierarchy */
	FT4LevelModelList& GetRootLevelList();

	/**	@return	All level list managed by this level collection */
	const FT4LevelModelList& GetAllLevels() const;

	/**	@return	List of filtered levels */
	const FT4LevelModelList& GetFilteredLevels() const;

	/**	@return	Currently selected level list */
	const FT4LevelModelList& GetSelectedLevels() const;

	/** Adds a filter which restricts the Levels shown in UI */
	void AddFilter(const TSharedRef<T4LevelFilter>& InFilter);

	/** Removes a filter which restricted the Levels shown in UI */
	void RemoveFilter(const TSharedRef<T4LevelFilter>& InFilter);

	/**	@return	Whether level filtering is active now */
	bool IsFilterActive() const;
	
	/**	Iterates through level hierarchy with given Visitor */
	void IterateHierarchy(FT4LevelModelVisitor& Visitor);

	/**	Sets selected level list */
	void SetSelectedLevels(const FT4LevelModelList& InList);
	
	/**	Sets selection to a levels that is currently marked as selected in UWorld */
	void SetSelectedLevelsFromWorld();

	/**	@return	Found level model which represents specified level object */
	TSharedPtr<FT4LevelModel> FindLevelModel(ULevel* InLevel) const;

	/**	@return	Found level model with specified level package name */
	TSharedPtr<FT4LevelModel> FindLevelModel(const FName& PackageName) const;

	/**	Hides level in the world */
	void HideLevels(const FT4LevelModelList& InLevelList);
	
	/**	Shows level in the world */
	void ShowLevels(const FT4LevelModelList& InLevelList);

	/** Toggles the selected levels to a visible state; toggles all other levels to an invisible state */
	void ShowOnlySelectedLevels();

	/** Toggles the selected levels to an invisible state; toggles all other levels to a visible state */
	void ShowAllButSelectedLevels();

	/**	Unlocks level in the world */
	void UnlockLevels(const FT4LevelModelList& InLevelList);
	
	/**	Locks level in the world */
	void LockLevels(const FT4LevelModelList& InLevelList);

	/** Toggles the selected levels to a locked state; toggles all other levels to an unlocked state */
	void LockOnlySelectedLevels();

	/** Toggles the selected levels to an unlocked state; toggles all other levels to a locked state */
	void LockAllButSelectedLevels();

	/**	Saves level to disk */
	void SaveLevels(const FT4LevelModelList& InLevelList, bool bSelection);

	/**	Loads level from disk */
	void LoadLevels(const FT4LevelModelList& InLevelList);
	
	/**	Unloads levels from the editor */
	virtual void UnloadLevels(const FT4LevelModelList& InLevelList);

	/** Translate levels by specified delta */
	virtual void TranslateLevels(const FT4LevelModelList& InLevelList, FVector2D InAbsoluteDelta, bool bSnapDelta = true);
	
	/** Snaps translation delta */
	virtual FVector2D SnapTranslationDelta(const FT4LevelModelList& InLevelList, FVector2D InAbsoluteDelta, bool bBoundsSnapping, float SnappingValue);

	/**	Updates current translation delta, when user drags levels on minimap */
	virtual void UpdateTranslationDelta(const FT4LevelModelList& InLevelList, FVector2D InTranslationDelta, bool bBoundsSnapping, float SnappingValue);

	/** Attach levels as children to specified level */
	void AssignParent(const FT4LevelModelList& InLevels, TSharedPtr<FT4LevelModel> InParent);

	/** Adds all levels in worlds represented by the supplied world list as sublevels */
	virtual void AddExistingLevelsFromAssetData(const TArray<struct FAssetData>& WorldList);

	/**	@return	Whether specified level passes all filters */
	virtual bool PassesAllFilters(const FT4LevelModel& InLevelModel) const;
	
	/**	Builds 'hierarchy' commands menu for a selected levels */
	virtual void BuildHierarchyMenu(FMenuBuilder& InMenuBuilder) const;
	
	/**	Customize 'File' section in main menu  */
	virtual void CustomizeFileMainMenu(FMenuBuilder& InMenuBuilder) const;
		
	/**	@return	Player view in the PIE/Simulation world */
	virtual bool GetPlayerView(FVector& OutCameraLocation, FRotator& OutCameraRotation, FVector& OutPlayerLocation) const;

	/**	@return	Observer view in the Editor/Similuation world */
	virtual bool GetObserverView(FVector& Location, FRotator& Rotation) const;

	/**	Compares 2 levels by Z order */
	virtual bool CompareLevelsZOrder(TSharedPtr<FT4LevelModel> InA, TSharedPtr<FT4LevelModel> InB) const;

	/** @return	Whether this level collection model is a tile world */
	virtual bool IsTileWorld() const { return false; };

	/** Returns true if this collection model will support folders */
	virtual bool HasFolderSupport() const { return false; }

	/** Rebuilds levels collection */
	void PopulateLevelsList();

	/** Rebuilds the list of filtered Levels */
	void PopulateFilteredLevelsList();

	/**	Request to update levels cached information */
	void RequestUpdateAllLevels();
	
	/**	Request to redraw all levels */
	void RequestRedrawAllLevels();

	/**	Updates all levels cached information */
	void UpdateAllLevels();

	/**	Redraws all levels */
	void RedrawAllLevels();

	/** Updates level actor count for all levels */
	void UpdateLevelActorsCount();

	/** @return	whether exactly one level is selected */
	bool IsOneLevelSelected() const;

	/** @return	whether at least one level is selected */
	bool AreAnyLevelsSelected() const;

	/** @return whether all the currently selected levels are loaded */
	bool AreAllSelectedLevelsLoaded() const;

	/** @return whether any of the currently selected levels is loaded */
	bool AreAnySelectedLevelsLoaded() const;
	
	/** @return whether all the currently selected levels are unloaded */
	bool AreAllSelectedLevelsUnloaded() const;
	
	/** @return whether any of the currently selected levels is unloaded */
	bool AreAnySelectedLevelsUnloaded() const;

	/** @return whether all the currently selected levels are editable */
	bool AreAllSelectedLevelsEditable() const;

	/** @return whether all the currently selected levels are editable and not persistent */
	bool AreAllSelectedLevelsEditableAndNotPersistent() const;

	/** @return whether all the currently selected levels are editable and visible*/
	bool AreAllSelectedLevelsEditableAndVisible() const;

	/** @return whether any of the currently selected levels is editable */
	bool AreAnySelectedLevelsEditable() const;

	/** @return whether any of the currently selected levels is editable and visible*/
	bool AreAnySelectedLevelsEditableAndVisible() const;
	
	/** @return whether currently only one level selected and it is editable */
	bool IsSelectedLevelEditable() const;

	/** @return whether currently only one level selected and a lighting scenario */
	bool IsNewLightingScenarioState(bool bExistingState) const;

	void SetIsLightingScenario(bool bNewLightingScenario);

	/** @return whether any of the currently selected levels is dirty */
	bool AreAnySelectedLevelsDirty() const;

	/** @return	whether at least one actor is selected */
	bool AreActorsSelected() const;

	/** @return whether moving the selected actors to the selected level is a valid action */
	bool IsValidMoveActorsToLevel() const;

	/** @return whether moving the selected foliage to the selected level is a valid action */
	bool IsValidMoveFoliageToLevel() const;

	/** delegate used to pickup when the selection has changed */
	void OnActorSelectionChanged(UObject* obj);

	/** Sets a flag to re-cache whether the selected actors move to the selected level is valid */
	void OnActorOrLevelSelectionChanged();

	/** @return	whether 'display paths' is enabled */
	bool GetDisplayPathsState() const;

	/** Sets 'display paths', whether to show long package name in level display name */
	void SetDisplayPathsState(bool bDisplayPaths);

	/** @return	whether 'display actors count' is enabled */
	bool GetDisplayActorsCountState() const;

	/** Sets 'display actors count', whether to show actors count next to level name */
	void SetDisplayActorsCountState(bool bDisplayActorsCount);

	/**	Broadcasts whenever items selection has changed */
	FSimpleEvent SelectionChanged;
	void BroadcastSelectionChanged();

	/**	Broadcasts whenever items collection has changed */
	FSimpleEvent CollectionChanged;
	void BroadcastCollectionChanged();
		
	/** Broadcasts whenever items hierarchy has changed */
	FSimpleEvent HierarchyChanged;
	void BroadcastHierarchyChanged();

	/** Broadcasts before levels are unloaded */
	FSimpleEvent PreLevelsUnloaded;
	void BroadcastPreLevelsUnloaded();

	/** Broadcasts after levels are unloaded */
	FSimpleEvent PostLevelsUnloaded;
	void BroadcastPostLevelsUnloaded();
	
	// #83
	FSimpleEvent OnSubLevelChanged;
	void BroadcastSubLevelChanged();
	// ~#83

	/** Editable world axis length  */
	static float EditableAxisLength();

	/** Editable world bounds */
	static FBox EditableWorldArea();

	/**  */
	static void SCCCheckOut(const FT4LevelModelList& InList);
	static void SCCCheckIn(const FT4LevelModelList& InList);
	static void SCCOpenForAdd(const FT4LevelModelList& InList);
	static void SCCHistory(const FT4LevelModelList& InList);
	static void SCCRefresh(const FT4LevelModelList& InList);
	static void SCCDiffAgainstDepot(const FT4LevelModelList& InList, UEditorEngine* InEditor);
	
	/** @return	List of valid level package names from a specified level model list*/
	static TArray<FName> GetPackageNamesList(const FT4LevelModelList& InList);
	
	/** @return	List of valid level package filenames from a specified level model list*/
	static TArray<FString> GetFilenamesList(const FT4LevelModelList& InList);
	
	/** @return	List of valid packages from a specified level model list*/
	static TArray<UPackage*> GetPackagesList(const FT4LevelModelList& InList);
	
	/** @return	List of valid level objects from a specified level model list*/
	static TArray<ULevel*> GetLevelObjectList(const FT4LevelModelList& InList);

	/** @return	List of loaded level models from a specified level model list*/
	static FT4LevelModelList GetLoadedLevels(const FT4LevelModelList& InList);

	/** @return	List of all level models found while traversing hierarchy of specified level models */
	static FT4LevelModelList GetLevelsHierarchy(const FT4LevelModelList& InList);

	/** @return	Total bounding box of specified level models */
	static FBox GetLevelsBoundingBox(const FT4LevelModelList& InList, bool bIncludeChildren);

	/** @return	Total bounding box of specified visible level models */
	static FBox GetVisibleLevelsBoundingBox(const FT4LevelModelList& InList, bool bIncludeChildren);

	/** @return	The UICommandList supported by this collection */
	const TSharedRef<const FUICommandList> GetCommandList() const;

	/**  */
	void LoadSettings();
	
	/**  */
	void SaveSettings();

	virtual UT4MapEntityAsset* GetThumbnailTargetAsset() const { return nullptr; } // #84

	const TSet<FName>& GetPreviewLoadedLevelLists() const { return PreviewLoadedLevelList; } // #104

	void LoadEditorSubLevel(const TArray<FName>& InSubLevelNames); // #104
	void LoadPreviewSubLevel(const TArray<FName>& InSubLevelNames); // #104
	void UnloadPreviewSubLevel(const TArray<FName>& InSubLevelNames); // #104

	FSimpleEvent PreviewLoadedLevelChanged; // #104
	void BroadcastPreviewLoadedLevelChanged(); // #104

	FSimpleEvent EditorLoadedLevelChanged; // #104
	void BroadcastEditorLoadedLevelChanged(); // #104

protected:
	/** Refreshes current cached data */
	void RefreshBrowser_Executed();
	
	/** Load selected levels to the world */
	void LoadSelectedLevels_Executed();

	/** Unload selected level from the world */
	void UnloadSelectedLevels_Executed();

	/** Make this Level the Current Level */
	void MakeLevelCurrent_Executed();

	/** Find selected levels in Content Browser */
	void FindInContentBrowser_Executed();

	/** Is FindInContentBrowser a valid action */
	bool IsValidFindInContentBrowser();

	/** Moves the selected actors to this level */
	void MoveActorsToSelected_Executed();

	/** Moves the selected foliage to this level */
	void MoveFoliageToSelected_Executed();

	/** Saves selected levels */
	void SaveSelectedLevels_Executed();

	/** Saves selected level under new name */
	void SaveSelectedLevelAs_Executed();
	
	/** Migrate selected levels */
	void MigrateSelectedLevels_Executed();

	/** Expand selected items hierarchy */
	void ExpandSelectedItems_Executed();
			
	/** Check-Out selected levels from SCC */
	void OnSCCCheckOut();

	/** Mark for Add selected levels from SCC */
	void OnSCCOpenForAdd();

	/** Check-In selected levels from SCC */
	void OnSCCCheckIn();

	/** Shows the SCC History of selected levels */
	void OnSCCHistory();

	/** Refreshes the states selected levels from SCC */
	void OnSCCRefresh();

	/** Diffs selected levels from with those in the SCC depot */
	void OnSCCDiffAgainstDepot();

	/** Enable source control features */
	void OnSCCConnect() const;

	/** Selects all levels in the collection view model */
	void SelectAllLevels_Executed();

	/** De-selects all levels in the collection view model */
	void DeselectAllLevels_Executed();

	/** Inverts level selection in the collection view model */
	void InvertSelection_Executed();
	
	/** Adds the Actors in the selected Levels from the viewport's existing selection */
	void SelectActors_Executed();

	/** Removes the Actors in the selected Levels from the viewport's existing selection */
	void DeselectActors_Executed();

	/** Toggles selected levels to a visible state in the viewports */
	void ShowSelectedLevels_Executed();

	/** Toggles selected levels to an invisible state in the viewports */
	void HideSelectedLevels_Executed();

	/** Toggles the selected levels to a visible state; toggles all other levels to an invisible state */
	void ShowOnlySelectedLevels_Executed();
	
	/** Toggles the selected levels to an invisible state; toggles all other levels to a visible state */
	void ShowAllButSelectedLevels_Executed();

	/** Toggles all levels to a visible state in the viewports */
	void ShowAllLevels_Executed();

	/** Hides all levels to an invisible state in the viewports */
	void HideAllLevels_Executed();
	
	/** Locks selected levels */
	void LockSelectedLevels_Executed();

	/** Unlocks selected levels */
	void UnlockSelectedLevels_Executed();

	/** Toggles the selected levels to a locked state; toggles all other levels to an unlocked state */
	void LockOnlySelectedLevels_Executed();

	/** Toggles the selected levels to an unlocked state; toggles all other levels to a locked state */
	void LockAllButSelectedLevels_Executed();

	/** Locks all levels */
	void LockAllLevels_Executed();

	/** Unlocks all levels */
	void UnockAllLevels_Executed();

	/** Toggle all read-only levels */
	void ToggleReadOnlyLevels_Executed();

	void ThumbnailSelectedLevels_Executed(); // #84

	virtual void PreviewCameraLookAtLocation_Executed() {} // #90
	virtual void EditorCameraLookAtLocation_Executed() {} // #90

	/** true if the SCC Check-Out option is available */
	bool CanExecuteSCCCheckOut() const
	{
		return bCanExecuteSCCCheckOut;
	}

	/** true if the SCC Check-In option is available */
	bool CanExecuteSCCCheckIn() const
	{
		return bCanExecuteSCCCheckIn;
	}

	/** true if the SCC Mark for Add option is available */
	bool CanExecuteSCCOpenForAdd() const
	{
		return bCanExecuteSCCOpenForAdd;
	}

	/** true if Source Control options are generally available. */
	bool CanExecuteSCC() const
	{
		return bCanExecuteSCC;
	}
	
	/** Fills MenuBulder with Lock level related commands */
	void FillLockSubMenu(class FMenuBuilder& MenuBuilder);
	
	/** Fills MenuBulder with level visisbility related commands */
	void FillVisibilitySubMenu(class FMenuBuilder& MenuBuilder);

	/** Fills MenuBulder with SCC related commands */
	void FillSourceControlSubMenu(class FMenuBuilder& MenuBuilder);

	/** Load selected levels to the Preview world */
	void LoadSelectedPreviewLevels_Executed(); // #104

	/** Unload selected level from the Preview world */
	void UnloadSelectedPreviewLevels_Executed(); // #104

	bool AreAnySelectedPreviewLevelsLoaded() const; // #104
	bool AreAnySelectedPreviewLevelsUnloaded() const; // #104

protected:
	/**  */
	virtual void Initialize(UWorld* InWorld);
	
	/**  */
	virtual void BindCommands();

	/** Removes the Actors in all read-only Levels from the viewport's existing selection */
	void DeselectActorsInAllReadOnlyLevel(const FT4LevelModelList& InLevelList);

	/** Removes the Actors in all read-only Levels from the viewport's existing selection */
	void DeselectSurfaceInAllReadOnlyLevel(const FT4LevelModelList& InLevelList);
	
	/** Called whenever level collection has been changed */
	virtual void OnLevelsCollectionChanged();
	
	/** Called whenever level selection has been changed */
	virtual void OnLevelsSelectionChanged();

	/** Called whenever level selection has been changed outside of this module, usually via World->SetSelectedLevels */
	void OnLevelsSelectionChangedOutside();
	
	/** Called whenever level collection hierarchy has been changed */
	virtual void OnLevelsHierarchyChanged();

	/** Called before loading specified level models into editor */
	virtual void OnPreLoadLevels(const FT4LevelModelList& InList) {};
	
	/** Called before making visible specified level models */
	virtual void OnPreShowLevels(const FT4LevelModelList& InList) {};

	/** Called when level was added to the world */
	void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);

	/** Called when level was removed from the world */
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

	/** Handler for FEditorSupportDelegates::RedrawAllViewports event */
	void OnRedrawAllViewports();

	/** Handler for when an actor was added to a level */
	void OnLevelActorAdded(AActor* InActor);	

	/** Handler for when an actor was removed from a level */
	void OnLevelActorDeleted(AActor* InActor);
	
	/** Handler for level filter collection changes */
	void OnFilterChanged();

	/** Caches the variables for which SCC menu options are available */
	void CacheCanExecuteSourceControlVars() const;
		
protected:
	
	// The editor world from where we pull our data
	TWeakObjectPtr<UWorld>				CurrentWorld;

	// Has request to update all levels cached 
	bool								bRequestedUpdateAllLevels;
	
	// Has request to redraw all levels
	bool								bRequestedRedrawAllLevels;

	// Has request to update actors count for all levels
	bool								bRequestedUpdateActorsCount;

	/** The list of commands with bound delegates for the Level collection */
	const TSharedRef<FUICommandList>	CommandList;

	/** The collection of filters used to restrict the Levels shown in UI */
	const TSharedRef<T4LevelFilterCollection> Filters;
	
	/** Levels in the root of hierarchy, persistent levels  */
	FT4LevelModelList						RootLevelsList;
	
	/** All levels found in the world */
	FT4LevelModelList						AllLevelsList;
	
	/** All levels in a map<PackageName, LevelModel> */
	TMap<FName, TSharedPtr<FT4LevelModel>> AllLevelsMap;

	/** Filtered levels from AllLevels list  */
	FT4LevelModelList						FilteredLevelsList;

	/** Currently selected levels  */
	FT4LevelModelList						SelectedLevelsList;

	/** Cached value of world size (sum of levels size) */
	FIntPoint							WorldSize;

	/** Whether we should show long package names in level display names */
	bool								bDisplayPaths;

	/** Whether we should show actors count next to level name */
	bool								bDisplayActorsCount;

	/** true if the SCC Check-Out option is available */
	mutable bool						bCanExecuteSCCCheckOut;

	/** true if the SCC Check-In option is available */
	mutable bool						bCanExecuteSCCOpenForAdd;

	/** true if the SCC Mark for Add option is available */
	mutable bool						bCanExecuteSCCCheckIn;

	/** true if Source Control options are generally available. */
	mutable bool						bCanExecuteSCC;

	/** Flag for whether the selection of levels or actors has changed */
	mutable bool						bSelectionHasChanged;

	/** Guard to avoid recursive level selection updates */
	bool								bUpdatingLevelsSelection;

	/** Currently selected editor levels  */
	FT4LevelModelList					SelectedEditorLevelsList; // #90

	/** Currently loaded preview levels  */
	TSet<FName>							PreviewLoadedLevelList; // #104
};

//
// Helper struct to temporally make specified UObject immune to dirtying
//
struct FT4UnmodifiableObject
{
	FT4UnmodifiableObject(UObject* InObject)
		: ImmuneObject(InObject)
		, bTransient(InObject->HasAnyFlags(RF_Transient))
	{
		if (!bTransient)
		{
			ImmuneObject->SetFlags(RF_Transient);
		}
	}
	
	~FT4UnmodifiableObject()
	{
		if (!bTransient)
		{
			ImmuneObject->ClearFlags(RF_Transient);
		}
	}

private:
	UObject*		ImmuneObject;
	bool			bTransient;
};

/**  */
struct FT4TiledLandscapeImportSettings
{
	FT4TiledLandscapeImportSettings()
		: Scale3D(100.f,100.f,100.f)
		, ComponentsNum(8)
		, QuadsPerSection(63)
		, SectionsPerComponent(1)
		, TilesCoordinatesOffset(0,0)
		, SizeX(1009)
		, bFlipYAxis(true)
	{}
	
	FVector				Scale3D;
	int32				ComponentsNum;
	int32				QuadsPerSection;
	int32				SectionsPerComponent;

	TArray<FString>		HeightmapFileList;
	TArray<FIntPoint>	TileCoordinates;
	FIntPoint			TilesCoordinatesOffset;
	int32				SizeX;
	bool				bFlipYAxis;


	TWeakObjectPtr<UMaterialInterface>	LandscapeMaterial;

	// Landscape layers 
	struct LandscapeLayerSettings
	{
		LandscapeLayerSettings()
			: bNoBlendWeight(false)
		{}

		FName						Name;
		bool						bNoBlendWeight;
		TMap<FIntPoint, FString>	WeightmapFiles;
	};

	TArray<LandscapeLayerSettings>	LandscapeLayerSettingsList;
};
