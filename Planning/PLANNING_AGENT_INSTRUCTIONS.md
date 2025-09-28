# Planning Agent Instructions

This document defines the responsibilities, workflow, and formatting rules for the **Planning Agent**, which is responsible for generating, managing, and updating tasks assigned to **Coding Agents** and coordinating with **Review Agents** in this project.

---

## Purpose

The Planning Agent ensures that:
- Each coding task is **self-contained**, **clear**, and **implementable** by a fresh agent without prior context.  
- All tasks are aligned with the **current global project state**, documented in the **Global Reference Doc**.  
- The development process follows an **iterative, phased roadmap**, with tasks broken down into the smallest testable units.  
- Review outcomes from Review Agents are incorporated into roadmap updates and future task planning.  

---

## Workflow Sequence

The workflow is defined as follows:

1. **Task Generation**  
   - The Planning Agent creates a task using the Task Template.  
   - Task must include objectives, requirements, implementation notes, and acceptance criteria.  

2. **Implementation**  
   - Coding Agent implements the task.  
   - Coding Agent submits both a Post-Mortem and a Testing Checklist.  

3. **Review**  
   - Review Agent evaluates the completed task using the Code Review Template.  
   - Review Agent determines outcome: **Pass / Conditional Pass / Fail**.  

4. **Integration**  
   - Planning Agent reviews the Code Review results.  
   - If Pass: update the Global Reference Doc with completed milestone(s), issues, and roadmap adjustments.  
   - If Conditional Pass: integrate changes with follow-up tasks.  
   - If Fail: generate a revised task for reimplementation.  

5. **Iteration**  
   - Planner generates the next task based on updated Global Reference Doc and Roadmap.  

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

## Code Reviews

Each completed task must be evaluated by a Review Agent using the **Code Review Template**.  
- Reviews verify functional correctness, code quality, style, and risk management.  
- Review outcomes (Pass / Conditional Pass / Fail) must be relayed back to the Planning Agent.  
- Review feedback is incorporated into future task planning.  

---

## Special Instructions

- Always reference **official specifications and documentation** when relevant:  
  - [Unreal Python API Reference](https://docs.unrealengine.com/5.3/en-US/PythonAPI/)  
  - [Unreal Engine C++ API Reference](https://docs.unrealengine.com/5.3/en-US/API/)  

- Keep tasks small and **independent** to reduce error propagation.  
- Ensure the Planning Agent updates the **Global Doc** whenever the codebase state changes.  
