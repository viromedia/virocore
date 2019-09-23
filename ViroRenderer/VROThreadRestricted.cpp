//
//  VROThreadRestricted.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/30/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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

#include "VROThreadRestricted.h"
#include "VROLog.h"
#include <map>
#include <atomic>

// We are permissive of all operations *until* a rendering thread is set.
// This alleviates startup issues.
static std::atomic<bool> sRenderingThreadSet;

// The thread-local name of the current thread.
static thread_local VROThreadName tThreadName = VROThreadName::Undefined;

std::string print_thread_id(pthread_t id) {
    char buf[1024];
    size_t i;
    for (i = sizeof(i); i; --i) {
        sprintf(buf, "%02x", *(((unsigned char *) &id) + i - 1));
    }
    buf[sizeof(i)] = 0;

    return std::string(buf);
}

void VROThreadRestricted::setThread(VROThreadName name) {
    // Note this will not extend to multiple named threads, we would need
    // bools for each one.
    sRenderingThreadSet = true;
    tThreadName = name;
}

void VROThreadRestricted::unsetThread() {
    sRenderingThreadSet = false;
    tThreadName = VROThreadName::Undefined;
}

bool VROThreadRestricted::isThread(VROThreadName name) {
    return tThreadName == name;
}

VROThreadRestricted::VROThreadRestricted(VROThreadName name) :
    _restricted_thread_name(name),
    _enabled(true) {
}

VROThreadRestricted::~VROThreadRestricted() {
    
}

void VROThreadRestricted::passert_thread(std::string method) {
    if (!sRenderingThreadSet || !_enabled) {
        return;
    }

// TODO: VIRO-3320 made us change this from a passert_msg to a pwarn so we don't immediately crash
//    passert_msg(tThreadName == _restricted_thread_name,
//                "For object %p, current thread [%d] does not match object's thread restriction [%d]. Method: %s",
//                this,
//                tThreadName,
//                _restricted_thread_name,
//                method.c_str());

    if (tThreadName != _restricted_thread_name) {
        pwarn("For object %p, current thread [%d] does not match object's thread restriction [%d]. Method: %s",
              this,
              tThreadName,
              _restricted_thread_name,
              method.c_str());
    }
}
