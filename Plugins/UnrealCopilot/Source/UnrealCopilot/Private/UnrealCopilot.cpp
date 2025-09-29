// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilot.h"
#include "UnrealCopilotExecutionManager.h"
#include "IPythonScriptPlugin.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FUnrealCopilotModule"

DEFINE_LOG_CATEGORY(LogUnrealCopilot);

void FUnrealCopilotModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogUnrealCopilot, Log, TEXT("UnrealCopilot module started"));
	
	// Initialize the execution manager
	UUnrealCopilotExecutionManager::GetInstance();
	UE_LOG(LogUnrealCopilot, Log, TEXT("UnrealCopilot execution manager initialized"));
}

void FUnrealCopilotModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogUnrealCopilot, Log, TEXT("UnrealCopilot module shutdown"));
}

bool UUnrealCopilotBlueprintLibrary::ExecutePythonString(const FString& PythonCode, FString& OutResult)
{
	// Legacy function - now uses the enhanced execution manager
	FPythonExecutionResult Result = ExecutePythonCodeEnhanced(PythonCode);
	
	if (Result.bSuccess)
	{
		OutResult = Result.Output;
	}
	else
	{
		OutResult = Result.ErrorMessage;
	}
	
	return Result.bSuccess;
}

FPythonExecutionResult UUnrealCopilotBlueprintLibrary::ExecutePythonCodeEnhanced(const FString& PythonCode)
{
	UUnrealCopilotExecutionManager* ExecutionManager = GetExecutionManager();
	if (!ExecutionManager)
	{
		FPythonExecutionResult ErrorResult;
		ErrorResult.bSuccess = false;
		ErrorResult.ErrorMessage = TEXT("Execution manager not available");
		ErrorResult.ErrorType = EPythonExecutionError::SystemError;
		return ErrorResult;
	}
	
	return ExecutionManager->ExecutePythonCode(PythonCode, false);
}

bool UUnrealCopilotBlueprintLibrary::ValidatePythonSyntax(const FString& PythonCode, FString& OutErrorMessage)
{
	UUnrealCopilotExecutionManager* ExecutionManager = GetExecutionManager();
	if (!ExecutionManager)
	{
		OutErrorMessage = TEXT("Execution manager not available");
		return false;
	}
	
	return ExecutionManager->ValidatePythonSyntax(PythonCode, OutErrorMessage);
}

UUnrealCopilotExecutionManager* UUnrealCopilotBlueprintLibrary::GetExecutionManager()
{
	return UUnrealCopilotExecutionManager::GetInstance();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealCopilotModule, UnrealCopilot)