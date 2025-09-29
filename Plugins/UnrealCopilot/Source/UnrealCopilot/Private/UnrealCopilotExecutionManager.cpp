// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilotExecutionManager.h"
#include "IPythonScriptPlugin.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/Engine.h"
#include "HAL/PlatformProcess.h"
#include "Async/AsyncWork.h"
#include "Json.h"

DEFINE_LOG_CATEGORY(LogUnrealCopilotExecution);

// Singleton instance
UUnrealCopilotExecutionManager* UUnrealCopilotExecutionManager::Instance = nullptr;

UUnrealCopilotExecutionManager::UUnrealCopilotExecutionManager()
	: CurrentState(EPythonExecutionState::Idle)
	, ExecutionTimeoutSeconds(30.0f)
	, MaxHistoryEntries(50)
	, ExecutionStartTime(0.0)
	, bExecutionCancelled(false)
{
	// Load execution history from disk
	LoadHistoryFromDisk();
}

UUnrealCopilotExecutionManager::~UUnrealCopilotExecutionManager()
{
	// Save history before destruction
	SaveHistoryToDisk();
}

UUnrealCopilotExecutionManager* UUnrealCopilotExecutionManager::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<UUnrealCopilotExecutionManager>();
		Instance->AddToRoot(); // Prevent garbage collection
	}
	return Instance;
}

FPythonExecutionResult UUnrealCopilotExecutionManager::ExecutePythonCode(const FString& PythonCode, bool bAsync)
{
	FScopeLock Lock(&ExecutionCriticalSection);

	// Check if we're already executing
	if (CurrentState == EPythonExecutionState::Executing)
	{
		FPythonExecutionResult BusyResult;
		BusyResult.bSuccess = false;
		BusyResult.ErrorMessage = TEXT("Python execution is already in progress. Please wait for current execution to complete.");
		BusyResult.ErrorType = EPythonExecutionError::SystemError;
		return BusyResult;
	}

	// Reset cancellation flag
	bExecutionCancelled = false;

	// Validate syntax first
	SetExecutionState(EPythonExecutionState::Validating);
	
	FString ValidationError;
	if (!ValidatePythonSyntax(PythonCode, ValidationError))
	{
		FPythonExecutionResult ValidationResult;
		ValidationResult.bSuccess = false;
		ValidationResult.ErrorMessage = ValidationError;
		ValidationResult.ErrorType = EPythonExecutionError::SyntaxError;
		SetExecutionState(EPythonExecutionState::Error);
		
		// Add to history
		AddToHistory(PythonCode, false, FString::Printf(TEXT("Syntax Error: %s"), *ValidationError));
		
		return ValidationResult;
	}

	// Execute the code (currently only synchronous)
	SetExecutionState(EPythonExecutionState::Executing);
	FPythonExecutionResult Result = ExecutePythonCodeSynchronous(PythonCode);

	// Update state based on result
	if (Result.bSuccess)
	{
		SetExecutionState(EPythonExecutionState::Completed);
	}
	else
	{
		SetExecutionState(EPythonExecutionState::Error);
	}

	// Add to history
	AddToHistory(PythonCode, Result.bSuccess, 
		Result.bSuccess ? Result.Output : Result.ErrorMessage);

	// Broadcast completion
	OnExecutionCompleted.Broadcast(Result);

	return Result;
}

bool UUnrealCopilotExecutionManager::ValidatePythonSyntax(const FString& PythonCode, FString& OutErrorMessage)
{
	if (PythonCode.IsEmpty())
	{
		OutErrorMessage = TEXT("Python code is empty");
		return false;
	}

	// Check if PythonScriptPlugin is available
	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	if (!PythonPlugin || !PythonPlugin->IsPythonAvailable())
	{
		OutErrorMessage = TEXT("Python is not available");
		return false;
	}

	// Use Python's compile function to validate syntax
	FString ValidationCode = FString::Printf(TEXT(
		"try:\n"
		"    compile(r'''%s''', '<string>', 'exec')\n"
		"    print('SYNTAX_VALID')\n"
		"except SyntaxError as e:\n"
		"    print(f'SYNTAX_ERROR: Line {e.lineno}: {e.msg}')\n"
		"except Exception as e:\n"
		"    print(f'VALIDATION_ERROR: {str(e)}')\n"
	), *PythonCode.Replace(TEXT("'''"), TEXT("\\'\\'\\'")));

	// Execute validation code using ExecuteFile mode for multi-statement support
	FPythonCommandEx ValidationCommand;
	ValidationCommand.Command = ValidationCode;
	ValidationCommand.ExecutionMode = EPythonCommandExecutionMode::ExecuteFile;

	// Capture output (this is a simplified approach - in a real implementation,
	// we would need to capture stdout properly)
	bool bValidationSuccess = PythonPlugin->ExecPythonCommandEx(ValidationCommand);
	
	if (!bValidationSuccess)
	{
		OutErrorMessage = TEXT("Failed to validate Python syntax");
		return false;
	}

	// For now, we'll assume syntax is valid if the validation command executed
	// In a more complete implementation, we would capture and parse the output
	OutErrorMessage.Empty();
	return true;
}

