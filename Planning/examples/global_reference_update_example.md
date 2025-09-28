# Global Reference Document (Updated Example)

This document serves as the **single source of truth** for the project. It records the current state of the codebase, dependencies, architecture decisions, and roadmap.  
The Planning Agent must always refer to this document when generating or updating tasks.

---

## Project Overview
This project is an **Unreal Engine plugin** designed to integrate a Large Language Model (LLM) as a **technical artist development partner**.  
The plugin enables prompting within the Unreal Editor and executes generated Python scripts to perform tasks such as:  
- Level creation and editing  
- Asset creation (Blueprints, animations, materials, meshes, effects, etc.)  
- Automation of technical art workflows  

---

## Current State of Codebase
- Active branches: `main`, `dev`  
- Plugin scaffold complete.  
- **Python execution bridge implemented (Task 001 complete).**  
- Task management structure established (Planning Agent + Coding Agents workflow).  
- Documentation templates completed.  

---

## Dependencies
- **Unreal Engine version**: 5.6 (Editor build)  
- **Python version**: 3.11 (Unreal embedded)  
- **Third-party plugins**: None yet  
- **External services**: GitHub Copilot (model billing/integration) under evaluation  

---

## Coding Conventions
- Tasks are defined in standalone `.md` files.  
- Post-mortems accompany each completed task.  
- Consistent Unreal naming conventions:  
  - Classes: `UClassName`, `AActorName`  
  - Functions: `CamelCaseFunctionNames`  
  - Variables: `bPrefixForBooleans`, `CamelCase`  
- Directory structure follows Unreal plugin standards.  

---

## Roadmap

### Current Milestone
- **Milestone 1: Plugin Framework Setup**  
  - Deliverables:  
    - Functional Unreal plugin scaffold ✅  
    - **Python execution bridge ✅ (Task 001 complete)**  
    - Task generation and execution pipeline (in progress)  

### Next Planned Milestones
- **Milestone 2: Editor Integration**  
  - Add in-editor UI for prompting and results.  
  - Support for executing generated Python scripts safely.  

- **Milestone 3: Asset Workflow Automation**  
  - Automate common technical art tasks (materials, animation graphs, level editing).  

- **Milestone 4: Agent Workflow Expansion**  
  - Multi-agent collaboration (Planner, Coder, Reviewer).  
  - GitHub Copilot integration for billing/quotas.  

---

## Known Issues / Blockers
- [ ] Error handling strategy for failed/invalid Python scripts incomplete.  
- [ ] No current mechanism for sandboxing editor operations.  
- [ ] Pending decision on GitHub Copilot vs. direct API model integration.  

---

## Testing & Validation Strategy
- **Unit Tests**: Python utility functions and Unreal-side wrappers.  
- **Integration Tests**: Ensure Python scripts execute correctly within the Unreal Editor environment.  
- **Editor Validation**: Manual validation of plugin UI/UX flows inside the Unreal Editor.  
- **Regression Testing**: Re-run previous successful test cases after significant updates.  
- **Acceptance Criteria Alignment**: Each task’s acceptance criteria must map to at least one validation method above.  

---

## References
- [Unreal Python API Reference](https://docs.unrealengine.com/5.3/en-US/PythonAPI/)  
- [Unreal Engine C++ API Reference](https://docs.unrealengine.com/5.3/en-US/API/)  

---

## Revision History
- Last updated: 2025-09-28  
- Updated by: Planning Agent  
- Notes: Added Task 001 completion, updated roadmap, carried forward known issues.  
