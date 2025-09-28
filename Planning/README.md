# Planning Documents

This folder contains all planning and coordination documents for the **Unreal Copilot Plugin Project**.  

## Contents
- **PLANNING_AGENT_INSTRUCTIONS.md** – Defines workflow for the Planning Agent.  
- **GLOBAL_REFERENCE_DOC.md** – Living canonical record of project state, roadmap, dependencies, and issues.  
- **GLOBAL_REFERENCE_DOC_TEMPLATE.md** – Blank template for initializing a new Global Reference Doc.  
- **TASK_TEMPLATE.md** – Standard format for individual coding tasks.  
- **POSTMORTEM_TEMPLATE.md** – Required format for coding agent post-mortems.  
- **TESTING_CHECKLIST_TEMPLATE.md** – Standard format for test cases and validation.  
- **CODE_REVIEW_TEMPLATE.md** – Standard format for review agent evaluations of completed tasks.  

## Workflow Overview

### 1. Planning
- The **Planning Agent** creates tasks from the Roadmap using the Task Template.  
- Tasks are atomic, self-contained, and testable.  

### 2. Implementation
- **Coding Agents** complete tasks and submit Post-Mortems plus Testing Checklists.  

### 3. Review
- **Review Agents** evaluate completed tasks using the Code Review Template.  
- Reviews assess functionality, code quality, adherence to conventions, testing coverage, and risks.  
- Review outcomes: **Pass / Conditional Pass / Fail**.  

### 4. Integration
- The **Planning Agent** updates the Global Reference Doc with results, roadmap progress, and new issues.  

## Usage
- The Planning Agent uses these docs to generate and manage tasks.  
- Coding Agents must follow the templates for delivering work and documenting outcomes.  
- Review Agents must complete a Code Review Template for each finished task.  
- The Global Reference Doc must be kept current to ensure all tasks are contextualized correctly.  
