// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SUnrealCopilotWidget.h"
#include "UnrealCopilot.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Images/SThrobber.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/MessageDialog.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

#define LOCTEXT_NAMESPACE "SUnrealCopilotWidget"

void SUnrealCopilotWidget::Construct(const FArguments& InArgs)
{
	// Initialize state
	CurrentExecutionState = EPythonExecutionState::Idle;
	CurrentGenerationState = ECodeGenerationState::Idle;
	CurrentUIMode = EUnrealCopilotUIMode::PromptMode; // Default to Prompt Mode
	HistoryIndex = -1;
	LastExecutionTime = 0.0f;

	// Get execution manager reference
	ExecutionManager = UUnrealCopilotBlueprintLibrary::GetExecutionManager();
	if (ExecutionManager.IsValid())
	{
		// Bind to execution manager delegates using lambda functions and store handles
		ExecutionStateChangedHandle = ExecutionManager->OnExecutionStateChanged.AddLambda([this](EPythonExecutionState NewState)
		{
			OnExecutionStateChanged(NewState);
		});

		ExecutionCompletedHandle = ExecutionManager->OnExecutionCompleted.AddLambda([this](const FPythonExecutionResult& Result)
		{
			OnExecutionCompleted(Result);
		});
	}

	// Get LLM manager reference
	LLMManager = UUnrealCopilotLLMManager::GetInstance();
	if (LLMManager.IsValid())
	{
		// Bind to LLM manager delegates using lambda functions
		GenerationStateChangedHandle = LLMManager->OnGenerationStateChanged.AddLambda([this](ECodeGenerationState NewState)
		{
			OnGenerationStateChanged(NewState);
		});
		
		GenerationCompletedHandle = LLMManager->OnCodeGenerationComplete.AddLambda([this](const FCodeGenerationResult& Result)
		{
			OnCodeGenerationComplete(Result);
		});
	}

	// Build model selection options
	BuildModelSelectionOptions();

	// Load auto-saved prompt text
	LoadAutoSavedPromptText();

	// Initialize default output text if prompt is empty
	if (PromptText.IsEmpty())
	{
		PromptText = LOCTEXT("DefaultPromptMode", "Ask the AI to help with Unreal Engine tasks!\n\nExample prompts:\n- Create a material with a scrolling texture\n- Add 10 cubes to the level in a circle\n- List all static meshes in the project\n\nPress Ctrl+Enter to generate code");
	}
	
	OutputText = LOCTEXT("DefaultOutput", "Output will appear here after execution...\nUse Up/Down arrow keys to navigate command history.");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Title and Mode Selection
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 8.0f, 8.0f, 4.0f)
			[
				SNew(SHorizontalBox)

				// Title
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Title", "UnrealCopilot - AI-Powered Python Assistant"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
					.Justification(ETextJustify::Left)
				]

				// Mode Toggle
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(16.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ModeLabel", "Mode:"))
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(8.0f, 0.0f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(ModeToggleCheckBox, SCheckBox)
						.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
						.IsChecked(CurrentUIMode == EUnrealCopilotUIMode::PromptMode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
						.OnCheckStateChanged(this, &SUnrealCopilotWidget::OnModeChanged)
						[
							SNew(STextBlock)
							.Text(CurrentUIMode == EUnrealCopilotUIMode::PromptMode ? LOCTEXT("PromptMode", "Ask AI") : LOCTEXT("CodeMode", "Write Code"))
							.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
						]
					]
				]
			]

			// Status Bar with Model Selection
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 0.0f, 8.0f, 4.0f)
			[
				SNew(SHorizontalBox)

				// Mode Description
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(ModeDescriptionTextBlock, STextBlock)
					.Text(this, &SUnrealCopilotWidget::GetModeDescriptionText)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
				]

				// Model Selection (Prompt Mode only)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					.Visibility(this, &SUnrealCopilotWidget::GetPromptModeVisibility)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ModelLabel", "Model:"))
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4.0f, 0.0f)
					[
						SAssignNew(ModelSelectionComboBox, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&ModelOptions)
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> Option) {
							return SNew(STextBlock).Text(FText::FromString(*Option));
						})
						.OnSelectionChanged(this, &SUnrealCopilotWidget::OnModelSelectionChanged)
						[
							SAssignNew(CurrentModelTextBlock, STextBlock)
							.Text(this, &SUnrealCopilotWidget::GetCurrentModelText)
						]
					]
				]

				// Execution Throbber
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 0.0f)
				[
					SAssignNew(ExecutionThrobber, SThrobber)
					.Visibility(this, &SUnrealCopilotWidget::GetThrobberVisibility)
				]

				// Status Text
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(StatusTextBlock, STextBlock)
					.Text(this, &SUnrealCopilotWidget::GetStatusText)
					.ColorAndOpacity(this, &SUnrealCopilotWidget::GetStatusColor)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]
			]

			// Execution Time Display
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 0.0f, 8.0f, 4.0f)
			[
				SAssignNew(ExecutionTimeTextBlock, STextBlock)
				.Text(this, &SUnrealCopilotWidget::GetExecutionTimeText)
				.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
				.Justification(ETextJustify::Right)
			]

			// Main content splitter
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(8.0f, 4.0f, 8.0f, 8.0f)
			[
				SNew(SSplitter)
				.Orientation(Orient_Vertical)

				// Prompt input section
				+ SSplitter::Slot()
				.Value(0.6f)
				[
					SNew(SVerticalBox)

					// Prompt label and shortcuts info
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() {
								return CurrentUIMode == EUnrealCopilotUIMode::PromptMode 
									? LOCTEXT("PromptLabelAI", "AI Prompt:")
									: LOCTEXT("PromptLabelCode", "Python Code:");
							})
							.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text_Lambda([this]() {
								return CurrentUIMode == EUnrealCopilotUIMode::PromptMode 
									? LOCTEXT("ShortcutsInfoPrompt", "Ctrl+Enter: Generate Code")
									: LOCTEXT("ShortcutsInfoCode", "Ctrl+Enter: Execute | Up/Down: History");
							})
							.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
						]
					]

					// Prompt text box
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SAssignNew(PromptTextBox, SMultiLineEditableTextBox)
							.Text(this, &SUnrealCopilotWidget::GetPromptText)
							.OnTextChanged(this, &SUnrealCopilotWidget::OnPromptTextChanged)
							.Font(FCoreStyle::GetDefaultFontStyle("Mono", 10))
							.BackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f))
							.ForegroundColor(FLinearColor::White)
							.AllowMultiLine(true)
							.IsReadOnly(false)
							.HScrollBar(
								SNew(SScrollBar)
								.Orientation(Orient_Horizontal)
							)
							.VScrollBar(
								SNew(SScrollBar)
								.Orientation(Orient_Vertical)
							)
						]
					]

					// Generated Code Preview (Prompt Mode only)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.MaxHeight(200.0f)
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(SVerticalBox)
						.Visibility(this, &SUnrealCopilotWidget::GetGeneratedCodePreviewVisibility)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 4.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("GeneratedCodeLabel", "Generated Code Preview:"))
							.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
						]

						+ SVerticalBox::Slot()
						.FillHeight(1)
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
							[
								SAssignNew(GeneratedCodePreviewBox, SMultiLineEditableTextBox)
								.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
								.BackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
								.ForegroundColor(FLinearColor(0.8f, 1.0f, 0.8f, 1.0f))
								.AllowMultiLine(true)
								.IsReadOnly(false) // Allow editing of generated code
								.HScrollBar(
									SNew(SScrollBar)
									.Orientation(Orient_Horizontal)
								)
								.VScrollBar(
									SNew(SScrollBar)
									.Orientation(Orient_Vertical)
								)
							]
						]
					]

					// Button row
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)

						// Generate Code button (Prompt Mode)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 8.0f, 0.0f)
						[
							SAssignNew(GenerateCodeButton, SButton)
							.Text(LOCTEXT("GenerateCodeButton", "Generate Code"))
							.OnClicked(this, &SUnrealCopilotWidget::OnGenerateCodeClicked)
							.ButtonStyle(FAppStyle::Get(), "FlatButton.Primary")
							.ContentPadding(FMargin(16.0f, 8.0f))
							.IsEnabled(this, &SUnrealCopilotWidget::IsGenerateCodeButtonEnabled)
							.Visibility(this, &SUnrealCopilotWidget::GetPromptModeVisibility)
						]

						// Execute button
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 8.0f, 0.0f)
						[
							SAssignNew(ExecuteButton, SButton)
							.Text_Lambda([this]() {
								return CurrentUIMode == EUnrealCopilotUIMode::PromptMode 
									? LOCTEXT("ExecuteGeneratedButton", "Execute Generated Code")
									: LOCTEXT("ExecuteButton", "Execute Python Code");
							})
							.OnClicked(this, &SUnrealCopilotWidget::OnExecuteClicked)
							.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
							.ContentPadding(FMargin(16.0f, 8.0f))
							.IsEnabled(this, &SUnrealCopilotWidget::IsExecuteButtonEnabled)
						]

						// Cancel button
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 8.0f, 0.0f)
						[
							SAssignNew(CancelButton, SButton)
							.Text(LOCTEXT("CancelButton", "Cancel"))
							.OnClicked(this, &SUnrealCopilotWidget::OnCancelExecutionClicked)
							.ButtonStyle(FAppStyle::Get(), "FlatButton.Danger")
							.ContentPadding(FMargin(16.0f, 8.0f))
							.Visibility(this, &SUnrealCopilotWidget::GetCancelButtonVisibility)
						]

						// Clear Output button
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(ClearOutputButton, SButton)
							.Text(LOCTEXT("ClearOutputButton", "Clear Output"))
							.OnClicked(this, &SUnrealCopilotWidget::OnClearOutputClicked)
							.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
							.ContentPadding(FMargin(16.0f, 8.0f))
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
					]
				]

				// Output section
				+ SSplitter::Slot()
				.Value(0.4f)
				[
					SNew(SVerticalBox)

					// Output label
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OutputLabel", "Execution Results:"))
						.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
					]

					// Output text box (read-only)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SAssignNew(OutputTextBox, SMultiLineEditableTextBox)
							.Text(this, &SUnrealCopilotWidget::GetOutputText)
							.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
							.BackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
							.ForegroundColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
							.AllowMultiLine(true)
							.IsReadOnly(true)
							.HScrollBar(
								SNew(SScrollBar)
								.Orientation(Orient_Horizontal)
							)
							.VScrollBar(
								SNew(SScrollBar)
								.Orientation(Orient_Vertical)
							)
						]
					]
				]
			]
		]
	];

	// Set up auto-save timer
	if (GEngine && GEngine->GetWorld())
	{
		GEngine->GetWorld()->GetTimerManager().SetTimer(
			AutoSaveTimerHandle,
			FTimerDelegate::CreateSP(this, &SUnrealCopilotWidget::AutoSavePromptText),
			5.0f, // Auto-save every 5 seconds
			true
		);
	}
}

