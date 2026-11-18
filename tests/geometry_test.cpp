import std;
import openmui.spec;
import boost.ut;

using namespace boost::ut::literals;
using boost::ut::cfg;
using boost::ut::expect;
namespace spec = omui::spec;

int main() {

    "rounding is half up, negatives included"_test = [] {
        expect((spec::snap(0.5f, 1)) == (1));
        expect((spec::snap(0.49f, 1)) == (0));
        expect((spec::snap(-0.5f, 1)) == (0));
        expect((spec::snap(-0.51f, 1)) == (-1));
        expect((spec::snap(10.25f, 2)) == (21));
        expect((spec::snap(1.0f / 3.0f, 3)) == (1));
    };

    "rectangles that share an edge share it at every scale"_test = [] {
        for (float scale : {1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f}) {
            float x = 0;
            spec::PixelRect prev{};
            for (int i = 0; i < 200; ++i) {
                float w = 7.3f + static_cast<float>(i % 7) * 0.37f;
                auto r = spec::snap(spec::Rect{x, 0, w, 10}, scale);
                if (i > 0) expect((r.x) == (prev.x + prev.width));
                prev = r;
                x += w;
            }
        }
    };

    "rectangle operations"_test = [] {
        spec::Rect a{0, 0, 10, 10}, b{5, 5, 10, 10};
        expect(a.intersects(b));
        expect(a.intersection(b) == spec::Rect{5, 5, 5, 5});
        expect(a.united(b) == spec::Rect{0, 0, 15, 15});
        expect(a.contains({0, 0}) && !a.contains({10, 5}));
        expect(!a.intersects(spec::Rect{10, 0, 5, 5})) << ("touching is not intersecting");
        expect(a.inset(spec::Insets::all(2)) == spec::Rect{2, 2, 6, 6});
        expect(spec::Rect{}.united(a) == a);
    };

    "colours"_test = [] {
        auto c = spec::Color::parse("#ff800080");
        expect(c.has_value());
        auto p = c->premultiplied8();
        expect((int(p[0])) == (128));
        expect((int(p[1])) == (64));
        expect((int(p[3])) == (128));
        expect(spec::Color::parse("#fff") == spec::Color::rgba8(255, 255, 255));
        expect(!spec::Color::parse("#ffff0").has_value());
        expect(spec::Color::hex(0x336699) == spec::Color::rgba8(0x33, 0x66, 0x99));
    };

    return static_cast<int>(cfg<>.run({.report_errors = true}));
}
