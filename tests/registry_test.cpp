// The generated registry tables: names round-trip, the clause helpers hold,
// and the tables are usable in constant expressions.
import std;
import openmui.spec;
import boost.ut;

using namespace boost::ut::literals;
using boost::ut::cfg;
using boost::ut::expect;
namespace spec = omui::spec;

static_assert(spec::parse_role("button") == spec::Role::button);
static_assert(spec::name(spec::Role::switch_) == "switch");
static_assert(spec::requires_answer(spec::Effect::irreversible));
static_assert(!spec::requires_answer(spec::Effect::read));
static_assert(spec::interactive(spec::Role::slider) && !spec::interactive(spec::Role::text));
// Found, and by name: a constant expression with or without the sanitizers
// (which make comparing an object's address with nullptr not constant).
static_assert(spec::catalog::find("slider")->name == "slider");

int main() {

    "every enumeration round-trips through its name"_test = [] {
        for (std::size_t i = 0; i < spec::roles.size(); ++i)
            expect(spec::parse_role(spec::roles[i].name) == static_cast<spec::Role>(i)) << (spec::roles[i].name);
        for (std::size_t i = 0; i < spec::states.size(); ++i)
            expect(spec::parse_state(spec::states[i].name) == static_cast<spec::State>(i)) << (spec::states[i].name);
        for (std::size_t i = 0; i < spec::effects.size(); ++i)
            expect(spec::parse_effect(spec::effects[i].name) == static_cast<spec::Effect>(i));
        for (std::size_t i = 0; i < spec::actions.size(); ++i)
            expect(spec::parse_action(spec::actions[i].name) == static_cast<spec::Action>(i));
        expect(!spec::parse_role("Button").has_value()) << ("names are case-sensitive");
        expect(spec::states.size() <= 32) << ("states fit a 32-bit set");
    };

    "effects: tiers order the classes, unknown is strongest"_test = [] {
        expect(spec::tier(spec::Effect::read) < spec::tier(spec::Effect::reversible));
        expect(spec::tier(spec::Effect::reversible) < spec::tier(spec::Effect::irreversible));
        expect(spec::tier(static_cast<spec::Effect>(200)) > spec::tier(spec::Effect::external));
        expect(spec::requires_answer(static_cast<spec::Effect>(200))) << ("an unknown effect needs an answer");
    };

    "actions carry their arguments"_test = [] {
        const auto& set_value = spec::actions[static_cast<std::size_t>(spec::Action::set_value)];
        expect(set_value.args.size() == 1 && set_value.args[0].name == "value");
        expect(spec::actions[static_cast<std::size_t>(spec::Action::activate)].effect == spec::Effect::reversible);
    };

    "extensions and their requirements exist"_test = [] {
        for (const auto& e : spec::extensions) {
            expect(e.name.starts_with("OMUI_EXT_")) << (e.name);
            for (auto r : e.requires_) expect(spec::find_extension(r) != nullptr) << (r);
        }
    };

    "motion tokens are timed curves or springs"_test = [] {
        for (const auto& m : spec::motion_tokens) expect(m.duration_ms >= 0 || m.stiffness > 0) << (m.name);
        expect(spec::find_motion_token("motion.fast")->duration_ms == 120);
    };

    "the full profile requires the three script languages"_test = [] {
        const auto& full = spec::profiles[0];
        expect((full.name) == (std::string_view("full")));
        expect(full.scripts.size() == 3);
    };

    "expression functions have combinators where code builds them"_test = [] {
        expect((spec::find_function("format")->combinators.cpp) == (std::string_view("fmt")));
        expect((spec::find_function("format")->combinators.python) == (std::string_view("fmt")));
        expect(spec::find_function("nope") == nullptr);
    };

    return static_cast<int>(cfg<>.run({.report_errors = true}));
}
