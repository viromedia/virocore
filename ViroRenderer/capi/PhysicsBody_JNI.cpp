//
// Created by Raj Advani on 10/16/17.
//

#include "PhysicsBody_JNI.h"
#include "Node_JNI.h"
#include "VRONode.h"
#include "VROPhysicsBody.h"
#include "PhysicsDelegate_JNI.h"
#include "VROPlatformUtil.h"

#if VRO_PLATFORM_ANDROID
#define VRO_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_PhysicsBody_##method_name
#else
#define VRO_METHOD(return_type, method_name) \
    return_type PhysicsBody_##method_name
#endif

extern "C" {

VRO_METHOD(void, nativeInitPhysicsBody)(VRO_ARGS
                                        VRO_REF(VRONode) nativeRef,
                                        VRO_STRING bodyTypeStr,
                                        VRO_FLOAT mass,
                                        VRO_STRING shapeTypeStr,
                                        VRO_FLOAT_ARRAY shapeParams) {
    VRO_METHOD_PREAMBLE;

    // Get Physics Body type
    std::string strBodyType = VRO_STRING_STL(bodyTypeStr);
    VROPhysicsBody::VROPhysicsBodyType bodyType = VROPhysicsBody::getBodyTypeForString(strBodyType);

    // Build a VROPhysicsShape if possible
    std::shared_ptr<VROPhysicsShape> propPhysicsShape = nullptr;
    if (!VRO_IS_STRING_EMPTY(shapeTypeStr)) {
        std::string strShapeType = VRO_STRING_STL(shapeTypeStr);
        VROPhysicsShape::VROShapeType shapeType = VROPhysicsShape::getTypeForString(strShapeType);

        int paramsLength = VRO_ARRAY_LENGTH(shapeParams);
        VRO_FLOAT *pointArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(shapeParams);
        std::vector<float> params;
        for (int i = 0; i < paramsLength; i++) {
            params.push_back(pointArray[i]);
        }
        VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(shapeParams, pointArray);
        propPhysicsShape = propPhysicsShape = std::make_shared<VROPhysicsShape>(shapeType, params);
    }

    // Create a VROPhysicsBody within VRONode.
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, bodyType, mass, propPhysicsShape] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->initPhysicsBody(bodyType, mass, propPhysicsShape);
        }
    });
}

VRO_METHOD(void, nativeClearPhysicsBody)(VRO_ARGS
                                         VRO_REF(VRONode) nativeRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node) {
            node->clearPhysicsBody();
        }
    });
}

VRO_METHOD(void, nativeSetPhysicsShape)(VRO_ARGS
                                        VRO_REF(VRONode) nativeRef,
                                        VRO_STRING shapeTypeStr,
                                        VRO_FLOAT_ARRAY shapeParams) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);

    // Build a VROPhysicsShape if possible
    std::shared_ptr<VROPhysicsShape> propPhysicsShape = nullptr;
    if (!VRO_IS_STRING_EMPTY(shapeTypeStr)) {
        // Get the shape type
        std::string strShapeType = VRO_STRING_STL(shapeTypeStr);
        VROPhysicsShape::VROShapeType shapeType = VROPhysicsShape::getTypeForString(strShapeType);

        // Get the shape params
        int paramsLength = VRO_ARRAY_LENGTH(shapeParams);
        VRO_FLOAT *pointArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(shapeParams);
        std::vector<float> params;
        for (int i = 0; i < paramsLength; i++) {
            params.push_back(pointArray[i]);
        }
        VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(shapeParams, pointArray);

        // Construct a VROPhysicsShape
        propPhysicsShape = propPhysicsShape = std::make_shared<VROPhysicsShape>(shapeType, params);
    }

    // Set the built VROPhysicsShape on the node's physics body
    VROPlatformDispatchAsyncRenderer([node_w, propPhysicsShape] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setPhysicsShape(propPhysicsShape);
        }
    });
}


VRO_METHOD(void, nativeSetPhysicsMass)(VRO_ARGS
                                       VRO_REF(VRONode) nativeRef,
                                       VRO_FLOAT mass) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, mass] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setMass(mass);
        }
    });
}


