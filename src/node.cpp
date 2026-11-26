module openmui.spec;

import std;

namespace omui::spec {

std::vector<Violation> check(const SemanticNode& node) {
    std::vector<Violation> out;
    if (!valid_path(node.id)) out.push_back({"S-8", node.id, "the id is not a valid path"});
    for (const ActionDesc& a : node.actions)
        if (requires_answer(a.effect) && a.answer.empty())
            out.push_back({"S-1", node.id,
                           std::format("action `{}` is {} but states no answer", a.name, name(a.effect))});
    if (interactive(node.role) && node.name.empty())
        out.push_back({"S-6", node.id, std::format("an interactive {} has no name", name(node.role))});
    if (const Value::Map* stated = node.properties.map())
        for (const auto& [key, value] : *stated) {
            const auto listed = parse_node_property(key);
            if (!listed) {
                // A vendor's (`vendor.name`) is kept and given no meaning.
                if (key.find('.') == std::string::npos)
                    out.push_back({"S-16", node.id, std::format("property `{}` is not in the registry", key)});
                continue;
            }
            const NodePropertyInfo& info = node_properties[static_cast<std::size_t>(*listed)];
            if (!info.roles.empty() && std::ranges::find(info.roles, node.role) == info.roles.end())
                out.push_back({"S-16", node.id, std::format("property `{}` is not stated on a {}", key, name(node.role))});
            bool typed = true;
            switch (info.type) {
                case PropType::boolean: typed = value.type() == Value::Type::boolean; break;
                case PropType::i64: typed = value.type() == Value::Type::integer; break;
                case PropType::string: typed = value.type() == Value::Type::text; break;
                case PropType::choice:
                    typed = value.type() == Value::Type::text && std::ranges::find(info.values, value.as_string()) != info.values.end();
                    break;
                default: break;
            }
            if (!typed) out.push_back({"S-16", node.id, std::format("property `{}` is not {}", key, name(info.type))});
        }
    else if (!node.properties.is_null())
        out.push_back({"S-16", node.id, "`properties` is not an object"});
    if (!node.marks.empty()) {
        const std::uint32_t length = code_points(node.name);
        std::vector<const Mark*> open;  // the marks the current one is inside of
        const Mark* before = nullptr;
        for (const Mark& mark : node.marks) {
            const std::string_view kind = name(mark.kind);
            if (mark.start > mark.end || mark.end > length) {
                out.push_back({"S-17", node.id, std::format("mark `{}` [{}, {}) is not a range of the name's {} code points", kind, mark.start, mark.end, length)});
                continue;
            }
            if (before && (mark.start < before->start || (mark.start == before->start && mark.end > before->end)))
                out.push_back({"S-17", node.id, std::format("mark `{}` [{}, {}) is out of order", kind, mark.start, mark.end)});
            while (!open.empty() && open.back()->end <= mark.start) open.pop_back();
            if (!open.empty() && mark.end > open.back()->end)
                out.push_back({"S-17", node.id, std::format("mark `{}` [{}, {}) crosses `{}`", kind, mark.start, mark.end, name(open.back()->kind))});
            open.push_back(&mark);
            before = &mark;
            const auto index = static_cast<std::size_t>(mark.kind);
            const bool placed = index < marks.size() && marks[index].node;
            if (placed && std::ranges::find(node.children, mark.node) == node.children.end())
                out.push_back({"S-17", node.id, std::format("mark `{}` names no child of the node", kind)});
            if (!placed && !mark.node.empty()) out.push_back({"S-17", node.id, std::format("mark `{}` has a node", kind)});
        }
    }
    return out;
}

namespace {

Value rect_value(const Rect& r) {
    return Value(Value::List{Value(static_cast<double>(r.x)), Value(static_cast<double>(r.y)),
                             Value(static_cast<double>(r.width)), Value(static_cast<double>(r.height))});
}

}  // namespace

Value to_value(const SemanticNode& node) {
    Value v;
    v.set("id", node.id);
    v.set("role", name(node.role));
    if (!node.name.empty()) v.set("name", node.name);
    if (!node.description.empty()) v.set("description", node.description);
    if (!node.value.is_null()) v.set("value", node.value);
    if (!node.properties.is_null()) v.set("properties", node.properties);
    if (!node.marks.empty()) {
        Value::List listed;
        for (const Mark& mark : node.marks) {
            Value entry;
            entry.set("kind", name(mark.kind));
            entry.set("start", static_cast<std::int64_t>(mark.start));
            entry.set("end", static_cast<std::int64_t>(mark.end));
            if (!mark.node.empty()) entry.set("node", mark.node);
            listed.push_back(std::move(entry));
        }
        v.set("marks", std::move(listed));
    }
    if (node.origin != Origin::app) v.set("origin", name(node.origin));
    if (!node.states.empty()) {
        Value::List states;
        for (std::size_t i = 0; i < spec::states.size(); ++i)
            if (node.states.has(static_cast<State>(i))) states.emplace_back(spec::states[i].name);
        v.set("state", std::move(states));
    }
    if (!node.actions.empty()) {
        Value::List actions;
        for (const ActionDesc& a : node.actions) {
            Value av;
            av.set("name", a.name);
            av.set("effect", name(a.effect));
            if (!a.verb.empty()) av.set("verb", a.verb);
            if (!a.answer.empty()) av.set("answer", a.answer);
            if (a.person) av.set("person", true);
            if (!a.args.empty()) {
                Value args;
                for (const auto& [n, t] : a.args) args.set(n, t);
                av.set("args", std::move(args));
            }
            actions.push_back(std::move(av));
        }
        v.set("actions", std::move(actions));
    }
    if (node.bounds) v.set("bounds", rect_value(*node.bounds));
    if (!node.children.empty()) {
        Value::List children;
        for (const auto& c : node.children) children.emplace_back(c);
        v.set("children", std::move(children));
    }
    return v;
}

std::optional<SemanticNode> node_from_value(const Value& v) {
    if (!v.map()) return std::nullopt;
    SemanticNode n;
    const Value* id = v.get("id");
    const Value* role = v.get("role");
    if (!id || !role) return std::nullopt;
    n.id = std::string(id->as_string());
    auto r = parse_role(role->as_string());
    if (!r) return std::nullopt;
    n.role = *r;
    if (const Value* x = v.get("name")) n.name = std::string(x->as_string());
    if (const Value* x = v.get("description")) n.description = std::string(x->as_string());
    if (const Value* x = v.get("value")) n.value = *x;
    if (const Value* x = v.get("properties")) n.properties = *x;
    if (const Value* x = v.get("marks"); x && x->list())
        for (const Value& entry : *x->list()) {
            const Value* kind = entry.get("kind");
            // A kind this reader does not know is kept by whoever holds the wire form; the value type has no place for it.
            const auto known = kind ? parse_mark(kind->as_string()) : std::nullopt;
            if (!known) continue;
            Mark mark;
            mark.kind = *known;
            if (const Value* f = entry.get("start")) mark.start = static_cast<std::uint32_t>(f->as_int());
            if (const Value* f = entry.get("end")) mark.end = static_cast<std::uint32_t>(f->as_int());
            if (const Value* f = entry.get("node")) mark.node = std::string(f->as_string());
            n.marks.push_back(std::move(mark));
        }
    if (const Value* x = v.get("origin")) n.origin = parse_origin(x->as_string()).value_or(Origin::remote);
    if (const Value* x = v.get("state"); x && x->list())
        for (const Value& s : *x->list())
            if (auto st = parse_state(s.as_string())) n.states.set(*st);
    if (const Value* x = v.get("actions"); x && x->list()) {
        for (const Value& av : *x->list()) {
            ActionDesc a;
            if (const Value* f = av.get("name")) a.name = std::string(f->as_string());
            // An effect a reader does not know is read as the strongest (effects.toml).
            if (const Value* f = av.get("effect")) a.effect = parse_effect(f->as_string()).value_or(Effect::external);
            if (const Value* f = av.get("verb")) a.verb = std::string(f->as_string());
            if (const Value* f = av.get("answer")) a.answer = std::string(f->as_string());
            if (const Value* f = av.get("person")) a.person = f->as_bool();
            if (const Value* f = av.get("args"); f && f->map())
                for (const auto& [k, t] : *f->map()) a.args.emplace_back(k, std::string(t.as_string()));
            n.actions.push_back(std::move(a));
        }
    }
    if (const Value* x = v.get("bounds"); x && x->list() && x->list()->size() == 4) {
        const auto& l = *x->list();
        n.bounds = Rect{static_cast<float>(l[0].as_double()), static_cast<float>(l[1].as_double()),
                        static_cast<float>(l[2].as_double()), static_cast<float>(l[3].as_double())};
    }
    if (const Value* x = v.get("children"); x && x->list())
        for (const Value& c : *x->list()) n.children.emplace_back(c.as_string());
    return n;
}

Value to_value(const SemanticDelta& delta) {
    Value v;
    v.set("revision", static_cast<std::int64_t>(delta.revision));
    Value::List added, removed, updated;
    for (const auto& n : delta.added) added.push_back(to_value(n));
    for (const auto& id : delta.removed) removed.emplace_back(id);
    for (const auto& n : delta.updated) updated.push_back(to_value(n));
    v.set("added", std::move(added));
    v.set("removed", std::move(removed));
    v.set("updated", std::move(updated));
    return v;
}

}  // namespace omui::spec
