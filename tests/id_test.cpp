import std;
import openmui.spec;
import boost.ut;

using namespace boost::ut::literals;
using boost::ut::cfg;
using boost::ut::expect;
namespace spec = omui::spec;

// Explicit ids are checked at compile time; an invalid literal does not compile
// (a literal that is not an id is refused by `consteval`).
constexpr spec::IdLiteral delete_id = "notes/delete";
static_assert(delete_id.text == "notes/delete");

int main() {

    "segments and paths"_test = [] {
        expect(spec::valid_segment("inc"));
        expect(spec::valid_segment("column:0"));
        expect(spec::valid_explicit_segment("notes.delete-1_x"));
        expect(!spec::valid_explicit_segment("column:0")) << ("':' is reserved for structural ids");
        expect(!spec::valid_segment(""));
        expect(!spec::valid_segment(".."));
        expect(!spec::valid_segment("a b"));
        expect(!spec::valid_segment("删除")) << ("S-9: no display text in ids");
        expect(!spec::valid_segment(std::string(129, 'a')));
        expect(spec::valid_path("app/column:0/inc"));
        expect(!spec::valid_path("app//inc"));
        expect(!spec::valid_path("/app"));
    };

    "structural ids (S-8)"_test = [] {
        expect((spec::structural_segment("text", 3)) == (std::string("text:3")));
        expect(spec::is_structural("column:0") && !spec::is_structural("inc"));
    };

    "paths"_test = [] {
        auto p = spec::Path::parse("app/column:0/inc");
        expect(p.has_value());
        expect((p->last()) == (std::string_view("inc")));
        expect((p->parent().str()) == (std::string("app/column:0")));
        expect((p->depth()) == (std::size_t{3}));
        expect(p->within(*spec::Path::parse("app")));
        expect(!p->within(*spec::Path::parse("ap")));
        expect((spec::Path{}.child("a").child("b").str()) == (std::string("a/b")));
        expect(!spec::Path::parse("bad path").has_value());
    };

    "glob matching"_test = [] {
        expect(spec::match_path("notes/*", "notes/delete"));
        expect(!spec::match_path("notes/*", "notes/list/item:0"));
        expect(spec::match_path("**/delete", "delete"));
        expect(spec::match_path("**/delete", "a/b/delete"));
        expect(spec::match_path("notes/**", "notes/list/item:0"));
        expect(spec::match_path("*/item:?", "list/item:7"));
        expect(!spec::match_path("*/item:?", "list/item:17"));
        expect(spec::match_text("删除*", "删除笔记"));
        expect(spec::match_text("*TODO*", "a TODO b"));
        expect(!spec::match_text("a*b", "a"));
        expect(spec::match_text(std::string(40, '*') + "x", std::string(10000, 'a') + "x"));
    };

    return static_cast<int>(cfg<>.run({.report_errors = true}));
}
