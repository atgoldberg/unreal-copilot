# UnrealCopilot Plugin - Getting Started Guide

**Version**: 1.0  
**Compatible with**: Unreal Engine 5.6+  
**Last Updated**: December 2024

## Overview

UnrealCopilot is an AI-powered Python assistant plugin for Unreal Engine that enables technical artists to generate and execute Python code through natural language prompts. The plugin provides a seamless interface between human intent and Unreal Engine automation.

### Key Features

- **Dual-Mode Operation**: Switch between AI-powered prompt mode and direct Python coding
- **OpenAI Integration**: Support for GPT-4, GPT-4 Turbo, and GPT-3.5 Turbo models
- **Context-Aware Code Generation**: Automatically includes project and level information
- **Safety Features**: Code validation, user confirmation, and execution sandboxing
- **Rich UI Experience**: Integrated editor panel with code preview and execution

## Installation

### Prerequisites

- Unreal Engine 5.6 or higher
- OpenAI API key (for AI features)
- Python Script Plugin enabled (usually enabled by default)

### Installation Steps

1. **Download the Plugin**
   - Clone or download the UnrealCopilot plugin
   - Place it in your project's `Plugins/` directory

2. **Enable the Plugin**
   - Open your Unreal project
   - Go to `Edit ? Plugins`
   - Search for "UnrealCopilot"
   - Check the "Enabled" checkbox
   - Restart the Unreal Editor when prompted

3. **Verify Installation**
   - After restart, look for `Tools ? UnrealCopilot Panel` in the menu bar
   - A toolbar button should also appear in the main toolbar

## First-Time Setup

### Configure OpenAI API Key

1. **Access Plugin Settings**
   - Go to `Edit ? Project Settings`
   - Navigate to `Plugins ? UnrealCopilot Settings`

2. **Set Your API Key**
   - Enter your OpenAI API key in the "OpenAI API Key" field
   - Select your preferred model (GPT-4 Turbo recommended)
   - Configure other settings as needed:
     - **Max Tokens**: 2000 (default, adjust based on needs)
     - **Temperature**: 0.7 (balance between creativity and consistency)
     - **Request Timeout**: 30 seconds
     - **Max Requests Per Minute**: 20 (adjust based on your API limits)

3. **Security Settings**
   - **Enable Code Safety Validation**: Recommended (enabled by default)
   - **Require User Confirmation**: Recommended for AI-generated code
   - **Blocked Operations**: Default list includes file system and network operations

### Open the UnrealCopilot Panel

1. **Access the Panel**
   - Method 1: `Tools ? UnrealCopilot Panel`
   - Method 2: Click the UnrealCopilot toolbar button
   - Method 3: Use the keyboard shortcut (if configured)

2. **Panel Layout**
   - **Title Bar**: Shows current mode and status
   - **Mode Toggle**: Switch between "Ask AI" and "Write Code" modes
   - **Input Area**: Enter prompts or Python code
   - **Preview Area**: Review AI-generated code (Prompt mode only)
   - **Output Area**: View execution results and error messages

## Using UnrealCopilot

### Ask AI Mode (Natural Language)

This mode allows you to describe what you want to accomplish in natural language.

#### Getting Started Examples

1. **Simple Asset Query**
   ```
   List all static meshes in the project
   ```

2. **Level Editing**
   ```
   Add 5 cubes to the level in a circle pattern
   ```

3. **Material Creation**
   ```
   Create a material with a scrolling texture that moves upward
   ```

4. **Actor Manipulation**
   ```
   Move all selected actors up by 100 units
   ```

#### Best Practices for Prompts

- **Be Specific**: Include details about what you want to accomplish
- **Mention Context**: Reference specific assets, levels, or selections when relevant
- **Use Unreal Terminology**: Mention "actors", "components", "materials", etc.
- **Specify Quantities**: Include numbers, dimensions, or counts when needed

#### AI Workflow

1. **Enter Your Prompt**: Type what you want to accomplish
2. **Generate Code**: Press Ctrl+Enter or click "Generate Code"
3. **Review Code**: Check the generated Python code in the preview area
4. **Edit if Needed**: Modify the code before execution
5. **Execute**: Click "Execute Generated Code" to run it
6. **Review Results**: Check the output area for results or errors

### Write Code Mode (Direct Python)

