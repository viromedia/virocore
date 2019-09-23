//
//  PhysicsDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
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

#ifndef ANDROID_PHYSICS_DELEGATE_JNI_H
#define ANDROID_PHYSICS_DELEGATE_JNI_H

#include "VROPhysicsBodyDelegate.h"

#include "VRODefines.h"
#include VRO_C_INCLUDE

/**
 * PhysicsDelegate_JNI implements a JNI abstraction of the VROPhysicsBodyDelegate to
 * both allow java objects to register for, and to notify them of physics delegate
 * events across the JNI bridge.
 */
class PhysicsDelegate_JNI : public VROPhysicsBodyDelegate {

public:
    PhysicsDelegate_JNI(VRO_OBJECT videoJavaObject);
    ~PhysicsDelegate_JNI();

    void onCollided(std::string key, VROPhysicsBody::VROCollision collision);

private:
        VRO_OBJECT _javaObject;
};
#endif
