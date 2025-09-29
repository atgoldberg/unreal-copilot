// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealCopilotLLMManager.h"
#include "UnrealCopilotSettings.h"
#include "Http.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/DateTime.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogUnrealCopilotLLM, Log, All);

UUnrealCopilotLLMManager* UUnrealCopilotLLMManager::Instance = nullptr;

UUnrealCopilotLLMManager::UUnrealCopilotLLMManager()
{
	CurrentState = ECodeGenerationState::Idle;
	GenerationStartTime = 0.0;
	CachedSettings = nullptr;

	// Create prompt processor
	PromptProcessor = CreateDefaultSubobject<UUnrealCopilotPromptProcessor>(TEXT("PromptProcessor"));
}

UUnrealCopilotLLMManager::~UUnrealCopilotLLMManager()
{
	// Cancel any active requests
	if (CurrentRequest.IsValid())
	{
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
	}
}

UUnrealCopilotLLMManager* UUnrealCopilotLLMManager::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<UUnrealCopilotLLMManager>();
		Instance->AddToRoot(); // Prevent garbage collection
	}
	return Instance;
}

void UUnrealCopilotLLMManager::ProcessNaturalLanguagePrompt(const FString& Prompt, const FOnCodeGenerationComplete& OnComplete)
{
	// Check if already processing
	if (CurrentState == ECodeGenerationState::Processing)
	{
		FCodeGenerationResult ErrorResult;
		ErrorResult.bSuccess = false;
		ErrorResult.ErrorMessage = TEXT("Another code generation request is already in progress");
		if (OnComplete.IsBound())
		{
			OnComplete.Execute(ErrorResult);
		}
		return;
	}

	// Validate settings
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	FString ValidationError;
	if (!Settings->ValidateSettings(ValidationError))
	{
		FCodeGenerationResult ErrorResult;
		ErrorResult.bSuccess = false;
		ErrorResult.ErrorMessage = FString::Printf(TEXT("Settings validation failed: %s"), *ValidationError);
		if (OnComplete.IsBound())
		{
			OnComplete.Execute(ErrorResult);
		}
		return;
	}

	// Check rate limits
	if (!CheckRateLimits())
	{
		FCodeGenerationResult ErrorResult;
		ErrorResult.bSuccess = false;
		ErrorResult.ErrorMessage = TEXT("Rate limit exceeded. Please wait before making another request.");
		if (OnComplete.IsBound())
		{
			OnComplete.Execute(ErrorResult);
		}
		return;
	}

	// Validate user prompt
	FString PromptValidationError;
	if (!PromptProcessor->ValidateUserPrompt(Prompt, PromptValidationError))
	{
		FCodeGenerationResult ErrorResult;
		ErrorResult.bSuccess = false;
		ErrorResult.ErrorMessage = FString::Printf(TEXT("Prompt validation failed: %s"), *PromptValidationError);
		if (OnComplete.IsBound())
		{
			OnComplete.Execute(ErrorResult);
		}
		return;
	}

	SetGenerationState(ECodeGenerationState::Processing);
	GenerationStartTime = FPlatformTime::Seconds();

	// Store delegate for later use
	ActiveDelegates.Add(OnComplete);

	// Process prompt with context
	FPromptContext Context = PromptProcessor->GatherCurrentContext();
	FString ProcessedPrompt = PromptProcessor->ProcessPrompt(Prompt, Context);

	// Send to appropriate provider
	if (Settings->CurrentProvider == ELLMProvider::OpenAI)
	{
		SendOpenAIRequest(ProcessedPrompt, OnComplete);
	}
	else
	{
		FCodeGenerationResult ErrorResult;
		ErrorResult.bSuccess = false;
		ErrorResult.ErrorMessage = TEXT("Selected LLM provider is not yet supported");
		SetGenerationState(ECodeGenerationState::Error);
		if (OnComplete.IsBound())
		{
			OnComplete.Execute(ErrorResult);
		}
	}
}

void UUnrealCopilotLLMManager::ProcessNaturalLanguagePromptBP(const FString& Prompt)
{
	// Create empty delegate for Blueprint version
	FOnCodeGenerationComplete EmptyDelegate;
	ProcessNaturalLanguagePrompt(Prompt, EmptyDelegate);
}

void UUnrealCopilotLLMManager::SetAPIKey(const FString& APIKey)
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	Settings->SetOpenAIAPIKey(APIKey);
}

bool UUnrealCopilotLLMManager::IsConfiguredForProvider(ELLMProvider Provider)
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	
	switch (Provider)
	{
	case ELLMProvider::OpenAI:
		return !Settings->GetOpenAIAPIKey().IsEmpty();
	case ELLMProvider::GitHubCopilot:
		// Not yet implemented
		return false;
	case ELLMProvider::AzureOpenAI:
		// Not yet implemented
		return false;
	default:
		return false;
	}
}

