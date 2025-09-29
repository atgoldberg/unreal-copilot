// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealCopilotExecutionManager.h"
#include "UnrealCopilot.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCopilot, Log, All);

class UNREALCOPILOT_API FUnrealCopilotModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

/**
 * UnrealCopilot Blueprint Function Library
 * Provides utility functions for Python execution and LLM integration
 */
UCLASS()
class UNREALCOPILOT_API UUnrealCopilotBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Execute a Python code string within the Unreal Editor environment (Legacy function)
	 * @param PythonCode - The Python code string to execute
	 * @param OutResult - The output result from Python execution (stdout or error message)
	 * @return true if execution was successful, false if there was an error
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot", CallInEditor)
	static bool ExecutePythonString(const FString& PythonCode, FString& OutResult);

	/**
	 * Execute Python code with enhanced error handling and timeout support
	 * @param PythonCode - The Python code string to execute
	 * @return Detailed execution result with error information
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot", CallInEditor)
	static FPythonExecutionResult ExecutePythonCodeEnhanced(const FString& PythonCode);

	/**
	 * Validate Python syntax without executing
	 * @param PythonCode - The Python code to validate
	 * @param OutErrorMessage - Error message if validation fails
	 * @return true if syntax is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot", CallInEditor)
	static bool ValidatePythonSyntax(const FString& PythonCode, FString& OutErrorMessage);

	/**
	 * Get the execution manager instance
	 * @return The UnrealCopilot execution manager
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	static UUnrealCopilotExecutionManager* GetExecutionManager();
};
