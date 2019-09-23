//
//  VROPropertyAnimation.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

#include "VROPropertyAnimation.h"
#include "VROStringUtil.h"
#include "VROMath.h"
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
        
        if (VROStringUtil::startsWith(name, "rotate")) {
            animationValue.valueFloat = toRadians(animationValue.valueFloat);
        }
    }
    
    return std::make_shared<VROPropertyAnimation>(name, animationValue, op);
}

std::string VROPropertyAnimation::toString() const {
    std::stringstream ss;
    if (_op == VROAnimationOperation::Assign) {
        ss << " assign ";
    }
    else if (_op == VROAnimationOperation::Add) {
        ss << " add ";
    }
    else if (_op == VROAnimationOperation::Multiply) {
        ss << " multiply ";
    }
    ss << (_value.type == VROValueType::Float ? _value.valueFloat : _value.valueInt);
   
    return ss.str();
}
