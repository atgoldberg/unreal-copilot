// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UnrealCopilotStyle.h"

class FUnrealCopilotCommands : public TCommands<FUnrealCopilotCommands>
{
public:

	FUnrealCopilotCommands()
		: TCommands<FUnrealCopilotCommands>(TEXT("UnrealCopilot"), NSLOCTEXT("Contexts", "UnrealCopilot", "UnrealCopilot Plugin"), NAME_None, FUnrealCopilotStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};