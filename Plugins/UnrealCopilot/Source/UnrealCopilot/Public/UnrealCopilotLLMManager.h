// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "UnrealCopilotPromptProcessor.h"
#include "UnrealCopilotSettings.h"
#include "UnrealCopilotLLMManager.generated.h"

/**
 * Enumeration for code generation states
 */
UENUM(BlueprintType)
enum class ECodeGenerationState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Processing UMETA(DisplayName = "Processing"),
	Validating UMETA(DisplayName = "Validating"),
	Completed UMETA(DisplayName = "Completed"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Structure containing the result of code generation
 */
USTRUCT(BlueprintType)
struct UNREALCOPILOT_API FCodeGenerationResult
{
	GENERATED_BODY()

	/** Whether the generation was successful */
	UPROPERTY(BlueprintReadOnly, Category = "Generation Result")
	bool bSuccess = false;

	/** Generated Python code */
	UPROPERTY(BlueprintReadOnly, Category = "Generation Result")
	FString GeneratedCode;

	/** Error message if generation failed */
	UPROPERTY(BlueprintReadOnly, Category = "Generation Result")
	FString ErrorMessage;

	/** Raw LLM response for debugging */
	UPROPERTY(BlueprintReadOnly, Category = "Generation Result")
	FString RawResponse;

	/** Time taken for generation in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Generation Result")
	float GenerationTimeSeconds = 0.0f;

	/** Tokens used in the request */
	UPROPERTY(BlueprintReadOnly, Category = "Generation Result")
	int32 TokensUsed = 0;

	/** API response code */
	UPROPERTY(BlueprintReadOnly, Category = "Generation Result")
	int32 ResponseCode = 0;

	FCodeGenerationResult()
	{
		Reset();
	}

	void Reset()
	{
		bSuccess = false;
		GeneratedCode.Empty();
		ErrorMessage.Empty();
		RawResponse.Empty();
		GenerationTimeSeconds = 0.0f;
		TokensUsed = 0;
		ResponseCode = 0;
	}
};

/**
 * Delegate for code generation completion
 */
DECLARE_DELEGATE_OneParam(FOnCodeGenerationComplete, const FCodeGenerationResult&);

/**
 * Delegate for generation state changes
 */
DECLARE_DELEGATE_OneParam(FOnGenerationStateChanged, ECodeGenerationState);

/**
 * Structure for tracking API usage
 */
USTRUCT()
struct FAPIUsageTracker
{
	GENERATED_BODY()

	/** Total requests made */
	int32 TotalRequests = 0;
	
	/** Requests in current minute */
	int32 RequestsThisMinute = 0;
	
	/** Last request time */
	double LastRequestTime = 0.0;
	
	/** Minute tracker reset time */
	double MinuteResetTime = 0.0;

	void UpdateUsage()
	{
		double CurrentTime = FPlatformTime::Seconds();
		
		// Reset minute counter if needed
		if (CurrentTime - MinuteResetTime >= 60.0)
		{
			RequestsThisMinute = 0;
			MinuteResetTime = CurrentTime;
		}
		
		TotalRequests++;
		RequestsThisMinute++;
		LastRequestTime = CurrentTime;
	}
	
	bool CanMakeRequest(int32 MaxPerMinute) const
	{
		return RequestsThisMinute < MaxPerMinute;
	}
};

/**
 * Main manager class for LLM integration and code generation
 */
UCLASS(BlueprintType, Blueprintable)
class UNREALCOPILOT_API UUnrealCopilotLLMManager : public UObject
{
	GENERATED_BODY()

public:
	UUnrealCopilotLLMManager();
	virtual ~UUnrealCopilotLLMManager();

	/**
	 * Get the singleton instance of the LLM manager
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	static UUnrealCopilotLLMManager* GetInstance();

	/**
	 * Process a natural language prompt and generate Python code
	 * @param Prompt - The user's natural language prompt
	 * @param OnComplete - Delegate called when generation completes
	 */
	void ProcessNaturalLanguagePrompt(const FString& Prompt, const FOnCodeGenerationComplete& OnComplete);

	/**
	 * Process a natural language prompt and generate Python code (Blueprint version)
	 * @param Prompt - The user's natural language prompt
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void ProcessNaturalLanguagePromptBP(const FString& Prompt);

	/**
	 * Set the API key for the current provider
	 * @param APIKey - The API key to use
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void SetAPIKey(const FString& APIKey);

	/**
	 * Check if the manager is configured for the specified provider
	 * @param Provider - The LLM provider to check
	 * @return True if properly configured
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	bool IsConfiguredForProvider(ELLMProvider Provider);

	/**
	 * Validate generated Python code for safety
	 * @param PythonCode - The code to validate
	 * @param OutErrorMessage - Error message if validation fails
	 * @return True if code is safe to execute
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	bool ValidateGeneratedCode(const FString& PythonCode, FString& OutErrorMessage);

	/**
	 * Get current generation state
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	ECodeGenerationState GetCurrentGenerationState() const { return CurrentState; }

	/**
	 * Cancel current code generation request
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void CancelGeneration();

	/**
	 * Get API usage statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void GetUsageStatistics(int32& OutTotalRequests, int32& OutRequestsThisMinute);

	/**
	 * Clear API usage statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void ClearUsageStatistics();

public:
	/** Delegate called when code generation completes */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCodeGenerationCompleteMulticast, const FCodeGenerationResult&);
	FOnCodeGenerationCompleteMulticast OnCodeGenerationComplete;

	/** Delegate called when generation state changes */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGenerationStateChangedMulticast, ECodeGenerationState);
	FOnGenerationStateChangedMulticast OnGenerationStateChanged;

