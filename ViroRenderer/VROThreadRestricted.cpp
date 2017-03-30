//
//  VROThreadRestricted.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/30/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROThreadRestricted.h"
#include "VROLog.h"

std::string print_thread_id(pthread_t id) {
    char buf[1024];
    size_t i;
    for (i = sizeof(i); i; --i) {
        sprintf(buf, "%02x", *(((unsigned char *) &id) + i - 1));
    }
    buf[sizeof(i)] = 0;

    return std::string(buf);
}

VROThreadRestricted::VROThreadRestricted() :
    _restricted_thread(pthread_self()) {
    
}

VROThreadRestricted::VROThreadRestricted(pthread_t thread) :
    _restricted_thread(thread) {
    
}

VROThreadRestricted::~VROThreadRestricted() {
    
}

void VROThreadRestricted::setThreadRestriction(pthread_t thread) {
    _restricted_thread = thread;
}


void VROThreadRestricted::passert_thread() {
    passert_msg(pthread_equal(pthread_self(), _restricted_thread),
                "For object %p, current thread [%s] does not match object's thread restriction [%s]",
                this,
                print_thread_id(pthread_self()).c_str(),
                print_thread_id(_restricted_thread).c_str());
}
