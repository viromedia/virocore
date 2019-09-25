//
//  VROTime.cpp
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

#include "VROTime.h"
#include "VRODefines.h"
#include <map>
#include <sys/time.h>
#include <math.h>

#if VRO_PLATFORM_IOS || VRO_PLATFORM_MACOS
#include <QuartzCore/QuartzCore.h>
#include <mach/mach_time.h>
#endif

uint64_t VROTimeGetCalendarTime() {
     return time(nullptr);
}

double VROTimeCurrentSeconds() {
#if VRO_PLATFORM_ANDROID || VRO_PLATFORM_WASM
    return VROTimeCurrentMillis() / 1000.0;
#elif VRO_PLATFORM_IOS || VRO_PLATFORM_MACOS
    return CACurrentMediaTime();
#endif
}

double VROTimeCurrentMillis() {
#if VRO_PLATFORM_ANDROID || VRO_PLATFORM_WASM
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    return time.tv_sec * 1000.0 + time.tv_nsec / 1000000.0;
#elif VRO_PLATFORM_IOS || VRO_PLATFORM_MACOS
    return CACurrentMediaTime() * 1000.0;
#endif
}

uint64_t VRONanoTime() {
#if VRO_PLATFORM_ANDROID || VRO_PLATFORM_WASM
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + (uint64_t)ts.tv_nsec;
#elif VRO_PLATFORM_IOS || VRO_PLATFORM_MACOS
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return ((mach_absolute_time() * info.numer) / info.denom);
#endif
}
