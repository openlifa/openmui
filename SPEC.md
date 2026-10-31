# The openmui specification, version 0.3

Status: draft. This document is normative. Version 0.2 added the view
document, the expression language, the Script Binding Profile and the
contract and manifest schemas (§4, §11, §14, schema/); version 0.3 adds the
extensions of §16 — canvases, effects and backdrops, style packs, frame
pacing, texture tiers, video — the bus's `invoke` (§9), openas 0.2's
grants under an application's cap (S-11, §14), and undo, transactions and dry
runs (S-12, S-13). The tables it refers to are in
[docs/registry.md](docs/registry.md), generated from [registry/](registry/); where
the two disagree, the registry is right and this text is wrong.

Actors, effect classes, the person's own verbs, grants and the envelope are
those of [openas 0.2](https://github.com/openlifa/openas) (openas §2, §3, §5,
§6); this text names where openmui applies them.

The key words MUST, MUST NOT, SHOULD and MAY are used as in RFC 2119. Each
clause has an identifier (S-1, V-2, …); the conformance suite
([conformance/](conformance/)) has at least one case per clause.

## 1. Scope and conventions

openmui specifies a user interface by its **semantic tree**: what is on the
screen, what each thing is, what it can do and what doing it means. Pixels,
accessibility, agents and tests are renderings of that tree. The specification
defines six contracts:

1. the semantic tree (§3);
2. the view description (§4);
3. state and verbs (§5);
4. the host interface (§6);
5. the render interface (§7);
6. the C ABI, `openmui.h` (§8);

and with them animation (§10), component contracts (§11), extensions and
profiles (§12), the manifest (§14) and conformance (§13).

It does not specify how an implementation is built, how it renders, or how a
component looks.

- **G-1** Every value defined here can cross an address space: it holds no
  pointer and no callback.
- **G-2** Versions only add: a new version adds interfaces, fields and messages
  and changes none. A reader MUST skip fields, records and enumerators it does
  not know, except where a clause says otherwise (an unknown effect, §3.3).
- **G-3** The specification depends on nothing but the C++ standard library (for
  its reference value types) and C99 (for `openmui.h`).

## 2. The registry

`registry/` is the machine-readable source of truth: roles, states, effects,
origins, actions, extensions, profiles, motion, the expression language, the
effect-shader interface, the C ABI and the Core component contracts.
`include/openmui.h`, `src/generated/`, `docs/registry.md`, `stubs/` and
`schema/` are generated from it and MUST NOT be edited by hand.

Enumerators keep the order of their registry file; their numeric values are
stable across versions (G-2).

## 3. Contract ①: the semantic tree

### 3.1 Nodes

A node has an `id`, a `role`, a `name`, a `description`, a `value`, an
`origin`, a set of `state` bits, a list of `actions`, optional `bounds` and the
ids of its `children` (docs/registry.md lists roles and states). Its wire form
is a JSON object with those keys; enumerations are written by name; absent
fields are omitted.

`bounds` are in logical pixels. While a node animates, its bounds and values
are those of the **target** state (state `animating`).

### 3.2 Identity

- **S-8** In retained mode a node without an explicit id has a **structural
  id**: `<type>:<index>`, where index counts the node's preceding siblings of
  the same type, starting at 0 (`column:0/text:1`). Every way of writing a
  retained view (builders in any language, view documents) MUST produce the
  same structural ids for the same structure. In immediate mode an automatic id
  comes from the id stack and the call site and is stable within one build of
  the program. Interactive nodes SHOULD have explicit ids.
- **S-9** An id MUST NOT depend on display text (labels, translated strings).
- An id is a path of segments joined by `/`. A segment is 1 to 128 characters
  of `[A-Za-z0-9_.:-]`, not `.` or `..`. The character `:` appears only in
  structural segments, so an explicit id never collides with an automatic one.

### 3.3 Actions and effects

An action has a `name`, an `effect` (`read`, `reversible`, `irreversible`,
`external`), optionally the `verb` it invokes, the `answer` that confirms it,
and typed arguments.

