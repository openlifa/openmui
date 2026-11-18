// openmui.spec:id — node ids: stable paths, never derived from display text.
//
//   S-8  in retained mode a node without an explicit id gets a structural id,
//        "<type>:<index among siblings of that type>", e.g. column:0/text:1;
//        every way of writing a retained view uses the same rule
//   S-9  an id never depends on display text (labels, translated strings):
//        a language switch must not change ids
//
// A path is segments joined by '/'. A segment is 1..128 characters of
// [A-Za-z0-9_.:-]; ':' appears only in structural segments, so an explicit id
// can never collide with an automatic one.
export module openmui.spec:id;

import std;

export namespace omui::spec {

inline constexpr std::size_t max_segment_length = 128;

[[nodiscard]] constexpr bool segment_char(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' ||
           c == '.' || c == ':';
}

// A segment an author may write: no ':' (reserved for structural ids).
[[nodiscard]] constexpr bool valid_explicit_segment(std::string_view s) noexcept {
    if (s.empty() || s.size() > max_segment_length || s == "." || s == "..") return false;
    for (char c : s)
        if (!segment_char(c) || c == ':') return false;
    return true;
}

[[nodiscard]] constexpr bool valid_segment(std::string_view s) noexcept {
    if (s.empty() || s.size() > max_segment_length || s == "." || s == "..") return false;
    for (char c : s)
        if (!segment_char(c)) return false;
    return true;
}

[[nodiscard]] constexpr bool valid_path(std::string_view p) noexcept {
    if (p.empty()) return false;
    while (true) {
        auto slash = p.find('/');
        if (!valid_segment(p.substr(0, slash))) return false;
        if (slash == std::string_view::npos) return true;
        p.remove_prefix(slash + 1);
    }
}

// S-8: the structural segment of the index-th child of a type among its
// siblings of the same type.
[[nodiscard]] std::string structural_segment(std::string_view type, std::size_t index);

[[nodiscard]] constexpr bool is_structural(std::string_view segment) noexcept {
    return segment.find(':') != std::string_view::npos;
}

namespace detail {
// Not constexpr: calling it in a constant evaluation is the compile error that
// rejects an invalid id literal, and its name is the message.
void invalid_id_literal_see_spec_S_8(std::string_view);
}

// An explicit id written in code, checked at compile time:
//     omui::button({.id = "notes.delete"})      // ok
//     omui::button({.id = "删除"})               // does not compile
struct IdLiteral {
    std::string_view text;

    consteval IdLiteral(const char* s) : text(s) {
        if (!valid_path_explicit(text)) detail::invalid_id_literal_see_spec_S_8(text);
    }

private:
    static consteval bool valid_path_explicit(std::string_view p) {
        if (p.empty()) return false;
        while (true) {
            auto slash = p.find('/');
            if (!valid_explicit_segment(p.substr(0, slash))) return false;
            if (slash == std::string_view::npos) return true;
            p.remove_prefix(slash + 1);
        }
    }
};

// A node path: the id of a node in the semantic tree.
class Path {
public:
    Path() = default;
    explicit Path(std::string text) : text_(std::move(text)) {}

    [[nodiscard]] static std::optional<Path> parse(std::string_view text);

    [[nodiscard]] const std::string& str() const noexcept { return text_; }
    [[nodiscard]] bool empty() const noexcept { return text_.empty(); }
    [[nodiscard]] std::string_view last() const noexcept;
    [[nodiscard]] Path parent() const;
    [[nodiscard]] Path child(std::string_view segment) const;
    [[nodiscard]] std::size_t depth() const noexcept;
    // True when this is `ancestor` or below it.
    [[nodiscard]] bool within(const Path& ancestor) const noexcept;

    friend bool operator==(const Path& a, const Path& b) noexcept { return a.text_ == b.text_; }
    friend std::strong_ordering operator<=>(const Path& a, const Path& b) noexcept { return a.text_ <=> b.text_; }

private:
    std::string text_;
};

// Glob over paths, as `find` uses it: `*` matches within one segment, `**`
// any number of whole segments, `?` one character. "notes/*" matches
// "notes/delete" but not "notes/list/item:0"; "**/delete" matches both
// "delete" and "a/b/delete".
[[nodiscard]] bool match_path(std::string_view pattern, std::string_view path) noexcept;

// Glob over a single string (names): `*` any run, `?` one character.
[[nodiscard]] bool match_text(std::string_view pattern, std::string_view text) noexcept;

}  // namespace omui::spec
