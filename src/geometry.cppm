// openmui.spec:geometry — points, sizes, rectangles in logical pixels, colours,
// and the normative rounding to physical pixels (spec §4, A-level).
export module openmui.spec:geometry;

import std;

export namespace omui::spec {

struct Point {
    float x = 0, y = 0;
    friend constexpr bool operator==(const Point& a, const Point& b) noexcept { return a.x == b.x && a.y == b.y; }
    friend constexpr Point operator+(Point a, Point b) noexcept { return {a.x + b.x, a.y + b.y}; }
    friend constexpr Point operator-(Point a, Point b) noexcept { return {a.x - b.x, a.y - b.y}; }
};

struct Size {
    float width = 0, height = 0;
    friend constexpr bool operator==(const Size& a, const Size& b) noexcept { return a.width == b.width && a.height == b.height; }
    [[nodiscard]] constexpr bool empty() const noexcept { return !(width > 0 && height > 0); }
};

struct Insets {
    float top = 0, right = 0, bottom = 0, left = 0;
    [[nodiscard]] static constexpr Insets all(float v) noexcept { return {v, v, v, v}; }
    [[nodiscard]] static constexpr Insets symmetric(float vertical, float horizontal) noexcept {
        return {vertical, horizontal, vertical, horizontal};
    }
    [[nodiscard]] constexpr float horizontal() const noexcept { return left + right; }
    [[nodiscard]] constexpr float vertical() const noexcept { return top + bottom; }
    friend constexpr bool operator==(const Insets& a, const Insets& b) noexcept {
        return a.top == b.top && a.right == b.right && a.bottom == b.bottom && a.left == b.left;
    }
};

struct Rect {
    float x = 0, y = 0, width = 0, height = 0;

    [[nodiscard]] static constexpr Rect from(Point origin, Size size) noexcept {
        return {origin.x, origin.y, size.width, size.height};
    }
    [[nodiscard]] static constexpr Rect ltrb(float l, float t, float r, float b) noexcept { return {l, t, r - l, b - t}; }

    [[nodiscard]] constexpr float left() const noexcept { return x; }
    [[nodiscard]] constexpr float top() const noexcept { return y; }
    [[nodiscard]] constexpr float right() const noexcept { return x + width; }
    [[nodiscard]] constexpr float bottom() const noexcept { return y + height; }
    [[nodiscard]] constexpr Point origin() const noexcept { return {x, y}; }
    [[nodiscard]] constexpr Size size() const noexcept { return {width, height}; }
    [[nodiscard]] constexpr Point center() const noexcept { return {x + width / 2, y + height / 2}; }
    [[nodiscard]] constexpr bool empty() const noexcept { return !(width > 0 && height > 0); }

    // Half-open: the right and bottom edges are outside.
    [[nodiscard]] constexpr bool contains(Point p) const noexcept {
        return p.x >= x && p.y >= y && p.x < right() && p.y < bottom();
    }
    [[nodiscard]] constexpr bool intersects(const Rect& o) const noexcept {
        return x < o.right() && o.x < right() && y < o.bottom() && o.y < bottom();
    }
    [[nodiscard]] constexpr Rect intersection(const Rect& o) const noexcept {
        float l = std::max(x, o.x), t = std::max(y, o.y);
        float r = std::min(right(), o.right()), b = std::min(bottom(), o.bottom());
        return r > l && b > t ? ltrb(l, t, r, b) : Rect{l, t, 0, 0};
    }
    // The smallest rectangle covering both; an empty rectangle contributes nothing.
    [[nodiscard]] constexpr Rect united(const Rect& o) const noexcept {
        if (empty()) return o;
        if (o.empty()) return *this;
        return ltrb(std::min(x, o.x), std::min(y, o.y), std::max(right(), o.right()), std::max(bottom(), o.bottom()));
    }
    [[nodiscard]] constexpr Rect inset(const Insets& i) const noexcept {
        return {x + i.left, y + i.top, std::max(0.0f, width - i.horizontal()), std::max(0.0f, height - i.vertical())};
    }
    [[nodiscard]] constexpr Rect offset(Point d) const noexcept { return {x + d.x, y + d.y, width, height}; }

    friend constexpr bool operator==(const Rect& a, const Rect& b) noexcept {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
    }
};

// A pixel rectangle in physical pixels (integers).
struct PixelRect {
    std::int32_t x = 0, y = 0, width = 0, height = 0;
    friend constexpr bool operator==(const PixelRect& a, const PixelRect& b) noexcept {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
    }
};

// ── rounding (normative) ───────────────────────────────────────────────────
//
// A logical coordinate v at scale s is the physical coordinate
// floor(v·s + 0.5): round half up, the same for negative coordinates, computed
// in double. Rectangles snap their EDGES, never their sizes, so two rectangles
// that share an edge in logical pixels share it in physical pixels and no gap
// or overlap appears between them at any scale.
[[nodiscard]] constexpr std::int32_t snap(float v, float scale) noexcept {
    double p = static_cast<double>(v) * static_cast<double>(scale) + 0.5;
    double f = static_cast<double>(static_cast<std::int64_t>(p));
    if (f > p) f -= 1.0;  // floor for negatives
    return static_cast<std::int32_t>(f);
}

[[nodiscard]] constexpr PixelRect snap(const Rect& r, float scale) noexcept {
    std::int32_t l = snap(r.x, scale), t = snap(r.y, scale);
    std::int32_t rr = snap(r.right(), scale), b = snap(r.bottom(), scale);
    return {l, t, std::max(0, rr - l), std::max(0, b - t)};
}

// ── colour ─────────────────────────────────────────────────────────────────

// An sRGB colour with straight alpha, channels in [0, 1]. The render interface
// (spec §7) composites in premultiplied sRGB-encoded space, as browsers do.
struct Color {
    float r = 0, g = 0, b = 0, a = 1;

    [[nodiscard]] static constexpr Color rgba8(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }
    [[nodiscard]] static constexpr Color hex(std::uint32_t rgb, float alpha = 1.0f) noexcept {
        return {((rgb >> 16) & 0xff) / 255.0f, ((rgb >> 8) & 0xff) / 255.0f, (rgb & 0xff) / 255.0f, alpha};
    }
    // #rgb, #rgba, #rrggbb, #rrggbbaa
    [[nodiscard]] static constexpr std::optional<Color> parse(std::string_view text) noexcept {
        if (!text.empty() && text.front() == '#') text.remove_prefix(1);
        auto digit = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        std::array<int, 4> v{0, 0, 0, 255};
        if (text.size() == 3 || text.size() == 4) {
            for (std::size_t i = 0; i < text.size(); ++i) {
                int d = digit(text[i]);
                if (d < 0) return std::nullopt;
                v[i] = d * 17;
            }
        } else if (text.size() == 6 || text.size() == 8) {
            for (std::size_t i = 0; i < text.size() / 2; ++i) {
                int hi = digit(text[2 * i]), lo = digit(text[2 * i + 1]);
                if (hi < 0 || lo < 0) return std::nullopt;
                v[i] = hi * 16 + lo;
            }
        } else {
            return std::nullopt;
        }
        return rgba8(static_cast<std::uint8_t>(v[0]), static_cast<std::uint8_t>(v[1]), static_cast<std::uint8_t>(v[2]),
                     static_cast<std::uint8_t>(v[3]));
    }

    [[nodiscard]] constexpr Color with_alpha(float alpha) const noexcept { return {r, g, b, alpha}; }

    // 8-bit premultiplied, rounded half up: the pixel format of the render
    // interface and the CPU reference renderer.
    [[nodiscard]] constexpr std::array<std::uint8_t, 4> premultiplied8() const noexcept {
        auto q = [](float v) {
            float c = v < 0 ? 0 : (v > 1 ? 1 : v);
            return static_cast<std::uint8_t>(static_cast<int>(c * 255.0f + 0.5f));
        };
        float alpha = a < 0 ? 0 : (a > 1 ? 1 : a);
        return {q(r * alpha), q(g * alpha), q(b * alpha), q(alpha)};
    }

    friend constexpr bool operator==(const Color& x, const Color& y) noexcept {
        return x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a;
    }
};

}  // namespace omui::spec
