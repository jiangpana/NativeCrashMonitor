//
// Created by Administrator on 2023/6/25.
//

#include <android/log.h>
#include <string>
#include "StackPrinter.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unwind.h>
#include <dlfcn.h>
#include <android/log.h>
#include <cxxabi.h>
#include <vector>
#include <inttypes.h>

 _Unwind_Reason_Code unwind_callback(struct _Unwind_Context *context, void *arg) {
    std::vector<_Unwind_Word> &stack = *(std::vector<_Unwind_Word> *) arg;
    stack.push_back(_Unwind_GetIP(context));
    return _URC_NO_REASON;
}


void print_stack() {
    std::string dump;
    std::vector<_Unwind_Word> stack;
    _Unwind_Backtrace(unwind_callback, (void *) &stack);
    dump.append("backtrace:\n");
    char buff[256];
    for (size_t i = 0; i < stack.size(); i++) {
        Dl_info info;
        uintptr_t pc = stack[i];
        if (dladdr((void *) pc, &info) > 0) {
            int addr = (char *) pc - (char *) info.dli_fbase - 1;
            if (info.dli_sname == NULL || strlen(info.dli_sname) == 0) {
                sprintf(buff, "#%02x pc %08x  %s\n", i, addr, info.dli_fname);
            } else {
                char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, 0, nullptr);
                sprintf(buff, "#%02x pc %08x  %s  (%s+%" PRIuPTR")\n", i, addr, info.dli_fname,
                        demangled, pc - (uintptr_t) info.dli_saddr);
            }
            dump.append(buff);
        }
    }
    __android_log_print(ANDROID_LOG_DEBUG, "jansir", "%s", dump.c_str());

}