SUnrealCopilotWidget::~SUnrealCopilotWidget()
{
	// Clean up timer
	if (GEngine && GEngine->GetWorld() && AutoSaveTimerHandle.IsValid())
	{
		GEngine->GetWorld()->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	// Auto-save on destruction
	AutoSavePromptText();

	// Unbind delegates using handles
	if (ExecutionManager.IsValid())
	{
		if (ExecutionStateChangedHandle.IsValid())
		{
			ExecutionManager->OnExecutionStateChanged.Remove(ExecutionStateChangedHandle);
		}
		if (ExecutionCompletedHandle.IsValid())
		{
			ExecutionManager->OnExecutionCompleted.Remove(ExecutionCompletedHandle);
		}
	}

	// Unbind LLM delegates
	if (LLMManager.IsValid())
	{
		if (GenerationStateChangedHandle.IsValid())
		{
			LLMManager->OnGenerationStateChanged.Remove(GenerationStateChangedHandle);
		}
		if (GenerationCompletedHandle.IsValid())
		{
			LLMManager->OnCodeGenerationComplete.Remove(GenerationCompletedHandle);
		}
	}
}

void SUnrealCopilotWidget::OnModeChanged(ECheckBoxState CheckState)
{
	EUnrealCopilotUIMode NewMode = (CheckState == ECheckBoxState::Checked) 
		? EUnrealCopilotUIMode::PromptMode 
		: EUnrealCopilotUIMode::CodeMode;

	if (NewMode != CurrentUIMode)
	{
		CurrentUIMode = NewMode;
		
		// Update prompt text based on mode
		if (CurrentUIMode == EUnrealCopilotUIMode::PromptMode)
		{
			PromptText = LOCTEXT("DefaultPromptMode", "Ask the AI to help with Unreal Engine tasks!\n\nExample prompts:\n- Create a material with a scrolling texture\n- Add 10 cubes to the level in a circle\n- List all static meshes in the project\n\nPress Ctrl+Enter to generate code");
		}
		else
		{
			PromptText = LOCTEXT("DefaultCodeMode", "# Enter your Python code here\n# Example:\nprint('Hello from UnrealCopilot!')\n# Use Ctrl+Enter to execute");
		}

		// Clear generated code preview when switching modes
		if (GeneratedCodePreviewBox.IsValid())
		{
			GeneratedCodePreviewBox->SetText(FText::GetEmpty());
		}
		LastGeneratedCode.Empty();
	}
}

void SUnrealCopilotWidget::OnModelSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		SelectedModel = SelectedItem;
		
		// Update settings based on selection
		UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
		if (*SelectedItem == TEXT("GPT-5"))
		{
			Settings->OpenAIModel = EOpenAIModel::GPT5;
		}
		else if (*SelectedItem == TEXT("GPT-4"))
		{
			Settings->OpenAIModel = EOpenAIModel::GPT4;
		}
		else if (*SelectedItem == TEXT("GPT-4 Turbo"))
		{
			Settings->OpenAIModel = EOpenAIModel::GPT4Turbo;
		}
		else if (*SelectedItem == TEXT("GPT-3.5 Turbo"))
		{
			Settings->OpenAIModel = EOpenAIModel::GPT35Turbo;
		}
		Settings->SaveConfig();
	}
}

