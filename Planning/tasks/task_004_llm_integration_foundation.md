# Task 004: LLM Integration Foundation

## Objective
Implement the foundational LLM integration system to enable natural language prompt-to-Python code generation within the UnrealCopilot plugin. This task establishes the core infrastructure for connecting to external LLM providers and processing natural language prompts to generate executable Python scripts for Unreal Engine automation.

## Requirements

### Core LLM Integration Infrastructure
- Create `UUnrealCopilotLLMManager` class to handle LLM provider connections and API communication
- Implement support for multiple LLM providers with extensible provider pattern:
  - OpenAI GPT API (primary implementation)  
  - GitHub Copilot API (future expansion)
  - Azure OpenAI Service (future expansion)
- Create configuration system for API keys, model selection, and provider-specific settings
- Implement secure API key storage using Unreal's settings system with encryption consideration
- Add prompt template system for technical artist workflows

### Natural Language Processing Pipeline
- Create prompt processing pipeline that converts natural language to Python code requests
- Implement context injection system that provides Unreal Engine project information to LLM
- Add Python code validation and safety checking for LLM-generated scripts
- Create prompt history and conversation context management
- Implement response parsing to extract Python code from LLM responses

### UI Integration for Prompt Workflow
- Modify existing `SUnrealCopilotWidget` to support natural language prompts alongside direct Python input
- Add toggle between "Prompt Mode" (natural language) and "Code Mode" (direct Python)
- Implement prompt input field with enhanced text editing capabilities
- Add "Generate Code" button that sends prompts to LLM and displays generated Python
- Create preview/review system for generated code before execution
- Add model selection dropdown in the UI

### Security and Validation
- Implement code safety analysis for LLM-generated Python scripts
- Add user confirmation dialog before executing AI-generated code
- Create blocklist for dangerous Python operations (file system access, network calls, etc.)
- Implement rate limiting and usage tracking for LLM API calls
- Add error handling for network failures, API quota limits, and invalid responses

### Files to Modify/Create
- **Create**: `Plugins/UnrealCopilot/Source/UnrealCopilot/Public/UnrealCopilotLLMManager.h`
  - LLM provider management and API communication
- **Create**: `Plugins/UnrealCopilot/Source/UnrealCopilot/Private/UnrealCopilotLLMManager.cpp`
  - Implementation of LLM integration with OpenAI API
- **Create**: `Plugins/UnrealCopilot/Source/UnrealCopilot/Public/UnrealCopilotPromptProcessor.h`
  - Prompt processing and context injection system
- **Create**: `Plugins/UnrealCopilot/Source/UnrealCopilot/Private/UnrealCopilotPromptProcessor.cpp`
  - Natural language processing pipeline implementation
- **Create**: `Plugins/UnrealCopilot/Source/UnrealCopilot/Public/UnrealCopilotSettings.h`
  - Plugin settings for API keys and configuration
- **Create**: `Plugins/UnrealCopilot/Source/UnrealCopilot/Private/UnrealCopilotSettings.cpp`
  - Settings implementation with secure storage
- **Modify**: `Plugins/UnrealCopilot/Source/UnrealCopilotEditor/Private/Widgets/SUnrealCopilotWidget.cpp`
  - Enhanced UI with prompt mode and LLM integration
- **Modify**: `Plugins/UnrealCopilot/Source/UnrealCopilotEditor/Public/Widgets/SUnrealCopilotWidget.h`
  - Updated widget interface for dual-mode operation
- **Modify**: `Plugins/UnrealCopilot/Source/UnrealCopilot/UnrealCopilot.Build.cs`
  - Add HTTP module dependency for API communication

## Implementation Notes

### LLM Provider Integration
- Use Unreal's `FHttpModule` for REST API communication with OpenAI endpoints
- Implement JSON request/response handling using Unreal's built-in JSON library
- Follow OpenAI API documentation for Chat Completions endpoint: `https://api.openai.com/v1/chat/completions`
- Use system prompts to establish Python/Unreal context for the LLM
- Implement proper error handling for HTTP requests, rate limits, and API errors

### Prompt Engineering Strategy
- Create system prompt template that explains Unreal Python API context
- Include information about available Unreal modules (unreal, typing, etc.)
- Provide examples of common technical art tasks (material creation, level editing)
- Implement user prompt preprocessing to enhance context and clarity
- Add safety instructions to prevent generation of dangerous or destructive code

### Security Considerations
- Store API keys using Unreal's `UDeveloperSettings` system with consideration for encryption
- Implement client-side validation of generated Python code patterns
- Create safeguard system that prevents execution of potentially dangerous operations
- Add user confirmation workflow before executing any AI-generated code
- Implement logging and audit trail for AI interactions and code generation

### UI/UX Design
- Create clean toggle between "Ask AI" and "Write Code" modes
- Design prompt input interface optimized for natural language (larger text area, word wrap)
- Implement "Generate & Review" workflow before code execution
- Add visual indicators for AI processing (loading states, progress feedback)
- Create clear separation between user prompts and AI-generated code

### Testing Considerations
- Test with various natural language prompts for common technical art tasks
- Validate API error handling with invalid keys, network failures, and rate limits
- Verify code safety validation catches dangerous operations
- Test UI responsiveness during LLM API calls (async processing)
- Validate prompt history and context management across sessions

## Acceptance Criteria