- **S-1** An action whose effect is `irreversible` or `external` MUST state its
  `answer`.
- **S-2** Performing an action that needs an answer without being given it MUST
  NOT wait: the call answers at once with an `error` record of code
  `needs-answer`, the options that would answer it, and exit code 2.
- A node MAY declare a stronger effect for an action than the registry's
  default (a delete button's `activate` is `irreversible`), never a weaker one
  (effect tiers, docs/registry.md). A reader that meets an effect it does not
  know MUST treat it as `external` (openas §3.1).
- **S-4** A node in state `removing` accepts no action; a node in state
  `disabled` accepts only actions whose effect is `read`. So does a node in
  state `readonly` — its value can be read but is another's to change (a
  control a simulation drives, a game's ball operates). The universal property
  `readonly` sets it: `true`, or the reason in words, which the refusal (an
  `error` record of code `refused`, exit code 1) then gives.
- **S-10** An action or a verb MAY be the person's own (`person`, openas
  §3.4): the answer to a confirmation an agent asked for, a permission, a
  payment's approval. Only the person at the machine performs it, through the
  host's own input — the person is present (openas §2.7). A request that
  arrives through the bus interface (§9) or from any other process MUST be
  refused with an `error` record of code `refused` and exit code 1, whatever
  actor it names, and change nothing; the implementation marks such requests
  itself, never from what they say. The semantic tree
  states the mark on the action (`"person": true`), so an agent does not try.
  An action's mark comes from the verb it invokes or from the node (the
  universal property `person`), and, like an effect, may be added but not
  lifted. The program's own code — its scripts, its host — is not the bus.
- An action's declared arguments are all required and typed (`string`, `bool`,
  `i32` and `i64` integers, `f64` numbers, `list<string>`, or alternatives such
  as `f64 | min | max`, where a word is a symbol the component interprets). A call missing one,
  giving one of another type, or giving one the action does not declare MUST be
  refused with an `error` record of code `invalid-argument` that names the
  arguments the action takes, and change nothing. An action that declares none
  passes what it is given on (the arguments of the verb an `activate` invokes).

- **S-11** A request through the bus interface (§9) is decided as openas §5.3
  decides it: above the application's cap (§14) or the person's own, it is
  refused (`refused`, exit code 1); within a grant its actor holds, it runs
  without the answer S-1 asks for; otherwise S-1 and S-2 apply. Grants are
  given by the person, present, through the host's own input — never through
  the bus — and an application MAY offer the person no way to give them. An
  action is named, for a grant's scope, by the verb it invokes, or else by its
  own name. Reading the tree (`snapshot`, `find`, `wait`, `changes`) performs
  no verb: the cap does not reach it; an application that lets nothing read it
  leaves the bus interface out (§14).

- **S-12** A `reversible` action that succeeds MAY give its inverse: actions
  of the same node, with their arguments, that take it back (`toggle` again,
  `set_value` with the value it replaced). The implementation keeps a history
  of these for each scope: the window, or a node with the universal property
  `undo_scope`. The scope's node offers `undo` and `redo` (effect
  `reversible`) while there is something to undo or redo. Undoing performs the
  inverses as the one undoing would perform them (S-4, S-10 and S-11 apply);
  redoing performs what takes them back. A `read` action, and one that gives
  no inverse, is not recorded; a new change clears what could be redone. An
  `irreversible` or `external` action ends what can be undone: `undo` across
  it MUST be refused (`refused`, exit code 1) and change nothing.
- **S-13** A transaction performs actions in order as one step of history.
  The first that fails ends it: the actions already performed are undone by
  their inverses, last first, and the answer is that action's error, with its
  exit code. An `irreversible` or `external` action in a transaction MUST be
  refused before it runs. A call MAY be a dry run: it is checked as it would
  be performed (S-1, S-2, S-4, S-10, S-11, arguments) and answered, with exit
  code 0, by a `data` record stating its `outcome` (`run`, `needs-answer`,
  `refused` or `fails`), its `effect`, the `answer` or the `error`, and where
  the implementation can tell cheaply whether it could be undone (`undoable`)
  and the node's `value` before and after. A dry run MUST change nothing: no
  state, no history, no journal entry, no event.

