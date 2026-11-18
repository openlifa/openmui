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
