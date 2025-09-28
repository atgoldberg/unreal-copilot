// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

/**
 * Main widget for the UnrealCopilot plugin editor panel
 * Provides interface for entering prompts and displaying Python execution results
 */
class SUnrealCopilotWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnrealCopilotWidget)
		{
		}

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	/** Handle clicking the Execute button */
	FReply OnExecuteClicked();

	/** Get the current prompt text */
	FText GetPromptText() const;

	/** Set the prompt text */
	void OnPromptTextChanged(const FText& InText);

	/** Get the current output text */
	FText GetOutputText() const;

	/** Update the output text */
	void SetOutputText(const FString& InText);

private:
	/** Text box for entering Python code prompts */
	TSharedPtr<SMultiLineEditableTextBox> PromptTextBox;
	
	/** Text block for displaying execution results */
	TSharedPtr<SMultiLineEditableTextBox> OutputTextBox;

	/** Current prompt text */
	FText PromptText;

	/** Current output text */
	FText OutputText;
};