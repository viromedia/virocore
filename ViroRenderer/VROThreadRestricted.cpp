//
//  VROThreadRestricted.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/30/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROThreadRestricted.h"
#include "VROLog.h"

VROThreadRestricted::VROThreadRestricted() :
    _restricted_thread(pthread_self()) {
    
}

VROThreadRestricted::VROThreadRestricted(pthread_t thread) :
    _restricted_thread(thread) {
    
}

VROThreadRestricted::~VROThreadRestricted() {
    
}

void VROThreadRestricted::passert_thread() {
    passert(pthread_equal(pthread_self(), _restricted_thread));
}
