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
#include "EditorStyleSet.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

#define LOCTEXT_NAMESPACE "SUnrealCopilotWidget"

void SUnrealCopilotWidget::Construct(const FArguments& InArgs)
{
	// Initialize default texts
	PromptText = LOCTEXT("DefaultPrompt", "# Enter your Python code here\n# Example:\nprint('Hello from UnrealCopilot!')");
	OutputText = LOCTEXT("DefaultOutput", "Output will appear here after execution...");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 8.0f, 8.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "UnrealCopilot - Python Execution Panel"))
				.Font(FEditorStyle::GetFontStyle("PropertyWindow.BoldFont"))
				.Justification(ETextJustify::Center)
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

					// Prompt label
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PromptLabel", "Python Code:"))
						.Font(FEditorStyle::GetFontStyle("PropertyWindow.BoldFont"))
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
							.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
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

					// Execute button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.Text(LOCTEXT("ExecuteButton", "Execute Python Code"))
							.OnClicked(this, &SUnrealCopilotWidget::OnExecuteClicked)
							.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
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
							.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
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
}

FReply SUnrealCopilotWidget::OnExecuteClicked()
{
	FString PythonCode = PromptText.ToString();
	FString ExecutionResult;

	// Call the Python execution function from Task 001
	bool bSuccess = UUnrealCopilotBlueprintLibrary::ExecutePythonString(PythonCode, ExecutionResult);

	// Update output with results
	if (bSuccess)
	{
		SetOutputText(FString::Printf(TEXT("[SUCCESS] %s"), *ExecutionResult));
	}
	else
	{
		SetOutputText(FString::Printf(TEXT("[ERROR] %s"), *ExecutionResult));
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

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION