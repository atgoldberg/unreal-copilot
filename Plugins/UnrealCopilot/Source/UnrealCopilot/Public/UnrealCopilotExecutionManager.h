// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Containers/Queue.h"
#include "UnrealCopilotExecutionManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCopilotExecution, Log, All);

/**
 * Enumeration for different types of Python execution errors
 */
UENUM(BlueprintType)
enum class EPythonExecutionError : uint8
{
	None UMETA(DisplayName = "None"),
	SyntaxError UMETA(DisplayName = "Syntax Error"),
	RuntimeError UMETA(DisplayName = "Runtime Error"),
	TimeoutError UMETA(DisplayName = "Timeout Error"),
	ValidationError UMETA(DisplayName = "Validation Error"),
	SystemError UMETA(DisplayName = "System Error")
};

/**
 * Enumeration for Python execution states
 */
UENUM(BlueprintType)
enum class EPythonExecutionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Validating UMETA(DisplayName = "Validating"),
	Executing UMETA(DisplayName = "Executing"),
	Completed UMETA(DisplayName = "Completed"),
	Error UMETA(DisplayName = "Error"),
	Timeout UMETA(DisplayName = "Timeout")
};

/**
 * Structure containing detailed results from Python execution
 */
USTRUCT(BlueprintType)
struct UNREALCOPILOT_API FPythonExecutionResult
{
	GENERATED_BODY()

	/** Whether the execution was successful */
	UPROPERTY(BlueprintReadOnly, Category = "Execution Result")
	bool bSuccess = false;

	/** Standard output from Python execution */
	UPROPERTY(BlueprintReadOnly, Category = "Execution Result")
	FString Output;

	/** Error message if execution failed */
	UPROPERTY(BlueprintReadOnly, Category = "Execution Result")
	FString ErrorMessage;

	/** Python stack trace for debugging */
	UPROPERTY(BlueprintReadOnly, Category = "Execution Result")
	FString StackTrace;

	/** Line number where error occurred (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Execution Result")
	int32 ErrorLineNumber = -1;

	/** Time taken to execute the code in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Execution Result")
	float ExecutionTimeSeconds = 0.0f;

	/** Type of error that occurred */
	UPROPERTY(BlueprintReadOnly, Category = "Execution Result")
	EPythonExecutionError ErrorType = EPythonExecutionError::None;

	FPythonExecutionResult()
	{
		Reset();
	}

	void Reset()
	{
		bSuccess = false;
		Output.Empty();
		ErrorMessage.Empty();
		StackTrace.Empty();
		ErrorLineNumber = -1;
		ExecutionTimeSeconds = 0.0f;
		ErrorType = EPythonExecutionError::None;
	}
};

/**
 * Structure for storing execution history entries
 */
USTRUCT()
struct UNREALCOPILOT_API FPythonExecutionHistoryEntry
{
	GENERATED_BODY()

	/** The Python code that was executed */
	UPROPERTY()
	FString Code;

	/** Timestamp when the code was executed */
	UPROPERTY()
	FDateTime Timestamp;

	/** Whether the execution was successful */
	UPROPERTY()
	bool bWasSuccessful = false;

	/** Brief result summary */
	UPROPERTY()
	FString ResultSummary;

	FPythonExecutionHistoryEntry() = default;

	FPythonExecutionHistoryEntry(const FString& InCode, bool bInSuccess, const FString& InSummary)
		: Code(InCode)
		, Timestamp(FDateTime::Now())
		, bWasSuccessful(bInSuccess)
		, ResultSummary(InSummary)
	{
	}
};

/**
 * Delegate for execution state changes
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExecutionStateChanged, EPythonExecutionState);

/**
 * Delegate for execution completion
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExecutionCompleted, const FPythonExecutionResult&);

/**
 * Manager class for handling Python execution with enhanced features
 */