void SUnrealCopilotWidget::OnGenerationStateChanged(ECodeGenerationState NewState)
{
	CurrentGenerationState = NewState;
	// UI will automatically update through bound visibility functions
}

void SUnrealCopilotWidget::OnCodeGenerationComplete(const FCodeGenerationResult& Result)
{
	if (Result.bSuccess)
	{
		// Display generated code in preview
		LastGeneratedCode = Result.GeneratedCode;
		if (GeneratedCodePreviewBox.IsValid())
		{
			GeneratedCodePreviewBox->SetText(FText::FromString(LastGeneratedCode));
		}

		// Update output with generation info
		SetOutputText(FString::Printf(TEXT("[CODE GENERATED] (%.2fs, %d tokens)\n\nGenerated code is ready for review and execution.\nYou can edit the code in the preview window before executing."), 
			Result.GenerationTimeSeconds, 
			Result.TokensUsed));

		// Check if user confirmation is required
		UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
		if (Settings->bRequireUserConfirmation)
		{
			ShowCodeConfirmationDialog(Result.GeneratedCode);
		}
	}
	else
	{
		SetOutputText(FString::Printf(TEXT("[GENERATION ERROR] %s\n\nGeneration time: %.2fs"), 
			*Result.ErrorMessage, 
			Result.GenerationTimeSeconds));
		
		// Clear preview
		if (GeneratedCodePreviewBox.IsValid())
		{
			GeneratedCodePreviewBox->SetText(FText::GetEmpty());
		}
		LastGeneratedCode.Empty();
	}
}

