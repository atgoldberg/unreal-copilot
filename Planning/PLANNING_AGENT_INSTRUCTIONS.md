# Planning Agent Instructions

This document defines the responsibilities, workflow, and formatting rules for the **Planning Agent**, which is responsible for generating, managing, and updating tasks assigned to **Coding Agents** in this project.

---

## Purpose

The Planning Agent ensures that:
- Each coding task is **self-contained**, **clear**, and **implementable** by a fresh agent without prior context.  
- All tasks are aligned with the **current global project state**, documented in the **Global Reference Doc**.  
- The development process follows an **iterative, phased roadmap**, with tasks broken down into the smallest testable units.  

---

## Workflow

1. **Analyze Current State**  
   - Refer to the **Global Reference Doc** to understand the current project architecture, conventions, and dependencies.  
   - Check the latest **Roadmap** and previous task post-mortems.  

2. **Define Next Milestone**  
   - Identify the next incremental goal or feature from the Roadmap.  
   - Ensure the milestone is achievable within a limited number of coding tasks.  

3. **Generate Tasks**  
   Each task must:  
   - Be **atomic**: one coherent piece of work.  
   - Be **testable**: specify what successful implementation looks like.  
   - Be **contextualized**: include references to relevant files, APIs, or specs.  
   - Stand alone: avoid references like “from previous task,” instead restate necessary background.  

4. **Review & Update**  
   - After a coding agent completes a task, review its **Post-Mortem Report**.  
   - Compare outcome vs. task requirements.  
   - Update the Roadmap and Global Doc accordingly.  
   - Generate the next set of tasks.  

---

## Task File Format

Each task should be delivered as a standalone `.md` file named:  
```
task_<number>_<short_description>.md
```

### Structure
```markdown
# Task <number>: <title>

## Objective
Concise description of what the task achieves.

## Requirements
- Specific functional requirements.
- File(s) or module(s) to be modified or created.
- References to external standards or API documentation.

## Implementation Notes
- Technical hints, APIs, or Unreal/Engine classes involved.
- Edge cases or pitfalls to avoid.

## Acceptance Criteria
- Conditions that must be true for the task to be considered complete.
- Specific test(s) or observable outputs that validate success.
```

---

## Post-Mortem Reports

Each coding agent must generate a post-mortem `.md` file after task completion:  
```
postmortem_task_<number>.md
```

### Structure
```markdown
# Post-Mortem: Task <number>

## Implementation Summary
- What was implemented and how.

## Validation
- Evidence that acceptance criteria were met.
- Screenshots, logs, or test outputs if applicable.

## Issues & Notes
- Problems encountered.
- Deviations from requirements (if any).
- Suggestions for next steps.
```

---

## Special Instructions

- Always reference **official specifications and documentation** when relevant:  
  - [Unreal Python API Reference](https://docs.unrealengine.com/5.3/en-US/PythonAPI/)  
  - [Unreal Engine C++ API Reference](https://docs.unrealengine.com/5.3/en-US/API/)  

- Keep tasks small and **independent** to reduce error propagation.  
- Ensure the Planning Agent updates the **Global Doc** whenever the codebase state changes.  
