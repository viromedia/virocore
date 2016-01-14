//
//  VROTime.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROTime.h"
#include <map>
#include <sys/time.h>
#include <math.h>

#ifdef __OBJC__
#include <QuartzCore/QuartzCore.h>
#include <mach/mach_time.h>
#endif

uint64_t VROTimeGetCalendarTime() {
     return time(nullptr);
}

double VROTimeCurrentSeconds() {
#ifdef ANDROID_BUILD
    return VROTimeCurrentMillis() / 1000.0;
#else
    return CACurrentMediaTime();
#endif
}

double VROTimeCurrentMillis() {
#ifdef ANDROID_BUILD
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    return time.tv_sec * 1000.0 + time.tv_nsec / 1000000.0;
#else
    return CACurrentMediaTime() * 1000.0;
#endif
}

uint64_t VRONanoTime() {
#ifdef ANDROID_BUILD
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + (uint64_t)ts.tv_nsec;
#else
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return ((mach_absolute_time() * info.numer) / info.denom);
#endif
}
