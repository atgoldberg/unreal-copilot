// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilotEditor.h"
#include "UnrealCopilotStyle.h"
#include "UnrealCopilotCommands.h"
#include "Widgets/SUnrealCopilotWidget.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName UnrealCopilotTabName("UnrealCopilot");

#define LOCTEXT_NAMESPACE "FUnrealCopilotEditorModule"

DEFINE_LOG_CATEGORY(LogUnrealCopilotEditor);

void FUnrealCopilotEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FUnrealCopilotStyle::Initialize();
	FUnrealCopilotStyle::ReloadTextures();

	FUnrealCopilotCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUnrealCopilotCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUnrealCopilotEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUnrealCopilotEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UnrealCopilotTabName, FOnSpawnTab::CreateRaw(this, &FUnrealCopilotEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FUnrealCopilotTabTitle", "UnrealCopilot"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UE_LOG(LogUnrealCopilotEditor, Log, TEXT("UnrealCopilotEditor module started"));
}

void FUnrealCopilotEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FUnrealCopilotStyle::Shutdown();

	FUnrealCopilotCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UnrealCopilotTabName);

	UE_LOG(LogUnrealCopilotEditor, Log, TEXT("UnrealCopilotEditor module shutdown"));
}

TSharedRef<SDockTab> FUnrealCopilotEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SUnrealCopilotWidget)
		];
}

void FUnrealCopilotEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UnrealCopilotTabName);
}

void FUnrealCopilotEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("Programming");
			Section.AddMenuEntryWithCommandList(FUnrealCopilotCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUnrealCopilotCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealCopilotEditorModule, UnrealCopilotEditor)