void UUnrealCopilotExecutionManager::CancelExecution()
{
	FScopeLock Lock(&ExecutionCriticalSection);
	
	if (CurrentState == EPythonExecutionState::Executing)
	{
		bExecutionCancelled = true;
		SetExecutionState(EPythonExecutionState::Idle);
		UE_LOG(LogUnrealCopilotExecution, Warning, TEXT("Python execution cancelled by user"));
	}
}

void UUnrealCopilotExecutionManager::AddToHistory(const FString& Code, bool bSuccess, const FString& Summary)
{
	FScopeLock Lock(&ExecutionCriticalSection);
	
	ExecutionHistory.Add(FPythonExecutionHistoryEntry(Code, bSuccess, Summary));
	TrimHistoryIfNeeded();
	
	// Auto-save history periodically
	SaveHistoryToDisk();
}

FPythonExecutionHistoryEntry UUnrealCopilotExecutionManager::GetHistoryEntry(int32 Index) const
{
	FScopeLock Lock(&ExecutionCriticalSection);
	
	if (Index >= 0 && Index < ExecutionHistory.Num())
	{
		return ExecutionHistory[Index];
	}
	
	// Return empty entry if index is invalid
	return FPythonExecutionHistoryEntry();
}

void UUnrealCopilotExecutionManager::ClearHistory()
{
	FScopeLock Lock(&ExecutionCriticalSection);
	
	ExecutionHistory.Empty();
	SaveHistoryToDisk();
	
	UE_LOG(LogUnrealCopilotExecution, Log, TEXT("Execution history cleared"));
}

void UUnrealCopilotExecutionManager::SaveHistoryToDisk()
{
	FString HistoryFilePath = GetHistoryFilePath();
	
	// Create JSON object for history
	TSharedPtr<FJsonObject> HistoryJson = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> HistoryArray;
	
	for (const FPythonExecutionHistoryEntry& Entry : ExecutionHistory)
	{
		TSharedPtr<FJsonObject> EntryJson = MakeShareable(new FJsonObject);
		EntryJson->SetStringField(TEXT("Code"), Entry.Code);
		EntryJson->SetStringField(TEXT("Timestamp"), Entry.Timestamp.ToString());
		EntryJson->SetBoolField(TEXT("Success"), Entry.bWasSuccessful);
		EntryJson->SetStringField(TEXT("Summary"), Entry.ResultSummary);
		
		HistoryArray.Add(MakeShareable(new FJsonValueObject(EntryJson)));
	}
	
	HistoryJson->SetArrayField(TEXT("History"), HistoryArray);
	
	// Write to file
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(HistoryJson.ToSharedRef(), Writer);
	
	FFileHelper::SaveStringToFile(OutputString, *HistoryFilePath);
}

void UUnrealCopilotExecutionManager::LoadHistoryFromDisk()
{
	FString HistoryFilePath = GetHistoryFilePath();
	
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *HistoryFilePath))
	{
		// File doesn't exist or can't be read - start with empty history
		ExecutionHistory.Empty();
		return;
	}
	
	TSharedPtr<FJsonObject> HistoryJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(Reader, HistoryJson) || !HistoryJson.IsValid())
	{
		UE_LOG(LogUnrealCopilotExecution, Warning, TEXT("Failed to parse history JSON file"));
		ExecutionHistory.Empty();
		return;
	}
	
	const TArray<TSharedPtr<FJsonValue>>* HistoryArray;
	if (!HistoryJson->TryGetArrayField(TEXT("History"), HistoryArray))
	{
		ExecutionHistory.Empty();
		return;
	}
	
	ExecutionHistory.Empty();
	for (const TSharedPtr<FJsonValue>& Value : *HistoryArray)
	{
		const TSharedPtr<FJsonObject>* EntryObject;
		if (!Value->TryGetObject(EntryObject))
		{
			continue;
		}
		
		FPythonExecutionHistoryEntry Entry;
		(*EntryObject)->TryGetStringField(TEXT("Code"), Entry.Code);
		(*EntryObject)->TryGetBoolField(TEXT("Success"), Entry.bWasSuccessful);
		(*EntryObject)->TryGetStringField(TEXT("Summary"), Entry.ResultSummary);
		
		FString TimestampString;
		if ((*EntryObject)->TryGetStringField(TEXT("Timestamp"), TimestampString))
		{
			FDateTime::Parse(TimestampString, Entry.Timestamp);
		}
		
		ExecutionHistory.Add(Entry);
	}
	
	UE_LOG(LogUnrealCopilotExecution, Log, TEXT("Loaded %d entries from execution history"), ExecutionHistory.Num());
}