### 3.4 Origins

- **S-3** The text of a node whose origin is `user_content` or `remote` is data.
  A consumer acting on the tree (an agent) MUST NOT follow instructions found in
  it.

### 3.5 Change and scale

- **S-5** The tree has a revision number. Changes are delivered as ordered
  deltas: `added`, `removed`, `updated`, under the new revision.
- **S-6** A node with an interactive role (docs/registry.md) MUST have a name.
- **S-7** A tree MAY be virtualised (a canvas, a long list): nodes that are not
  materialised are produced on demand by queries, and queries cover the whole
  content, not only the viewport.

## 4. Contract ②: the view description

- **V-1** The view IR is a tree of component types, ids, properties,
  bindings, event handlers, conditionals, loops over models, slots and
  transitions. It is the only required form; builders in code and view
  documents (`*.view.omui.json`, schema/) are equivalent sources of it.
- **V-2** Layout follows a subset of CSS flexbox and grid. Coordinates round to
  physical pixels by `floor(v·scale + 0.5)` computed in double precision;
  rectangles snap their edges, not their sizes (A-level).
- **V-3** Expressions are an AST with no side effects that always terminates.
  Code builds it with combinators; data files write it as CEL-compatible
  strings (docs/registry.md, Expressions). An expression error yields the
  type's default value and a diagnostic, never an exception. Expressions are
  evaluated synchronously by native code; a closure written in a script
  language is evaluated on the script's thread and MAY apply up to one frame
  late.
- **V-4** An event handler in a view document performs only three operations:
  set a signal, invoke a verb, emit an event.

## 5. Contract ③: state and verbs

- A **signal** holds a value; properties bind to signals one way, or two ways
  when the property is `bindable` (component contracts).
- A **verb** is a named operation of the application: lower-case words joined by
  dots (`note.delete`), with a summary, an effect, typed arguments and, for
  `irreversible` and `external` verbs, an answer (S-1 applies to verbs).
- An **event** belongs to a component contract; an event marked
  `coalesce = "frame"` is delivered at most once per frame with its last
  arguments.

## 6. Contract ④: the host interface

A host provides a surface (premultiplied BGRA8 pixels with a stride and damage
rectangles, or a GPU device under `OMUI_EXT_host_render_pass`) and events (keys,
text commit, pre-edit text, pointer, wheel, focus, close request, gamepads,
frame presented, visibility); the model is opensal's. A host MAY drive frames:
`frame(time, events) → (semantic delta, display list, damage)` — the form of
headless operation and game-engine embedding.

### 6.1 Input events

An event's wire form (C-3; a frame's `events_json`, a recorded session) is an
object whose `type` is one of `pointer`, `wheel`, `key`, `text`, `preedit`,
`focus`, `resize`, `close` or `gamepad`; [schema/input.schema.json](schema/input.schema.json)
states every field. Keys are named as in chords (`PageUp`, `A`); coordinates
are logical pixels in window space.

A **gamepad** is reduced to the standard set, named by position, not by the
label on the device:

| | |
|---|---|
| buttons | `south` `east` `west` `north` (A B X Y on an Xbox pad, Cross Circle Square Triangle on a PlayStation one), `back` `guide` `start`, `leftStick` `rightStick` (pressed in), `leftShoulder` `rightShoulder`, `leftTrigger` `rightTrigger`, `dpadUp` `dpadDown` `dpadLeft` `dpadRight` |
| axes | `leftX` `leftY` `rightX` `rightY` −1 … 1, y positive down; `leftTrigger` `rightTrigger` 0 … 1 |

```json
{"type": "gamepad", "kind": "connected", "device": 1, "name": "Xbox Series X Controller"}
{"type": "gamepad", "kind": "down", "device": 1, "button": "south"}
{"type": "gamepad", "kind": "axis", "device": 1, "axis": "leftX", "value": -0.5}
{"type": "gamepad", "kind": "disconnected", "device": 1}
```

