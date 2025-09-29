# Global Reference Document

This document serves as the **single source of truth** for the Unreal Copilot project. It records the current state of the codebase, dependencies, architecture decisions, and roadmap.  
The Planning Agent must always refer to this document when generating or updating tasks.

---

## Project Vision & Goals

**Unreal Copilot** is an integrated LLM agent plugin for **Unreal Engine** that acts as a **technical artist development partner**. The core vision is to enable users to prompt a selected AI model to perform Unreal technical art and design tasks through natural language interfaces.

### Primary Goals
- **🔹 AI-Driven Technical Art Assistant**: Generate and execute Python code in-editor via natural language prompts
- **🔹 Task Automation**: Automate repetitive Unreal Editor workflows (level creation, asset setup, animation pipelines)  
- **🔹 Extensible LLM Integration**: Support multiple model providers (GitHub Copilot, OpenAI, etc.) for flexible access and billing
- **🔹 Iterative Development Framework**: Phased development with clear milestones and deliverables

### Technical Vision
The plugin implements a **prompt-to-Python workflow** where:
1. Users enter natural language prompts in the Unreal Editor
2. Selected LLM generates executable Python scripts  
3. Scripts execute safely within Unreal's embedded Python environment
4. Results are displayed with error handling and rollback capabilities

---

## Current State of Codebase
- **Repository**: GitHub - atgoldberg/unreal-copilot  
- **Active branches**: `main`, `dev`  
- **Development Status**: Phase 1 (Core Plugin Skeleton) - COMPLETED  
- **Next Phase**: Phase 2 (Prompt-to-Python Workflow) - IN PROGRESS

### ✅ Phase 1 Completed Components
- **✅ Core Plugin Framework**: Unreal Engine plugin scaffold fully implemented
- **✅ Python Execution Pipeline**: Fully operational (Task 001)  
- **✅ Editor UI Foundation**: Dockable panel with core functionality (Task 002)
- **✅ Enhanced Python Execution**: Async execution and improved error handling (Task 003)
- **✅ Task Management Infrastructure**: Planning Agent + Coding Agents workflow established

### Implemented Modules & Features

#### Python Execution System (Task 001)
- **Core Function**: `UUnrealCopilotBlueprintLibrary::ExecutePythonString`
- **Capabilities**: 
  - Execute arbitrary Python code in Unreal's embedded environment
  - Comprehensive error capture with stack traces
  - Blueprint callable API with proper exposure
  - Fallback mechanisms for compatibility across UE versions
- **Status**: ✅ Production ready

#### Editor UI Foundation (Task 002)  
- **Module**: UnrealCopilotEditor with native Slate UI framework
- **Features**:
  - Dockable panel accessible from Tools menu and toolbar
  - Multi-line Python code input with monospace styling
  - Real-time execution with formatted result display
  - Integrated status indicators and error reporting
- **Status**: ✅ Production ready

#### Enhanced Python Execution (Task 003)
- **Module**: UnrealCopilotExecutionManager for advanced execution handling
- **Features**:
  - Asynchronous Python execution (prevents UI blocking)
  - Enhanced error reporting with line numbers and stack traces
  - Input validation and syntax checking
  - Execution history and result caching
- **Status**: ✅ Production ready

---

## Technical Requirements & Dependencies

### Core Requirements
- **Unreal Engine**: 5.6+ (Editor build required)
- **Python Environment**: 3.11 (Unreal embedded)  
- **Development Tools**: Visual Studio 2022 (for plugin development)
- **Target Platforms**: Windows (primary), Mac/Linux (future consideration)

### Plugin Dependencies
- **PythonScriptPlugin**: Required for Python execution (built-in UE plugin)
- **Editor Modules**: Projects, ToolMenus, EditorStyle, Slate
- **Third-party Libraries**: None (minimal external dependencies by design)

### External Service Integration (Planned)
- **LLM Providers**: GitHub Copilot, OpenAI API, Azure OpenAI
- **Version Control**: Git integration for issue-based task execution
- **Documentation**: Unreal Python API, VRM specification (for import tasks)

---

## Development Roadmap

### ✅ Phase 1: Core Plugin Skeleton (COMPLETED)
**Objective**: Establish foundation for LLM-assisted technical art workflows

**Deliverables**:
- ✅ Functional Unreal Engine plugin framework
- ✅ Python execution bridge with comprehensive error handling  
- ✅ In-editor UI for script input and execution
- ✅ Asynchronous execution pipeline
- ✅ Task generation and management infrastructure

### 🚧 Phase 2: Prompt-to-Python Workflow (CURRENT FOCUS)
**Objective**: Implement natural language to executable Python conversion

**Priority Deliverables**:
- [ ] LLM API integration layer (GitHub Copilot/OpenAI)
- [ ] Natural language prompt parsing and context management
- [ ] Python code generation with safety validation
- [ ] Prompt template system for common technical art tasks
- [ ] Result interpretation and user feedback mechanisms

### 📋 Phase 3: Tooling & Workflow Expansion (PLANNED)
**Objective**: Expand capabilities for comprehensive technical art automation

**Deliverables**:
- [ ] Asset creation workflows (Blueprints, materials, meshes)
- [ ] Level editing and scene composition automation  
- [ ] Animation pipeline integration
- [ ] Error handling and rollback systems for complex operations
- [ ] Batch processing and workflow templates

### 📋 Phase 4: Advanced Features (FUTURE)
**Objective**: Production-ready features and marketplace distribution