This mode allows you to write Python code directly using the Unreal Engine Python API.

#### Common Unreal Python Patterns

1. **Asset Operations**
   ```python
   import unreal
   
   # Get all static meshes
   asset_library = unreal.EditorAssetLibrary()
   assets = asset_library.list_assets("/Game", recursive=True, include_folder=False)
   static_meshes = [asset for asset in assets if asset_library.get_asset_class(asset) == unreal.StaticMesh]
   
   for mesh_path in static_meshes:
       print(f"Found static mesh: {mesh_path}")
   ```

2. **Level Editing**
   ```python
   import unreal
   
   # Create a cube in the level
   actor_class = unreal.StaticMeshActor
   location = unreal.Vector(0, 0, 100)
   rotation = unreal.Rotator(0, 0, 0)
   
   actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, location, rotation)
   print(f"Created actor: {actor.get_name()}")
   ```

3. **Working with Selection**
   ```python
   import unreal
   
   # Get selected actors
   selected_actors = unreal.EditorLevelLibrary.get_selected_level_actors()
   print(f"Selected {len(selected_actors)} actors:")
   
   for actor in selected_actors:
       print(f"  - {actor.get_name()} at {actor.get_actor_location()}")
   ```

#### Direct Code Workflow

1. **Enter Python Code**: Type or paste your Python code
2. **Execute**: Press Ctrl+Enter or click "Execute Python Code"
3. **Review Results**: Check output area for results or error messages
4. **Use History**: Press Up/Down arrows to navigate previous commands

## Advanced Features

### Context Injection

UnrealCopilot automatically gathers context about your project when generating AI responses:

- **Current Project**: Project name and basic information
- **Active Level**: Currently loaded level name
- **Selected Assets**: Assets selected in the Content Browser
- **Selected Actors**: Actors selected in the level viewport

### Conversation History

The AI maintains context from previous interactions within the same session:
- Previous prompts and responses are remembered
- Follow-up questions can reference earlier requests
- History is cleared when switching modes or restarting

### Safety Features

#### Code Validation
- Automatic detection of potentially dangerous operations
- Blocked operations include file system access, network calls, and code evaluation
- User confirmation required for AI-generated code (configurable)

#### Rate Limiting
- Configurable request limits to prevent API quota exhaustion
- Usage tracking displays current API consumption
- Automatic throttling when limits are approached

### Keyboard Shortcuts

- **Ctrl+Enter**: Execute code or generate code (depending on mode)
- **Up Arrow**: Navigate to previous command in history
- **Down Arrow**: Navigate to next command in history
- **Mode Toggle**: Click the toggle button to switch between modes

## Troubleshooting

### Common Issues

#### "LLM manager not available"
- **Cause**: Plugin not properly initialized or OpenAI API key not set
- **Solution**: Check plugin settings and ensure API key is configured

#### "Generation failed" or API errors
- **Cause**: Invalid API key, network issues, or rate limiting
- **Solutions**: 
  - Verify API key is correct and active
  - Check internet connection
  - Reduce request frequency
  - Check OpenAI service status

#### "No generated code to execute"
- **Cause**: Trying to execute in Prompt mode without generating code first
- **Solution**: Use "Generate Code" before "Execute Generated Code"

#### Python execution errors
- **Cause**: Invalid Python syntax or Unreal API usage
- **Solutions**:
  - Check the error message in output area
  - Verify Python syntax
  - Consult Unreal Python API documentation
  - Try simpler operations first

### Getting Help

1. **Check Output Area**: Error messages provide specific details
2. **Review Generated Code**: In Prompt mode, check if the AI understood your request correctly
3. **Try Simpler Prompts**: Start with basic operations and build complexity
4. **Use Direct Mode**: Test Python code directly if AI generation fails
5. **Check Logs**: Editor logs may contain additional error information

### Performance Tips

1. **Optimize Prompts**: Specific, clear prompts generate better code
2. **Manage History**: Clear conversation history for unrelated tasks
3. **Monitor Usage**: Keep track of API requests to manage costs
4. **Use Appropriate Models**: GPT-3.5 Turbo is faster and cheaper for simple tasks

## API Reference

### Settings Configuration

All plugin settings are available in `Project Settings ? Plugins ? UnrealCopilot Settings`:

| Setting | Description | Default | Range |
|---------|-------------|---------|--------|
| Current Provider | LLM service to use | OpenAI | OpenAI only (currently) |
| OpenAI Model | Specific model version | GPT-4 Turbo | GPT-4, GPT-4 Turbo, GPT-3.5 Turbo |
| Max Tokens | Maximum response length | 2000 | 100-4000 |
| Temperature | Response creativity | 0.7 | 0.0-1.0 |
| Request Timeout | API request timeout | 30s | 5-300s |
| Max Requests Per Minute | Rate limiting | 20 | 1-100 |
| Enable Code Safety | Validate generated code | True | True/False |
| Require User Confirmation | Confirm before execution | True | True/False |

### Useful Unreal Python Modules

When writing direct Python code, these modules are commonly used:

- `unreal.EditorAssetLibrary` - Asset management operations
- `unreal.EditorLevelLibrary` - Level and actor operations  
- `unreal.MaterialEditingLibrary` - Material creation and editing
- `unreal.StaticMeshEditorSubsystem` - Static mesh operations
- `unreal.EditorActorSubsystem` - Actor manipulation

## Examples Gallery

### Asset Management

```python
# Find all textures larger than 1024x1024
import unreal

asset_lib = unreal.EditorAssetLibrary()
texture_paths = asset_lib.list_assets("/Game", recursive=True)

large_textures = []
for path in texture_paths:
    if asset_lib.get_asset_class(path) == unreal.Texture2D:
        texture = asset_lib.load_asset(path)
        if texture and (texture.get_surface_width() > 1024 or texture.get_surface_height() > 1024):
            large_textures.append(path)
            
print(f"Found {len(large_textures)} large textures:")
for texture in large_textures:
    print(f"  - {texture}")
```

### Level Organization

```python
# Organize actors by type into folders
import unreal

actors = unreal.EditorLevelLibrary.get_all_level_actors()
actor_types = {}

for actor in actors:
    actor_type = type(actor).__name__
    if actor_type not in actor_types:
        actor_types[actor_type] = []
    actor_types[actor_type].append(actor)

for actor_type, actor_list in actor_types.items():
    folder_name = f"Organized/{actor_type}"
    for actor in actor_list:
        actor.set_folder_path(folder_name)
    print(f"Moved {len(actor_list)} {actor_type} actors to {folder_name}")
```

### Material Automation

```python
# Create a simple material with base color
import unreal

# Create material
material_name = "MyGeneratedMaterial"
package_path = "/Game/Materials/"
material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    material_name,
    package_path,
    unreal.Material,
    unreal.MaterialFactoryNew()
)

# Add base color input
material_lib = unreal.MaterialEditingLibrary()
base_color = material_lib.create_material_expression(
    material, 
    unreal.MaterialExpressionConstant3Vector
)
base_color.set_editor_property("constant", unreal.LinearColor(1.0, 0.0, 0.0, 1.0))

# Connect to material output
material_lib.connect_material_property(
    base_color, 
    "output", 
    unreal.MaterialProperty.MP_BASE_COLOR
)

# Save the material
unreal.EditorAssetLibrary.save_asset(f"{package_path}{material_name}")
print(f"Created material: {material_name}")
```

## Best Practices

### For AI Prompts
1. **Be descriptive but concise**
2. **Include context about your current selection or focus**
3. **Specify quantities, dimensions, and parameters**
4. **Use Unreal Engine terminology**
5. **Break complex tasks into smaller steps**

### For Direct Python Code  
1. **Import required modules at the start**
2. **Use error handling for asset operations**
3. **Print progress for long operations**
4. **Comment your code for future reference**
5. **Test with small datasets first**

### General Workflow
1. **Start simple and build complexity**
2. **Review generated code before execution**
3. **Save successful code snippets for reuse**
4. **Monitor API usage to manage costs**
5. **Keep the output panel visible for feedback**

## Support and Resources

- **Plugin Documentation**: Check the `Documentation/` folder in the plugin
- **Unreal Python API**: [Official Unreal Engine Python API Documentation](https://docs.unrealengine.com/5.6/en-US/PythonAPI/)
- **OpenAI API**: [OpenAI API Documentation](https://platform.openai.com/docs)
- **Community**: Share examples and get help from other technical artists

---

**Happy Automating with UnrealCopilot!** ??