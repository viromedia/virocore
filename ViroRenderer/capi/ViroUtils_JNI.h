//
//  ViroUtils_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIROUTILS_JNI_H
#define ANDROID_VIROUTILS_JNI_H

#include "VRODefines.h"
#include VRO_C_INCLUDE

#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROBoundingBox.h"

class VROARHitTestResultARCore;

VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromVector3f(VROVector3f vector);
VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromMatrix(VROMatrix4f matrix);
VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromBoundingBox(VROBoundingBox boundingBox);
VRO_FLOAT_ARRAY ARUtilsCreatePointsArray(std::vector<VROVector3f> points);

#endif //ANDROID_VIROUTILS_JNI_H