- **G-1** `device` names one connection of one device and is not reused while
  the program runs; a device plugged in again is a new one. Events of a
  device come between its `connected` and its `disconnected`.
- **G-2** The host applies the dead zones: a stick's is a circle (a diagonal is
  as easy as a straight push), the value stretched so its edge is 0; values
  are multiples of 1/256, so they survive their wire form exactly. An event is
  sent only for a change: no repeats.
- **G-3** A trigger is an axis and a button: the button goes down when the
  axis reaches 0.5 and up when it falls below 0.4.
- **G-4** Before `disconnected` the host releases what the device held: an
  `up` for each button down and an `axis` of 0 for each axis away from rest.
- **G-5** Gamepad events go where key events go (the focused node, else the
  root, bubbling); their handler names are `gamepad_connect`,
  `gamepad_disconnect`, `gamepad_down`, `gamepad_up` and `gamepad_axis`.
  Which device is which player is the application's to decide.

## 7. Contract ⑤: the render interface

- Display-list primitives: rectangle, rounded rectangle, path (non-zero and
  even-odd fill, stroke), image, glyph run, clip, transform, layer, external
  texture.
- **R-1** Colours are sRGB with straight alpha at the interface; compositing is
  in premultiplied, sRGB-encoded space (as browsers composite); 8-bit channels
  round half up.
- **R-2** The CPU reference renderer is byte-for-byte deterministic: the same
  display list gives the same pixels on every platform (B-level baseline).
- **R-3** Effect shaders (`OMUI_EXT_shader_effect`) have the interface of
  registry/effect.toml: source language Slang, interchange format SPIR-V. A
  backend that cannot run an effect draws the node without it and reports a
  diagnostic.

## 8. Contract ⑥: the C ABI

`include/openmui.h` is generated from registry/abi.toml.

- **C-1** Only handles and values cross it; no C++ type and no exception.
- **C-2** Every function returns `OmuiResult`. Text and structured results are
  passed to a caller-supplied sink, so no memory changes hands.
- **C-3** Structured values travel as JSON text in the wire forms of this
  specification.
- **C-4** A struct that can grow starts with `type` and `next`.
- **C-5** The header's only macros are its include guard and `OMUI_API`;
  constants are enumerators.

## 9. The bus interface `openmui.tree@0.1`

Path: `host/<h>/system/<s>/app/<app-id>/ui` (openbus §2.4). Operations:
`snapshot` (call), `changes` (topic), `find` (call: role, name glob, state,
spatial range), `act` (call, with an actor), `invoke` (call: an application
verb, with an actor; S-1 as for `act`), `undo` and `redo` (call: a scope's
history, with an actor; S-12), `history` (call: what can be undone and
redone), `transaction` (call: steps of `act`, with an actor; S-13), `wait`
(call: a node appears, a state changes, an animation is `settled`; with a
timeout), `capture` (optional pixels of a node). `act`, `invoke`, `undo`,
`redo` and `transaction` take `dry_run` (S-13).

Calls answer with envelopes: newline-delimited JSON records of kinds `phase`,
`progress`, `log`, `notice`, `error`, `data` and a final `result` with
`exitCode` (0 success, 1 failure, 2 needs an answer, 3 not ready), with the
fields and error codes of openas §6. An actor is written in openas's text form
(openas §2.4): `agent:reader;via=person:alice,agent:planner`.

## 10. Animation

- **M-1** Easing curves and the spring are defined by the formulas of
  registry/motion.toml. A value is a function of time and parameters only: an
  implementation MUST be able to evaluate any instant directly.
- **M-2** Retargeting a spring keeps position and velocity continuous.
- **M-3** A spring is settled from the first instant its envelope bounds the
  distance to the target by `delta` and the speed by `speed`; from then on it
  is exactly at the target.
- **M-4** A node in state `animating` reports its target state; `wait` for
  `settled` returns when every animation of the node has settled and never
  before.
