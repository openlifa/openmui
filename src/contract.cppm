// openmui.spec:contract — a component contract as data (component-binding §2.2).
//
// The Core contracts (registry/components) and every component package's own
// contracts become these tables, through the same generator; nothing about an
// official component is privileged.
export module openmui.spec:contract;

import std;
import :types;
import :registry;

export namespace omui::spec {

struct PropInfo {
    std::string_view                  name;
    PropType                          type;
    std::span<const std::string_view> values;  // a choice's values
    DefaultValue                      default_value;
    bool                              bindable;
    std::string_view                  doc;
};

struct EventInfo {
    std::string_view         name;
    std::span<const ArgInfo> args;
    bool                     coalesce_per_frame;
    std::string_view         doc;
};

// An action a contract offers, with the effect this component declares for it
// (never weaker than the registry's default).
struct ActionSpec {
    std::string_view         name;
    Effect                   effect;
    std::span<const ArgInfo> args;
    std::string_view         doc;
    std::string_view         answer;  // what confirms it (S-1); empty when the effect needs none
};

// A key chord ("Right|Up", "Alt+Down") bound to an action or a command, with
// optional arguments ("set_value(min)").
struct KeyInfo {
    std::string_view chord;
    std::string_view target;
};

// A key-bound behaviour that is not a semantic action (not offered to agents).
struct CommandInfo {
    std::string_view name;
    std::string_view doc;
};

// The motion token a part animates with.
struct MotionInfo {
    std::string_view part;
    std::string_view token;
};

struct ContractInfo {
    std::string_view                  name;
    Role                              role;
    std::string_view                  spec;
    std::span<const PropInfo>         props;
    std::span<const EventInfo>        events;
    std::span<const ActionSpec>       actions;
    std::span<const KeyInfo>          keys;
    std::span<const CommandInfo>      commands;
    std::span<const std::string_view> parts;
    std::span<const MotionInfo>       motion;
    std::string_view                  doc;

    [[nodiscard]] constexpr const PropInfo* prop(std::string_view n) const noexcept {
        for (const PropInfo& p : props)
            if (p.name == n) return &p;
        return nullptr;
    }
    [[nodiscard]] constexpr const EventInfo* event(std::string_view n) const noexcept {
        for (const EventInfo& e : events)
            if (e.name == n) return &e;
        return nullptr;
    }
    [[nodiscard]] constexpr const ActionSpec* action(std::string_view n) const noexcept {
        for (const ActionSpec& a : actions)
            if (a.name == n) return &a;
        return nullptr;
    }
};

}  // namespace omui::spec
