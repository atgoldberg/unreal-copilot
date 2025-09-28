# Task 001: Implement Python Execution Bridge

## Objective
Create a minimal Python execution bridge that allows the plugin to run Python commands from within the Unreal Editor.

## Requirements
- Implement a function in the plugin that accepts a string of Python code and executes it in the embedded Unreal Python environment.  
- Ensure error handling captures exceptions and logs them in the Unreal Output Log.  
- Modify the plugin's primary module file.  

## Implementation Notes
- Use `unreal.PythonScriptLibrary.execute_python_command()` as the entry point.  
- Consider sandboxing or validating input later, but for now allow direct execution.  
- Place bridge code in `Source/<PluginName>/Private/` and expose via a BlueprintCallable function.  

## Acceptance Criteria
- Able to enter a Python command in editor console and see it execute.  
- Errors in Python code produce readable log messages.  
- Function documented in the Global Reference Doc as available for tasks.  
