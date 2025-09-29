// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilotSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

const FString UUnrealCopilotSettings::EncryptionKey = TEXT("UnrealCopilot2024");

UUnrealCopilotSettings::UUnrealCopilotSettings()
{
	CategoryName = TEXT("Plugins");
	
	// Set default system prompt template
	SystemPromptTemplate = TEXT(
		"You are an AI assistant specialized in Unreal Engine Python scripting for technical artists.\n"
		"Generate Python code that uses the Unreal Engine Python API to accomplish technical art tasks.\n\n"
		"Key Guidelines:\n"
		"- Use only Unreal Engine Python API (import unreal)\n"
		"- Focus on technical art workflows (materials, levels, assets, animation)\n"
		"- Provide clean, well-commented code\n"
		"- Handle errors gracefully\n"
		"- Use appropriate Unreal Python patterns and best practices\n"
		"- Avoid file system operations or external network calls unless specifically requested\n\n"
		"Available modules: unreal, typing\n"
		"Common classes: unreal.EditorAssetLibrary, unreal.EditorLevelLibrary, unreal.MaterialEditingLibrary\n\n"
		"Respond with executable Python code only, wrapped in ```python code blocks."
	);

	// Set default blocked operations for security
	BlockedOperations.Add(TEXT("os."));
	BlockedOperations.Add(TEXT("subprocess"));
	BlockedOperations.Add(TEXT("__import__"));
	BlockedOperations.Add(TEXT("eval("));
	BlockedOperations.Add(TEXT("exec("));
	BlockedOperations.Add(TEXT("open("));
	BlockedOperations.Add(TEXT("file("));
	BlockedOperations.Add(TEXT("input("));
	BlockedOperations.Add(TEXT("raw_input("));
}

FName UUnrealCopilotSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

UUnrealCopilotSettings* UUnrealCopilotSettings::Get()
{
	return GetMutableDefault<UUnrealCopilotSettings>();
}

FString UUnrealCopilotSettings::GetOpenAIAPIKey() const
{
	// If we have a value in the public field, use it (new system)
	if (!OpenAIAPIKey.IsEmpty())
	{
		return OpenAIAPIKey;
	}
	
	// Fall back to encrypted storage (backward compatibility)
	if (!EncryptedAPIKey.IsEmpty())
	{
		return DecryptString(EncryptedAPIKey);
	}
	
	return FString();
}

void UUnrealCopilotSettings::SetOpenAIAPIKey(const FString& APIKey)
{
	// Store in the public field (new system)
	OpenAIAPIKey = APIKey;
	
	// Also store encrypted version for backward compatibility and security
	if (APIKey.IsEmpty())
	{
		EncryptedAPIKey.Empty();
	}
	else
	{
		EncryptedAPIKey = EncryptString(APIKey);
	}
	
	SaveConfig();
}

FString UUnrealCopilotSettings::GetModelNameForAPI() const
{
	switch (OpenAIModel)
	{
	case EOpenAIModel::GPT4:
		return TEXT("gpt-4");
	case EOpenAIModel::GPT4Turbo:
		return TEXT("gpt-4-turbo-preview");
	case EOpenAIModel::GPT5:
		return TEXT("gpt-5");
	case EOpenAIModel::GPT35Turbo:
		return TEXT("gpt-3.5-turbo");
	default:
		return TEXT("gpt-5");
	}
}

bool UUnrealCopilotSettings::ValidateSettings(FString& OutErrorMessage) const
{
	if (CurrentProvider == ELLMProvider::OpenAI)
	{
		FString APIKey = GetOpenAIAPIKey();
		if (APIKey.IsEmpty())
		{
			OutErrorMessage = TEXT("OpenAI API key is required. Please set it in the plugin settings.");
			return false;
		}
		
		// Basic API key format validation
		if (APIKey.Len() < 20 || !APIKey.StartsWith(TEXT("sk-")))
		{
			OutErrorMessage = TEXT("OpenAI API key appears to be invalid. Please check the format.");
			return false;
		}
	}
	
	if (MaxTokens <= 0 || MaxTokens > 4000)
	{
		OutErrorMessage = TEXT("Max tokens must be between 1 and 4000.");
		return false;
	}
	
	if (Temperature < 0.0f || Temperature > 1.0f)
	{
		OutErrorMessage = TEXT("Temperature must be between 0.0 and 1.0.");
		return false;
	}
	
	if (RequestTimeoutSeconds <= 0.0f)
	{
		OutErrorMessage = TEXT("Request timeout must be greater than 0.");
		return false;
	}
	
	return true;
}

#if WITH_EDITOR
void UUnrealCopilotSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// Handle API key changes
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UUnrealCopilotSettings, OpenAIAPIKey))
	{
		// Update encrypted storage when API key is changed in UI
		if (OpenAIAPIKey.IsEmpty())
		{
			EncryptedAPIKey.Empty();
		}
		else
		{
			EncryptedAPIKey = EncryptString(OpenAIAPIKey);
		}
	}
}
#endif

FString UUnrealCopilotSettings::EncryptString(const FString& PlainText) const
{
	if (PlainText.IsEmpty())
	{
		return FString();
	}

	FString Result;
	const TCHAR* KeyPtr = *EncryptionKey;
	int32 KeyLength = EncryptionKey.Len();
	
	for (int32 i = 0; i < PlainText.Len(); ++i)
	{
		TCHAR EncryptedChar = PlainText[i] ^ KeyPtr[i % KeyLength];
		Result.AppendChar(EncryptedChar);
	}
	
	return Result;
}

FString UUnrealCopilotSettings::DecryptString(const FString& EncryptedText) const
{
	// XOR encryption is symmetric, so decryption is the same as encryption
	return EncryptString(EncryptedText);
}