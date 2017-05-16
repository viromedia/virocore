//
//  VROSkeletalAnimation.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/16/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSkeletalAnimation.h"
#include "VROTransaction.h"
#include "VROLog.h"
#include <sstream>

std::shared_ptr<VROExecutableAnimation> VROSkeletalAnimation::copy() {
    pabort("Skeletal animations may not be copied");
}

void VROSkeletalAnimation::execute(std::shared_ptr<VRONode> node, std::function<void()> onFinished) {
    
}

void VROSkeletalAnimation::pause() {
    if (_transaction) {
        VROTransaction::pause(_transaction);
    }
}

void VROSkeletalAnimation::resume() {
    if (_transaction) {
        VROTransaction::resume(_transaction);
    }
}

void VROSkeletalAnimation::terminate() {
    if (_transaction) {
        VROTransaction::terminate(_transaction);
        _transaction.reset();
    }
}

std::string VROSkeletalAnimation::toString() const {
    std::stringstream ss;
    ss << "[skeletal: " << _name << "]";
    return ss.str();
}
