// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilotPromptProcessor.h"
#include "UnrealCopilotSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

#if WITH_EDITOR
#include "EditorAssetLibrary.h"
#include "EditorLevelLibrary.h"
#include "Selection.h"
#include "Editor.h"
#endif

UUnrealCopilotPromptProcessor::UUnrealCopilotPromptProcessor()
{
	MaxConversationHistory = 10;
	LastContextUpdateTime = 0.0;
}

FString UUnrealCopilotPromptProcessor::ProcessPrompt(const FString& UserPrompt, const FPromptContext& Context)
{
	// Sanitize user input
	FString SanitizedPrompt = SanitizeUserInput(UserPrompt);
	
	// Validate the prompt
	FString ValidationError;
	if (!ValidateUserPrompt(SanitizedPrompt, ValidationError))
	{
		return FString::Printf(TEXT("Invalid prompt: %s"), *ValidationError);
	}

	// Build enhanced prompt with context
	FString ProcessedPrompt = BuildSystemPrompt(Context);
	ProcessedPrompt += TEXT("\n\nUser Request: ");
	ProcessedPrompt += SanitizedPrompt;

	// Add conversation history if available
	FString FormattedConversationHistory = GetFormattedConversationHistory();
	if (!FormattedConversationHistory.IsEmpty())
	{
		ProcessedPrompt += TEXT("\n\nPrevious Conversation:\n");
		ProcessedPrompt += FormattedConversationHistory;
	}

	// Fire delegate
	OnPromptProcessed.Broadcast(ProcessedPrompt, Context);

	return ProcessedPrompt;
}

FString UUnrealCopilotPromptProcessor::BuildSystemPrompt(const FPromptContext& Context)
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	FString SystemPrompt = Settings->SystemPromptTemplate;

	// Add project context
	if (!Context.ProjectName.IsEmpty())
	{
		SystemPrompt += FString::Printf(TEXT("\n\nCurrent Project: %s"), *Context.ProjectName);
	}

	// Add level context
	if (!Context.CurrentLevel.IsEmpty())
	{
		SystemPrompt += FString::Printf(TEXT("\nCurrent Level: %s"), *Context.CurrentLevel);
	}

	// Add selected actors context
	if (Context.SelectedActors.Num() > 0)
	{
		SystemPrompt += TEXT("\n\nCurrently Selected Actors:");
		for (const FString& Actor : Context.SelectedActors)
		{
			SystemPrompt += FString::Printf(TEXT("\n- %s"), *Actor);
		}
	}

	// Add available assets context
	if (Context.AvailableAssets.Num() > 0 && Context.AvailableAssets.Num() <= 20) // Limit to avoid token overflow
	{
		SystemPrompt += TEXT("\n\nAvailable Assets:");
		for (int32 i = 0; i < FMath::Min(Context.AvailableAssets.Num(), 20); ++i)
		{
			SystemPrompt += FString::Printf(TEXT("\n- %s"), *Context.AvailableAssets[i]);
		}
	}

	// Add workflow-specific context
	SystemPrompt += GetWorkflowSpecificContext(Context.WorkflowType);

	return SystemPrompt;
}

FPromptContext UUnrealCopilotPromptProcessor::GatherCurrentContext()
{
	// Check if cached context is still valid
	double CurrentTime = FPlatformTime::Seconds();
	if (CurrentTime - LastContextUpdateTime < ContextCacheDuration)
	{
		return CachedContext;
	}

	FPromptContext Context;

	// Gather project info
	Context.ProjectName = GetCurrentProjectInfo();
	
	// Gather level info
	Context.CurrentLevel = GetCurrentLevelInfo();
	
	// Gather available assets
	Context.AvailableAssets = GetAvailableAssets();
	
	// Gather selected actors
	Context.SelectedActors = GetSelectedActors();

	// Default workflow type
	Context.WorkflowType = EUnrealCopilotWorkflowType::General;

	// Cache the context
	CachedContext = Context;
	LastContextUpdateTime = CurrentTime;

	return Context;
}

