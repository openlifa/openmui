// openmui.spec:envelope — how a call answers: records, error codes, exit codes,
// and the actor who asked (spec §9): openas 0.2's (§2, §6).
//
// A call (act, invoke, wait) answers with a sequence of records, one JSON
// object per line: phase, progress, log, notice, error, data, and one final
// result carrying the exit code. Nothing waits for a human: an action that
// needs confirmation answers `needs-answer` at once (S-2) with the options.
export module openmui.spec:envelope;

import std;
import :value;

export namespace omui::spec {

enum class RecordKind : std::uint8_t { phase, progress, log, notice, error, data, result };

[[nodiscard]] constexpr std::string_view name(RecordKind k) noexcept {
    switch (k) {
        case RecordKind::phase:    return "phase";
        case RecordKind::progress: return "progress";
        case RecordKind::log:      return "log";
        case RecordKind::notice:   return "notice";
        case RecordKind::error:    return "error";
        case RecordKind::data:     return "data";
        case RecordKind::result:   return "result";
    }
    return "log";
}

// Exit codes of the final `result` record (openas §6.2).
namespace exit_code {
inline constexpr int success      = 0;
inline constexpr int failure      = 1;
inline constexpr int needs_answer = 2;
inline constexpr int not_ready    = 3;
}  // namespace exit_code

// The `code` of an `error` record (openas §6.3).
namespace error_code {
inline constexpr std::string_view needs_answer     = "needs-answer";
inline constexpr std::string_view not_found        = "not-found";
inline constexpr std::string_view refused          = "refused";
inline constexpr std::string_view invalid_argument = "invalid-argument";
inline constexpr std::string_view stale            = "stale";
inline constexpr std::string_view timeout          = "timeout";
inline constexpr std::string_view internal         = "internal";
inline constexpr std::string_view failed           = "failed";  // the work ran and did not succeed
}  // namespace error_code

struct Record {
    RecordKind kind = RecordKind::log;
    Value      body;  // the record's fields besides `kind`

    [[nodiscard]] std::string json() const;  // {"kind": "...", ...}
};

// A whole answer, built record by record.
class Envelope {
public:
    Envelope& phase(std::string_view name);
    Envelope& progress(double fraction, std::string_view message = {});
    Envelope& log(std::string_view message);
    Envelope& notice(std::string_view message);
    Envelope& error(std::string_view code, std::string_view message, Value extra = {});
    Envelope& data(Value data);
    Envelope& result(int exit_code);

    [[nodiscard]] std::span<const Record> records() const noexcept { return records_; }
    [[nodiscard]] std::optional<int> exit_code() const noexcept;
    [[nodiscard]] const Value* data_value() const noexcept;  // the last data record
    [[nodiscard]] std::string ndjson() const;                // one record per line

private:
    std::vector<Record> records_;
};

// Who performs an action, in openas's text form (openas §2.4):
//     agent:reader;via=person:alice,agent:planner
// `kind[:name]`, then `;key=value`s: `via` (once) lists whom it acts for, the
// outermost first; other keys are skipped.
struct Actor {
    std::string kind;  // person | agent | program | service
    std::string name;
    std::vector<std::string> via;  // "person:alice"

    [[nodiscard]] static std::optional<Actor> parse(std::string_view text);
    [[nodiscard]] std::string text() const;
};

}  // namespace omui::spec