bool UUnrealCopilotLLMManager::ValidateGeneratedCode(const FString& PythonCode, FString& OutErrorMessage)
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	
	if (!Settings->bEnableCodeSafetyValidation)
	{
		return true; // Safety validation disabled
	}

	// Check for blocked operations
	for (const FString& BlockedOp : Settings->BlockedOperations)
	{
		if (PythonCode.Contains(BlockedOp))
		{
			OutErrorMessage = FString::Printf(TEXT("Generated code contains blocked operation: %s"), *BlockedOp);
			return false;
		}
	}

	// Check for common dangerous patterns
	TArray<FString> DangerousPatterns = {
		TEXT("__import__"),
		TEXT("eval("),
		TEXT("exec("),
		TEXT("compile("),
		TEXT("globals()"),
		TEXT("locals()"),
		TEXT("getattr("),
		TEXT("setattr("),
		TEXT("delattr("),
		TEXT("hasattr(")
	};

	for (const FString& Pattern : DangerousPatterns)
	{
		if (PythonCode.Contains(Pattern))
		{
			OutErrorMessage = FString::Printf(TEXT("Generated code contains potentially unsafe pattern: %s"), *Pattern);
			return false;
		}
	}

	// Check for file system access if not explicitly allowed
	if (PythonCode.Contains(TEXT("open(")) || PythonCode.Contains(TEXT("file(")))
	{
		OutErrorMessage = TEXT("Generated code contains file system access which is not allowed");
		return false;
	}

	return true;
}

void UUnrealCopilotLLMManager::CancelGeneration()
{
	if (CurrentRequest.IsValid())
	{
		CurrentRequest->CancelRequest();
		CurrentRequest.Reset();
	}
	
	SetGenerationState(ECodeGenerationState::Idle);
	
	// Call all active delegates with cancellation result
	FCodeGenerationResult CancelResult;
	CancelResult.bSuccess = false;
	CancelResult.ErrorMessage = TEXT("Generation cancelled by user");
	
	for (const FOnCodeGenerationComplete& Delegate : ActiveDelegates)
	{
		if (Delegate.IsBound())
		{
			Delegate.Execute(CancelResult);
		}
	}
	
	ActiveDelegates.Empty();
}

void UUnrealCopilotLLMManager::GetUsageStatistics(int32& OutTotalRequests, int32& OutRequestsThisMinute)
{
	OutTotalRequests = UsageTracker.TotalRequests;
	OutRequestsThisMinute = UsageTracker.RequestsThisMinute;
}

void UUnrealCopilotLLMManager::ClearUsageStatistics()
{
	UsageTracker = FAPIUsageTracker();
}

void UUnrealCopilotLLMManager::SendOpenAIRequest(const FString& ProcessedPrompt, const FOnCodeGenerationComplete& OnComplete)
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	
	// Create HTTP request
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	
	// Set URL
	FString URL = Settings->CustomEndpointURL.IsEmpty() 
		? TEXT("https://api.openai.com/v1/chat/completions")
		: Settings->CustomEndpointURL;
	Request->SetURL(URL);
	
	// Set method and headers
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings->GetOpenAIAPIKey()));
	
	// Create request payload
	FPromptContext Context = PromptProcessor->GatherCurrentContext();
	FString SystemPrompt = PromptProcessor->BuildSystemPrompt(Context);
	TSharedPtr<FJsonObject> PayloadJson = PromptProcessor->CreateOpenAIRequestPayload(SystemPrompt, ProcessedPrompt);
	
	// Serialize JSON
	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(PayloadJson.ToSharedRef(), Writer);
	
	Request->SetContentAsString(PayloadString);
	
	// Set timeout
	Request->SetTimeout(Settings->RequestTimeoutSeconds);
	
	// Bind response handler
	Request->OnProcessRequestComplete().BindUObject(this, &UUnrealCopilotLLMManager::OnOpenAIResponse, OnComplete);
	
	// Update usage tracking
	UsageTracker.UpdateUsage();
	
	// Log request if enabled
	if (Settings->bEnableAPILogging)
	{
		LogAPIInteraction(PayloadString, TEXT(""), false);
	}
	
	// Store request reference and send
	CurrentRequest = Request;
	if (!Request->ProcessRequest())
	{
		FCodeGenerationResult ErrorResult;
		ErrorResult.bSuccess = false;
		ErrorResult.ErrorMessage = TEXT("Failed to send HTTP request");
		SetGenerationState(ECodeGenerationState::Error);
		if (OnComplete.IsBound())
		{
			OnComplete.Execute(ErrorResult);
		}
		CurrentRequest.Reset();
	}
}