- **M-5** With reduced motion, transitions show their target immediately and
  springs settle at once.
- **M-6** The clock is injectable: under a virtual or render clock results are
  determined by the times given (A-level: identical to the reference
  implementation's bits).

## 11. Component contracts

A component contract (`*.contract.omui.toml`) declares a component's role,
typed properties (with defaults and bindability), events, actions (with
effects), key bindings, commands, parts and motion. The Core contracts are in
registry/components and use the same format as any third-party contract.

A contract property may share its name with a property every node has
(layout, look, motion, semantics: `runtime_property_name` in
schema/view.schema.json). Its value then reaches both the component and the
runtime: a glass group's `gap` spaces its members and is the layout's gap; a
checkbox's `label` is what it shows and its accessible name. Neither shadows
the other.

## 12. Extensions and profiles

Capabilities beyond the Core profile are extensions (`OMUI_EXT_*`), queried at
run time. A profile states what a distribution provides; `full` requires the
TS/JS, Python and Lua script languages.

## 13. Conformance

| Level | Content | Status |
|---|---|---|
| S | Semantics and behaviour: contracts, keys, effects; every way of writing a view yields the same tree; every script language yields the same results | normative |
| A | Layout snapshots, animation values at given instants, text layout | normative |
| B | Rendering: CPU byte-identical, GPU within tolerance | advisory |
| C | Platform adaptation tables | informative |

Cross-language cases are written as language-neutral JSON steps and run in
each language.

## 14. The manifest

`[package.metadata.openmui]` of `mcpp.toml` describes a package: `kind`,
`contracts`, `views`, `shaders`, `behaviors`, `themes`, `id`, `spec`,
`extensions`, `profile`, `scripts`, `natives`, `deps.<lang>`, `verbs`,
`permissions`, `agent`. Other build systems MAY carry the same structure as
JSON.

`agent` is the application's cap (openas §5.1) for what is requested through
the bus: `none`, `read`, `reversible`, `irreversible` or `external` (the
default). A build MAY leave the bus interface out altogether; it then answers
nothing, to anyone.

## 15. Not addressed by 0.3

How grants are stored and carried, their expiry, and budgets (openas §8); mapping to native controls
of other frameworks beyond the Core profile; world-space UI, gamepad
navigation, XR; contracts for rich-text and code editors; the material-shader
interface of canvas node types; build-time markers (they are a tooling
convention, not a contract).

## 16. Extensions (0.3)

An implementation that reports an extension (§12) MUST meet its clauses.

### 16.1 `OMUI_EXT_canvas`

- **K-1** A canvas states as its children the items on screen and those that
  are focused or chosen; every other item is produced on demand (S-7). An
  item's id is `<canvas>/<key>`, where the key is the node's stable name in
  the scene, never derived from display text (S-9).
- **K-2** The canvas's `find` action takes `{in_rect: [x, y, w, h], near:
  [x, y], radius, name, kind, role}` (each optional) and answers the keys of
  matching items over the whole scene; with `near`, nearest first.
- **K-3** The viewport's actions — `pan_to`, `zoom_to`, `fit`, `focus_node`,
  `camera_preset` — are `read`: they change what is seen, not what is.
- **K-4** An item drawn as a live component (materialised) is still the item:
  the node states what its canvas says of it where it says nothing itself, and
  the canvas's actions on the item reach the canvas.
- **K-5** Behaviours: a press is offered to a canvas's behaviours in priority
  order until one claims it; the claimant receives the gesture until it ends;
  a press no behaviour claims moves the camera. What a behaviour makes
  possible is stated as actions on the items (`select`, `move_to`, `resize`,
  `connect`, …; `delete` is `irreversible` and states its answer, S-1).
- **K-6** A declarative behaviour (`*.behavior.omui.toml`) is a statechart:
  states, transitions on `pointer_down`, `pointer_move`, `pointer_up`,
  `pointer_cancel`, `tap` and `key` with an optional `when` expression (§4,
  V-3), operations the canvas defines, and actions it adds to items. It MUST be
  checked when it is loaded — every state, event, operation, argument count and
  name — and refused whole if anything is wrong; it never runs a script.

### 16.2 `OMUI_EXT_shader_effect` and `OMUI_EXT_backdrop_effect`

- **E-1** An effect is a fragment program run once per pixel of a layer's
  surface, written against the effect template (registry/effect.toml): it reads
  the layer's pixels, an auxiliary texture, and the frame's values — the
  surface's size and scale, the layer's rectangle, its parameters, values
  derived from them, its shapes — and nothing else. A program that binds
  anything else MUST be refused.
- **E-2** Parameters are typed numbers with a default, a minimum and a
  maximum; a value outside is clamped, a missing one is its default.
- **E-3** A built-in effect has a CPU reference; a GPU path gives the
  reference's pixels within one level of 8 bits, except where the effect
  declares a tolerance (B-level).
- **E-4** An effect that reads its backdrop gets what lies under its layer
  where the layer begins, blurred by the radius it declares with the same
  integer box blur on every renderer.
- **E-5** A layer carries up to eight shapes — rounded rectangles in its
  space — that effects read as signed distances (negative inside) and merge
  smoothly by the template's `smooth_min`. A node's layer carries the surfaces
  it paints: its own rounded box, a component's reported surfaces (a switch's
  track and thumb), and its children that paint a background or a border.
- **E-6** A node's effect is its own `pixel_effect` property, else the effect
  its style gives its type, else its role (§16.3).

### 16.3 `OMUI_EXT_style_pack`

- **ST-1** A style pack (`*.style.omui.toml`) is a theme to start from, the
  tokens it changes, and the effects it puts on the window, on component types
  and on roles (`role:<role>`). It changes pixels only: the semantic tree of
  any interface is identical under every style (S-level).
- **ST-2** A pack is checked when it is read: an unknown effect or parameter
  is refused with the nearest names; a token it changes MUST exist in the theme
  it starts from.
- **ST-3** A change of style MAY be shown as a cross-fade: during it, a frame
  shows the frame before the change fading over the new one; actions, focus
  and the semantic tree follow the new style from the change on.
- **ST-4** A part of a window MAY be in a style of its own: a theme scope. The
  universal property `theme_scope` names one the application registered (a
  theme and effect rules); the node and its descendants, and the overlays they
  open, are drawn in it. The nearest scope applies whole — its theme and its
  rules replace the window's, nothing merges — and a name not registered
  leaves the node in its parent's. A rule for the window applies to the node
  that joins the scope, and a change of a scope's style cross-fades its area
  only (ST-3). A scope changes pixels only: the semantic tree, actions and
  focus are those of one window.

### 16.4 `OMUI_EXT_frame_pacing`

- **P-1** Each frame states how often frames are wanted next: a minimum rate,
  a maximum (none: no frame is wanted; unbounded: the display's fastest), a
  preferred rate and a reason. Demands merge by the largest maximum, the
  largest minimum and the largest preferred rate, whose reason is kept.
- **P-2** Animations in progress and input that moves something (in the last
  300 ms of the runtime's clock) want the display's fastest; a pending timer
  wants frames as often as it is due, at most 60 a second; a component MAY
  hold a demand of its own for as long as it lives. With no demand, no frame
  is drawn.
- **P-3** Under an interactive clock a frame is evaluated at the time the host
  predicts it is presented, so motion keeps pace with what is seen whatever
  the refresh rate.

### 16.5 `OMUI_EXT_render_to_texture`

- **T-1** A subtree MAY be drawn into a texture and composited many times; a
  texture of a node's content MAY be shown at another resolution until one at
  the screen's is ready, never a blank frame where it was drawn before.
- **T-2** Under a virtual or render clock, work an implementation hands to
  other threads for a frame (textures) is waited for at the next frame, so the
  frames shown depend on the frames before only (M-6).

### 16.6 `OMUI_EXT_video`

- **VD-1** Under a render clock at rate r, frame n is the instant n/r; any
  frame may be rendered in any order and gives the same pixels (M-1, M-6).
