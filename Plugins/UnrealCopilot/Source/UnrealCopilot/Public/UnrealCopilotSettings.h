// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UnrealCopilotSettings.generated.h"

/**
 * Enumeration for supported LLM providers
 */
UENUM(BlueprintType)
enum class ELLMProvider : uint8
{
	OpenAI UMETA(DisplayName = "OpenAI"),
	GitHubCopilot UMETA(DisplayName = "GitHub Copilot"),
	AzureOpenAI UMETA(DisplayName = "Azure OpenAI")
};

/**
 * Enumeration for OpenAI model types
 */
UENUM(BlueprintType)
enum class EOpenAIModel : uint8
{
	GPT4 UMETA(DisplayName = "GPT-4"),
	GPT4Turbo UMETA(DisplayName = "GPT-4 Turbo"),
	GPT35Turbo UMETA(DisplayName = "GPT-3.5 Turbo")
};

/**
 * Settings class for UnrealCopilot plugin configuration
 */
UCLASS(config=Game, defaultconfig, meta = (DisplayName = "UnrealCopilot Settings"))
class UNREALCOPILOT_API UUnrealCopilotSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUnrealCopilotSettings();

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override;

	/** Get the settings instance */
	static UUnrealCopilotSettings* Get();

	/** Get the OpenAI API key */
	FString GetOpenAIAPIKey() const;

	/** Set the OpenAI API key */
	void SetOpenAIAPIKey(const FString& APIKey);

	/** Get the model name string for API calls */
	FString GetModelNameForAPI() const;

	/** Validate current settings */
	bool ValidateSettings(FString& OutErrorMessage) const;

public:
	/** Currently selected LLM provider */
	UPROPERTY(Config, EditAnywhere, Category = "LLM Integration", meta = (DisplayName = "LLM Provider"))
	ELLMProvider CurrentProvider = ELLMProvider::OpenAI;

	/** OpenAI model selection */
	UPROPERTY(Config, EditAnywhere, Category = "OpenAI Settings", meta = (DisplayName = "OpenAI Model", EditCondition = "CurrentProvider == ELLMProvider::OpenAI"))
	EOpenAIModel OpenAIModel = EOpenAIModel::GPT4Turbo;

	/** Custom OpenAI endpoint URL (leave empty for default) */
	UPROPERTY(Config, EditAnywhere, Category = "OpenAI Settings", meta = (DisplayName = "Custom Endpoint URL", EditCondition = "CurrentProvider == ELLMProvider::OpenAI"))
	FString CustomEndpointURL;

	/** Maximum tokens for API responses */
	UPROPERTY(Config, EditAnywhere, Category = "LLM Integration", meta = (ClampMin = "100", ClampMax = "4000", DisplayName = "Max Response Tokens"))
	int32 MaxTokens = 2000;

	/** Temperature for LLM responses (0.0 = deterministic, 1.0 = creative) */
	UPROPERTY(Config, EditAnywhere, Category = "LLM Integration", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "Response Temperature"))
	float Temperature = 0.7f;

	/** Request timeout in seconds */
	UPROPERTY(Config, EditAnywhere, Category = "LLM Integration", meta = (ClampMin = "5.0", ClampMax = "300.0", DisplayName = "Request Timeout (seconds)"))
	float RequestTimeoutSeconds = 30.0f;

	/** Rate limiting: Max requests per minute */
	UPROPERTY(Config, EditAnywhere, Category = "Rate Limiting", meta = (ClampMin = "1", ClampMax = "100", DisplayName = "Max Requests Per Minute"))
	int32 MaxRequestsPerMinute = 20;

	/** Enable code safety validation */
	UPROPERTY(Config, EditAnywhere, Category = "Security", meta = (DisplayName = "Enable Code Safety Validation"))
	bool bEnableCodeSafetyValidation = true;

	/** Enable user confirmation for AI-generated code */
	UPROPERTY(Config, EditAnywhere, Category = "Security", meta = (DisplayName = "Require User Confirmation"))
	bool bRequireUserConfirmation = true;

	/** Blocked Python functions/modules for safety */
	UPROPERTY(Config, EditAnywhere, Category = "Security", meta = (DisplayName = "Blocked Operations"))
	TArray<FString> BlockedOperations;

	/** Enable request/response logging */
	UPROPERTY(Config, EditAnywhere, Category = "Debugging", meta = (DisplayName = "Enable API Logging"))
	bool bEnableAPILogging = false;

	/** System prompt template for technical art tasks */
	UPROPERTY(Config, EditAnywhere, Category = "Prompt Engineering", meta = (DisplayName = "System Prompt Template", MultiLine = true))
	FString SystemPromptTemplate;

private:
	/** Encrypted API key storage */
	UPROPERTY(Config)
	FString EncryptedAPIKey;

	/** Simple encryption key (not secure for production) */
	static const FString EncryptionKey;

	/** Encrypt a string using simple XOR */
	FString EncryptString(const FString& PlainText) const;

	/** Decrypt a string using simple XOR */
	FString DecryptString(const FString& EncryptedText) const;
};