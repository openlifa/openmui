# CTS level S: scenarios every language runs alike

A scenario (`<name>.scenario.omui.json`) is a view, its state and a sequence of steps, written in no language. Each language binding has a **runner** that builds the view with **its own** builders, operators and closures, then performs the steps through its own API: `act`, `invoke`, `snapshot`, signals and models. What a runner records is the scenario's **trace**. Every language must record the same trace as `<name>.trace.omui.json` beside it (plan §10, M3 acceptance ③).

| language | runner | run by |
|---|---|---|
| Lua | `lang/lua/tests/cts.lua` | `lang/lua/tests/lua_test.cpp`, through `openmui.cts` |
| JavaScript | `lang/js/tests/cts.js` | `lang/js/tests/js_test.cpp`, through `openmui.cts` |
| Python | `lang/python/tests/cts.py` | `lang/python/tests/test_conformance.py` |

The trace is semantic. Its trees are the agent protocol's nodes (id, role, name, value, states, actions) without `bounds`, and an empty list and an empty map are the same value. A layout change therefore leaves the traces alone. A change of meaning shows as a difference, with the step it happened at. `OMUI_CTS_WRITE=1` writes the traces instead of comparing them, for a change that is meant; review the diff.

## The file

```json
{
  "omui": "0.1",
  "scenario": {
    "doc": "what it covers",
    "signals": {"count": 0},
    "models": {"notes": [{"key": "n1", "title": "Milk"}]},
    "verbs": {"note.archive": {"effect": "reversible", "summary": "…", "person": false, "do": [actions], "result": value}},
    "view": element,
    "steps": [step, …]
  }
}
```

Runners create the signals in the order of their names. The models get their rows before the view is mounted at `main`. The viewport is 320 × 480 logical pixels at scale 1, so that the states a viewport decides (offscreen) are the same everywhere.

### Elements

- `{"type": "button", "id": "plus", "props": {…}, "on": {"activated": [actions]}, "children": [elements]}`. Builders are found by name: `omui.button` (Python), `omui.Button` (Lua), `button` (JavaScript, with camel-case properties and `onActivated`).
- `{"when": operand, "then": [elements], "else": [elements]}`.
- `{"each": "notes", "where": operand, "body": element}`, where the body is built once and `{"row": "title"}` is the row's field.

### Operands (property values)

- A constant. A list or a map is written `{"const": …}`.
- `{"signal": "count"}` is the signal object. It binds both ways where the property is bindable.
- `{"row": "title"}` is a field of the loop's row.
- `{"op": "+", "args": [a, b]}`, with `+ - * / % == != < <= > >= and or`, and `not` and `neg` with one argument. Each runner uses its language's own spelling: Python's operators, Lua's metamethods and `omui.eq` …, JavaScript's methods.
- `{"call": "max", "args": [...]}`, `{"fmt": "{} to go", "args": [...]}`, `{"select": [cond, then, otherwise]}`.

### Actions (handlers and verbs, run in the language)

- `{"set": "count", "to": value}`.
- `{"add": "count", "by": 1, "min": 0, "max": 10}`: an update, clamped by `min` and `max` when they are given.
- `{"invoke": "verb", "args": {"k": value}}`.
- `{"push": "notes", "row": {…}}`, `{"remove": "notes", "key": value}` and `{"change": "notes", "key": value, "set": {…}}`.
- `{"native": "fuzzy.rank", "args": [values], "into": "best"}`: a native module's function (component-binding §8) through the language's generated binding (awaited where the language returns a promise), its result into a signal.

A value in an action is a constant, `{"event": "value"}` (the event's argument), `{"row": "key"}` (the field of the row the handler is in), `{"arg": "key"}` (the verb's argument) or `{"get": "count"}` (a signal's value now).

### Steps

Each step runs once the runtime has settled after the previous one.

| step | records |
|---|---|
| `{"tree": "main"}` | `{"tree": [nodes]}` |
| `{"act": "main/plus", "action": "activate", "args": {…}}` | `{"act": "ok"}` or `{"error": code}` |
| `{"invoke": "verb", "args": {…}}` | `{"invoke": answer}` or `{"error": code}` |
| `{"get": "count"}` | `{"get": value}` |
| `{"rows": "notes"}` | `{"rows": [rows]}` |
| `{"native": "fuzzy.match", "args": [...]}` | `{"native": result}` or `{"error": code}` |
| `{"theme": ["dark", "compact"]}` | `{"theme": "ok"}` or `{"error": code}` (openmui.theme: a theme, then overlays) |
| `{"themes": true}` | `{"themes": {themes, overlays, current, dark}}` |
| `{"set": …}`, `{"add": …}`, `{"push": …}`, `{"remove": …}`, `{"change": …}` | nothing; the action is done |

Every record also carries `"step"`, the step's index. `"expect"` on a step is checked as well as the trace:
- on a `tree` step, it maps node ids to the fields they have;
- on any other step, it is the recorded value.

An error's code is what the runtime says: the text before the first `:` of the message.
