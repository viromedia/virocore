//
//  VROLog.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROLog.h"

#define DECAF_BAD 0xdecafbad
#define LOG_BUFFER_SIZE 1024

#if VRO_PLATFORM_ANDROID
#include "debug_stacktrace.h"
#include <cstdio>

/////////////////////////////////////////////////////////////////////////////////
//
//  Android: Logging
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Android Logging

void pstack(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    va_end(args);
    pwarn(fmt, args);
    pstack();
}

void pstack() {
    DebugStacktrace::getInstance().logStacktrace(2);
}

#else

void pstack(const char *fmt, ...) {}
void pstack() {}

#endif

/////////////////////////////////////////////////////////////////////////////////
//
//  Common: Abort
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Common: Abort

static void _pabort_location(const char *file, int line, const char *func,
                             const char *msg, int *location) __attribute__((noreturn));

static void _pabort_location(const char *file, int line, const char *func,
                             const char *msg, int *location) {
    pwarn("%s[******************************** ABORT ********************************]%s\n", ANSIYellow, ANSINoColor);
    pwarn("%s        File: %s%s\n", ANSIYellow, file, ANSINoColor);
    pwarn("%s        Line: %d%s\n", ANSIYellow, line, ANSINoColor);
    pwarn("%s    Function: %s%s\n", ANSIYellow, func, ANSINoColor);
    pwarn("%s      Reason: %s%s", ANSIDarkYellow, msg, ANSINoColor);
    pstack();
    pwarn("%s[***********************************************************************]%s", ANSIYellow, ANSINoColor);
    /*
     * Cause SEGfault. Note that this does not always result in a crash dump, hence the
     * pstack call to ensure a stack trace is logged.
     */
    *((int **) DECAF_BAD) = location;
    __builtin_unreachable();
}

static void _pabort_args(const char *file, int line, const char *func,
                         const char *fmt, va_list args) __attribute__((noreturn));

static void _pabort_args(const char *file, int line, const char *func,
                         const char *fmt, va_list args) {
    char buf[LOG_BUFFER_SIZE];
    vsnprintf(buf, LOG_BUFFER_SIZE, fmt, args);
    _pabort_location(file, line, func, buf, (int *)DECAF_BAD);
}

void _pabort(const char *file, int line, const char *func) {
    _pabort_location(file, line, func, "", (int *)DECAF_BAD);
}

void _pabort(const char *file, int line, const char *func, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _pabort_args(file, line, func, fmt, args);
    va_end(args);
}