void SUnrealCopilotWidget::AutoSavePromptText()
{
	FString AutoSaveFilePath = FPaths::ProjectSavedDir() / TEXT("UnrealCopilot") / TEXT("AutoSavedPrompt.txt");
	FString PromptString = PromptText.ToString();
	
	// Only save if there's actual content
	if (!PromptString.IsEmpty() && PromptString != TEXT("# Enter your Python code here\n# Example:\nprint('Hello from UnrealCopilot!')\n# Use Ctrl+Enter to execute"))
	{
		FFileHelper::SaveStringToFile(PromptString, *AutoSaveFilePath);
	}
}

void SUnrealCopilotWidget::LoadAutoSavedPromptText()
{
	FString AutoSaveFilePath = FPaths::ProjectSavedDir() / TEXT("UnrealCopilot") / TEXT("AutoSavedPrompt.txt");
	FString LoadedText;
	
	if (FFileHelper::LoadFileToString(LoadedText, *AutoSaveFilePath) && !LoadedText.IsEmpty())
	{
		PromptText = FText::FromString(LoadedText);
	}
}

FText SUnrealCopilotWidget::GetExecutionTimeText() const
{
	if (LastExecutionTime > 0.0f)
	{
		return FText::FromString(FString::Printf(TEXT("Last execution: %.3f seconds"), LastExecutionTime));
	}
	return FText::GetEmpty();
}