VRO_METHOD(void, nativeSetPhysicsInertia)(VRO_ARGS
                                          VRO_REF(VRONode) nativeRef,
                                          VRO_FLOAT_ARRAY inertiaArray) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VRO_FLOAT *inertia = VRO_FLOAT_ARRAY_GET_ELEMENTS(inertiaArray);
    VROVector3f vectorInertia = VROVector3f(inertia[0], inertia[1], inertia[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(inertiaArray, inertia);

    VROPlatformDispatchAsyncRenderer([node_w, vectorInertia] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setInertia(vectorInertia);
        }
    });
}

VRO_METHOD(void, nativeSetPhysicsFriction)(VRO_ARGS
                                           VRO_REF(VRONode) nativeRef,
                                           VRO_FLOAT friction) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, friction] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setFriction(friction);
        }
    });
}

VRO_METHOD(void, nativeSetPhysicsRestitution)(VRO_ARGS
                                              VRO_REF(VRONode) nativeRef,
                                              VRO_FLOAT restitution) {

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, restitution] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setRestitution(restitution);
        }
    });
}

VRO_METHOD(void, nativeSetPhysicsUseGravity)(VRO_ARGS
                                             VRO_REF(VRONode) nativeRef,
                                             VRO_BOOL useGravity) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, useGravity] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setUseGravity(useGravity);
        }
    });
}

VRO_METHOD(void, nativeClearPhysicsForce)(VRO_ARGS
                                          VRO_REF(VRONode) nativeRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->clearForces();
        }
    });
}

VRO_METHOD(void, nativeApplyPhysicsForce)(VRO_ARGS
                                          VRO_REF(VRONode) nativeRef,
                                          VRO_FLOAT_ARRAY forceArray,
                                          VRO_FLOAT_ARRAY positionArray) {
    // Grab the physics force to be applied
    VRO_FLOAT *force = VRO_FLOAT_ARRAY_GET_ELEMENTS(forceArray);
    VROVector3f vectorForce = VROVector3f(force[0], force[1], force[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(forceArray, force);

    // Grab the position at which to apply the force at
    VRO_FLOAT *position = VRO_FLOAT_ARRAY_GET_ELEMENTS(positionArray);
    VROVector3f vectorPosition = VROVector3f(position[0], position[1], position[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(positionArray, force);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, vectorForce, vectorPosition] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyForce(vectorForce, vectorPosition);
        }
    });
}


VRO_METHOD(void, nativeApplyPhysicsTorque)(VRO_ARGS
                                           VRO_REF(VRONode) nativeRef,
                                           VRO_FLOAT_ARRAY torqueArray) {
    VRO_FLOAT *torque = VRO_FLOAT_ARRAY_GET_ELEMENTS(torqueArray);
    VROVector3f vectorTorque = VROVector3f(torque[0], torque[1], torque[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(torqueArray, torque);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, vectorTorque] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyTorque(vectorTorque);
        }
    });
}


VRO_METHOD(void, nativeApplyPhysicsImpulse)(VRO_ARGS
                                            VRO_REF(VRONode) nativeRef,
                                            VRO_FLOAT_ARRAY forceArray,
                                            VRO_FLOAT_ARRAY positionArray) {
    // Grab the physics impulse to be applied
    VRO_FLOAT *force = VRO_FLOAT_ARRAY_GET_ELEMENTS(forceArray);
    VROVector3f vectorForce = VROVector3f(force[0], force[1], force[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(forceArray, force);

    // Grab the position at which to apply the impulse at
    VRO_FLOAT *jPos = VRO_FLOAT_ARRAY_GET_ELEMENTS(positionArray);
    VROVector3f vectorPos = VROVector3f(jPos[0], jPos[1], jPos[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(positionArray, jPos);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, vectorForce, vectorPos] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyImpulse(vectorForce, vectorPos);
        }
    });
}