UCLASS(BlueprintType, Blueprintable)
class UNREALCOPILOT_API UUnrealCopilotExecutionManager : public UObject
{
	GENERATED_BODY()

public:
	UUnrealCopilotExecutionManager();
	virtual ~UUnrealCopilotExecutionManager();

	/**
	 * Get the singleton instance of the execution manager
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	static UUnrealCopilotExecutionManager* GetInstance();

	/**
	 * Execute Python code with enhanced error handling and timeout support
	 * @param PythonCode - The Python code to execute
	 * @param bAsync - Whether to execute asynchronously (default: false)
	 * @return Execution result with detailed information
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	FPythonExecutionResult ExecutePythonCode(const FString& PythonCode, bool bAsync = false);

	/**
	 * Validate Python syntax without executing the code
	 * @param PythonCode - The Python code to validate
	 * @return True if syntax is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	bool ValidatePythonSyntax(const FString& PythonCode, FString& OutErrorMessage);

	/**
	 * Cancel currently executing Python code (if async)
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void CancelExecution();

	/**
	 * Get current execution state
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	EPythonExecutionState GetCurrentExecutionState() const { return CurrentState; }

	/**
	 * Add entry to execution history
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void AddToHistory(const FString& Code, bool bSuccess, const FString& Summary);

	/**
	 * Get execution history (C++ only - not Blueprint exposed due to TArray limitation)
	 */
	const TArray<FPythonExecutionHistoryEntry>& GetExecutionHistory() const { return ExecutionHistory; }

	/**
	 * Get number of history entries
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	int32 GetHistoryCount() const { return ExecutionHistory.Num(); }

	/**
	 * Get specific history entry by index (C++ only - not Blueprint exposed)
	 */
	FPythonExecutionHistoryEntry GetHistoryEntry(int32 Index) const;

	/**
	 * Clear execution history
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void ClearHistory();

	/**
	 * Set execution timeout in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void SetExecutionTimeout(float TimeoutSeconds) { ExecutionTimeoutSeconds = FMath::Max(1.0f, TimeoutSeconds); }

	/**
	 * Get execution timeout in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	float GetExecutionTimeout() const { return ExecutionTimeoutSeconds; }

	/**
	 * Save history to persistent storage
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void SaveHistoryToDisk();

	/**
	 * Load history from persistent storage
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void LoadHistoryFromDisk();

public:
	/** Delegate called when execution state changes */
	FOnExecutionStateChanged OnExecutionStateChanged;

	/** Delegate called when execution completes */
	FOnExecutionCompleted OnExecutionCompleted;

private:
	/** Set the current execution state */
	void SetExecutionState(EPythonExecutionState NewState);

	/** Execute Python code synchronously with enhanced error capture */
	FPythonExecutionResult ExecutePythonCodeSynchronous(const FString& PythonCode);

	/** Parse Python error message to extract line number and error type */
	void ParsePythonError(const FString& ErrorMessage, FPythonExecutionResult& Result);

	/** Format stack trace for better readability */
	FString FormatStackTrace(const FString& RawStackTrace);

	/** Get history file path */
	FString GetHistoryFilePath() const;

	/** Trim history if it exceeds maximum entries */
	void TrimHistoryIfNeeded();

private:
	/** Singleton instance */
	static UUnrealCopilotExecutionManager* Instance;

	/** Current execution state */
	EPythonExecutionState CurrentState;

	/** Execution timeout in seconds */
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = "1.0", ClampMax = "300.0"))
	float ExecutionTimeoutSeconds;

	/** Maximum number of history entries to keep */
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = "10", ClampMax = "500"))
	int32 MaxHistoryEntries;

	/** Execution history */
	UPROPERTY()
	TArray<FPythonExecutionHistoryEntry> ExecutionHistory;

	/** Start time of current execution */
	double ExecutionStartTime;

	/** Flag indicating if execution was cancelled */
	bool bExecutionCancelled;

	/** Critical section for thread safety */
	mutable FCriticalSection ExecutionCriticalSection;
};