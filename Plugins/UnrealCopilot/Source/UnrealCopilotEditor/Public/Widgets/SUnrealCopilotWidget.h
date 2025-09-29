// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SButton.h"
#include "UnrealCopilotExecutionManager.h"
#include "Input/Reply.h"

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

	/** Destructor */
	virtual ~SUnrealCopilotWidget();

	// SWidget interface
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }

private:
	/** Handle clicking the Execute button */
	FReply OnExecuteClicked();

	/** Handle clicking the Clear Output button */
	FReply OnClearOutputClicked();

	/** Handle clicking the Cancel Execution button */
	FReply OnCancelExecutionClicked();

	/** Get the current prompt text */
	FText GetPromptText() const;

	/** Set the prompt text */
	void OnPromptTextChanged(const FText& InText);

	/** Get the current output text */
	FText GetOutputText() const;

	/** Update the output text */
	void SetOutputText(const FString& InText);

	/** Handle execution state changes */
	void OnExecutionStateChanged(EPythonExecutionState NewState);

	/** Handle execution completion */
	void OnExecutionCompleted(const FPythonExecutionResult& Result);

	/** Navigate history (Up = true for previous, false for next) */
	void NavigateHistory(bool bUp);

	/** Update UI based on current execution state */
	void UpdateExecutionStateUI();

	/** Get status text based on execution state */
	FText GetStatusText() const;

	/** Get status color based on execution state */
	FSlateColor GetStatusColor() const;

	/** Check if execute button should be enabled */
	bool IsExecuteButtonEnabled() const;

	/** Get visibility for execution throbber */
	EVisibility GetThrobberVisibility() const;

	/** Get visibility for cancel button */
	EVisibility GetCancelButtonVisibility() const;

	/** Auto-save prompt text to prevent loss */
	void AutoSavePromptText();

	/** Load auto-saved prompt text */
	void LoadAutoSavedPromptText();

	/** Get execution time text */
	FText GetExecutionTimeText() const;

private:
	/** Text box for entering Python code prompts */
	TSharedPtr<SMultiLineEditableTextBox> PromptTextBox;
	
	/** Text block for displaying execution results */
	TSharedPtr<SMultiLineEditableTextBox> OutputTextBox;

	/** Execute button */
	TSharedPtr<SButton> ExecuteButton;

	/** Clear output button */
	TSharedPtr<SButton> ClearOutputButton;

	/** Cancel execution button */
	TSharedPtr<SButton> CancelButton;

	/** Status text block */
	TSharedPtr<STextBlock> StatusTextBlock;

	/** Execution time text block */
	TSharedPtr<STextBlock> ExecutionTimeTextBlock;

	/** Throbber for showing execution progress */
	TSharedPtr<SThrobber> ExecutionThrobber;

	/** Current prompt text */
	FText PromptText;

	/** Current output text */
	FText OutputText;

	/** Reference to execution manager */
	TWeakObjectPtr<UUnrealCopilotExecutionManager> ExecutionManager;

	/** Current execution state */
	EPythonExecutionState CurrentExecutionState;

	/** History navigation index */
	int32 HistoryIndex;

	/** Last execution time */
	float LastExecutionTime;

	/** Timer handle for auto-save */
	FTimerHandle AutoSaveTimerHandle;

	/** Delegate handles for cleanup */
	FDelegateHandle ExecutionStateChangedHandle;
	FDelegateHandle ExecutionCompletedHandle;
};