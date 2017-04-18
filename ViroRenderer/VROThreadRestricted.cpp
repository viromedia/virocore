//
//  VROThreadRestricted.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/30/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROThreadRestricted.h"
#include "VROLog.h"
#include <map>

static std::map<VROThreadName, pthread_t> sThreadMap;

std::string print_thread_id(pthread_t id) {
    char buf[1024];
    size_t i;
    for (i = sizeof(i); i; --i) {
        sprintf(buf, "%02x", *(((unsigned char *) &id) + i - 1));
    }
    buf[sizeof(i)] = 0;

    return std::string(buf);
}

void VROThreadRestricted::setThread(VROThreadName name, pthread_t thread) {
    sThreadMap[name] = thread;
}

void VROThreadRestricted::unsetThread(VROThreadName name) {
    sThreadMap.erase(name);
}

pthread_t VROThreadRestricted::getThread(VROThreadName name, pthread_t ret_not_found) {
    auto it = sThreadMap.find(name);
    if (it == sThreadMap.end()) {
        return ret_not_found;
    }
    else {
        return it->second;
    }
}

VROThreadRestricted::VROThreadRestricted() :
    _restricted_thread_name(VROThreadName::Undefined),
    _restricted_thread(pthread_self()) {
    
}

VROThreadRestricted::VROThreadRestricted(pthread_t thread) :
    _restricted_thread_name(VROThreadName::Undefined),
    _restricted_thread(thread) {
    
}

VROThreadRestricted::VROThreadRestricted(VROThreadName name) :
    _restricted_thread_name(name) {
}


VROThreadRestricted::~VROThreadRestricted() {
    
}

void VROThreadRestricted::setThreadRestriction(pthread_t thread) {
    _restricted_thread = thread;
    _restricted_thread_name = VROThreadName::Undefined;
}


void VROThreadRestricted::passert_thread() {
    pthread_t restricted_thread = _restricted_thread;
    if (_restricted_thread_name != VROThreadName::Undefined) {
        restricted_thread = getThread(_restricted_thread_name, pthread_self());
    }

    passert_msg(pthread_equal(pthread_self(), restricted_thread),
                "For object %p, current thread [%s] does not match object's thread restriction [%s]",
                this,
                print_thread_id(pthread_self()).c_str(),
                print_thread_id(restricted_thread).c_str());
}
