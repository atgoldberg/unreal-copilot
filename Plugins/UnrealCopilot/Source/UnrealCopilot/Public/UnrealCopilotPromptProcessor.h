// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Dom/JsonObject.h"
#include "UnrealCopilotPromptProcessor.generated.h"

/**
 * Enumeration for different workflow types that can influence prompt processing
 */
UENUM(BlueprintType)
enum class EUnrealCopilotWorkflowType : uint8
{
	General UMETA(DisplayName = "General"),
	MaterialCreation UMETA(DisplayName = "Material Creation"),
	LevelEditing UMETA(DisplayName = "Level Editing"),
	AssetManagement UMETA(DisplayName = "Asset Management"),
	Animation UMETA(DisplayName = "Animation"),
	VFX UMETA(DisplayName = "Visual Effects")
};

/**
 * Structure containing context information for prompt processing
 */
USTRUCT(BlueprintType)
struct UNREALCOPILOT_API FPromptContext
{
	GENERATED_BODY()

	/** Current project name */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FString ProjectName;

	/** Current level name */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FString CurrentLevel;

	/** Available assets in current context */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TArray<FString> AvailableAssets;

	/** Previous conversation history */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FString PreviousConversation;

	/** Current workflow type */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	EUnrealCopilotWorkflowType WorkflowType = EUnrealCopilotWorkflowType::General;

	/** Selected actors in the level */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TArray<FString> SelectedActors;

	FPromptContext()
	{
		ProjectName = TEXT("");
		CurrentLevel = TEXT("");
		WorkflowType = EUnrealCopilotWorkflowType::General;
	}
};

/**
 * Delegate for prompt processing completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPromptProcessed, const FString&, ProcessedPrompt, const FPromptContext&, Context);

/**
 * Class responsible for processing natural language prompts and preparing them for LLM consumption
 */
UCLASS(BlueprintType, Blueprintable)
class UNREALCOPILOT_API UUnrealCopilotPromptProcessor : public UObject
{
	GENERATED_BODY()

public:
	UUnrealCopilotPromptProcessor();

	/**
	 * Process a natural language prompt and inject context
	 * @param UserPrompt - The raw user prompt
	 * @param Context - Additional context information
	 * @return Processed prompt ready for LLM consumption
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	FString ProcessPrompt(const FString& UserPrompt, const FPromptContext& Context);

	/**
	 * Build system prompt based on current context and workflow type
	 * @param Context - Context information for the session
	 * @return Complete system prompt for LLM
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	FString BuildSystemPrompt(const FPromptContext& Context);

	/**
	 * Gather current project context automatically
	 * @return Populated context structure
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	FPromptContext GatherCurrentContext();

	/**
	 * Validate user input for safety and appropriateness
	 * @param UserPrompt - The user's input
	 * @param OutErrorMessage - Error message if validation fails
	 * @return True if prompt is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	bool ValidateUserPrompt(const FString& UserPrompt, FString& OutErrorMessage);

	/**
	 * Extract Python code from LLM response
	 * @param LLMResponse - Raw response from the LLM
	 * @return Extracted Python code
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	FString ExtractPythonCode(const FString& LLMResponse);

	/**
	 * Add a conversation entry to history
	 * @param UserPrompt - The user's prompt
	 * @param LLMResponse - The LLM's response
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void AddToConversationHistory(const FString& UserPrompt, const FString& LLMResponse);

	/**
	 * Clear conversation history
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	void ClearConversationHistory();

	/**
	 * Get formatted conversation history for context injection
	 * @return Formatted conversation history
	 */
	UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
	FString GetFormattedConversationHistory() const;

	/** Create OpenAI API request payload in JSON format */
	TSharedPtr<FJsonObject> CreateOpenAIRequestPayload(const FString& SystemPrompt, const FString& UserPrompt);

	/** Create GPT-5 Responses API request payload (different format from Chat Completions) */
	TSharedPtr<FJsonObject> CreateGPT5ResponsesPayload(const FString& UserPrompt);

public:
	/** Delegate called when prompt processing is complete */
	UPROPERTY(BlueprintAssignable, Category = "UnrealCopilot")
	FOnPromptProcessed OnPromptProcessed;

private:
	/**
	 * Get workflow-specific context and examples
	 * @param WorkflowType - Type of workflow
	 * @return Additional context for the workflow
	 */
	FString GetWorkflowSpecificContext(EUnrealCopilotWorkflowType WorkflowType) const;

	/**
	 * Sanitize user input to prevent injection attacks
	 * @param Input - Raw user input
	 * @return Sanitized input
	 */
	FString SanitizeUserInput(const FString& Input) const;

	/**
	 * Get current project information
	 * @return Project info string
	 */
	FString GetCurrentProjectInfo() const;

	/**
	 * Get current level information
	 * @return Level info string
	 */
	FString GetCurrentLevelInfo() const;

	/**
	 * Get available assets in content browser selection
	 * @return Array of asset names
	 */
	TArray<FString> GetAvailableAssets() const;

	/**
	 * Get currently selected actors
	 * @return Array of selected actor names
	 */
	TArray<FString> GetSelectedActors() const;

private:
	/** Conversation history for context */
	UPROPERTY()
	TArray<FString> ConversationHistory;

	/** Maximum conversation history entries to keep */
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxConversationHistory = 10;

	/** Current context cache */
	UPROPERTY()
	FPromptContext CachedContext;

	/** Last context update time */
	double LastContextUpdateTime = 0.0;

	/** Context cache validity duration in seconds */
	static constexpr double ContextCacheDuration = 5.0;
};