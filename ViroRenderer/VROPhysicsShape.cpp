//
//  VROPhysicsShape.m
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPhysicsShape.h"
#include "VROBox.h"
#include "VROSphere.h"
#include "VROLog.h"

VROPhysicsShape::VROPhysicsShape(VROShapeType type,std::vector<float> params) {
    if (type != VROShapeType::Sphere || VROShapeType::Box){
        perror("Attempted to construct unsupported VROPhysicsShape type!");
    }

    if ((type == VROShapeType::Sphere && params.size() < 1)
        || (type == VROShapeType::Box && params.size() < 3)){
        perror("Attempted to construct VROPhysics shape with incorrect parameters!");
    }

    _type = type;
    _params = params;
    _bulletShape = nullptr;
}

VROPhysicsShape::VROPhysicsShape(std::shared_ptr<VROGeometry> geometry){
    if (dynamic_cast<VROSphere*>(geometry.get()) != nullptr){
        _type = VROShapeType::Sphere;

        // Grab the max span to account for skewed spheres - we simply
        // assume a perfect sphere for these situations.
        VROBoundingBox bb = geometry->getBoundingBox();
        float maxSpan = std::max(std::max(bb.getSpanX(), bb.getSpanY()), bb.getSpanZ());
        _params.push_back(maxSpan/2);
    } else {
        _type = VROShapeType::Box;
        VROBoundingBox bb = geometry->getBoundingBox();
        _params.push_back(bb.getSpanX() / 2);
        _params.push_back(bb.getSpanY() / 2);
        _params.push_back(bb.getSpanZ() / 2);
    }
    _bulletShape = nullptr;
}

VROPhysicsShape::~VROPhysicsShape() {
    if (_bulletShape != nullptr){
        delete(_bulletShape);
    }
}

btCollisionShape* VROPhysicsShape::getBulletShape() {
    if (_bulletShape != nullptr){
        return _bulletShape;
    }

    if (_type == VROShapeType::Box){
        _bulletShape = new btBoxShape(btVector3(_params[0],_params[1],_params[2]));
    } else if (_type == VROShapeType::Sphere){
        _bulletShape = new btSphereShape(btScalar(_params[0]));
    } else {
        _bulletShape = nullptr;
        perror("Attempted to grab a bullet shape from a mis-configured VROPhysicsShape!");
    }
    return _bulletShape;
}