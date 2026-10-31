# openmui

A user interface specified as one semantic tree — what is on the screen, what
each thing is, what it can do and what doing it means — of which pixels,
accessibility, agents and tests are renderings.

[![CI](https://github.com/openlifa/openmui/actions/workflows/ci.yml/badge.svg?event=pull_request)](https://github.com/openlifa/openmui/actions/workflows/ci.yml)
[![spec](https://img.shields.io/badge/spec-0.3%20draft-blue)](SPEC.md)
[![mcpp-index](https://img.shields.io/badge/mcpp--index-openlifa.openmui%200.3.0-orange)](https://github.com/mcpplibs/mcpp-index)
[![code](https://img.shields.io/badge/code-Apache--2.0-green)](LICENSE)
[![text](https://img.shields.io/badge/text-CC%20BY--SA%204.0-lightgrey)](LICENSE-SPEC)

| | |
|---|---|
| [SPEC.md](SPEC.md) | the specification (normative) |
| [registry/](registry/) | its source of truth: roles, states, effects, actions, motion, expressions, the C ABI, the Core components |
| [include/openmui.h](include/openmui.h) | the C ABI |
| [src/](src/) | its C++ module: `openmui.spec`, the value types |
| [conformance/](conformance/) | the conformance suite, levels S, A, B, C |
| [docs/](docs/), [schema/](schema/), [stubs/](stubs/) | the registry as tables; the view-document, input-event and scenario schemas; TypeScript, Python and Lua stubs |
| [tests/](tests/) | the module's and the header's tests |

`include/`, `src/generated/`, `docs/`, `stubs/` and the view schema are
generated from `registry/`; edit the registry, not them.

```toml
[dependencies]
openlifa.openmui = "0.3.0"
```

```cpp
import openmui.spec;

auto actor = omui::spec::Actor::parse("agent:reader;via=person:alice");
const auto& slider = omui::spec::catalog::slider;   // a Core component's contract
```

Builds on [openas](https://github.com/openlifa/openas) for actors, effects,
grants and the envelope.

## License

Code: [Apache-2.0](LICENSE). The specification text: [CC BY-SA 4.0](LICENSE-SPEC).
Copyright 2026 OpenLifa & Sunrisepeak.
