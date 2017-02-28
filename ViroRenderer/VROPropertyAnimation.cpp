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
#include "VROLog.h"

enum class PropertyAnimationType {
    Additive,
    Subtractive,
    Assign,
};

std::shared_ptr<VROPropertyAnimation> VROPropertyAnimation::parse(const std::string &name, const std::string &value) {
    int indexOfNumber = 0;
    PropertyAnimationType type = PropertyAnimationType::Assign;
    
    std::string additiveStr = "+=";
    std::string subtractiveStr = "-=";
    
    std::string typeStr = value.substr(0, 2);
    if (typeStr == additiveStr) {
        type = PropertyAnimationType::Additive;
        indexOfNumber = 2;
    }
    else if (typeStr == subtractiveStr) {
        type = PropertyAnimationType::Subtractive;
        indexOfNumber = 2;
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
     
        if (type == PropertyAnimationType::Subtractive) {
            animationValue.valueFloat *= -1;
        }
    }
    
    return std::make_shared<VROPropertyAnimation>(name, animationValue, type != PropertyAnimationType::Assign);
}

std::string VROPropertyAnimation::toString() const {
    std::stringstream ss;
    ss << (_value.type == VROValueType::Float ? _value.valueFloat : _value.valueInt);
   
    return ss.str();
}
