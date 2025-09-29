// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "UnrealCopilotExecutionManager.h"
#include "UnrealCopilotLLMManager.h"
#include "UnrealCopilotSettings.h"
#include "Input/Reply.h"

/**
 * Enumeration for UI operation modes
 */
UENUM()
enum class EUnrealCopilotUIMode : uint8
{
	PromptMode UMETA(DisplayName = "Ask AI"),
	CodeMode UMETA(DisplayName = "Write Code")
};

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
	/** Handle clicking the Execute button (Code Mode) */
	FReply OnExecuteClicked();

	/** Handle clicking the Generate Code button (Prompt Mode) */
	FReply OnGenerateCodeClicked();

	/** Handle clicking the Clear Output button */
	FReply OnClearOutputClicked();

	/** Handle clicking the Cancel Execution button */
	FReply OnCancelExecutionClicked();

	/** Handle mode toggle change */
	void OnModeChanged(ECheckBoxState CheckState);

	/** Handle model selection change */
	void OnModelSelectionChanged(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);

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

	/** Handle code generation state changes */
	void OnGenerationStateChanged(ECodeGenerationState NewState);

	/** Handle code generation completion */
	void OnCodeGenerationComplete(const FCodeGenerationResult& Result);

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

	/** Check if generate code button should be enabled */
	bool IsGenerateCodeButtonEnabled() const;

	/** Get visibility for execution throbber */
	EVisibility GetThrobberVisibility() const;

	/** Get visibility for cancel button */
	EVisibility GetCancelButtonVisibility() const;

	/** Get visibility for prompt mode UI elements */
	EVisibility GetPromptModeVisibility() const;

	/** Get visibility for code mode UI elements */
	EVisibility GetCodeModeVisibility() const;

	/** Get visibility for generated code preview */
	EVisibility GetGeneratedCodePreviewVisibility() const;

	/** Auto-save prompt text to prevent loss */
	void AutoSavePromptText();

	/** Load auto-saved prompt text */
	void LoadAutoSavedPromptText();

	/** Get execution time text */
	FText GetExecutionTimeText() const;

	/** Get current mode description text */
	FText GetModeDescriptionText() const;

	/** Build model selection options */
	void BuildModelSelectionOptions();

	/** Get current model name for display */
	FText GetCurrentModelText() const;

	/** Handle code review and confirmation for AI-generated code */
	void ShowCodeConfirmationDialog(const FString& GeneratedCode);

	/** Execute the generated code after user confirmation */
	void ExecuteGeneratedCode(const FString& Code);

private:
	/** Text box for entering Python code prompts */
	TSharedPtr<SMultiLineEditableTextBox> PromptTextBox;
	
	/** Text block for displaying execution results */
	TSharedPtr<SMultiLineEditableTextBox> OutputTextBox;

	/** Text box for displaying generated code preview */
	TSharedPtr<SMultiLineEditableTextBox> GeneratedCodePreviewBox;

	/** Execute button (Code Mode) */
	TSharedPtr<SButton> ExecuteButton;

	/** Generate Code button (Prompt Mode) */
	TSharedPtr<SButton> GenerateCodeButton;

	/** Clear output button */
	TSharedPtr<SButton> ClearOutputButton;

	/** Cancel execution button */
	TSharedPtr<SButton> CancelButton;

	/** Mode toggle checkbox */
	TSharedPtr<SCheckBox> ModeToggleCheckBox;

	/** Model selection combo box */
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ModelSelectionComboBox;

	/** Status text block */
	TSharedPtr<STextBlock> StatusTextBlock;

	/** Execution time text block */
	TSharedPtr<STextBlock> ExecutionTimeTextBlock;

	/** Mode description text block */
	TSharedPtr<STextBlock> ModeDescriptionTextBlock;

	/** Current model text block */
	TSharedPtr<STextBlock> CurrentModelTextBlock;

	/** Throbber for showing execution progress */
	TSharedPtr<SThrobber> ExecutionThrobber;

	/** Current prompt text */
	FText PromptText;

	/** Current output text */
	FText OutputText;

	/** Current UI mode */
	EUnrealCopilotUIMode CurrentUIMode;

	/** Reference to execution manager */
	TWeakObjectPtr<UUnrealCopilotExecutionManager> ExecutionManager;

	/** Reference to LLM manager */
	TWeakObjectPtr<UUnrealCopilotLLMManager> LLMManager;

	/** Current execution state */
	EPythonExecutionState CurrentExecutionState;

	/** Current generation state */
	ECodeGenerationState CurrentGenerationState;

	/** History navigation index */
	int32 HistoryIndex;

	/** Last execution time */
	float LastExecutionTime;

	/** Timer handle for auto-save */
	FTimerHandle AutoSaveTimerHandle;

	/** Delegate handles for cleanup */
	FDelegateHandle ExecutionStateChangedHandle;
	FDelegateHandle ExecutionCompletedHandle;
	FDelegateHandle GenerationStateChangedHandle;
	FDelegateHandle GenerationCompletedHandle;

	/** Model selection options */
	TArray<TSharedPtr<FString>> ModelOptions;

	/** Currently selected model */
	TSharedPtr<FString> SelectedModel;

	/** Last generated code for review */
	FString LastGeneratedCode;
};