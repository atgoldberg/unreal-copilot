// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SUnrealCopilotWidget.h"
#include "UnrealCopilot.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Images/SThrobber.h"
#include "EditorStyleSet.h"
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

	// Load auto-saved prompt text
	LoadAutoSavedPromptText();

	// Initialize default output text if prompt is empty
	if (PromptText.IsEmpty())
	{
		PromptText = LOCTEXT("DefaultPrompt", "# Enter your Python code here\n# Example:\nprint('Hello from UnrealCopilot!')\n# Use Ctrl+Enter to execute");
	}
	
	OutputText = LOCTEXT("DefaultOutput", "Output will appear here after execution...\nUse Up/Down arrow keys to navigate command history.");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Title and Status Bar
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
					.Text(LOCTEXT("Title", "UnrealCopilot - Enhanced Python Execution Panel"))
					.Font(FEditorStyle::GetFontStyle("PropertyWindow.BoldFont"))
					.Justification(ETextJustify::Left)
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
					.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]
			]

			// Execution Time Display
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 0.0f, 8.0f, 4.0f)
			[
				SAssignNew(ExecutionTimeTextBlock, STextBlock)
				.Text(this, &SUnrealCopilotWidget::GetExecutionTimeText)
				.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
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
							.Text(LOCTEXT("PromptLabel", "Python Code:"))
							.Font(FEditorStyle::GetFontStyle("PropertyWindow.BoldFont"))
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ShortcutsInfo", "Ctrl+Enter: Execute | Up/Down: History"))
							.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
						]
					]

					// Prompt text box
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
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

					// Button row
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)

						// Execute button
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 8.0f, 0.0f)
						[
							SAssignNew(ExecuteButton, SButton)
							.Text(LOCTEXT("ExecuteButton", "Execute Python Code"))
							.OnClicked(this, &SUnrealCopilotWidget::OnExecuteClicked)
							.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
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
							.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
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
							.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
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
						.Font(FEditorStyle::GetFontStyle("PropertyWindow.BoldFont"))
					]

					// Output text box (read-only)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
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
}

FReply SUnrealCopilotWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Handle keyboard shortcuts
	if (InKeyEvent.GetModifierKeys().IsControlDown())
	{
		if (InKeyEvent.GetKey() == EKeys::Enter)
		{
			// Ctrl+Enter: Execute code
			return OnExecuteClicked();
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

	FString PythonCode = PromptText.ToString();

	// Execute using the enhanced execution manager
	FPythonExecutionResult Result = ExecutionManager->ExecutePythonCode(PythonCode, false);

	// The result will be handled by the OnExecutionCompleted delegate
	// but we can also handle it immediately for synchronous execution

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

EVisibility SUnrealCopilotWidget::GetThrobberVisibility() const
{
	return (CurrentExecutionState == EPythonExecutionState::Validating ||
			CurrentExecutionState == EPythonExecutionState::Executing) ? 
			EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SUnrealCopilotWidget::GetCancelButtonVisibility() const
{
	return (CurrentExecutionState == EPythonExecutionState::Executing) ? 
			EVisibility::Visible : EVisibility::Collapsed;
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

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION