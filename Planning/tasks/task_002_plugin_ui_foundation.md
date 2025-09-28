# Task 002: Plugin UI Foundation

## Objective
Create a basic editor UI panel for the UnrealCopilot plugin that provides an interface for entering prompts and displaying execution results. This establishes the foundation for user interaction with the LLM integration.

## Requirements
- Create a new Editor module for the plugin (UnrealCopilotEditor)
- Implement a dockable tab window that can be opened from the Tools menu
- Add a multi-line text input field for prompts
- Add an output/results display area for Python execution results
- Add an "Execute" button that connects to the existing Python execution bridge from Task 001
- Register the UI panel with Unreal's editor tab system

## Implementation Notes
- Use Slate UI framework for the editor panel
- Module structure should follow: `Source/UnrealCopilotEditor/` 
- Key classes to utilize:
  - `FTabManager` for tab registration
  - `SDockTab` for the dockable window
  - `SMultiLineEditableTextBox` for prompt input
  - `STextBlock` or `SMultiLineEditableTextBox` (read-only) for output
- Reference the existing `ExecutePythonString` function from Task 001
- Add the new module to the `.uplugin` file with `"Type": "Editor"`
- Update `UnrealCopilot.Build.cs` to include dependencies: `"UnrealEd"`, `"ToolMenus"`, `"EditorSubsystem"`

## Acceptance Criteria
- Plugin compiles successfully with the new Editor module
- "UnrealCopilot" menu item appears under Tools menu in the editor
- Clicking the menu item opens a dockable panel
- Panel contains:
  - Prompt input area (multi-line text box)
  - Output display area
  - Execute button
- Clicking Execute with Python code in the prompt field executes it via the Task 001 bridge
- Execution results/errors appear in the output display area
- Panel can be docked, undocked, and closed without crashes
- Panel state persists across editor sessions (remembers if it was open)