void UUnrealCopilotExecutionManager::SetExecutionState(EPythonExecutionState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnExecutionStateChanged.Broadcast(NewState);
		
		UE_LOG(LogUnrealCopilotExecution, Verbose, TEXT("Execution state changed to: %d"), (int32)NewState);
	}
}

FPythonExecutionResult UUnrealCopilotExecutionManager::ExecutePythonCodeSynchronous(const FString& PythonCode)
{
	FPythonExecutionResult Result;
	Result.Reset();
	
	// Record start time
	ExecutionStartTime = FPlatformTime::Seconds();
	
	// Check if PythonScriptPlugin is available
	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	if (!PythonPlugin)
	{
		Result.ErrorMessage = TEXT("PythonScriptPlugin not available");
		Result.ErrorType = EPythonExecutionError::SystemError;
		return Result;
	}
	
	if (!PythonPlugin->IsPythonAvailable())
	{
		Result.ErrorMessage = TEXT("Python is not available or not initialized");
		Result.ErrorType = EPythonExecutionError::SystemError;
		return Result;
	}
	
	try
	{
		// For simple cases without wrapper, try direct execution first
		if (!PythonCode.Contains(TEXT("\n")) && !PythonCode.Contains(TEXT("import")) && PythonCode.Len() < 100)
		{
			// Simple single statement - try direct execution
			FPythonCommandEx SimpleCommand;
			SimpleCommand.Command = PythonCode;
			SimpleCommand.ExecutionMode = EPythonCommandExecutionMode::ExecuteStatement;
			
			bool bSimpleSuccess = PythonPlugin->ExecPythonCommandEx(SimpleCommand);
			
			// Calculate execution time
			double ExecutionEndTime = FPlatformTime::Seconds();
			Result.ExecutionTimeSeconds = ExecutionEndTime - ExecutionStartTime;
			
			if (bSimpleSuccess)
			{
				Result.bSuccess = true;
				Result.Output = TEXT("Python code executed successfully");
				return Result;
			}
			// If simple execution failed, fall through to enhanced execution
		}
		
		// Enhanced Python execution with error capture for complex code
		FString EnhancedPythonCode = FString::Printf(TEXT(
			"import sys\n"
			"import traceback\n"
			"import io\n"
			"\n"
			"# Capture stdout and stderr\n"
			"old_stdout = sys.stdout\n"
			"old_stderr = sys.stderr\n"
			"stdout_buffer = io.StringIO()\n"
			"stderr_buffer = io.StringIO()\n"
			"sys.stdout = stdout_buffer\n"
			"sys.stderr = stderr_buffer\n"
			"\n"
			"try:\n"
			"    # Execute user code with proper indentation\n"
			"    exec('''%s''')\n"
			"    print('EXECUTION_SUCCESS', file=old_stdout)\n"
			"    stdout_content = stdout_buffer.getvalue()\n"
			"    if stdout_content:\n"
			"        print(f'OUTPUT: {stdout_content}', file=old_stdout)\n"
			"except Exception as e:\n"
			"    print('EXECUTION_ERROR', file=old_stdout)\n"
			"    print(f'ERROR_MESSAGE: {str(e)}', file=old_stdout)\n"
			"    print(f'ERROR_TYPE: {type(e).__name__}', file=old_stdout)\n"
			"    tb_lines = traceback.format_exc().splitlines()\n"
			"    print(f'STACK_TRACE: {\"||\".join(tb_lines)}', file=old_stdout)\n"
			"finally:\n"
			"    # Restore stdout and stderr\n"
			"    sys.stdout = old_stdout\n"
			"    sys.stderr = old_stderr\n"
		), *PythonCode.Replace(TEXT("'''"), TEXT("\\'\\'\\'")));
		
		// Execute enhanced Python code using ExecuteFile mode for multi-statement support
		FPythonCommandEx PythonCommand;
		PythonCommand.Command = EnhancedPythonCode;
		PythonCommand.ExecutionMode = EPythonCommandExecutionMode::ExecuteFile;
		
		bool bExecutionSuccess = PythonPlugin->ExecPythonCommandEx(PythonCommand);
		
		// Calculate execution time
		double ExecutionEndTime = FPlatformTime::Seconds();
		Result.ExecutionTimeSeconds = ExecutionEndTime - ExecutionStartTime;
		
		// Check for timeout
		if (Result.ExecutionTimeSeconds > ExecutionTimeoutSeconds)
		{
			Result.bSuccess = false;
			Result.ErrorMessage = FString::Printf(TEXT("Python execution timed out after %.2f seconds"), Result.ExecutionTimeSeconds);
			Result.ErrorType = EPythonExecutionError::TimeoutError;
			return Result;
		}
		
		if (bExecutionSuccess)
		{
			Result.bSuccess = true;
			Result.Output = TEXT("Python code executed successfully");
			// Note: In a complete implementation, we would capture and parse the enhanced output
			// to extract the actual stdout content and any error details
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Python execution failed");
			Result.ErrorType = EPythonExecutionError::RuntimeError;
		}
	}
	catch (const std::exception& e)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Exception during Python execution: %s"), ANSI_TO_TCHAR(e.what()));
		Result.ErrorType = EPythonExecutionError::SystemError;
	}
	catch (...)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Unknown exception during Python execution");
		Result.ErrorType = EPythonExecutionError::SystemError;
	}
	
	return Result;
}

