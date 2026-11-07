module openmui.spec;

import std;

namespace omui::spec {

std::string Record::json() const {
    std::string out = "{\"kind\":";
    append_json_string(out, name(kind));
    if (const Value::Map* m = body.map()) {
        for (const auto& [k, v] : *m) {
            out += ',';
            append_json_string(out, k);
            out += ':';
            out += v.json();
        }
    }
    out += '}';
    return out;
}

Envelope& Envelope::phase(std::string_view name) {
    Value b;
    b.set("phase", name);
    b.set("state", "begin");
    records_.push_back({RecordKind::phase, std::move(b)});
    return *this;
}

Envelope& Envelope::progress(double fraction, std::string_view message) {
    Value b;
    b.set("percent", fraction);
    if (!message.empty()) b.set("text", message);
    records_.push_back({RecordKind::progress, std::move(b)});
    return *this;
}

Envelope& Envelope::log(std::string_view message) {
    Value b;
    b.set("level", "info");
    b.set("text", message);
    records_.push_back({RecordKind::log, std::move(b)});
    return *this;
}

Envelope& Envelope::notice(std::string_view message) {
    Value b;
    b.set("text", message);
    records_.push_back({RecordKind::notice, std::move(b)});
    return *this;
}

Envelope& Envelope::error(std::string_view code, std::string_view message, Value extra) {
    Value b;
    b.set("code", code);
    b.set("text", message);
    if (const Value::Map* m = extra.map())
        for (const auto& [k, v] : *m) b.set(k, v);
    records_.push_back({RecordKind::error, std::move(b)});
    return *this;
}

Envelope& Envelope::data(Value data) {
    Value b;
    b.set("data", std::move(data));
    records_.push_back({RecordKind::data, std::move(b)});
    return *this;
}

Envelope& Envelope::result(int exit_code) {
    Value b;
    b.set("exitCode", exit_code);
    records_.push_back({RecordKind::result, std::move(b)});
    return *this;
}

std::optional<int> Envelope::exit_code() const noexcept {
    for (auto it = records_.rbegin(); it != records_.rend(); ++it)
        if (it->kind == RecordKind::result)
            if (const Value* c = it->body.get("exitCode")) return static_cast<int>(c->as_int());
    return std::nullopt;
}

const Value* Envelope::data_value() const noexcept {
    for (auto it = records_.rbegin(); it != records_.rend(); ++it)
        if (it->kind == RecordKind::data) return it->body.get("data");
    return nullptr;
}

std::string Envelope::ndjson() const {
    std::string out;
    for (const Record& r : records_) {
        out += r.json();
        out += '\n';
    }
    return out;
}

namespace {

constexpr std::array<std::string_view, 4> actor_kinds{"person", "agent", "program", "service"};

bool actor_name_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
           c == '@';
}

// `<kind>[:<name>]`
bool short_actor(std::string_view text, std::string* kind, std::string* name) {
    const auto colon = text.find(':');
    const auto k = text.substr(0, colon);
    if (std::ranges::find(actor_kinds, k) == actor_kinds.end()) return false;
    std::string_view n;
    if (colon != std::string_view::npos) {
        n = text.substr(colon + 1);
        if (n.empty() || !std::ranges::all_of(n, actor_name_char)) return false;
    }
    if (kind) *kind = std::string(k);
    if (name) *name = std::string(n);
    return true;
}

}  // namespace

std::optional<Actor> Actor::parse(std::string_view text) {
    Actor a;
    const auto semi = text.find(';');
    if (!short_actor(text.substr(0, semi), &a.kind, &a.name)) return std::nullopt;
    bool via = false;
    for (auto rest = text.substr(std::min(text.size(), semi)); !rest.empty();) {
        rest.remove_prefix(1);  // ';'
        const auto item = rest.substr(0, rest.find(';'));
        rest.remove_prefix(item.size());
        const auto eq = item.find('=');
        if (eq == std::string_view::npos || eq == 0) return std::nullopt;
        if (item.substr(0, eq) != "via") continue;  // reserved for later versions: skipped
        if (via) return std::nullopt;               // at most once
        via = true;
        for (auto list = item.substr(eq + 1);;) {
            const auto comma = list.find(',');
            const auto one = list.substr(0, comma);
            if (!short_actor(one, nullptr, nullptr)) return std::nullopt;
            a.via.emplace_back(one);
            if (comma == std::string_view::npos) break;
            list.remove_prefix(comma + 1);
        }
    }
    return a;
}

std::string Actor::text() const {
    std::string out = name.empty() ? kind : kind + ":" + name;
    for (std::size_t i = 0; i < via.size(); ++i) {
        out += i ? "," : ";via=";
        out += via[i];
    }
    return out;
}

}  // namespace omui::spec
