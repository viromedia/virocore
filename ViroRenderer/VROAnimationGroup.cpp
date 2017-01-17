//
//  VROAnimationGroup.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAnimationGroup.h"
#include "VROVector4f.h"
#include "VROGeometry.h"
#include "VRONode.h"
#include "VROMaterial.h"
#include "VROTransaction.h"
#include "VROStringUtil.h"
#include <sstream>

std::shared_ptr<VROAnimationGroup> VROAnimationGroup::parse(float duration, float delay, std::string functionName,
                                                            std::map<std::string, std::string> &propertyAnimations) {
   
    VROTimingFunctionType timingFunction = parseTimingFunction(functionName);
    std::map<std::string, std::shared_ptr<VROPropertyAnimation>> animations;
    for (auto kv : propertyAnimations) {
        std::string name = kv.first;
        animations[name] = VROPropertyAnimation::parse(name, kv.second);
    }
    
    return std::make_shared<VROAnimationGroup>(duration, delay, timingFunction, animations);
}

VROTimingFunctionType VROAnimationGroup::parseTimingFunction(std::string &name) {
    if (VROStringUtil::strcmpinsensitive(name, "Linear")) {
        return VROTimingFunctionType::Linear;
    }
    else if (VROStringUtil::strcmpinsensitive(name, "EaseIn")) {
        return VROTimingFunctionType::EaseIn;
    }
    else if (VROStringUtil::strcmpinsensitive(name, "EaseOut")) {
        return VROTimingFunctionType::EaseOut;
    }
    else if (VROStringUtil::strcmpinsensitive(name, "EaseInEaseOut")) {
        return VROTimingFunctionType::EaseInEaseOut;
    }
    else if (VROStringUtil::strcmpinsensitive(name, "Bounce")) {
        return VROTimingFunctionType::Bounce;
    }
    else if (VROStringUtil::strcmpinsensitive(name, "PowerDecel")) {
        return VROTimingFunctionType::PowerDecel;
    }
    
    //return default if nothing else matches
    return VROTimingFunctionType::Linear;
}

void VROAnimationGroup::execute(std::shared_ptr<VRONode> node,
                                std::function<void()> onFinished) {
    VROTransaction::begin();
    VROTransaction::setAnimationDelay(_delayMillis);
    VROTransaction::setAnimationDuration(_durationMillis);
    VROTransaction::setTimingFunction(_timingFunctionType);
    
    animatePosition(node);
    animateColor(node);
    animateOpacity(node);
    animateScale(node);
    animateRotation(node);

    VROTransaction::setFinishCallback([this, onFinished]{
        _transaction.reset();
        onFinished();
    });
        
    _transaction = VROTransaction::commit();
}

void VROAnimationGroup::resume() {
    if (_transaction) {
        VROTransaction::resume(_transaction);
    }
}

void VROAnimationGroup::pause() {
    if (_transaction) {
        VROTransaction::pause(_transaction);
    }
}

void VROAnimationGroup::terminate() {
    if (_transaction) {
        VROTransaction::terminate(_transaction);
        _transaction.reset();
    }
}

void VROAnimationGroup::animatePosition(std::shared_ptr<VRONode> &node) {
    auto posX_it = _animations.find("positionX");
    auto posY_it = _animations.find("positionY");
    auto posZ_it = _animations.find("positionZ");

    float posX = node->getPosition().x;
    if (posX_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = posX_it->second;
        if (a->isAdditive()) {
            posX += a->getValue();
        }
        else {
            posX = a->getValue();
        }
        node->setPositionX(posX);
    }
    
    float posY = node->getPosition().y;
    if (posY_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = posY_it->second;
        if (a->isAdditive()) {
            posY += a->getValue();
        }
        else {
            posY = a->getValue();
        }
        node->setPositionY(posY);
    }
    
    float posZ = node->getPosition().z;
    if (posZ_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = posZ_it->second;
        if (a->isAdditive()) {
            posZ += a->getValue();
        }
        else {
            posZ = a->getValue();
        }
        node->setPositionZ(posZ);
    }
}