**Deliverables**:
- [ ] Multi-agent workflows (Planner, Implementer, Reviewer)
- [ ] GitHub/Git integration for issue-based task execution
- [ ] Advanced security and sandboxing for production use
- [ ] Performance optimization and resource management
- [ ] Marketplace-ready packaging and distribution

---

## Architecture & Design Patterns

### Plugin Architecture
- **Modular Design**: Separate core (`UnrealCopilot`) and editor (`UnrealCopilotEditor`) modules
- **Blueprint Integration**: Core functions exposed to Blueprint system for accessibility
- **Event-Driven UI**: Slate-based reactive interface with proper Unreal Editor integration
- **Async Processing**: Non-blocking execution for long-running Python operations

### Code Organization
- **Unreal Naming Conventions**: 
  - Classes: `UClassName`, `AActorName`, `FNonUObjectClass`, `SSlateWidget`
  - Functions: `CamelCaseFunctionNames`  
  - Variables: `bPrefixForBooleans`, `CamelCase`
- **Directory Structure**: Standard Unreal plugin layout with clear separation of concerns
- **Documentation**: Comprehensive inline documentation and external references

### Safety & Security Considerations
- **Execution Sandboxing**: Planned for production environments
- **Input Validation**: Multi-layer validation for Python code generation
- **Error Recovery**: Rollback mechanisms for failed operations
- **Resource Management**: Memory and performance monitoring for long operations

---

## Current Known Issues & Limitations

### Resolved Issues
- [x] ~~Python execution pipeline implementation~~ → **RESOLVED** (Task 001)
- [x] ~~UI blocking during Python execution~~ → **RESOLVED** (Task 003)  
- [x] ~~Basic error handling for failed scripts~~ → **RESOLVED** (Tasks 001-003)
- [x] ~~In-editor UI for code input and execution~~ → **RESOLVED** (Task 002)

### Current Limitations
- [ ] **No LLM Integration**: Still requires manual Python code input (Phase 2 objective)
- [ ] **Limited Asset Workflow Support**: Basic Python execution only, no specialized templates
- [ ] **No Production Sandboxing**: Full Python access without restrictions
- [ ] **Single Model Support**: No multi-provider LLM integration yet
- [ ] **No Session Persistence**: Execution history not saved between sessions

### Phase 2 Blockers
- [ ] **LLM Provider Selection**: Decision needed between GitHub Copilot vs. direct API integration
- [ ] **Prompt Engineering Strategy**: Template system design for technical art tasks  
- [ ] **Code Generation Safety**: Validation mechanisms for LLM-generated Python
- [ ] **Context Management**: How to provide Unreal project context to LLM

---

## Testing & Quality Assurance

### Implemented Testing
- **✅ Unit Tests**: Python execution bridge with comprehensive edge cases
- **✅ Integration Tests**: UI-Python execution pipeline validation  
- **✅ Editor Integration**: Manual validation in Unreal Editor environment
- **✅ Code Reviews**: All implementations receive formal review and approval
- **✅ Regression Testing**: Backward compatibility maintained across updates

### Quality Standards
- **Documentation**: Every task includes detailed post-mortem analysis
- **Code Coverage**: Comprehensive testing for core functionality
- **Performance Benchmarks**: Execution time and memory usage monitoring
- **User Experience**: Consistent with Unreal Editor design patterns

---

## Installation & Usage Instructions

### Developer Installation
1. Clone repository into Unreal project's `Plugins/` directory
2. Enable plugin in Unreal Editor (`Edit → Plugins → Installed`)
3. Restart editor for full integration
4. Access via `Tools → Unreal Copilot Panel`

### Current Usage (Phase 1)
1. Open Unreal Copilot Panel in editor
2. Enter Python code directly in input field
3. Click Execute to run code in embedded Python environment  
4. Review results in output display with error handling

### Future Usage (Phase 2+)
1. Enter natural language prompts (e.g., "Create a scrolling texture material")
2. LLM generates appropriate Python code
3. Review and execute generated code  
4. Iterate with follow-up prompts for refinement

---

## References & External Resources
- **[Unreal Python API Reference](https://docs.unrealengine.com/5.6/en-US/PythonAPI/)**: Primary API documentation
- **[Unreal Engine C++ API Reference](https://docs.unrealengine.com/5.6/en-US/API/)**: Core engine integration  
- **[Unreal Engine Slate UI Framework](https://docs.unrealengine.com/5.6/en-US/slate-ui-framework-in-unreal-engine/)**: UI development
- **[Unreal Engine Plugin Documentation](https://docs.unrealengine.com/5.6/en-US/unreal-engine-plugins/)**: Plugin development standards
- **[VRM Specification](https://github.com/vrm-c/vrm-specification/tree/master/specification)**: For import-related tasks
- **GitHub Repository**: https://github.com/atgoldberg/unreal-copilot

---

## Project Metadata
- **License**: MIT License  
- **Primary Author**: Athomas Goldberg  
- **Development Model**: Iterative phases with agent-driven task management
- **Target Market**: Technical artists, Unreal Engine developers, automation specialists

---

## Revision History
- **2024-12-19**: Major revision to align with PROJECT_DESCRIPTION.md vision and phased development approach
- **2024-12-19**: Updated with Task 001, 002, & 003 completion status  
- **2024-09-28**: Initial document creation  
- **Last updated**: 2024-12-19  
- **Updated by**: Planning Agent