bool UUnrealCopilotPromptProcessor::ValidateUserPrompt(const FString& UserPrompt, FString& OutErrorMessage)
{
	// Check for empty prompt
	if (UserPrompt.IsEmpty() || UserPrompt.TrimStartAndEnd().IsEmpty())
	{
		OutErrorMessage = TEXT("Prompt cannot be empty");
		return false;
	}

	// Check prompt length (reasonable limits for API)
	if (UserPrompt.Len() > 2000)
	{
		OutErrorMessage = TEXT("Prompt is too long (max 2000 characters)");
		return false;
	}

	// Check for potentially harmful content patterns
	TArray<FString> ProhibitedPatterns = {
		TEXT("DELETE"),
		TEXT("DROP TABLE"),
		TEXT("TRUNCATE"),
		TEXT("format C:"),
		TEXT("rm -rf"),
		TEXT("del /f")
	};

	FString UpperPrompt = UserPrompt.ToUpper();
	for (const FString& Pattern : ProhibitedPatterns)
	{
		if (UpperPrompt.Contains(Pattern))
		{
			OutErrorMessage = FString::Printf(TEXT("Prompt contains prohibited pattern: %s"), *Pattern);
			return false;
		}
	}

	return true;
}

FString UUnrealCopilotPromptProcessor::ExtractPythonCode(const FString& LLMResponse)
{
	FString PythonCode;
	
	// Look for Python code blocks
	FString SearchString = TEXT("```python");
	int32 StartIndex = LLMResponse.Find(SearchString, ESearchCase::IgnoreCase);
	
	if (StartIndex != INDEX_NONE)
	{
		StartIndex += SearchString.Len();
		
		// Find the end of the code block
		int32 EndIndex = LLMResponse.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex);
		
		if (EndIndex != INDEX_NONE)
		{
			PythonCode = LLMResponse.Mid(StartIndex, EndIndex - StartIndex);
		}
		else
		{
			// No closing ```, take everything after the opening
			PythonCode = LLMResponse.Mid(StartIndex);
		}
	}
	else
	{
		// Try alternative patterns
		SearchString = TEXT("```");
		StartIndex = LLMResponse.Find(SearchString);
		if (StartIndex != INDEX_NONE)
		{
			StartIndex += SearchString.Len();
			int32 EndIndex = LLMResponse.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex);
			if (EndIndex != INDEX_NONE)
			{
				PythonCode = LLMResponse.Mid(StartIndex, EndIndex - StartIndex);
			}
		}
		else
		{
			// Assume the entire response is code if no code blocks found
			PythonCode = LLMResponse;
		}
	}

	// Clean up the extracted code
	PythonCode = PythonCode.TrimStartAndEnd();
	
	return PythonCode;
}

void UUnrealCopilotPromptProcessor::AddToConversationHistory(const FString& UserPrompt, const FString& LLMResponse)
{
	FString HistoryEntry = FString::Printf(TEXT("User: %s\nAssistant: %s"), 
		*UserPrompt.Left(500), // Limit length to avoid token overflow
		*LLMResponse.Left(500));
	
	ConversationHistory.Add(HistoryEntry);
	
	// Trim history if it exceeds max entries
	while (ConversationHistory.Num() > MaxConversationHistory)
	{
		ConversationHistory.RemoveAt(0);
	}
}

void UUnrealCopilotPromptProcessor::ClearConversationHistory()
{
	ConversationHistory.Empty();
}

FString UUnrealCopilotPromptProcessor::GetFormattedConversationHistory() const
{
	if (ConversationHistory.Num() == 0)
	{
		return FString();
	}

	FString FormattedHistory = TEXT("Previous conversation context:");
	for (const FString& Entry : ConversationHistory)
	{
		FormattedHistory += TEXT("\n") + Entry;
	}
	
	return FormattedHistory;
}

