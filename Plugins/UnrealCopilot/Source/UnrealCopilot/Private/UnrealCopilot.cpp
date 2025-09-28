// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilot.h"
#include "IPythonScriptPlugin.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FUnrealCopilotModule"

DEFINE_LOG_CATEGORY(LogUnrealCopilot);

void FUnrealCopilotModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogUnrealCopilot, Log, TEXT("UnrealCopilot module started"));
}

void FUnrealCopilotModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogUnrealCopilot, Log, TEXT("UnrealCopilot module shutdown"));
}

bool UUnrealCopilotBlueprintLibrary::ExecutePythonString(const FString& PythonCode, FString& OutResult)
{
	// Clear the output result
	OutResult.Empty();
	
	// Handle empty or null input
	if (PythonCode.IsEmpty())
	{
		OutResult = TEXT("Error: Python code string is empty");
		UE_LOG(LogUnrealCopilot, Warning, TEXT("ExecutePythonString called with empty code string"));
		return false;
	}

	// Check if PythonScriptPlugin is available
	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	if (!PythonPlugin)
	{
		// Fallback to using GEngine->Exec if PythonScriptPlugin is unavailable
		UE_LOG(LogUnrealCopilot, Warning, TEXT("PythonScriptPlugin not available, trying GEngine->Exec fallback"));
		
		if (GEngine)
		{
			FString PythonExecCommand = FString::Printf(TEXT("py %s"), *PythonCode);
			
			// Create custom output device to capture results
			class FPythonOutputCapture : public FOutputDevice
			{
			public:
				FString CapturedOutput;
				virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
				{
					if (!CapturedOutput.IsEmpty())
					{
						CapturedOutput += TEXT("\n");
					}
					CapturedOutput += V;
				}
			};
			
			FPythonOutputCapture OutputCapture;
			bool bResult = GEngine->Exec(nullptr, *PythonExecCommand, OutputCapture);
			
			if (bResult)
			{
				OutResult = OutputCapture.CapturedOutput.IsEmpty() ? TEXT("Python command executed successfully (no output)") : OutputCapture.CapturedOutput;
				UE_LOG(LogUnrealCopilot, Log, TEXT("Python execution via GEngine->Exec successful"));
				return true;
			}
			else
			{
				OutResult = FString::Printf(TEXT("Error: Failed to execute Python code via GEngine->Exec: %s"), *OutputCapture.CapturedOutput);
				UE_LOG(LogUnrealCopilot, Error, TEXT("Python execution via GEngine->Exec failed"));
				return false;
			}
		}
		
		OutResult = TEXT("Error: Both PythonScriptPlugin and GEngine are not available");
		UE_LOG(LogUnrealCopilot, Error, TEXT("Neither PythonScriptPlugin nor GEngine are available"));
		return false;
	}

	// Check if Python is initialized
	if (!PythonPlugin->IsPythonAvailable())
	{
		OutResult = TEXT("Error: Python is not available or not initialized");
		UE_LOG(LogUnrealCopilot, Error, TEXT("Python is not available or not initialized"));
		return false;
	}

	bool bExecutionSuccess = false;

	try 
	{
		// Try using ExecPythonCommandEx first (more modern API)
		FPythonCommandEx PythonCommand;
		PythonCommand.Command = PythonCode;
		PythonCommand.ExecutionMode = EPythonCommandExecutionMode::ExecuteStatement;
		
		bExecutionSuccess = PythonPlugin->ExecPythonCommandEx(PythonCommand);
		
		if (bExecutionSuccess)
		{
			// If execution was successful, try to capture any output from logs
			OutResult = TEXT("Python code executed successfully");
			UE_LOG(LogUnrealCopilot, Log, TEXT("Python execution successful: %s"), *PythonCode.Left(100));
		}
		else
		{
			// If ExecPythonCommandEx failed, try the simpler ExecPythonCommand
			bExecutionSuccess = PythonPlugin->ExecPythonCommand(*PythonCode);
			
			if (bExecutionSuccess)
			{
				OutResult = TEXT("Python code executed successfully");
				UE_LOG(LogUnrealCopilot, Log, TEXT("Python execution successful (fallback method): %s"), *PythonCode.Left(100));
			}
			else
			{
				OutResult = FString::Printf(TEXT("Python execution failed for code: %s"), *PythonCode.Left(100));
				UE_LOG(LogUnrealCopilot, Error, TEXT("Python execution failed: %s"), *OutResult);
			}
		}
	}
	catch (const std::exception& e)
	{
		OutResult = FString::Printf(TEXT("Exception during Python execution: %s"), ANSI_TO_TCHAR(e.what()));
		UE_LOG(LogUnrealCopilot, Error, TEXT("Exception during Python execution: %s"), *OutResult);
		bExecutionSuccess = false;
	}
	catch (...)
	{
		OutResult = TEXT("Unknown exception during Python execution");
		UE_LOG(LogUnrealCopilot, Error, TEXT("Unknown exception during Python execution"));
		bExecutionSuccess = false;
	}

	return bExecutionSuccess;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnrealCopilotModule, UnrealCopilot)