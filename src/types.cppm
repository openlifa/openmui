// openmui.spec:types — small value types the generated tables are made of.
export module openmui.spec:types;

import std;

export namespace omui::spec {

// A named, typed argument of an action or an event. Types: bool, i32, i64,
// f64, string, point, rect, size, any (registry/actions.toml).
struct ArgInfo {
    std::string_view name;
    std::string_view type;
};

// The type of a contract property.
enum class PropType : std::uint8_t {
    boolean,
    i32,
    i64,
    f64,
    string,
    color,        // a theme colour token or #rrggbb[aa]
    choice,       // one of the property's `values`
    string_list,
    any,
};

[[nodiscard]] constexpr std::string_view name(PropType t) noexcept {
    switch (t) {
        case PropType::boolean:     return "bool";
        case PropType::i32:         return "i32";
        case PropType::i64:         return "i64";
        case PropType::f64:         return "f64";
        case PropType::string:      return "string";
        case PropType::color:       return "color";
        case PropType::choice:      return "enum";
        case PropType::string_list: return "list<string>";
        case PropType::any:         return "any";
    }
    return "any";
}

// A property's default as the contract states it.
struct DefaultValue {
    enum class Kind : std::uint8_t { none, boolean, integer, real, text, list };

    Kind                              kind = Kind::none;
    bool                              b    = false;
    std::int64_t                      i    = 0;
    double                            d    = 0;
    std::string_view                  s;
    std::span<const std::string_view> list;

    [[nodiscard]] static constexpr DefaultValue of_bool(bool v) noexcept {
        DefaultValue x;
        x.kind = Kind::boolean;
        x.b = v;
        return x;
    }
    [[nodiscard]] static constexpr DefaultValue of_int(std::int64_t v) noexcept {
        DefaultValue x;
        x.kind = Kind::integer;
        x.i = v;
        return x;
    }
    [[nodiscard]] static constexpr DefaultValue of_double(double v) noexcept {
        DefaultValue x;
        x.kind = Kind::real;
        x.d = v;
        return x;
    }
    [[nodiscard]] static constexpr DefaultValue of_string(std::string_view v) noexcept {
        DefaultValue x;
        x.kind = Kind::text;
        x.s = v;
        return x;
    }
    [[nodiscard]] static constexpr DefaultValue of_list(std::span<const std::string_view> v) noexcept {
        DefaultValue x;
        x.kind = Kind::list;
        x.list = v;
        return x;
    }
};

}  // namespace omui::spec