TSharedPtr<FJsonObject> UUnrealCopilotPromptProcessor::CreateOpenAIRequestPayload(const FString& SystemPrompt, const FString& UserPrompt)
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	// Set model
	FString ModelName = Settings->GetModelNameForAPI();
	JsonObject->SetStringField(TEXT("model"), ModelName);
	
	// Set parameters - use correct token parameter based on model
	// Newer models (GPT-5, GPT-4 Turbo, etc.) use max_completion_tokens
	// Older models (GPT-4, GPT-3.5) use max_tokens
	bool bUseNewTokenParameter = (Settings->OpenAIModel == EOpenAIModel::GPT5) ||
								 (Settings->OpenAIModel == EOpenAIModel::GPT4Turbo) ||
								 ModelName.Contains(TEXT("gpt-4-turbo")) ||
								 ModelName.Contains(TEXT("gpt-5"));
	
	if (bUseNewTokenParameter)
	{
		JsonObject->SetNumberField(TEXT("max_completion_tokens"), Settings->MaxTokens);
	}
	else
	{
		JsonObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);
	}
	
	// Temperature parameter - GPT-5 only supports default value of 1.0
	// Other models can use custom temperature values
	bool bSupportsCustomTemperature = (Settings->OpenAIModel != EOpenAIModel::GPT5) &&
									  !ModelName.Contains(TEXT("gpt-5"));
	
	if (bSupportsCustomTemperature)
	{
		JsonObject->SetNumberField(TEXT("temperature"), Settings->Temperature);
	}
	// For GPT-5, omit temperature parameter to use default (1.0)
	
	// Create messages array
	TArray<TSharedPtr<FJsonValue>> Messages;
	
	// Add system message
	TSharedPtr<FJsonObject> SystemMessage = MakeShareable(new FJsonObject);
	SystemMessage->SetStringField(TEXT("role"), TEXT("system"));
	SystemMessage->SetStringField(TEXT("content"), SystemPrompt);
	Messages.Add(MakeShareable(new FJsonValueObject(SystemMessage)));
	
	// Add user message
	TSharedPtr<FJsonObject> UserMessage = MakeShareable(new FJsonObject);
	UserMessage->SetStringField(TEXT("role"), TEXT("user"));
	UserMessage->SetStringField(TEXT("content"), UserPrompt);
	Messages.Add(MakeShareable(new FJsonValueObject(UserMessage)));
	
	JsonObject->SetArrayField(TEXT("messages"), Messages);
	
	return JsonObject;
}

TSharedPtr<FJsonObject> UUnrealCopilotPromptProcessor::CreateGPT5ResponsesPayload(const FString& UserPrompt)
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	// Set model
	JsonObject->SetStringField(TEXT("model"), Settings->GetModelNameForAPI());
	
	// Build context + user messages (Responses API expects structured input rather than raw string prompt)
	FPromptContext Context = GatherCurrentContext();
	FString SystemPrompt = BuildSystemPrompt(Context);

	// Construct input array: [{role: system, content: ...}, {role: user, content: ...}]
	TArray<TSharedPtr<FJsonValue>> InputArray;
	{
		TSharedPtr<FJsonObject> SystemObj = MakeShareable(new FJsonObject);
		SystemObj->SetStringField(TEXT("role"), TEXT("system"));
		SystemObj->SetStringField(TEXT("content"), SystemPrompt);
		InputArray.Add(MakeShareable(new FJsonValueObject(SystemObj)));
	}
	{
		TSharedPtr<FJsonObject> UserObj = MakeShareable(new FJsonObject);
		UserObj->SetStringField(TEXT("role"), TEXT("user"));
		UserObj->SetStringField(TEXT("content"), UserPrompt);
		InputArray.Add(MakeShareable(new FJsonValueObject(UserObj)));
	}
	JsonObject->SetArrayField(TEXT("input"), InputArray);

	// GPT-5 controls: max_output_tokens, reasoning.effort, text.verbosity
	JsonObject->SetNumberField(TEXT("max_output_tokens"), Settings->MaxTokens);

	// Reasoning effort object
	TSharedPtr<FJsonObject> ReasoningObj = MakeShareable(new FJsonObject);
	switch (Settings->GPT5ReasoningEffort)
	{
	case EGPT5ReasoningEffort::Minimal: ReasoningObj->SetStringField(TEXT("effort"), TEXT("minimal")); break;
	case EGPT5ReasoningEffort::Low: ReasoningObj->SetStringField(TEXT("effort"), TEXT("low")); break;
	case EGPT5ReasoningEffort::Medium: ReasoningObj->SetStringField(TEXT("effort"), TEXT("medium")); break;
	case EGPT5ReasoningEffort::High: ReasoningObj->SetStringField(TEXT("effort"), TEXT("high")); break;
	}
	JsonObject->SetObjectField(TEXT("reasoning"), ReasoningObj);

	// Text verbosity object
	TSharedPtr<FJsonObject> TextObj = MakeShareable(new FJsonObject);
	switch (Settings->GPT5Verbosity)
	{
	case EGPT5Verbosity::Low: TextObj->SetStringField(TEXT("verbosity"), TEXT("low")); break;
	case EGPT5Verbosity::Medium: TextObj->SetStringField(TEXT("verbosity"), TEXT("medium")); break;
	case EGPT5Verbosity::High: TextObj->SetStringField(TEXT("verbosity"), TEXT("high")); break;
	}
	JsonObject->SetObjectField(TEXT("text"), TextObj);

	return JsonObject;
}

