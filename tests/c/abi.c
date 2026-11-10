/* openmui.h is plain C99 and plain C++11 (SPEC.md §8, C-5): CI compiles this
 * file both ways with every warning as an error. */
#include "openmui.h"

static OmuiResult call(PFN_omuiSink sink) {
    OmuiString s;
    s.data = "x";
    s.size = 1;
    sink(0, s.data, s.size);
    return OMUI_SUCCESS;
}

static void ignore(void* user, const char* data, size_t size) {
    (void)user;
    (void)data;
    (void)size;
}

int main(void) {
    OmuiRuntimeCreateInfo info;
    info.type = OMUI_STRUCTURE_TYPE_RUNTIME_CREATE_INFO;
    info.next = 0;
    info.spec_version = OMUI_SPEC_VERSION;
    (void)info;
    return call(ignore) == OMUI_SUCCESS && OMUI_ROLE_BUTTON > 0 ? 0 : 1;
}
