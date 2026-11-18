// The Core component contracts (spec §11): present, consistent with the
// registry, and complete enough for agents to operate them.
import std;
import openmui.spec;
import boost.ut;

using namespace boost::ut::literals;
using boost::ut::cfg;
using boost::ut::expect;
namespace spec = omui::spec;

int main() {

    "the Core profile's contracts exist"_test = [] {
        for (auto name : {"text", "image", "button", "toggle", "checkbox", "radio", "slider", "text_box", "select",
                          "menu", "menu_item", "tabs", "tab", "dialog", "tooltip", "scroll_view", "list", "progress"})
            expect(spec::catalog::find(name) != nullptr) << (name);
    };

    "effects are never weaker than the registry's"_test = [] {
        for (const spec::ContractInfo* c : spec::catalog::core)
            for (const auto& a : c->actions)
                if (auto common = spec::parse_action(a.name))
                    expect(spec::tier(a.effect) >= spec::tier(spec::actions[static_cast<std::size_t>(*common)].effect)) << (std::format("{}.{}", c->name, a.name));
    };

    "interactive contracts offer actions; keys name actions or commands"_test = [] {
        for (const spec::ContractInfo* c : spec::catalog::core) {
            if (spec::interactive(c->role)) expect(!c->actions.empty()) << (c->name);
            for (const auto& k : c->keys) {
                auto base = k.target.substr(0, k.target.find('('));
                bool known = c->action(base) != nullptr ||
                             std::ranges::any_of(c->commands, [&](const auto& cmd) { return cmd.name == base; });
                expect(known) << (std::format("{}: {}", c->name, k.target));
            }
        }
    };

    "slider: properties, defaults, events, actions"_test = [] {
        const spec::ContractInfo& slider = spec::catalog::slider;
        expect(slider.role == spec::Role::slider);
        const spec::PropInfo* value = slider.prop("value");
        expect(value && value->type == spec::PropType::f64 && value->bindable);
        expect(value->default_value.kind == spec::DefaultValue::Kind::real && value->default_value.d == 0.0);
        expect(slider.prop("max")->default_value.d == 100.0);
        expect(slider.event("changed")->coalesce_per_frame);
        expect(slider.action("set_value")->args.size() == 1);
        expect(slider.action("activate") == nullptr);
    };

    "choice properties default to one of their values"_test = [] {
        for (const spec::ContractInfo* c : spec::catalog::core)
            for (const auto& p : c->props)
                if (p.type == spec::PropType::choice)
                    expect(std::ranges::find(p.values, p.default_value.s) != p.values.end()) << (std::format("{}.{}", c->name, p.name));
    };

    "motion parts name registry tokens"_test = [] {
        for (const spec::ContractInfo* c : spec::catalog::core)
            for (const auto& m : c->motion)
                expect(m.token == "decay" || spec::find_motion_token(m.token) != nullptr) << (std::format("{}.{}", c->name, m.part));
    };

    return static_cast<int>(cfg<>.run({.report_errors = true}));
}
