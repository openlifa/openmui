// openmui.spec — the specification's value types (spec §2–§10).
//
// Standard library only: a second implementation of openmui, a test suite or
// a tool reads the same tables and types without taking the runtime. The
// registry and catalog partitions are generated from registry/
// (`omui-gen registry registry --out .`); the rest is written here.
//
//   :types     argument and property types, contract defaults
//   :registry  roles, states, effects, origins, actions, extensions, profiles,
//              motion tokens, the expression language          (generated)
//   :contract  a component contract as data
//   :catalog   the Core component contracts                    (generated)
//   :geometry  points, sizes, rectangles, colours; normative rounding
//   :id        node ids, structural ids (S-8), glob matching
//   :value     the value model of every boundary
//   :node      the semantic node, clause checks, wire form, deltas
//   :envelope  records, exit and error codes, actors
export module openmui.spec;

export import :types;
export import :registry;
export import :contract;
export import :catalog;
export import :geometry;
export import :id;
export import :value;
export import :node;
export import :envelope;
