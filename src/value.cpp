module openmui.spec;

import std;

namespace omui::spec {

bool Value::as_bool(bool fallback) const noexcept {
    if (auto b = std::get_if<bool>(&v_)) return *b;
    return fallback;
}

std::int64_t Value::as_int(std::int64_t fallback) const noexcept {
    if (auto i = std::get_if<std::int64_t>(&v_)) return *i;
    if (auto d = std::get_if<double>(&v_)) {
        if (!(*d == *d) || *d >= 9.2233720368547758e18 || *d <= -9.2233720368547758e18) return fallback;
        return static_cast<std::int64_t>(*d);
    }
    return fallback;
}

double Value::as_double(double fallback) const noexcept {
    if (auto d = std::get_if<double>(&v_)) return *d;
    if (auto i = std::get_if<std::int64_t>(&v_)) return static_cast<double>(*i);
    return fallback;
}

std::string_view Value::as_string(std::string_view fallback) const noexcept {
    if (auto s = std::get_if<std::string>(&v_)) return *s;
    return fallback;
}

const Value* Value::get(std::string_view key) const noexcept {
    const Map* m = map();
    if (!m) return nullptr;
    for (const auto& [k, v] : *m)
        if (k == key) return &v;
    return nullptr;
}

Value& Value::set(std::string_view key, Value v) {
    if (!map()) v_ = Map{};
    Map& m = *map();
    for (auto& [k, existing] : m)
        if (k == key) {
            existing = std::move(v);
            return *this;
        }
    m.emplace_back(std::string(key), std::move(v));
    return *this;
}

void append_json_string(std::string& out, std::string_view text) {
    out += '"';
    for (unsigned char c : text) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            default:
                if (c < 0x20) out += std::format("\\u{:04x}", c);
                else out += static_cast<char>(c);
        }
    }
    out += '"';
}

namespace {

void write(std::string& out, const Value& v) {
    switch (v.type()) {
        case Value::Type::null: out += "null"; return;
        case Value::Type::boolean: out += v.as_bool() ? "true" : "false"; return;
        case Value::Type::integer: out += std::format("{}", v.as_int()); return;
        case Value::Type::real: {
            double d = v.as_double();
            if (d != d || d == std::numeric_limits<double>::infinity() || d == -std::numeric_limits<double>::infinity()) {
                out += "null";
                return;
            }
            std::string s = std::format("{}", d);
            out += s;
            return;
        }
        case Value::Type::text: append_json_string(out, v.as_string()); return;
        case Value::Type::list: {
            out += '[';
            bool first = true;
            for (const Value& e : *v.list()) {
                if (!first) out += ',';
                first = false;
                write(out, e);
            }
            out += ']';
            return;
        }
        case Value::Type::map: {
            out += '{';
            bool first = true;
            for (const auto& [k, e] : *v.map()) {
                if (!first) out += ',';
                first = false;
                append_json_string(out, k);
                out += ':';
                write(out, e);
            }
            out += '}';
            return;
        }
    }
}

}  // namespace

std::string Value::json() const {
    std::string out;
    write(out, *this);
    return out;
}

bool operator==(const Value& a, const Value& b) noexcept { return a.v_ == b.v_; }

std::string_view name(Value::Type t) noexcept {
    switch (t) {
        case Value::Type::null:    return "null";
        case Value::Type::boolean: return "bool";
        case Value::Type::integer: return "int";
        case Value::Type::real:    return "double";
        case Value::Type::text:    return "string";
        case Value::Type::list:    return "list";
        case Value::Type::map:     return "map";
    }
    return "null";
}

}  // namespace omui::spec
