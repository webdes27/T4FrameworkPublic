// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #60
 */
#define DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(cls, prop)								\
	InBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(##cls, ##prop), ##cls##::StaticClass());

#define DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(cls, prop)								\
	TSharedRef<IPropertyHandle> ##prop##Handle = DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(cls, prop); \
	InBuilder.HideProperty(##prop##Handle);

#define DEFINE_DETAIL_GET_CHILD_ROPERTY_MACRO(parent, prop)								\
	parent->GetChildHandle(TEXT("" #prop ""), false);

#define DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(parent, prop)					\
	TSharedPtr<IPropertyHandle> ##prop##Handle = parent->GetChildHandle(TEXT("" #prop ""), false);	\
	if (##prop##Handle.IsValid())														\
	{																					\
		DetailCategoryBuilder.AddProperty(##prop##Handle);								\
	}																					\
	else																				\
	{																					\
		UE_LOG(LogT4RehearsalEditor, Warning, TEXT("Add property in Category: " #parent "->GetChildHandle not found " #prop ""));	\
	}

 // #85
#define DEFINE_DETAIL_ADD_GROUP_PROPERTY_HANDLE_MACRO(group, prop)						\
	group##.AddPropertyRow(##prop##Handle);

// #76
#define DEFINE_DETAIL_HEADER_GROUP_CHILD_PROPERTY_MACRO(group, parent, prop)			\
	TSharedPtr<IPropertyHandle> ##prop##Handle = parent->GetChildHandle(TEXT("" #prop ""), false);	\
	if (##prop##Handle.IsValid())														\
	{																					\
		##group##.HeaderProperty(##prop##Handle.ToSharedRef());							\
	}																					\
	else																				\
	{																					\
		UE_LOG(LogT4RehearsalEditor, Warning, TEXT("Set Header property in Group: " #parent "->GetChildHandle not found " #prop ""));	\
	}

#define DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(group, parent, prop)				\
	TSharedPtr<IPropertyHandle> ##prop##Handle = parent->GetChildHandle(TEXT("" #prop ""), false);	\
	if (##prop##Handle.IsValid())														\
	{																					\
		##group##.AddPropertyRow(##prop##Handle.ToSharedRef());							\
	}																					\
	else																				\
	{																					\
		UE_LOG(LogT4RehearsalEditor, Warning, TEXT("Add property in Group: " #parent "->GetChildHandle not found " #prop ""));	\
	}

#define DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(parent, prop)					\
	{																					\
		##prop##HandlePtr = parent->GetChildHandle(TEXT("" #prop ""), false);			\
		if (!##prop##HandlePtr.IsValid())												\
		{																				\
			UE_LOG(LogT4RehearsalEditor, Warning, TEXT("Get Handle Pointer: " #parent "->GetChildHandle not found " #prop ""));	\
		}																				\
	}
