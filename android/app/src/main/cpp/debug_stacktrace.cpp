/*
 * Copyright (C) 2012 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "debug_stacktrace.h"
#include <dlfcn.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <unwind.h>
#include <sys/types.h>
#include "debug_mapinfo.h"
#include <android/log.h>
#include "VROLog.h"

// +---------------------------------------------------------------------------+
// | AOSP :: debug_stacktrace :: definition
// |
// | Taken from AOSP Bionic internals.
// +---------------------------------------------------------------------------+

int get_backtrace(uintptr_t* frames, size_t max_depth, unsigned int ignoreFrames);

void log_backtrace(uintptr_t* frames, size_t frame_count);

typedef char* DemanglerFn(const char*, char*, size_t*, int*);

// +---------------------------------------------------------------------------+
// | Amazon :: DebugStacktrace
// +---------------------------------------------------------------------------+

struct DebugStacktracePrivate {

    DebugStacktracePrivate(pid_t pid) :
        map_info(mapinfo_create(pid)),
        demangler(dlopen("libgccdemangle.so", RTLD_NOW)) {
        if (demangler) {
            void* sym = dlsym(demangler, "__cxa_demangle");
            demangler_fn = reinterpret_cast<DemanglerFn*>(sym);
        }
    }

    ~DebugStacktracePrivate() {
        mapinfo_destroy(map_info);
        dlclose(demangler);
    }

    mapinfo_t* map_info;
    void* demangler;
    char* (*demangler_fn)(const char*, char*, size_t*, int*);
};

void
DebugStacktrace::logStacktrace(unsigned int ignoreFrames) {
    uintptr_t frames[MAX_DEPTH];
    int frameCount = get_backtrace(frames, MAX_DEPTH, ignoreFrames);
    log_backtrace(frames, frameCount);
}

DebugStacktrace::DebugStacktrace() : mPimpl(new DebugStacktracePrivate(getpid())) {

}

DebugStacktrace::~DebugStacktrace() {
    DebugStacktracePrivate *impl = static_cast<DebugStacktracePrivate*>(mPimpl);
    delete impl;
}

DebugStacktrace& DebugStacktrace::getInstance() {
    static DebugStacktrace gSingleton;
    return gSingleton;
}

DebugStacktracePrivate* getPrivateInstance() {
    DebugStacktrace& publicInstance = DebugStacktrace::getInstance();
    return static_cast<DebugStacktracePrivate*>(publicInstance.getPimpl());
}

// +---------------------------------------------------------------------------+
// | AOSP :: debug_stacktrace :: implementation
// +---------------------------------------------------------------------------+

#if defined(__LP64__)
#define PAD_PTR "016lx"
#else
#define PAD_PTR "08x"
#endif
/* depends how the system includes define this */
#ifdef HAVE_UNWIND_CONTEXT_STRUCT
typedef struct _Unwind_Context __unwind_context;
#else
typedef _Unwind_Context __unwind_context;
#endif


static char* demangle(const char* symbol) {

    DebugStacktracePrivate* stp = getPrivateInstance();
    if (stp->demangler_fn == nullptr) {
        return nullptr;
    }
    return stp->demangler_fn(symbol, nullptr, nullptr, nullptr);
}

struct stack_crawl_state_t {
    uintptr_t* frames;
    size_t frame_count;
    size_t max_depth;
    unsigned int ignore_frames;
    stack_crawl_state_t(uintptr_t* frames, size_t max_depth, unsigned int ignoreFrames) :
            frames(frames), frame_count(0), max_depth(max_depth), ignore_frames(ignoreFrames + 1){
    }
};

static _Unwind_Reason_Code trace_function(__unwind_context* context, void* arg) {
    stack_crawl_state_t* state = static_cast<stack_crawl_state_t*>(arg);
    uintptr_t ip = _Unwind_GetIP(context);
    // The first stack frame is get_backtrace itself. Skip it.
    if (ip != 0 && state->ignore_frames) {
        state->ignore_frames--;
        return _URC_NO_REASON;
    }
#if defined(__arm__)
    /*
     * The instruction pointer is pointing at the instruction after the bl(x), and
     * the _Unwind_Backtrace routine already masks the Thumb mode indicator (LSB
     * in PC). So we need to do a quick check here to find out if the previous
     * instruction is a Thumb-mode BLX(2). If so subtract 2 otherwise 4 from PC.
     */
    if (ip != 0) {
        short* ptr = reinterpret_cast<short*>(ip);
        // Thumb BLX(2)
        if ((*(ptr-1) & 0xff80) == 0x4780) {
            ip -= 2;
        } else {
            ip -= 4;
        }
    }
#endif
    state->frames[state->frame_count++] = ip;
    return (state->frame_count >= state->max_depth) ? _URC_END_OF_STACK : _URC_NO_REASON;
}

int get_backtrace(uintptr_t* frames, size_t max_depth, unsigned int ignoreFrames) {
    stack_crawl_state_t state(frames, max_depth, ignoreFrames);
    _Unwind_Backtrace(trace_function, &state);
    return state.frame_count;
}

void log_backtrace(uintptr_t* frames, size_t frame_count) {
    DebugStacktracePrivate* stp = getPrivateInstance();

    if (frames == nullptr) {
        return;
    }
    pinfo(  "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n");
    for (size_t i = 0; i < frame_count; ++i) {
        uintptr_t offset = 0;
        const char* symbol = nullptr;
        Dl_info info;
        if (dladdr((void*) frames[i], &info) != 0) {
            offset = reinterpret_cast<uintptr_t>(info.dli_saddr);
            symbol = info.dli_sname;
        }
        uintptr_t rel_pc = offset;
        const mapinfo_t* mi =
                (stp->map_info != nullptr) ?
                        mapinfo_find(stp->map_info, frames[i], &rel_pc) : nullptr;
        const char* soname = (mi != nullptr) ? mi->name : info.dli_fname;
        if (soname == nullptr) {
            soname = "<unknown>";
        }
        if (symbol != nullptr) {
            char* demangled_symbol = demangle(symbol);
            const char* best_name =
                    (demangled_symbol != nullptr) ? demangled_symbol : symbol;
            pinfo(  "          #%02zd  pc %" PAD_PTR "  %s (%s+%dx)", i, rel_pc,
                    soname, best_name, (int) (frames[i] - offset));
            free(demangled_symbol);
        } else {
            pinfo(  "          #%02zd  pc %" PAD_PTR "  %s", i, rel_pc, soname);
        }
    }
}
