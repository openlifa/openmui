import std;
import openmui.spec;
import boost.ut;

using namespace boost::ut::literals;
using boost::ut::cfg;
using boost::ut::expect;
namespace spec = omui::spec;

namespace {

spec::SemanticNode delete_button() {
    spec::SemanticNode n;
    n.id = "notes/delete";
    n.role = spec::Role::button;
    n.name = "Delete";
    n.states.set(spec::State::focusable);
    n.actions.push_back({.name = "activate", .effect = spec::Effect::irreversible, .verb = "note.delete", .answer = "yes"});
    n.bounds = spec::Rect{10, 20, 80, 32};
    return n;
}

}  // namespace

int main() {

    "clauses S-1 and S-6"_test = [] {
        auto n = delete_button();
        expect(spec::check(n).empty());
        n.actions[0].answer.clear();
        auto v = spec::check(n);
        expect(v.size() == 1 && v[0].clause == "S-1");
        n = delete_button();
        n.name.clear();
        v = spec::check(n);
        expect(v.size() == 1 && v[0].clause == "S-6");
        n.role = spec::Role::text;
        expect(spec::check(n).empty()) << ("static text needs no name");
        n.id = "bad id";
        expect(spec::check(n)[0].clause == "S-8");
    };

    "wire form round-trips"_test = [] {
        auto n = delete_button();
        n.value = spec::Value(42.5);
        n.origin = spec::Origin::user_content;
        n.children = {"notes/delete/icon"};
        n.states.set(spec::State::animating);
        auto back = spec::node_from_value(spec::to_value(n));
        expect(back.has_value() && *back == n);
    };

    "wire form is the documented JSON"_test = [] {
        auto json = spec::to_value(delete_button()).json();
        expect((json) == (std::string(R"({"id":"notes/delete","role":"button","name":"Delete","state":["focusable"],)"
                                   R"("actions":[{"name":"activate","effect":"irreversible","verb":"note.delete","answer":"yes"}],)"
                                   R"("bounds":[10,20,80,32]})")));
    };

    "an unknown effect is read as external (openas §3.1)"_test = [] {
        auto v = spec::to_value(delete_button());
        (*v.get("actions")->list())[0];
        spec::Value patched = v;
        auto& actions = *patched.map();
        for (auto& [k, val] : actions)
            if (k == "actions") (*val.list())[0].set("effect", "teleport");
        auto n = spec::node_from_value(patched);
        expect(n && n->actions[0].effect == spec::Effect::external);
    };

    "deltas"_test = [] {
        spec::SemanticDelta d;
        d.revision = 7;
        expect(d.empty());
        d.removed.push_back("a");
        expect((spec::to_value(d).json()) == (std::string(R"({"revision":7,"added":[],"removed":["a"],"updated":[]})")));
    };

    "values"_test = [] {
        spec::Value v;
        v.set("b", true).set("x", 1);
        v.set("n", spec::Value::List{1, 2.5, "s", nullptr});
        v.set("b", false);
        expect((v.json()) == (std::string(R"({"b":false,"x":1,"n":[1,2.5,"s",null]})")));
        expect(v.get("missing") == nullptr);
        expect((spec::Value(1e300 * 1e300).json()) == (std::string("null")));
        expect((spec::Value("a\"b\n").json()) == (std::string(R"("a\"b\n")")));
        expect((spec::Value(3).as_double()) == (3.0));
        expect((spec::Value(3.9).as_int()) == (std::int64_t{3}));
        expect(spec::Value("x").as_int(-1) == -1);
    };

    return static_cast<int>(cfg<>.run({.report_errors = true}));
}