### LLM Integration Core
- [ ] Successfully connects to OpenAI API using configured API key
- [ ] Sends properly formatted requests to Chat Completions endpoint
- [ ] Parses LLM responses and extracts Python code blocks
- [ ] Handles API errors gracefully (invalid keys, rate limits, network failures)
- [ ] Implements proper async HTTP communication without blocking UI

### Natural Language Processing
- [ ] Accepts natural language prompts and converts them to LLM requests
- [ ] Injects appropriate Unreal Engine context into system prompts
- [ ] Generates Python code that targets Unreal Python API correctly
- [ ] Maintains conversation context for follow-up prompts and refinements
- [ ] Validates and sanitizes generated Python code before presentation

### User Interface Integration
- [ ] UI provides clear toggle between "Prompt Mode" and "Code Mode"
- [ ] Prompt input field supports multi-line natural language input
- [ ] "Generate Code" button triggers LLM processing with visual feedback
- [ ] Generated code appears in review area before execution
- [ ] User can edit generated code before execution
- [ ] Model selection dropdown allows choosing different OpenAI models (GPT-4, GPT-3.5-turbo)

### Security and Safety
- [ ] API keys are stored securely using Unreal settings system
- [ ] Generated Python code is analyzed for potentially dangerous operations
- [ ] User confirmation dialog appears before executing AI-generated code
- [ ] Rate limiting prevents excessive API usage
- [ ] All LLM interactions are logged for audit purposes

### Configuration and Settings
- [ ] Plugin settings panel allows API key configuration
- [ ] Model selection and provider settings are configurable
- [ ] Prompt templates can be customized for different workflow types
- [ ] Settings persist across Unreal Editor sessions
- [ ] Invalid configurations provide clear error messages

### Code Quality and Integration
- [ ] Plugin compiles successfully with new LLM integration modules
- [ ] No regressions in existing Python execution or UI functionality
- [ ] New features integrate seamlessly with Task 003 execution manager
- [ ] Error handling follows Unreal Engine patterns and logging standards
- [ ] Memory usage remains reasonable for typical AI interaction scenarios

### Testing and Validation
- [ ] Comprehensive test cases cover various prompt types and AI responses
- [ ] Manual validation confirms AI-generated code executes correctly in Unreal
- [ ] Network error scenarios are handled gracefully without crashes
- [ ] UI remains responsive during LLM processing operations
- [ ] Generated code follows Unreal Python API best practices

## Technical Specifications

### LLM Manager Implementation
```cpp
// Example structure for LLM integration
UCLASS()
class UNREALCOPILOT_API UUnrealCopilotLLMManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Unreal Copilot")
    void ProcessNaturalLanguagePrompt(const FString& Prompt, FOnCodeGenerationComplete OnComplete);

    UFUNCTION(BlueprintCallable, Category = "Unreal Copilot") 
    void SetAPIKey(const FString& APIKey);

    UFUNCTION(BlueprintCallable, Category = "Unreal Copilot")
    bool IsConfiguredForProvider(ELLMProvider Provider);

private:
    void SendOpenAIRequest(const FString& ProcessedPrompt, FOnCodeGenerationComplete OnComplete);
    void ParseOpenAIResponse(const FString& ResponseJSON, FOnCodeGenerationComplete OnComplete);
    FString BuildSystemPrompt() const;
    bool ValidateGeneratedCode(const FString& PythonCode) const;
};
```

### Prompt Processing System
```cpp
// Structure for prompt processing and context injection
USTRUCT()
struct FPromptContext
{
    GENERATED_BODY()

    FString ProjectName;
    FString CurrentLevel;
    TArray<FString> AvailableAssets;
    FString PreviousConversation;
    EUnrealCopilotWorkflowType WorkflowType;
};
```

### Security Configuration
- Implement settings validation for API keys and endpoints
- Create code pattern analysis for dangerous operations detection
- Add configurable safety rules that can be updated without recompilation
- Implement usage tracking and quota management for API calls

## Dependencies & References
- **HTTP Module**: For REST API communication with LLM providers
- **Json Module**: For request/response parsing and generation
- **DeveloperSettings**: For secure configuration and API key storage
- **OpenAI API Documentation**: Chat Completions endpoint specification
- **Unreal Python API**: For context injection and code generation targeting
- **Task 003 Integration**: Leverage existing execution manager and UI foundation

## Risk Considerations
- **API Costs**: LLM API usage can incur significant costs; implement usage tracking and rate limiting
- **Security**: API keys and generated code require careful validation and secure storage
- **Reliability**: Network dependencies introduce potential points of failure
- **Performance**: LLM API calls may have significant latency; implement proper async handling
- **Code Safety**: AI-generated code may contain errors or dangerous operations
- **Legal/Compliance**: Ensure OpenAI API usage complies with terms of service and data policies

## Success Metrics
- Users can enter natural language prompts and receive executable Python code
- Generated code successfully performs common technical art tasks in Unreal Engine
- AI interactions feel responsive and provide helpful feedback during processing
- Security measures prevent execution of dangerous or malformed code
- Plugin maintains stability and performance with LLM integration enabled
- Integration provides clear value over direct Python coding for complex workflows

## Future Expansion Opportunities
- Support for additional LLM providers (GitHub Copilot, Azure OpenAI, local models)
- Advanced prompt templates for specific technical art domains
- Multi-turn conversation support for iterative code refinement
- Integration with Unreal asset browser for context-aware asset manipulation
- Custom model fine-tuning for Unreal-specific code generation
- Collaborative features allowing sharing of prompts and generated workflows