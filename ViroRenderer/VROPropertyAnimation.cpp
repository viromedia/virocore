//
//  VROPropertyAnimation.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROPropertyAnimation.h"
#include "VROStringUtil.h"
#include <sstream>

std::shared_ptr<VROPropertyAnimation> VROPropertyAnimation::parse(const std::string &name, const std::string &value) {
    const char firstChar = value.at(0);
    
    int indexOfNumber = 0;
    bool isAdditive = false;
    
    // Detect if this is an additive value (e.g. +45)
    if (firstChar == '+') {
        indexOfNumber = 1;
        isAdditive = true;
    }
    
    // Detect if this is a += additive value (e.g. +=45) (same behavior as +)
    if (value.size() > 2) {
        const char secondChar = value.at(1);
        if (secondChar == '=') {
            
            // Ensures we have += and not something invalid like 0=
            if (indexOfNumber == 1) {
                indexOfNumber = 2;
                isAdditive = true;
            }
        }
    }
    
    VROAnimationValue animationValue;
    if (name == "color") {
        std::string numberStr = value.substr(indexOfNumber);
        animationValue.type = VROValueType::Int;
        animationValue.valueInt = VROStringUtil::toInt(numberStr);
    }
    else {
        std::string numberStr = value.substr(indexOfNumber);
        animationValue.type = VROValueType::Float;
        animationValue.valueFloat = VROStringUtil::toFloat(numberStr);
    }
    
    return std::make_shared<VROPropertyAnimation>(name, animationValue, isAdditive);
}

std::string VROPropertyAnimation::toString() const {
    std::stringstream ss;
    ss << (_value.type == VROValueType::Float ? _value.valueFloat : _value.valueInt);
   
    return ss.str();
}
