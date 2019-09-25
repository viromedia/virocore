//
//  ViroUtils_JNI.cpp
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

#include <VROPlatformUtil.h>
#include "ViroUtils_JNI.h"
#include "VROHitTestResult.h"
#include "VRONode.h"
#

VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromVector3f(VROVector3f vector) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_FLOAT_ARRAY returnArray = VRO_NEW_FLOAT_ARRAY(3);
    VRO_FLOAT tempArr[3];
    tempArr[0] = vector.x; tempArr[1] = vector.y; tempArr[2] = vector.z;
    VRO_FLOAT_ARRAY_SET(returnArray, 0, 3, tempArr);
    return returnArray;
}

VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromMatrix(VROMatrix4f matrix) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_FLOAT_ARRAY returnArray = VRO_NEW_FLOAT_ARRAY(16);

    const float *array = matrix.getArray();
    VRO_FLOAT_ARRAY_SET(returnArray, 0, 16, array);
    return returnArray;
}

VRO_FLOAT_ARRAY ARUtilsCreateFloatArrayFromBoundingBox(VROBoundingBox boundingBox) {
    VRO_ENV env = VROPlatformGetJNIEnv();

    VRO_FLOAT tempArr[6];
    tempArr[0] = boundingBox.getMinX(); tempArr[1] = boundingBox.getMaxX();
    tempArr[2] = boundingBox.getMinY(); tempArr[3] = boundingBox.getMaxY();
    tempArr[4] = boundingBox.getMinZ(); tempArr[5] = boundingBox.getMaxZ();

    VRO_FLOAT_ARRAY returnArray = VRO_NEW_FLOAT_ARRAY(6);
    VRO_FLOAT_ARRAY_SET(returnArray, 0, 6, tempArr);
    return returnArray;
}

VRO_FLOAT_ARRAY ARUtilsCreatePointsArray(std::vector<VROVector3f> points) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    VRO_FLOAT tempPointsArr[points.size() * 3];

    // populate the array with Vector objects
    for (int i = 0; i < points.size(); i++) {
        tempPointsArr[i * 3] = points[i].x;
        tempPointsArr[i * 3 + 1] = points[i].y;
        tempPointsArr[i * 3 + 2] = points[i].z;
    }

    VRO_FLOAT_ARRAY jPointsArray = VRO_NEW_FLOAT_ARRAY(points.size() * 3);
    VRO_FLOAT_ARRAY_SET(jPointsArray, 0, points.size() * 3, tempPointsArr);
    return jPointsArray;
}

VRO_OBJECT ARUtilsCreateHitTestResult(VROHitTestResult result) {
    VRO_ENV env = VROPlatformGetJNIEnv();

    VRO_FLOAT distance;
    VRO_FLOAT_ARRAY jIntersectionPoint = VRO_NEW_FLOAT_ARRAY(3);
    VROVector3f intersectionVec = result.getLocation();

    VRO_STRING tag = NULL;
    if(result.getNode() != NULL) {
        std::string tag_s = result.getNode()->getTag();
        if (!tag_s.empty()) {
            tag = VRO_NEW_STRING(tag_s.c_str());
        }
    }

    float intersectionPoint[3] = {intersectionVec.x, intersectionVec.y, intersectionVec.z};
    VRO_FLOAT_ARRAY_SET(jIntersectionPoint, 0, 3, intersectionPoint);

    return VROPlatformConstructHostObject("com/viro/core/HitTestResult",
                                          "(Ljava/lang/String;F[F)V", tag, distance, jIntersectionPoint);
}

