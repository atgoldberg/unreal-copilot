// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilotStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FUnrealCopilotStyle::StyleInstance = nullptr;

void FUnrealCopilotStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUnrealCopilotStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FUnrealCopilotStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UnrealCopilotStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FUnrealCopilotStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("UnrealCopilotStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UnrealCopilot")->GetBaseDir() / TEXT("Resources"));

	Style->Set("UnrealCopilot.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon40x40));

	return Style;
}

void FUnrealCopilotStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUnrealCopilotStyle::Get()
{
	return *StyleInstance;
}

#undef RootToContentDir