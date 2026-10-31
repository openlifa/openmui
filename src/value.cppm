// openmui.spec:value — the value model every boundary shares.
//
// Properties, signal contents, action arguments and model rows are Values, so
// the same thing can be set from C++, from a script, from a view document or
// by an agent over the bus (spec §1, rule 2: every value crosses address
// spaces — no pointers, no callbacks).
export module openmui.spec:value;

import std;

export namespace omui::spec {

class Value {
public:
    using List = std::vector<Value>;
    // Insertion-ordered; keys are unique (set() replaces).
    using Map = std::vector<std::pair<std::string, Value>>;

    enum class Type : std::uint8_t { null, boolean, integer, real, text, list, map };

    Value() = default;
    Value(std::nullptr_t) noexcept {}
    Value(bool v) noexcept : v_(v) {}
    Value(int v) noexcept : v_(static_cast<std::int64_t>(v)) {}
    Value(std::int64_t v) noexcept : v_(v) {}
    Value(std::uint32_t v) noexcept : v_(static_cast<std::int64_t>(v)) {}
    Value(double v) noexcept : v_(v) {}
    Value(float v) noexcept : v_(static_cast<double>(v)) {}
    Value(std::string v) : v_(std::move(v)) {}
    Value(std::string_view v) : v_(std::string(v)) {}
    Value(const char* v) : v_(std::string(v)) {}
    Value(List v) : v_(std::move(v)) {}
    Value(Map v) : v_(std::move(v)) {}

    [[nodiscard]] Type type() const noexcept { return static_cast<Type>(v_.index()); }
    [[nodiscard]] bool is_null() const noexcept { return type() == Type::null; }
    [[nodiscard]] bool is_number() const noexcept { return type() == Type::integer || type() == Type::real; }

    // Conversions answer the fallback when the value is of another type;
    // integers and reals convert to each other.
    [[nodiscard]] bool as_bool(bool fallback = false) const noexcept;
    [[nodiscard]] std::int64_t as_int(std::int64_t fallback = 0) const noexcept;
    [[nodiscard]] double as_double(double fallback = 0) const noexcept;
    [[nodiscard]] std::string_view as_string(std::string_view fallback = {}) const noexcept;
    [[nodiscard]] const List* list() const noexcept { return std::get_if<List>(&v_); }
    [[nodiscard]] const Map* map() const noexcept { return std::get_if<Map>(&v_); }
    [[nodiscard]] List* list() noexcept { return std::get_if<List>(&v_); }
    [[nodiscard]] Map* map() noexcept { return std::get_if<Map>(&v_); }

    // Map access: nullptr when absent or not a map.
    [[nodiscard]] const Value* get(std::string_view key) const noexcept;
    // Sets a key of a map and returns this map, so sets chain. A value that
    // is not a map becomes an empty map first.
    Value& set(std::string_view key, Value v);

    // Canonical JSON: insertion order, shortest round-trip numbers, integers
    // without a fraction; NaN and infinities are written as null.
    [[nodiscard]] std::string json() const;

    friend bool operator==(const Value& a, const Value& b) noexcept;

private:
    std::variant<std::monostate, bool, std::int64_t, double, std::string, List, Map> v_;
};

[[nodiscard]] std::string_view name(Value::Type t) noexcept;

// JSON string literal for text (shared by everything that writes JSON).
void append_json_string(std::string& out, std::string_view text);

}  // namespace omui::spec
