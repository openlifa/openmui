import std;
import openmui.spec;
import boost.ut;

using namespace boost::ut::literals;
using boost::ut::cfg;
using boost::ut::expect;
namespace spec = omui::spec;

int main() {

    "needs-answer is an error record and exit code 2 (S-2)"_test = [] {
        spec::Envelope e;
        spec::Value options;
        options.set("answer", spec::Value::List{"yes"});
        e.error(spec::error_code::needs_answer, "confirm deleting the note", options).result(spec::exit_code::needs_answer);
        expect((e.exit_code().value_or(-1)) == (2));
        expect((e.ndjson()) == (std::string(
            "{\"kind\":\"error\",\"code\":\"needs-answer\",\"text\":\"confirm deleting the note\",\"answer\":[\"yes\"]}\n"
            "{\"kind\":\"result\",\"exitCode\":2}\n")));
    };

    "data and the last data record"_test = [] {
        spec::Envelope e;
        e.phase("find").data(spec::Value(1)).data(spec::Value(2)).result(0);
        expect(e.data_value() && e.data_value()->as_int() == 2);
        expect(e.records().size() == 4);
    };

    "actors in openas text form"_test = [] {
        auto a = spec::Actor::parse("agent:planner;via=person:alice");
        expect(a.has_value());
        expect((a->kind) == (std::string("agent")));
        expect((a->name) == (std::string("planner")));
        expect(a->via.size() == 1 && a->via[0] == "person:alice");
        expect((a->text()) == (std::string("agent:planner;via=person:alice")));
        expect(spec::Actor::parse("agent:x;budget=3").has_value()) << ("unknown keys are skipped");
        expect(!spec::Actor::parse("planner").has_value());
        expect(!spec::Actor::parse("agent:x;via=alice").has_value());
        expect(!spec::Actor::parse("script:x").has_value()) << ("openas's four kinds");
        auto chain = spec::Actor::parse("agent:reader;via=person:alice,agent:planner");
        expect(chain && chain->via.size() == 2 && chain->via[1] == "agent:planner");
        expect(chain && chain->text() == "agent:reader;via=person:alice,agent:planner");
        expect(!spec::Actor::parse("agent:x;via=person:a;via=person:b").has_value()) << ("via once");
        expect(spec::Actor::parse("agent").has_value() && spec::Actor::parse("agent")->text() == "agent");
    };

    return static_cast<int>(cfg<>.run({.report_errors = true}));
}
