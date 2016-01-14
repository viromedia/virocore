//
//  VROTimingFunction.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTimingFunction.h"
#include "VROTimingFunctionEaseIn.h"
#include "VROTimingFunctionEaseOut.h"
#include "VROTimingFunctionEaseInEaseOut.h"
#include "VROTimingFunctionBounce.h"
#include "VROTimingFunctionPowerDeceleration.h"
#include "VROTimingFunctionLinear.h"
#include "VROLog.h"

std::unique_ptr<VROTimingFunction> VROTimingFunction::forType(VROTimingFunctionType type) {
    switch (type) {
        case VROTimingFunctionType::Linear:
            return std::unique_ptr<VROTimingFunction>(new VROTimingFunctionLinear());
        case VROTimingFunctionType::EaseIn:
            return std::unique_ptr<VROTimingFunction>(new VROTimingFunctionEaseIn());
        case VROTimingFunctionType::EaseOut:
            return std::unique_ptr<VROTimingFunction>(new VROTimingFunctionEaseOut());
        case VROTimingFunctionType::EaseInEaseOut:
            return std::unique_ptr<VROTimingFunction>(new VROTimingFunctionEaseInEaseOut());
        case VROTimingFunctionType::Bounce:
            return std::unique_ptr<VROTimingFunction>(new VROTimingFunctionBounce());
        case VROTimingFunctionType::PowerDecel:
            return std::unique_ptr<VROTimingFunction>(new VROTimingFunctionPowerDeceleration());
        default:
            pabort();
    }
}