FReply SUnrealCopilotWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle keyboard shortcuts
	if (InKeyEvent.GetModifierKeys().IsControlDown())
	{
		if (InKeyEvent.GetKey() == EKeys::Enter)
		{
			// Ctrl+Enter: Execute code or generate code depending on mode
			if (CurrentUIMode == EUnrealCopilotUIMode::PromptMode)
			{
				return OnGenerateCodeClicked();
			}
			else
			{
				return OnExecuteClicked();
			}
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::Up)
	{
		// Up arrow: Navigate to previous command in history
		NavigateHistory(true);
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Down)
	{
		// Down arrow: Navigate to next command in history
		NavigateHistory(false);
		return FReply::Handled();
	}

	return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SUnrealCopilotWidget::OnExecuteClicked()
{
	if (!ExecutionManager.IsValid())
	{
		SetOutputText(TEXT("[ERROR] Execution manager not available"));
		return FReply::Handled();
	}

	FString PythonCode;
	
	if (CurrentUIMode == EUnrealCopilotUIMode::PromptMode)
	{
		// In Prompt Mode, execute the generated code from preview
		if (GeneratedCodePreviewBox.IsValid())
		{
			PythonCode = GeneratedCodePreviewBox->GetText().ToString();
		}
		else
		{
			PythonCode = LastGeneratedCode;
		}

		if (PythonCode.IsEmpty())
		{
			SetOutputText(TEXT("[ERROR] No generated code to execute. Please generate code first."));
			return FReply::Handled();
		}
	}
	else
	{
		// In Code Mode, execute the code from the main text box
		PythonCode = PromptText.ToString();
	}

	// Execute using the enhanced execution manager
	FPythonExecutionResult Result = ExecutionManager->ExecutePythonCode(PythonCode, false);

	return FReply::Handled();
}

FReply SUnrealCopilotWidget::OnGenerateCodeClicked()
{
	if (!LLMManager.IsValid())
	{
		SetOutputText(TEXT("[ERROR] LLM manager not available. Please check plugin configuration."));
		return FReply::Handled();
	}

	FString Prompt = PromptText.ToString().TrimStartAndEnd();
	if (Prompt.IsEmpty())
	{
		SetOutputText(TEXT("[ERROR] Please enter a prompt describing what you want to accomplish."));
		return FReply::Handled();
	}

	// Create delegate for completion using lambda
	FOnCodeGenerationComplete CompletionDelegate;
	CompletionDelegate.BindLambda([this](const FCodeGenerationResult& Result)
	{
		OnCodeGenerationComplete(Result);
	});

	// Process the prompt
	LLMManager->ProcessNaturalLanguagePrompt(Prompt, CompletionDelegate);

	return FReply::Handled();
}

FReply SUnrealCopilotWidget::OnClearOutputClicked()
{
	// Show confirmation dialog
	EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		LOCTEXT("ClearOutputConfirmation", "Are you sure you want to clear the output? This action cannot be undone.")
	);

	if (Result == EAppReturnType::Yes)
	{
		SetOutputText(TEXT("Output cleared."));
	}

	return FReply::Handled();
}

FReply SUnrealCopilotWidget::OnCancelExecutionClicked()
{
	if (ExecutionManager.IsValid())
	{
		ExecutionManager->CancelExecution();
		SetOutputText(TEXT("[CANCELLED] Python execution was cancelled by user."));
	}

	return FReply::Handled();
}

FText SUnrealCopilotWidget::GetPromptText() const
{
	return PromptText;
}

void SUnrealCopilotWidget::OnPromptTextChanged(const FText& InText)
{
	PromptText = InText;
}

FText SUnrealCopilotWidget::GetOutputText() const
{
	return OutputText;
}

void SUnrealCopilotWidget::SetOutputText(const FString& InText)
{
	OutputText = FText::FromString(InText);
}

void SUnrealCopilotWidget::OnExecutionStateChanged(EPythonExecutionState NewState)
{
	CurrentExecutionState = NewState;
	UpdateExecutionStateUI();
}

void SUnrealCopilotWidget::OnExecutionCompleted(const FPythonExecutionResult& Result)
{
	// Update execution time
	LastExecutionTime = Result.ExecutionTimeSeconds;

	// Format and display result
	FString ResultText;
	if (Result.bSuccess)
	{
		ResultText = FString::Printf(TEXT("[SUCCESS] (%.3fs)\n%s"), 
			Result.ExecutionTimeSeconds, 
			*Result.Output);
	}
	else
	{
		ResultText = FString::Printf(TEXT("[ERROR] %s (%.3fs)\n%s"), 
			*UEnum::GetValueAsString(Result.ErrorType),
			Result.ExecutionTimeSeconds,
			*Result.ErrorMessage);

		if (!Result.StackTrace.IsEmpty())
		{
			ResultText += FString::Printf(TEXT("\n\nStack Trace:\n%s"), *Result.StackTrace);
		}

		if (Result.ErrorLineNumber > 0)
		{
			ResultText += FString::Printf(TEXT("\nError on line: %d"), Result.ErrorLineNumber);
		}
	}

	SetOutputText(ResultText);

	// Reset history navigation index
	HistoryIndex = -1;
}

