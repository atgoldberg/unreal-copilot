// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FUnrealCopilotModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Executes a Python string and returns the result
	 * This is a placeholder that will be implemented in Task 001
	 * @param PythonCode The Python code to execute
	 * @return The execution result or error message
	 */
	static FString ExecutePythonString(const FString& PythonCode);
};
