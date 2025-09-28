// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilotCommands.h"

#define LOCTEXT_NAMESPACE "FUnrealCopilotModule"

void FUnrealCopilotCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "UnrealCopilot", "Bring up UnrealCopilot window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE