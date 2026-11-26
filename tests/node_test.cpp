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

    "properties: registered, typed, on their roles (S-16)"_test = [] {
        spec::SemanticNode n;
        n.id = "doc/title";
        n.role = spec::Role::heading;
        n.name = "Install";
        n.value = spec::Value(2);
        n.properties.set("level", spec::Value(2));
        n.properties.set("anchor", spec::Value("install"));
        expect(spec::check(n).empty());
        auto back = spec::node_from_value(spec::to_value(n));
        expect(back.has_value() && *back == n);
        expect(spec::to_value(n).json() == std::string(R"({"id":"doc/title","role":"heading","name":"Install","value":2,"properties":{"level":2,"anchor":"install"}})"));

        n.properties.set("language", spec::Value("cpp"));  // a block of code's, not a heading's
        auto v = spec::check(n);
        expect(v.size() == 1 && v[0].clause == "S-16");
        n.properties = spec::Value();
        n.properties.set("level", spec::Value("two"));
        v = spec::check(n);
        expect(v.size() == 1 && v[0].clause == "S-16") << ("a level is an integer");
        n.properties = spec::Value();
        n.properties.set("colour", spec::Value("red"));
        expect(spec::check(n).size() == 1) << ("not in the registry");
        n.properties = spec::Value();
        n.properties.set("acme.colour", spec::Value("red"));
        expect(spec::check(n).empty()) << ("a vendor's is kept");

        spec::SemanticNode cell;
        cell.id = "doc/table/cell";
        cell.role = spec::Role::cell;
        cell.properties.set("align", spec::Value("right"));
        expect(spec::check(cell).empty());
        cell.properties.set("align", spec::Value("justify"));
        expect(spec::check(cell).size() == 1) << ("an enum's value is one of its values");
    };

    "marks: ranges of code points, nested, a link's naming its child (S-17)"_test = [] {
        spec::SemanticNode n;
        n.id = "doc/p";
        n.role = spec::Role::paragraph;
        n.name = "Read the “guide” first.";  // 23 code points, 27 bytes
        n.children = {"doc/p/link:0"};
        n.marks = {{spec::MarkKind::link, 9, 16, "doc/p/link:0"}, {spec::MarkKind::emphasis, 10, 15, {}}};
        expect(spec::check(n).empty());
        auto back = spec::node_from_value(spec::to_value(n));
        expect(back.has_value() && *back == n);
        expect(spec::to_value(n).get("marks")->json() ==
               std::string(R"([{"kind":"link","start":9,"end":16,"node":"doc/p/link:0"},{"kind":"emphasis","start":10,"end":15}])"));

        expect(spec::code_points(n.name) == 23u && n.name.size() == 27u);
        expect(spec::byte_of_code_point(n.name, 9) == 9u && spec::byte_of_code_point(n.name, 10) == 12u && spec::byte_of_code_point(n.name, 99) == n.name.size());
        expect(std::string_view(n.name).substr(spec::byte_of_code_point(n.name, 10), spec::byte_of_code_point(n.name, 15) - spec::byte_of_code_point(n.name, 10)) == "guide");
        n.marks = {{spec::MarkKind::strong, 0, 24, {}}};
        expect(spec::check(n).size() == 1 && spec::check(n)[0].clause == "S-17") << ("past the last code point, though inside the bytes");
        n.marks = {{spec::MarkKind::strong, 0, 23, {}}};
        expect(spec::check(n).empty());
        n.marks = {{spec::MarkKind::emphasis, 5, 8, {}}, {spec::MarkKind::strong, 0, 4, {}}};
        expect(spec::check(n).size() == 1) << ("out of order");
        n.marks = {{spec::MarkKind::emphasis, 0, 8, {}}, {spec::MarkKind::strong, 4, 12, {}}};
        expect(spec::check(n).size() == 1) << ("crossing");
        n.marks = {{spec::MarkKind::emphasis, 0, 8, {}}, {spec::MarkKind::strong, 0, 4, {}}, {spec::MarkKind::code, 8, 12, {}}};
        expect(spec::check(n).empty()) << ("nested, then beside");
        n.marks = {{spec::MarkKind::link, 0, 4, "elsewhere"}};
        expect(spec::check(n).size() == 1) << ("a link names a child");
        n.marks = {{spec::MarkKind::code, 0, 4, "doc/p/link:0"}};
        expect(spec::check(n).size() == 1) << ("no other mark has a node");

        n.marks = {{spec::MarkKind::emphasis, 0, 1, {}}, {spec::MarkKind::code, 1, 2, {}}};
        spec::Value wire = spec::to_value(n);
        for (auto& [key, listed] : *wire.map())
            if (key == "marks") (*listed.list())[0].set("kind", "sparkle");
        const auto read = spec::node_from_value(wire);
        expect(read.has_value() && read->marks.size() == 1 && read->marks[0].kind == spec::MarkKind::code) << ("a kind not known is ignored");
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
