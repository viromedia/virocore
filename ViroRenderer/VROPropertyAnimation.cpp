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

std::shared_ptr<VROPropertyAnimation> VROPropertyAnimation::parse(const std::string &name, const std::string &value) {
    int indexOfNumber = 0;
    VROAnimationOperation op = VROAnimationOperation::Assign;
    bool inverseOp = false;
  
    std::string addStr = "+=";
    std::string subtractStr = "-=";
    std::string multiplyStr = "*=";
    std::string divideStr = "/=";
    
    std::string typeStr = value.substr(0, 2);
    if (typeStr == addStr) {
        op = VROAnimationOperation::Add;
        indexOfNumber = 2;
        inverseOp = false;
    }
    else if (typeStr == subtractStr) {
        op = VROAnimationOperation::Add;
        indexOfNumber = 2;
        inverseOp = true;
    }
    else if (typeStr == multiplyStr) {
        op = VROAnimationOperation::Multiply;
        indexOfNumber = 2;
        inverseOp = false;
    }
    else if (typeStr == divideStr) {
        op = VROAnimationOperation::Multiply;
        indexOfNumber = 2;
        inverseOp = true;
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
     
        if (inverseOp) {
            if (op == VROAnimationOperation::Add) {
                animationValue.valueFloat *= -1;
            }
            if (op == VROAnimationOperation::Multiply) {
                animationValue.valueFloat = 1 / animationValue.valueFloat;
            }
        }
    }
    
    return std::make_shared<VROPropertyAnimation>(name, animationValue, op);
}

std::string VROPropertyAnimation::toString() const {
    std::stringstream ss;
    ss << (_value.type == VROValueType::Float ? _value.valueFloat : _value.valueInt);
   
    return ss.str();
}