VRO_METHOD(void, nativeApplyPhysicsTorqueImpulse)(VRO_ARGS
                                                  VRO_REF(VRONode) nativeRef,
                                                  VRO_FLOAT_ARRAY torqueArray) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VRO_FLOAT *torque = VRO_FLOAT_ARRAY_GET_ELEMENTS(torqueArray);
    VROVector3f vectorTorque = VROVector3f(torque[0], torque[1], torque[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(torqueArray, torque);

    VROPlatformDispatchAsyncRenderer([node_w, vectorTorque] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->applyTorqueImpulse(vectorTorque);
        }
    });
}

VRO_METHOD(VRO_STRING, nativeIsValidBodyType)(VRO_ARGS
                                              VRO_STRING bodyType,
                                              VRO_FLOAT mass) {
    // Grab the physics body type
    std::string strBodyType = VRO_STRING_STL(bodyType);

    // Verify if the physics body type is valid and return
    std::string errorMsg;
    bool isValid = VROPhysicsBody::isValidType(strBodyType, mass, errorMsg);
    if (isValid) {
        return nullptr;
    } else {
        return VRO_NEW_STRING(errorMsg.c_str());
    }
}

VRO_METHOD(VRO_STRING, nativeIsValidShapeType)(VRO_ARGS
                                               VRO_STRING shapeType,
                                               VRO_FLOAT_ARRAY shapeParams) {
    // Grab the shape type
    std::string strShapeType = VRO_STRING_STL(shapeType);

    // Grab the shape params
    int paramsLength = VRO_ARRAY_LENGTH(shapeParams);
    VRO_FLOAT *pointArray = VRO_FLOAT_ARRAY_GET_ELEMENTS(shapeParams);
    std::vector<float> params;
    for (int i = 0; i < paramsLength; i++) {
        params.push_back(pointArray[i]);
    }
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(shapeParams, pointArray);

// Verify if the shape type and params are valid and return
    std::string errorMsg;
    bool isValid = VROPhysicsShape::isValidShape(strShapeType, params, errorMsg);
    if (isValid) {
        return nullptr;
    } else {
        return VRO_NEW_STRING(errorMsg.c_str());
    }
}

VRO_METHOD(VRO_REF(PhysicsDelegate_JNI), nativeSetPhysicsDelegate)(VRO_ARGS
                                                                   VRO_REF(VRONode) nativeRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    std::shared_ptr<PhysicsDelegate_JNI> delegate = std::make_shared<PhysicsDelegate_JNI>(obj);

    VROPlatformDispatchAsyncRenderer([node_w, delegate] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setPhysicsDelegate(delegate);
        }
    });

    return PhysicsDelegate_JNI::jptr(delegate);
}

VRO_METHOD(void, nativeClearPhysicsDelegate)(VRO_ARGS
                                             VRO_REF(VRONode) nativeRef,
                                             VRO_REF(PhysicsDelegate_JNI) delegateRef) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setPhysicsDelegate(nullptr);
        }
    });
    delete reinterpret_cast<PersistentRef<PhysicsDelegate_JNI> *>(delegateRef);
}


VRO_METHOD(void, nativeSetPhysicsVelocity)(VRO_ARGS
                                           VRO_REF(VRONode) nativeRef,
                                           VRO_FLOAT_ARRAY velocityArray,
                                           VRO_BOOL isConstant) {
    VRO_FLOAT *jVelocity = VRO_FLOAT_ARRAY_GET_ELEMENTS(velocityArray);
    VROVector3f velocity = VROVector3f(jVelocity[0], jVelocity[1], jVelocity[2]);
    VRO_FLOAT_ARRAY_RELEASE_ELEMENTS(velocityArray, jVelocity);

    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, velocity, isConstant] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setVelocity(velocity, isConstant);
        }
    });
}

VRO_METHOD(void, nativeSetPhysicsEnabled)(VRO_ARGS
                                          VRO_REF(VRONode) nativeRef,
                                          VRO_BOOL enabled) {
    std::weak_ptr<VRONode> node_w = Node::native(nativeRef);
    VROPlatformDispatchAsyncRenderer([node_w, enabled] {
        std::shared_ptr<VRONode> node = node_w.lock();
        if (node && node->getPhysicsBody()) {
            node->getPhysicsBody()->setIsSimulated(enabled);
        }
    });
}
}