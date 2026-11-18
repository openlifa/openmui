// openmui.spec:node — contract ①, the semantic tree (spec §3).
//
// The semantic tree is the single source of truth: pixels, accessibility,
// agents and tests are renderings of it. A node is a value: it holds no
// pointer and refers to its children by id.
export module openmui.spec:node;

import std;
import :registry;
import :geometry;
import :value;

export namespace omui::spec {

// The state bits of a node.
struct StateSet {
    std::uint32_t bits = 0;

    [[nodiscard]] constexpr bool has(State s) const noexcept { return (bits >> static_cast<unsigned>(s)) & 1u; }
    constexpr StateSet& set(State s, bool on = true) noexcept {
        if (on) bits |= 1u << static_cast<unsigned>(s);
        else bits &= ~(1u << static_cast<unsigned>(s));
        return *this;
    }
    [[nodiscard]] constexpr StateSet with(State s) const noexcept { return StateSet{bits | (1u << static_cast<unsigned>(s))}; }
    [[nodiscard]] constexpr bool empty() const noexcept { return bits == 0; }
    friend constexpr bool operator==(StateSet a, StateSet b) noexcept { return a.bits == b.bits; }
};

// An action a node offers right now.
struct ActionDesc {
    std::string name;
    Effect      effect = Effect::reversible;
    std::string verb;    // the application verb it invokes, if any
    std::string answer;  // the answer that confirms it (S-1)
    std::vector<std::pair<std::string, std::string>> args;  // name, type
    // Only the person at the machine performs it, through the host's own
    // input; a request over the bus is refused, whatever its actor (S-10).
    bool        person = false;

    // Written out: GCC 16.1 crashes on defaulted comparisons of module types.
    friend bool operator==(const ActionDesc& a, const ActionDesc& b) noexcept {
        return a.name == b.name && a.effect == b.effect && a.verb == b.verb && a.answer == b.answer && a.args == b.args && a.person == b.person;
    }
};

struct SemanticNode {
    std::string             id;
    Role                    role = Role::generic;
    std::string             name;
    std::string             description;
    Value                   value;
    Origin                  origin = Origin::app;
    StateSet                states;
    std::vector<ActionDesc> actions;
    std::optional<Rect>     bounds;  // logical pixels; the target state while animating
    std::vector<std::string> children;

    [[nodiscard]] const ActionDesc* action(std::string_view n) const noexcept {
        for (const ActionDesc& a : actions)
            if (a.name == n) return &a;
        return nullptr;
    }

    friend bool operator==(const SemanticNode& a, const SemanticNode& b) noexcept {
        return a.id == b.id && a.role == b.role && a.name == b.name && a.description == b.description &&
               a.value == b.value && a.origin == b.origin && a.states == b.states && a.actions == b.actions &&
               a.bounds == b.bounds && a.children == b.children;
    }
};

// A violation of a clause of spec §3.2, found by `check`.
struct Violation {
    std::string clause;  // "S-1"
    std::string id;
    std::string message;
};

// The clauses that a single node can violate on its own: S-1 (an
// irreversible or external action states its answer), S-6 (an interactive
// node has a name), and the id rules (a valid path).
[[nodiscard]] std::vector<Violation> check(const SemanticNode& node);

// Wire form (bus interface `openmui.tree@0.1`): a map with the fields above,
// enumerations by name, absent fields omitted.
[[nodiscard]] Value to_value(const SemanticNode& node);
[[nodiscard]] std::optional<SemanticNode> node_from_value(const Value& v);

// S-5: changes come as ordered deltas under a revision number.
struct SemanticDelta {
    std::uint64_t             revision = 0;
    std::vector<SemanticNode> added;
    std::vector<std::string>  removed;
    std::vector<SemanticNode> updated;

    [[nodiscard]] bool empty() const noexcept { return added.empty() && removed.empty() && updated.empty(); }
};

[[nodiscard]] Value to_value(const SemanticDelta& delta);

}  // namespace omui::spec
