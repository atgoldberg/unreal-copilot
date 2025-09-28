# Testing Checklist Template

Attach this file to the task or post-mortem. Keep it self-contained and executable by a fresh agent.

---

## Metadata
- **Task ID / Title**:  
- **Author**:  
- **Date**:  
- **UE Version**:  
- **Plugin Commit SHA**:  
- **Test Environment**: (OS, Editor config, relevant plugins)

---

## Preconditions
- Build succeeds for target: `UnrealEditor Development`  
- Required content/assets present (list paths)  
- Test project loads with no warnings/errors in log related to this plugin

---

## Test Cases

> For each case: write exact steps, expected results, and artifacts to attach.

| ID | Type (Unit/Integration/Editor) | Steps | Expected Result | Artifacts |
|---|---|---|---|---|
| TC-001 | Unit |  |  |  |
| TC-002 | Integration |  |  |  |
| TC-003 | Editor |  |  |  |

### Notes
- Unit = Python utility or C++ helper run outside editor loop
- Integration = Python->Unreal bridge/automation inside editor runtime
- Editor = Manual UI workflow and UX validation

---

## Editor Validation Script (Optional)
Paste an `Automation` or Python snippet that can be run in the editor to reproduce checks.

```python
# Example skeleton; replace with real checks
import unreal
log = unreal.log
log("Begin validation")
# ... your assertions / asset queries / commandlets ...
```

---

## Regression Matrix
List previously green features that could be affected and re-test them.

| Feature | Risk | Re-tested (Y/N) | Result | Notes |
|---|---|---|---|---|
|  |  |  |  |  |

---

## Acceptance Criteria Mapping
Map each Acceptance Criterion from the task to one or more Test Cases.

| Acceptance Criterion | Covered By (IDs) | Evidence |
|---|---|---|
|  |  |  |

---

## Artifacts
- Logs: attach `Saved/Logs/UnrealEditor.log` excerpt
- Screenshots / recordings: paths
- JSON / CSV outputs: paths

---

## Outcomes
- **Pass/Fail**:  
- **Follow-ups**: (bugs filed, roadmap updates)
- **Reviewer Sign-off**: (name/date)