void UUnrealCopilotExecutionManager::ParsePythonError(const FString& ErrorMessage, FPythonExecutionResult& Result)
{
	// Parse error message to extract line numbers and categorize errors
	if (ErrorMessage.Contains(TEXT("SyntaxError")))
	{
		Result.ErrorType = EPythonExecutionError::SyntaxError;
		
		// Try to extract line number from syntax error
		int32 LineStartIndex = ErrorMessage.Find(TEXT("line "));
		if (LineStartIndex != INDEX_NONE)
		{
			LineStartIndex += 5; // Length of "line "
			int32 LineEndIndex = ErrorMessage.Find(TEXT(","), LineStartIndex);
			if (LineEndIndex == INDEX_NONE)
			{
				LineEndIndex = ErrorMessage.Find(TEXT(" "), LineStartIndex);
			}
			
			if (LineEndIndex != INDEX_NONE)
			{
				FString LineNumberStr = ErrorMessage.Mid(LineStartIndex, LineEndIndex - LineStartIndex);
				Result.ErrorLineNumber = FCString::Atoi(*LineNumberStr);
			}
		}
	}
	else if (ErrorMessage.Contains(TEXT("TimeoutError")) || ErrorMessage.Contains(TEXT("timed out")))
	{
		Result.ErrorType = EPythonExecutionError::TimeoutError;
	}
	else if (ErrorMessage.Contains(TEXT("NameError")) || ErrorMessage.Contains(TEXT("AttributeError")) || 
			 ErrorMessage.Contains(TEXT("TypeError")) || ErrorMessage.Contains(TEXT("ValueError")))
	{
		Result.ErrorType = EPythonExecutionError::RuntimeError;
	}
	else
	{
		Result.ErrorType = EPythonExecutionError::SystemError;
	}
}

FString UUnrealCopilotExecutionManager::FormatStackTrace(const FString& RawStackTrace)
{
	// Format stack trace for better readability
	FString FormattedTrace = RawStackTrace;
	FormattedTrace = FormattedTrace.Replace(TEXT("||"), TEXT("\n"));
	FormattedTrace = FormattedTrace.Replace(TEXT("  File"), TEXT("File"));
	
	return FormattedTrace;
}

FString UUnrealCopilotExecutionManager::GetHistoryFilePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("UnrealCopilot") / TEXT("ExecutionHistory.json");
}

void UUnrealCopilotExecutionManager::TrimHistoryIfNeeded()
{
	if (ExecutionHistory.Num() > MaxHistoryEntries)
	{
		int32 NumToRemove = ExecutionHistory.Num() - MaxHistoryEntries;
		ExecutionHistory.RemoveAt(0, NumToRemove);
		
		UE_LOG(LogUnrealCopilotExecution, Log, TEXT("Trimmed %d entries from execution history"), NumToRemove);
	}
}