private:
	/**
	 * Send request to OpenAI API
	 * @param ProcessedPrompt - The processed prompt to send
	 * @param OnComplete - Completion delegate
	 */
	void SendOpenAIRequest(const FString& ProcessedPrompt, const FOnCodeGenerationComplete& OnComplete);

	/**
	 * Handle HTTP response from OpenAI
	 * @param Request - The HTTP request
	 * @param Response - The HTTP response
	 * @param bWasSuccessful - Whether the request was successful
	 * @param OnComplete - Completion delegate
	 */
	void OnOpenAIResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnCodeGenerationComplete OnComplete);

	/**
	 * Parse OpenAI response and extract code
	 * @param ResponseJSON - The JSON response from OpenAI
	 * @param OutResult - The parsed result
	 */
	void ParseOpenAIResponse(const FString& ResponseJSON, FCodeGenerationResult& OutResult);

	/** Parse GPT-5 Responses API response (different format than Chat Completions) */
	void ParseGPT5ResponsesAPI(const FString& ResponseJSON, FCodeGenerationResult& OutResult);

	/**
	 * Build system prompt for current context
	 * @return Complete system prompt
	 */
	FString BuildSystemPrompt() const;

	/**
	 * Set the current generation state
	 * @param NewState - The new state to set
	 */
	void SetGenerationState(ECodeGenerationState NewState);

	/**
	 * Check rate limits before making request
	 * @return True if request can be made
	 */
	bool CheckRateLimits();

	/**
	 * Log API interaction for debugging
	 * @param Request - The request content
	 * @param Response - The response content
	 * @param bSuccess - Whether the interaction was successful
	 */
	void LogAPIInteraction(const FString& Request, const FString& Response, bool bSuccess);

private:
	/** Singleton instance */
	static UUnrealCopilotLLMManager* Instance;

	/** Prompt processor instance */
	UPROPERTY()
	UUnrealCopilotPromptProcessor* PromptProcessor;

	/** Current generation state */
	ECodeGenerationState CurrentState;

	/** Current HTTP request */
	FHttpRequestPtr CurrentRequest;

	/** Generation start time */
	double GenerationStartTime;

	/** API usage tracker */
	FAPIUsageTracker UsageTracker;

	/** Cached settings */
	UUnrealCopilotSettings* CachedSettings;

	/** Active completion delegates */
	TArray<FOnCodeGenerationComplete> ActiveDelegates;
};