void SUnrealCopilotWidget::NavigateHistory(bool bUp)
{
	if (!ExecutionManager.IsValid())
	{
		return;
	}

	const TArray<FPythonExecutionHistoryEntry>& History = ExecutionManager->GetExecutionHistory();
	if (History.Num() == 0)
	{
		return;
	}

	if (bUp)
	{
		// Navigate to previous (older) command
		if (HistoryIndex == -1)
		{
			HistoryIndex = History.Num() - 1;
		}
		else if (HistoryIndex > 0)
		{
			HistoryIndex--;
		}
	}
	else
	{
		// Navigate to next (newer) command
		if (HistoryIndex != -1 && HistoryIndex < History.Num() - 1)
		{
			HistoryIndex++;
		}
		else
		{
			// Reset to current text
			HistoryIndex = -1;
			return;
		}
	}

	// Update prompt text with history entry
	if (HistoryIndex >= 0 && HistoryIndex < History.Num())
	{
		PromptText = FText::FromString(History[HistoryIndex].Code);
	}
}

void SUnrealCopilotWidget::UpdateExecutionStateUI()
{
	// Force UI refresh - Slate will handle the visibility and text updates
	// through the bound functions
}

FText SUnrealCopilotWidget::GetStatusText() const
{
	// Prioritize generation state in Prompt Mode
	if (CurrentUIMode == EUnrealCopilotUIMode::PromptMode && CurrentGenerationState != ECodeGenerationState::Idle)
	{
		switch (CurrentGenerationState)
		{
		case ECodeGenerationState::Processing:
			return LOCTEXT("StatusGenerating", "Generating...");
		case ECodeGenerationState::Validating:
			return LOCTEXT("StatusValidatingCode", "Validating...");
		case ECodeGenerationState::Completed:
			return LOCTEXT("StatusGenerated", "Code Generated");
		case ECodeGenerationState::Error:
			return LOCTEXT("StatusGenerationError", "Generation Error");
		default:
			break;
		}
	}

	// Fall back to execution state
	switch (CurrentExecutionState)
	{
	case EPythonExecutionState::Idle:
		return LOCTEXT("StatusIdle", "Ready");
	case EPythonExecutionState::Validating:
		return LOCTEXT("StatusValidating", "Validating...");
	case EPythonExecutionState::Executing:
		return LOCTEXT("StatusExecuting", "Executing...");
	case EPythonExecutionState::Completed:
		return LOCTEXT("StatusCompleted", "Completed");
	case EPythonExecutionState::Error:
		return LOCTEXT("StatusError", "Error");
	case EPythonExecutionState::Timeout:
		return LOCTEXT("StatusTimeout", "Timeout");
	default:
		return LOCTEXT("StatusUnknown", "Unknown");
	}
}

FSlateColor SUnrealCopilotWidget::GetStatusColor() const
{
	switch (CurrentExecutionState)
	{
	case EPythonExecutionState::Idle:
		return FLinearColor::White;
	case EPythonExecutionState::Validating:
	case EPythonExecutionState::Executing:
		return FLinearColor::Yellow;
	case EPythonExecutionState::Completed:
		return FLinearColor::Green;
	case EPythonExecutionState::Error:
	case EPythonExecutionState::Timeout:
		return FLinearColor::Red;
	default:
		return FLinearColor::Gray;
	}
}

bool SUnrealCopilotWidget::IsExecuteButtonEnabled() const
{
	return CurrentExecutionState == EPythonExecutionState::Idle ||
		   CurrentExecutionState == EPythonExecutionState::Completed ||
		   CurrentExecutionState == EPythonExecutionState::Error;
}

bool SUnrealCopilotWidget::IsGenerateCodeButtonEnabled() const
{
	return CurrentGenerationState == ECodeGenerationState::Idle &&
		   CurrentExecutionState != EPythonExecutionState::Executing &&
		   CurrentUIMode == EUnrealCopilotUIMode::PromptMode;
}

