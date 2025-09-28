# Task 001: Implement Python Execution Bridge

## Objective
Create a Python execution bridge that allows the UnrealCopilot plugin to execute Python code strings within the Unreal Editor environment. This establishes the foundational capability for all future LLM-generated script execution.

## Requirements
- Add a static function `ExecutePythonString` to the main plugin module that accepts a string of Python code
- Function must execute the Python code using Unreal's embedded Python environment
- Capture and return execution results (success/failure and output)
- Log any Python errors to the Unreal Output Log without crashing the editor
- Expose the function as BlueprintCallable for testing purposes
- Add proper error handling for invalid/malformed Python code

## Implementation Notes
- Modify `UnrealCopilot.h` and `UnrealCopilot.cpp` in the plugin source
- Add dependency on `PythonScriptPlugin` module in `UnrealCopilot.Build.cs`
- Use `IPythonScriptPlugin` interface to access Python execution
- Key APIs to utilize:
  - `IPythonScriptPlugin::Get()` - Get the Python plugin instance
  - `ExecutePythonCommand()` or `ExecutePythonCommandEx()` for execution
  - `FOutputDevice` for capturing output
- Function signature should be:
  ```cpp
  UFUNCTION(BlueprintCallable, Category = "UnrealCopilot")
  static bool ExecutePythonString(const FString& PythonCode, FString& OutResult);
  ```
- Ensure the function is marked with `UNREALCOPILOT_API` for proper DLL export
- Consider using `GEngine->Exec()` as a fallback if PythonScriptPlugin is unavailable

## Acceptance Criteria
- Plugin compiles successfully with the new Python execution function
- Function can be called from C++ code within the plugin
- Function is visible and callable from Blueprint graphs
- Executing valid Python code (e.g., `print("Hello from Python")`) produces expected output in OutResult
- Executing invalid Python code (e.g., `prit("typo")`) returns false and captures error message in OutResult
- Python errors are logged to Output Log but do not crash the editor
- Function handles empty strings and null inputs gracefully
- Test with multi-line Python scripts to ensure proper execution