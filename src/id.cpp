module openmui.spec;

import std;

namespace omui::spec {

namespace detail {
void invalid_id_literal_see_spec_S_8(std::string_view) {}
}  // namespace detail

std::string structural_segment(std::string_view type, std::size_t index) {
    return std::format("{}:{}", type, index);
}

std::optional<Path> Path::parse(std::string_view text) {
    if (!valid_path(text)) return std::nullopt;
    return Path(std::string(text));
}

std::string_view Path::last() const noexcept {
    std::string_view t = text_;
    auto slash = t.rfind('/');
    return slash == std::string_view::npos ? t : t.substr(slash + 1);
}

Path Path::parent() const {
    auto slash = text_.rfind('/');
    return slash == std::string::npos ? Path{} : Path(text_.substr(0, slash));
}

Path Path::child(std::string_view segment) const {
    return Path(text_.empty() ? std::string(segment) : text_ + "/" + std::string(segment));
}

std::size_t Path::depth() const noexcept {
    if (text_.empty()) return 0;
    return 1 + static_cast<std::size_t>(std::ranges::count(text_, '/'));
}

bool Path::within(const Path& ancestor) const noexcept {
    if (ancestor.text_.empty()) return true;
    if (!text_.starts_with(ancestor.text_)) return false;
    return text_.size() == ancestor.text_.size() || text_[ancestor.text_.size()] == '/';
}

bool match_text(std::string_view pattern, std::string_view text) noexcept {
    // Iterative glob with one backtrack point: linear in practice, never
    // exponential.
    std::size_t p = 0, t = 0, star = std::string_view::npos, mark = 0;
    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t])) {
            ++p;
            ++t;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star = p++;
            mark = t;
        } else if (star != std::string_view::npos) {
            p = star + 1;
            t = ++mark;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == '*') ++p;
    return p == pattern.size();
}

namespace {

std::vector<std::string_view> split(std::string_view s) {
    std::vector<std::string_view> out;
    if (s.empty()) return out;
    while (true) {
        auto slash = s.find('/');
        out.push_back(s.substr(0, slash));
        if (slash == std::string_view::npos) break;
        s.remove_prefix(slash + 1);
    }
    return out;
}

bool match_segments(std::span<const std::string_view> pat, std::span<const std::string_view> path) {
    if (pat.empty()) return path.empty();
    if (pat.front() == "**") {
        for (std::size_t skip = 0; skip <= path.size(); ++skip)
            if (match_segments(pat.subspan(1), path.subspan(skip))) return true;
        return false;
    }
    if (path.empty()) return false;
    return match_text(pat.front(), path.front()) && match_segments(pat.subspan(1), path.subspan(1));
}

}  // namespace

bool match_path(std::string_view pattern, std::string_view path) noexcept {
    auto pat = split(pattern);
    auto segs = split(path);
    return match_segments(pat, segs);
}

}  // namespace omui::spec