void UUnrealCopilotLLMManager::OnOpenAIResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnCodeGenerationComplete OnComplete)
{
	CurrentRequest.Reset();
	
	FCodeGenerationResult Result;
	Result.GenerationTimeSeconds = FPlatformTime::Seconds() - GenerationStartTime;
	
	if (bWasSuccessful && Response.IsValid())
	{
		Result.ResponseCode = Response->GetResponseCode();
		Result.RawResponse = Response->GetContentAsString();
		
		// Log response if enabled
		UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
		if (Settings->bEnableAPILogging)
		{
			LogAPIInteraction(TEXT(""), Result.RawResponse, Response->GetResponseCode() == 200);
		}
		
		if (Response->GetResponseCode() == 200)
		{
			ParseOpenAIResponse(Result.RawResponse, Result);
			
			if (Result.bSuccess)
			{
				// Validate generated code
				FString ValidationError;
				if (ValidateGeneratedCode(Result.GeneratedCode, ValidationError))
				{
					SetGenerationState(ECodeGenerationState::Completed);
				}
				else
				{
					Result.bSuccess = false;
					Result.ErrorMessage = ValidationError;
					SetGenerationState(ECodeGenerationState::Error);
				}
			}
			else
			{
				SetGenerationState(ECodeGenerationState::Error);
			}
		}
		else
		{
			Result.bSuccess = false;
			Result.ErrorMessage = FString::Printf(TEXT("HTTP Error %d: %s"), 
				Response->GetResponseCode(), *Response->GetContentAsString());
			SetGenerationState(ECodeGenerationState::Error);
		}
	}
	else
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("HTTP request failed or timed out");
		SetGenerationState(ECodeGenerationState::Error);
	}
	
	// Execute completion delegate
	if (OnComplete.IsBound())
	{
		OnComplete.Execute(Result);
	}
	
	// Execute multicast delegates
	OnCodeGenerationComplete.Broadcast(Result);
	
	// Clear active delegates
	ActiveDelegates.Empty();
}

void UUnrealCopilotLLMManager::ParseOpenAIResponse(const FString& ResponseJSON, FCodeGenerationResult& OutResult)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseJSON);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// Check for error
		if (JsonObject->HasField(TEXT("error")))
		{
			TSharedPtr<FJsonObject> ErrorObject = JsonObject->GetObjectField(TEXT("error"));
			OutResult.bSuccess = false;
			OutResult.ErrorMessage = ErrorObject->GetStringField(TEXT("message"));
			return;
		}
		
		// Get choices array
		const TArray<TSharedPtr<FJsonValue>>* ChoicesArray;
		if (JsonObject->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
		{
			TSharedPtr<FJsonObject> FirstChoice = (*ChoicesArray)[0]->AsObject();
			if (FirstChoice.IsValid())
			{
				TSharedPtr<FJsonObject> Message = FirstChoice->GetObjectField(TEXT("message"));
				if (Message.IsValid())
				{
					FString Content = Message->GetStringField(TEXT("content"));
					
					// Extract Python code from the response
					OutResult.GeneratedCode = PromptProcessor->ExtractPythonCode(Content);
					OutResult.bSuccess = !OutResult.GeneratedCode.IsEmpty();
					
					if (!OutResult.bSuccess)
					{
						OutResult.ErrorMessage = TEXT("No Python code found in LLM response");
					}
				}
			}
		}
		
		// Get usage information if available
		if (JsonObject->HasField(TEXT("usage")))
		{
			TSharedPtr<FJsonObject> UsageObject = JsonObject->GetObjectField(TEXT("usage"));
			OutResult.TokensUsed = UsageObject->GetIntegerField(TEXT("total_tokens"));
		}
	}
	else
	{
		OutResult.bSuccess = false;
		OutResult.ErrorMessage = TEXT("Failed to parse JSON response from OpenAI");
	}
}

FString UUnrealCopilotLLMManager::BuildSystemPrompt() const
{
	if (PromptProcessor)
	{
		FPromptContext Context = PromptProcessor->GatherCurrentContext();
		return PromptProcessor->BuildSystemPrompt(Context);
	}
	
	return TEXT("You are an AI assistant for Unreal Engine Python scripting.");
}

void UUnrealCopilotLLMManager::SetGenerationState(ECodeGenerationState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnGenerationStateChanged.Broadcast(NewState);
	}
}

bool UUnrealCopilotLLMManager::CheckRateLimits()
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	return UsageTracker.CanMakeRequest(Settings->MaxRequestsPerMinute);
}

void UUnrealCopilotLLMManager::LogAPIInteraction(const FString& Request, const FString& Response, bool bSuccess)
{
	FString LogEntry = FString::Printf(TEXT("[%s] LLM API %s"), 
		*FDateTime::Now().ToString(), 
		bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	
	if (!Request.IsEmpty())
	{
		LogEntry += FString::Printf(TEXT("\nRequest: %s"), *Request.Left(500)); // Limit log size
	}
	
	if (!Response.IsEmpty())
	{
		LogEntry += FString::Printf(TEXT("\nResponse: %s"), *Response.Left(500)); // Limit log size
	}
	
	UE_LOG(LogUnrealCopilotLLM, Log, TEXT("%s"), *LogEntry);
}