FString UUnrealCopilotPromptProcessor::GetWorkflowSpecificContext(EUnrealCopilotWorkflowType WorkflowType) const
{
	switch (WorkflowType)
	{
	case EUnrealCopilotWorkflowType::MaterialCreation:
		return TEXT("\n\nFocus on material creation workflows using unreal.MaterialEditingLibrary and related classes.");
		
	case EUnrealCopilotWorkflowType::LevelEditing:
		return TEXT("\n\nFocus on level editing operations using unreal.EditorLevelLibrary and actor manipulation.");
		
	case EUnrealCopilotWorkflowType::AssetManagement:
		return TEXT("\n\nFocus on asset management using unreal.EditorAssetLibrary and content browser operations.");
		
	case EUnrealCopilotWorkflowType::Animation:
		return TEXT("\n\nFocus on animation workflows including skeletal mesh, animation blueprints, and sequences.");
		
	case EUnrealCopilotWorkflowType::VFX:
		return TEXT("\n\nFocus on visual effects including particle systems, Niagara, and material effects.");
		
	case EUnrealCopilotWorkflowType::General:
	default:
		return TEXT("");
	}
}

FString UUnrealCopilotPromptProcessor::SanitizeUserInput(const FString& Input) const
{
	// Remove potentially harmful characters and normalize whitespace
	FString Sanitized = Input;
	
	// Remove control characters except common whitespace
	FString CleanInput;
	for (TCHAR Char : Sanitized)
	{
		if (Char >= 32 || Char == '\n' || Char == '\r' || Char == '\t')
		{
			CleanInput.AppendChar(Char);
		}
	}
	
	return CleanInput.TrimStartAndEnd();
}

FString UUnrealCopilotPromptProcessor::GetCurrentProjectInfo() const
{
	if (GEngine && GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull))
	{
		return FApp::GetProjectName();
	}
	return TEXT("Unknown Project");
}

FString UUnrealCopilotPromptProcessor::GetCurrentLevelInfo() const
{
	#if WITH_EDITOR
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World && World->GetCurrentLevel())
		{
			return World->GetCurrentLevel()->GetOuter()->GetName();
		}
	}
	#endif
	
	return TEXT("Unknown Level");
}

TArray<FString> UUnrealCopilotPromptProcessor::GetAvailableAssets() const
{
	TArray<FString> Assets;
	
	#if WITH_EDITOR
	// Get assets from the Game directory
	// Using a simplified approach for now
	TArray<FString> AssetPaths = UEditorAssetLibrary::ListAssets(TEXT("/Game"));
	
	// Limit to first 10 assets to avoid overwhelming the context
	for (int32 i = 0; i < FMath::Min(AssetPaths.Num(), 10); ++i)
	{
		FString AssetName = FPaths::GetBaseFilename(AssetPaths[i]);
		Assets.AddUnique(AssetName);
	}
	#endif
	
	return Assets;
}

TArray<FString> UUnrealCopilotPromptProcessor::GetSelectedActors() const
{
	TArray<FString> SelectedActorNames;
	
	#if WITH_EDITOR
	if (GEditor)
	{
		USelection* Selection = GEditor->GetSelectedActors();
		if (Selection)
		{
			for (FSelectionIterator Iter(*Selection); Iter; ++Iter)
			{
				if (AActor* Actor = Cast<AActor>(*Iter))
				{
					SelectedActorNames.Add(Actor->GetName());
				}
			}
		}
	}
	#endif
	
	return SelectedActorNames;
}