EVisibility SUnrealCopilotWidget::GetThrobberVisibility() const
{
	bool bShowThrobber = (CurrentExecutionState == EPythonExecutionState::Validating ||
						  CurrentExecutionState == EPythonExecutionState::Executing) ||
						 (CurrentGenerationState == ECodeGenerationState::Processing ||
						  CurrentGenerationState == ECodeGenerationState::Validating);
	
	return bShowThrobber ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SUnrealCopilotWidget::GetCancelButtonVisibility() const
{
	return (CurrentExecutionState == EPythonExecutionState::Executing) ? 
			EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SUnrealCopilotWidget::GetPromptModeVisibility() const
{
	return CurrentUIMode == EUnrealCopilotUIMode::PromptMode ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SUnrealCopilotWidget::GetCodeModeVisibility() const
{
	return CurrentUIMode == EUnrealCopilotUIMode::CodeMode ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SUnrealCopilotWidget::GetGeneratedCodePreviewVisibility() const
{
	return (CurrentUIMode == EUnrealCopilotUIMode::PromptMode && !LastGeneratedCode.IsEmpty()) 
		? EVisibility::Visible : EVisibility::Collapsed;
}

FText SUnrealCopilotWidget::GetModeDescriptionText() const
{
	if (CurrentUIMode == EUnrealCopilotUIMode::PromptMode)
	{
		return LOCTEXT("PromptModeDescription", "Describe what you want to accomplish in natural language, and the AI will generate Python code for you.");
	}
	else
	{
		return LOCTEXT("CodeModeDescription", "Write Python code directly using the Unreal Engine Python API.");
	}
}

void SUnrealCopilotWidget::BuildModelSelectionOptions()
{
	ModelOptions.Empty();
	ModelOptions.Add(MakeShareable(new FString(TEXT("GPT-5"))));
	ModelOptions.Add(MakeShareable(new FString(TEXT("GPT-4 Turbo"))));
	ModelOptions.Add(MakeShareable(new FString(TEXT("GPT-4"))));
	ModelOptions.Add(MakeShareable(new FString(TEXT("GPT-3.5 Turbo"))));

	// Set initial selection based on settings
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	switch (Settings->OpenAIModel)
	{
	case EOpenAIModel::GPT5:
		SelectedModel = ModelOptions[0];
		break;
	case EOpenAIModel::GPT4Turbo:
		SelectedModel = ModelOptions[1];
		break;
	case EOpenAIModel::GPT4:
		SelectedModel = ModelOptions[2];
		break;
	case EOpenAIModel::GPT35Turbo:
		SelectedModel = ModelOptions[3];
		break;
	default:
		SelectedModel = ModelOptions[0]; // Default to GPT-5
		break;
	}
}

FText SUnrealCopilotWidget::GetCurrentModelText() const
{
	UUnrealCopilotSettings* Settings = UUnrealCopilotSettings::Get();
	switch (Settings->OpenAIModel)
	{
	case EOpenAIModel::GPT5:
		return LOCTEXT("GPT5Model", "GPT-5");
	case EOpenAIModel::GPT4:
		return LOCTEXT("GPT4Model", "GPT-4");
	case EOpenAIModel::GPT4Turbo:
		return LOCTEXT("GPT4TurboModel", "GPT-4 Turbo");
	case EOpenAIModel::GPT35Turbo:
		return LOCTEXT("GPT35TurboModel", "GPT-3.5 Turbo");
	default:
		return LOCTEXT("UnknownModel", "Unknown");
	}
}

void SUnrealCopilotWidget::ShowCodeConfirmationDialog(const FString& GeneratedCode)
{
	FText DialogText = FText::FromString(FString::Printf(TEXT("The AI has generated the following code:\n\n%s\n\nDo you want to execute this code?"), 
		*GeneratedCode.Left(500))); // Limit display length

	EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		DialogText,
		LOCTEXT("CodeConfirmationTitle", "Confirm Code Execution")
	);

	if (Result == EAppReturnType::Yes)
	{
		ExecuteGeneratedCode(GeneratedCode);
	}
}

void SUnrealCopilotWidget::ExecuteGeneratedCode(const FString& Code)
{
	if (ExecutionManager.IsValid())
	{
		// Execute the generated code
		FPythonExecutionResult Result = ExecutionManager->ExecutePythonCode(Code, false);
		// Result will be handled by the OnExecutionCompleted delegate
	}
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION