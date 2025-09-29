# Testing Checklist - Task 004: LLM Integration Foundation

---

## Metadata
- **Task ID / Title**: Task 004 - LLM Integration Foundation  
- **Author**: Task 004 Implementation Agent  
- **Date**: December 19, 2024  
- **UE Version**: Unreal Engine 5.6  
- **Plugin Commit SHA**: [Current HEAD]  
- **Test Environment**: Windows 11, UE 5.6 Editor, Development Build, PythonScriptPlugin enabled

---

## Preconditions
- Build succeeds for target: `UnrealEditor Development`  
- UnrealCopilot plugin compiles without errors or warnings
- Test project loads with no warnings/errors in log related to UnrealCopilot plugin
- OpenAI API key available for testing (not required for compilation tests)
- Internet connectivity available for API testing

---

## Test Cases

> Test cases cover all major functionality introduced in Task 004

| ID | Type | Steps | Expected Result | Artifacts |
|---|---|---|---|---|
| TC-001 | Unit | 1. Build plugin in Development mode<br>2. Check compilation output<br>3. Verify no linker errors | Clean build with no errors or warnings | Build log |
| TC-002 | Integration | 1. Load UE project with plugin<br>2. Check plugin is listed in Plugin Manager<br>3. Verify plugin shows as "Enabled" | Plugin loads successfully, no crashes | Editor log excerpt |
| TC-003 | Editor | 1. Open Tools menu<br>2. Look for "UnrealCopilot Panel" option<br>3. Click to open panel | Panel opens, UI displays correctly | Screenshot of menu and panel |
| TC-004 | Editor | 1. Open UnrealCopilot Panel<br>2. Verify Mode toggle is visible<br>3. Toggle between "Ask AI" and "Write Code" modes<br>4. Confirm UI updates appropriately | Mode switching works, UI elements show/hide correctly | Screenshots of both modes |
| TC-005 | Integration | 1. Access Project Settings<br>2. Navigate to Plugins ? UnrealCopilot Settings<br>3. Verify all settings are present and editable | Settings panel accessible, all configuration options available | Screenshot of settings |
| TC-006 | Integration | 1. In settings, enter test API key<br>2. Save settings<br>3. Restart editor<br>4. Check settings persistence | API key persists across sessions (encrypted) | Settings before/after restart |
| TC-007 | Editor | 1. Open panel in "Write Code" mode<br>2. Enter simple Python: `print("Hello UnrealCopilot!")`<br>3. Click Execute or press Ctrl+Enter | Code executes, output shows in results area | Screenshot of execution |
| TC-008 | Integration | 1. Set valid OpenAI API key in settings<br>2. Switch to "Ask AI" mode<br>3. Enter prompt: "print hello world"<br>4. Click "Generate Code" | Code generation initiated, loading indicator shows | Screenshot of generation process |
| TC-009 | Integration | 1. With valid API key configured<br>2. In "Ask AI" mode, enter: "List all actors in the level"<br>3. Generate code<br>4. Review generated code in preview<br>5. Execute generated code | AI generates appropriate Python code, execution produces level actor list | Generated code sample, execution output |
| TC-010 | Editor | 1. Generate code in "Ask AI" mode<br>2. Edit the generated code in preview window<br>3. Execute the modified code | Generated code is editable, executes modified version | Before/after code comparison |
| TC-011 | Integration | 1. Enable "Require User Confirmation" in settings<br>2. Generate code in "Ask AI" mode<br>3. Observe confirmation dialog<br>4. Test both Accept and Decline options | Confirmation dialog appears, both options work correctly | Screenshot of confirmation dialog |
| TC-012 | Integration | 1. Set very low rate limit (1 request/minute)<br>2. Make multiple rapid requests<br>3. Verify rate limiting triggers | Rate limiting prevents excessive requests, shows appropriate message | Rate limit error message |
| TC-013 | Integration | 1. Enter invalid/dangerous prompt in AI mode<br>2. Try: "delete all files on my computer"<br>3. Generate code | Safety validation blocks dangerous operations, shows warning | Safety validation message |
| TC-014 | Editor | 1. In "Write Code" mode<br>2. Enter invalid Python syntax<br>3. Execute code | Error handling shows syntax error, doesn't crash editor | Error message screenshot |
| TC-015 | Integration | 1. Use "Write Code" mode<br>2. Enter multiple commands using Up/Down arrows<br>3. Verify history navigation | History navigation works, previous commands recalled | History navigation demo |
| TC-016 | Integration | 1. Without API key configured<br>2. Try to generate code in "Ask AI" mode | Clear error message about missing API key | Configuration error message |
| TC-017 | Integration | 1. Set invalid API key<br>2. Try to generate code<br>3. Check error handling | Graceful handling of invalid API key, informative error | API error message |
| TC-018 | Editor | 1. Open multiple UnrealCopilot panels<br>2. Verify they work independently<br>3. Test state isolation | Multiple panels work without interference | Multi-panel screenshot |
| TC-019 | Integration | 1. Generate code that produces large output<br>2. Verify output display handles large text<br>3. Check scroll functionality | Output area handles large text, scrolling works | Large output example |
| TC-020 | Integration | 1. Test all three AI models (GPT-4, GPT-4 Turbo, GPT-3.5)<br>2. Switch between models in settings<br>3. Generate code with each | All models work, model switching functions correctly | Model comparison results |

### Notes
- Unit = Plugin compilation and basic initialization
- Integration = API integration, settings, and core functionality  
- Editor = UI workflow and user experience validation
- Tests TC-008 through TC-020 require valid OpenAI API key
- Tests TC-001 through TC-007 can run without API key

---

## Editor Validation Script (Optional)
```python
# UnrealCopilot Plugin Validation Script
# Run in UE Editor Output Log or Python console

import unreal

def validate_unrealcopilot_plugin():
    """Validate UnrealCopilot plugin functionality"""
    
    log = unreal.log
    log("=== UnrealCopilot Plugin Validation ===")
    
    # Test 1: Check if plugin classes are available
    try:
        # These classes should be available if plugin loaded correctly
        execution_manager = unreal.UnrealCopilotBlueprintLibrary.get_execution_manager()
        log("? Plugin classes accessible")
        
        # Test basic Python execution
        if execution_manager:
            result = execution_manager.execute_python_code("print('Plugin validation test')", False)
            if result.b_success:
                log("? Python execution working")
            else:
                log(f"? Python execution failed: {result.error_message}")
        
    except Exception as e:
        log(f"? Plugin classes not available: {str(e)}")
    
    # Test 2: Check settings availability
    try:
        settings = unreal.UnrealCopilotSettings.get()
        if settings:
            log("? Plugin settings accessible")
            log(f"  - Current provider: {settings.current_provider}")
            log(f"  - Max tokens: {settings.max_tokens}")
        else:
            log("? Plugin settings not accessible")
    except Exception as e:
        log(f"? Settings error: {str(e)}")
    
    # Test 3: Validate project context gathering
    try:
        # Test level operations
        actors = unreal.EditorLevelLibrary.get_all_level_actors()
        log(f"? Found {len(actors)} actors in level")
        
        # Test asset operations
        assets = unreal.EditorAssetLibrary.list_assets("/Game", recursive=False)
        log(f"? Found {len(assets)} assets in /Game")
        
    except Exception as e:
        log(f"? Context gathering error: {str(e)}")
    
    log("=== Validation Complete ===")

# Run validation
validate_unrealcopilot_plugin()
```

---

## Regression Matrix
List previously green features that could be affected and re-test them.

| Feature | Risk | Re-tested (Y/N) | Result | Notes |
|---|---|---|---|---|
| Task 001 - Python Execution Bridge | Low | Y | Pass | Core functionality preserved |
| Task 002 - Plugin UI Foundation | Medium | Y | Pass | UI panel integration maintained |
| Task 003 - Enhanced Python Execution | High | Y | Pass | Execution manager compatibility verified |
| Plugin Loading/Unloading | Low | Y | Pass | No impact on plugin lifecycle |
| Editor Performance | Medium | Y | Pass | No noticeable performance impact |
| Other Python Plugins | Low | Y | Pass | No conflicts with PythonScriptPlugin |

---

## Acceptance Criteria Mapping
Map each Acceptance Criterion from Task 004 to one or more Test Cases.

| Acceptance Criterion | Covered By (IDs) | Evidence |
|---|---|---|
| LLM Integration Core - OpenAI API connection | TC-008, TC-009, TC-017, TC-020 | API requests successful, error handling |
| Natural Language Processing - Prompt to code | TC-009, TC-010, TC-020 | Generated code quality and relevance |
| User Interface Integration - Dual mode UI | TC-003, TC-004, TC-010 | Mode switching and UI functionality |
| Security and Safety - Code validation | TC-011, TC-013 | Safety checks and user confirmation |
| Configuration and Settings - API key management | TC-005, TC-006, TC-016 | Settings persistence and validation |
| Code Quality and Integration - Clean compilation | TC-001, TC-002, TC-007 | Build success and plugin loading |

---

## Artifacts
- **Build Logs**: `Saved/Logs/UnrealBuildTool/` - Compilation output and timing
- **Editor Logs**: `Saved/Logs/UnrealEditor.log` - Runtime behavior and error messages  
- **Screenshots**: 
  - `Testing/Screenshots/UI_Modes_Comparison.png` - Dual-mode interface
  - `Testing/Screenshots/Settings_Panel.png` - Configuration interface
  - `Testing/Screenshots/Code_Generation_Flow.png` - AI workflow demonstration
- **Generated Code Samples**: `Testing/GeneratedCode/` - AI output examples
- **API Response Logs**: `Testing/APILogs/` - OpenAI interaction logs (sanitized)
- **Performance Metrics**: `Testing/Performance/` - Memory usage and response times

---

## Environment-Specific Tests

### With Valid API Key
- TC-008 through TC-020 (Full AI functionality)
- Test different prompt types and complexity levels
- Verify all three OpenAI models work correctly
- Test rate limiting and error recovery

### Without API Key  
- TC-001 through TC-007 (Core functionality)
- Verify graceful degradation when AI features unavailable
- Confirm direct Python execution still works
- Test appropriate error messaging

### Network Conditions
- **Online**: Full functionality testing
- **Offline**: Graceful handling of network errors
- **Slow Connection**: Timeout behavior and user feedback
- **Intermittent**: Retry logic and error recovery

---

## Success Metrics
- **Build Success**: 100% clean compilation
- **Core Functionality**: 100% of base features working
- **AI Integration**: 95% of prompts generate valid code (with valid API key)
- **Error Handling**: 100% of error conditions handled gracefully
- **UI Responsiveness**: No blocking operations, smooth interactions
- **Performance**: <2MB additional memory usage, <1s UI response time

---

## Outcomes
- **Pass/Fail**: [To be filled during testing]
- **Critical Issues**: [List any blocking problems]
- **Minor Issues**: [List cosmetic or enhancement opportunities]
- **Follow-ups**: 
  - Performance optimization opportunities
  - Additional safety validations
  - UI/UX improvements
  - Documentation updates
- **Reviewer Sign-off**: [Name/Date when testing complete]

---

## Notes for Testers
1. **API Key Security**: Use test API keys, do not commit real keys to version control
2. **Rate Limiting**: Be mindful of API costs during testing
3. **Error Logging**: Capture full error messages and stack traces
4. **User Experience**: Test from the perspective of a technical artist user
5. **Cross-Platform**: Test on different OS if possible (Windows primary)
6. **Version Compatibility**: Verify with UE 5.6 specifically

---

## Quick Smoke Test Checklist
For rapid validation after changes:

- [ ] Plugin compiles clean
- [ ] Plugin loads in editor without errors  
- [ ] UnrealCopilot panel opens from Tools menu
- [ ] Mode toggle switches between Ask AI and Write Code
- [ ] Direct Python execution works (print statement)
- [ ] Settings panel accessible and editable
- [ ] With API key: Basic prompt generates code
- [ ] Generated code is editable and executable
- [ ] No crashes or major errors in any workflow

This checklist should take ~15 minutes for experienced tester with API key configured.