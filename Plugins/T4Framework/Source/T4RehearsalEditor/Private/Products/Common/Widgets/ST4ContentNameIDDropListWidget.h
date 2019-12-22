// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/T4EngineConstants.h" // #39
#include "T4Frame/Public/T4Frame.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Types/SlateEnums.h"
#include "Widgets/Input/SComboBox.h"
#include "PropertyCustomizationHelpers.h"

/**
  * #60 
 */
class SWidget;
class SComboButton;
class UT4EditorGameplaySettingObject;
class IPropertyHandle;
class ST4ContentNameIDDropListWidget : public SCompoundWidget 
{
public:
	DECLARE_DELEGATE_OneParam(FT4OnSelected, const FName);

public:
	SLATE_BEGIN_ARGS(ST4ContentNameIDDropListWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)
		SLATE_EVENT(FT4OnSelected, OnSelected)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		UT4EditorGameplaySettingObject* InEditorPlaySettings,
		ET4EditorGameDataType InEditorGameDataType
	);

	ST4ContentNameIDDropListWidget();
	~ST4ContentNameIDDropListWidget();

private: 
	void OnRefresh();

	void UpdateContentNameIDLists(const FName& InSelectedName);

	void HandleOnEditorPlaySettingsChanged();

	FReply HandleOnRefresh();
	void HandleOnComboOpening();

	void HandleOnSelectionChanged(TSharedPtr<FName> InNewSelection, ESelectInfo::Type);

	void HandleOnSearchBoxTextChanged(const FText& InSearchText);
	void HandleOnSearchBoxTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);
	
	TSharedRef<SWidget> MakeSelectionWidget(TSharedPtr<FName> InItem);
	
	FText GetComboText() const;
	
	TSharedRef<ITableRow> GenerateAddElementRow(
		TSharedPtr<FName> Entry, 
		const TSharedRef<STableViewBase> &OwnerTable
	) const;

	void GenerateFilteredElementList(const FString& InSearchText);
	
	FText GetCurrentSearchString() const { return CurrentSearchString; };

private:
	FT4OnSelected OnSelected;
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TWeakObjectPtr<UT4EditorGameplaySettingObject> EditorPlaySettings;
	FDelegateHandle EditorPlaySettingsDelegate;

	ET4EditorGameDataType EditorGameDataType;

	TArray<TSharedPtr<FName>> ContentNameIDList;
	TArray<TSharedPtr<FName>> FilteredContentNameIDList;

	FName SelectContentNameIDDesc;

	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<SListView<TSharedPtr<FName>>> ElementsListView;
	
	FText CurrentSearchString;
	TSharedPtr<SComboButton> ElementButton;
};