void VROAnimationGroup::animateScale(std::shared_ptr<VRONode> &node) {
    auto scaleX_it = _animations.find("scaleX");
    auto scaleY_it = _animations.find("scaleY");
    auto scaleZ_it = _animations.find("scaleZ");
    
    float scaleX = node->getScale().x;
    if (scaleX_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = scaleX_it->second;
        if (a->isAdditive()) {
            scaleX += a->getValue();
        }
        else {
            scaleX = a->getValue();
        }
        node->setScaleX(scaleX);
    }
    
    float scaleY = node->getScale().y;
    if (scaleY_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = scaleY_it->second;
        if (a->isAdditive()) {
            scaleY += a->getValue();
        }
        else {
            scaleY = a->getValue();
        }
        node->setScaleY(scaleY);
    }
    
    float scaleZ = node->getScale().z;
    if (scaleZ_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = scaleZ_it->second;
        if (a->isAdditive()) {
            scaleZ += a->getValue();
        }
        else {
            scaleZ = a->getValue();
        }
        node->setScaleZ(scaleZ);
    }
}

void VROAnimationGroup::animateColor(std::shared_ptr<VRONode> &node) {
    auto color_it = _animations.find("color");
    if (color_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &animation = color_it->second;
        
        int color = animation->getValue();
        float a = ((color >> 24) & 0xFF) / 255.0;
        float r = ((color >> 16) & 0xFF) / 255.0;
        float g = ((color >> 8)  & 0xFF) / 255.0;
        float b =  (color & 0xFF) / 255.0;

        VROVector4f vecColor(r, g, b, a);
        if (node->getGeometry()) {
            std::shared_ptr<VROGeometry> geometry = node->getGeometry();
            geometry->getMaterials()[0]->getDiffuse().setColor(vecColor);
        }
    }
}

void VROAnimationGroup::animateOpacity(std::shared_ptr<VRONode> &node) {
    auto opacity_it = _animations.find("opacity");
    if (opacity_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = opacity_it->second;
        if (a->isAdditive()) {
            node->setOpacity(node->getOpacity() + a->getValue());
        }
        else {
            node->setOpacity(a->getValue());
        }
    }
}

void VROAnimationGroup::animateRotation(std::shared_ptr<VRONode> &node) {
    auto rotateX_it = _animations.find("rotateX");
    auto rotateY_it = _animations.find("rotateY");
    auto rotateZ_it = _animations.find("rotateZ");
    
    VROVector3f rotation = node->getRotationEuler();
    
    float rotateX = rotation.x;
    if (rotateX_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = rotateX_it->second;
        if (a->isAdditive()) {
            rotateX += toRadians(a->getValue());
        }
        else {
            rotateX = toRadians(a->getValue());
        }
        node->setRotationEulerX(rotateX);
    }
    
    float rotateY = rotation.y;
    if (rotateY_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = rotateY_it->second;
        if (a->isAdditive()) {
            rotateY += toRadians(a->getValue());
        }
        else {
            rotateY = toRadians(a->getValue());
        }
        node->setRotationEulerY(rotateY);
    }
    
    float rotateZ = rotation.z;
    if (rotateZ_it != _animations.end()) {
        std::shared_ptr<VROPropertyAnimation> &a = rotateZ_it->second;
        if (a->isAdditive()) {
            rotateZ += toRadians(a->getValue());
        }
        else {
            rotateZ = toRadians(a->getValue());
        }
        node->setRotationEulerZ(rotateZ);
    }
}

std::string VROAnimationGroup::toString() const {
    std::stringstream ss;
    ss << "[duration: " << _durationMillis << ", delay: " << _delayMillis;
    
    for (auto kv : _animations) {
        ss << ", " << kv.first << ":" << kv.second->toString();
    }
    ss << "]";
    
    return ss.str();
}
