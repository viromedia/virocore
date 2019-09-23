//
//  ARUtils_JNI.cpp
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

#include "ARUtils_JNI.h"
#include <VROPlatformUtil.h>
#include "VROARHitTestResult.h"
#include "arcore/VROARAnchorARCore.h"
#include "VROARImageAnchor.h"
#include "ViroUtils_JNI.h"
#include "VROARPointCloud.h"
#include "VROARPlaneAnchor.h"

/**
 * Creates an ARAnchor from the given VROARPlaneAnchor.
 */
VRO_OBJECT ARUtilsCreateJavaARAnchorFromAnchor(std::shared_ptr<VROARAnchor> anchor) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    std::shared_ptr<VROARAnchorARCore> vAnchor = std::dynamic_pointer_cast<VROARAnchorARCore>(anchor);

    const char *anchorId_c = anchor->getId().c_str();
    VRO_STRING anchorId = VRO_NEW_STRING(anchorId_c);

    VRO_STRING cloudAnchorId = NULL;
    std::string cloudAnchorId_s = vAnchor->getCloudAnchorId();
    if (!cloudAnchorId_s.empty()) {
        cloudAnchorId = VRO_NEW_STRING(cloudAnchorId_s.c_str());
    }

    VROMatrix4f transform = anchor->getTransform();
    VROVector3f rotationRads = transform.extractRotation(transform.extractScale()).toEuler();
    VRO_FLOAT_ARRAY positionArray = ARUtilsCreateFloatArrayFromVector3f(transform.extractTranslation());
    VRO_FLOAT_ARRAY rotationArray = ARUtilsCreateFloatArrayFromVector3f( {rotationRads.x,
                                                                          rotationRads.y,
                                                                          rotationRads.z });
    VRO_FLOAT_ARRAY scaleArray = ARUtilsCreateFloatArrayFromVector3f(transform.extractScale());


    // Create an ARPlaneAnchor if necessary and return.
    std::shared_ptr<VROARPlaneAnchor> plane = std::dynamic_pointer_cast<VROARPlaneAnchor>(vAnchor->getTrackable());
    if (plane) {
        /*
         ARPlaneAnchor's constructor has the following args:
         String anchorId, String type, float[] position, float[] rotation,
         float[] scale, String alignment, float[] extent, float[] center
         */
        VRO_STRING alignment = ARUtilsCreateStringFromAlignment(plane->getAlignment());
        VRO_FLOAT_ARRAY extentArray = ARUtilsCreateFloatArrayFromVector3f(plane->getExtent());
        VRO_FLOAT_ARRAY centerArray = ARUtilsCreateFloatArrayFromVector3f(plane->getCenter());
        VRO_FLOAT_ARRAY polygonPointsArray = ARUtilsCreatePointsArray(plane->getBoundaryVertices());

        const char *typeArr = "plane";
        VRO_STRING type = VRO_NEW_STRING(typeArr);

        return VROPlatformConstructHostObject("com/viro/core/ARPlaneAnchor", "(Ljava/lang/String;Ljava/lang/String;[F[F[FLjava/lang/String;[F[F[F)V",
                                              anchorId, type, positionArray, rotationArray,
                                              scaleArray, alignment, extentArray, centerArray,
                                              polygonPointsArray);
    }

    // Create an ARImageAnchor in necessary and return.
    std::shared_ptr<VROARImageAnchor> imageAnchor = std::dynamic_pointer_cast<VROARImageAnchor>(vAnchor->getTrackable());
    if (imageAnchor) {
        /*
         ARImageAnchor's constructor has the following args:
         String anchorId, String type, float[] position, float[] rotation, float[] scale
         */
        const char *typeArr = "image";
        VRO_STRING type = VRO_NEW_STRING(typeArr);

        VRO_STRING trackingMethod;
        switch(imageAnchor->getTrackingMethod()) {
            case VROARImageTrackingMethod::NotTracking:
                trackingMethod = VRO_NEW_STRING("notTracking");
                break;
            case VROARImageTrackingMethod::Tracking:
                trackingMethod = VRO_NEW_STRING("tracking");
                break;
            case VROARImageTrackingMethod::LastKnownPose:
                trackingMethod = VRO_NEW_STRING("lastKnownPose");
                break;
        }

        return VROPlatformConstructHostObject("com/viro/core/ARImageAnchor", "(Ljava/lang/String;Ljava/lang/String;[F[F[FLjava/lang/String;)V",
                                              anchorId, type, positionArray, rotationArray,
                                              scaleArray, trackingMethod);
    }

    // Otherwise this anchor has no associated trackable: create a normal ARAnchor object and return it
    /*
     ARAnchor's constructor has the following args:
     String anchorId, String type, float[] position, float[] rotation, float[] scale
     */
    const char *typeArr = "anchor";
    VRO_STRING type = VRO_NEW_STRING(typeArr);
    return VROPlatformConstructHostObject("com/viro/core/ARAnchor",
                                          "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[F[F[F)V",
                                          anchorId, cloudAnchorId, type, positionArray, rotationArray,
                                          scaleArray);
}

VRO_STRING ARUtilsCreateStringFromAlignment(VROARPlaneAlignment alignment) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    const char *strArr;
    if (alignment == VROARPlaneAlignment::Horizontal) {
        strArr = "Horizontal";
    }
    else if (alignment == VROARPlaneAlignment::HorizontalUpward) {
        strArr = "HorizontalUpward";
    }
    else if (alignment == VROARPlaneAlignment::HorizontalDownward) {
        strArr = "HorizontalDownward";
    }
    else {
        strArr = "Vertical";
    }
    return VRO_NEW_STRING(strArr);
}

VRO_OBJECT ARUtilsCreateARHitTestResult(std::shared_ptr<VROARHitTestResult> result) {
    VRO_ENV env = VROPlatformGetJNIEnv();

    VRO_STRING jtype;
    VRO_FLOAT_ARRAY jposition = VRO_NEW_FLOAT_ARRAY(3);
    VRO_FLOAT_ARRAY jscale = VRO_NEW_FLOAT_ARRAY(3);
    VRO_FLOAT_ARRAY jrotation = VRO_NEW_FLOAT_ARRAY(3);

    VROVector3f positionVec = result->getWorldTransform().extractTranslation();
    VROVector3f scaleVec = result->getWorldTransform().extractScale();
    VROVector3f rotationVec = result->getWorldTransform().extractRotation(scaleVec).toEuler();

    float position[3] = {positionVec.x, positionVec.y, positionVec.z};
    float scale[3] = {scaleVec.x, scaleVec.y, scaleVec.z};
    float rotation[3] = {rotationVec.x, rotationVec.y, rotationVec.z};

    VRO_FLOAT_ARRAY_SET(jposition, 0, 3, position);
    VRO_FLOAT_ARRAY_SET(jscale, 0, 3, scale);
    VRO_FLOAT_ARRAY_SET(jrotation, 0, 3, rotation);

    const char *type;
    // Note: ARCore currently only supports Plane & FeaturePoint. See VROARFrameARCore::hitTest.
    switch (result->getType()) {
        case VROARHitTestResultType::ExistingPlaneUsingExtent:
            type = "ExistingPlaneUsingExtent";
            break;
        default: // FeaturePoint
            type = "FeaturePoint";
            break;
    }

    jtype = VRO_NEW_STRING(type);
    VRO_REF(VROARHitTestResult) ref = VRO_REF_NEW(VROARHitTestResult, result);
    return VROPlatformConstructHostObject("com/viro/core/ARHitTestResult",
                                          "(JLjava/lang/String;[F[F[F)V",
                                          ref, jtype, jposition, jscale, jrotation);
}

VRO_OBJECT ARUtilsCreateARPointCloud(std::shared_ptr<VROARPointCloud> pointCloud) {
    VRO_ENV env = VROPlatformGetJNIEnv();

    std::vector<VROVector4f> points = pointCloud->getPoints();
    std::vector<uint64_t> identifiers = pointCloud->getIdentifiers();
    VRO_FLOAT tempConfidencesArr[points.size() * 4];
    VRO_LONG identifiersArr[identifiers.size()];

    // populate the array with Vector objects
    for (int i = 0; i < points.size(); i++) {
        tempConfidencesArr[i * 4] = points[i].x;
        tempConfidencesArr[i * 4 + 1] = points[i].y;
        tempConfidencesArr[i * 4 + 2] = points[i].z;
        tempConfidencesArr[i * 4 + 3] = points[i].w;
    }

    for(int i = 0; i< identifiers.size(); i++) {
        identifiersArr[i] = (jlong)identifiers[i];
    }

    // copy confidence values over to a VRO_FLOAT_ARRAY
    VRO_FLOAT_ARRAY jConfidencesArray = VRO_NEW_FLOAT_ARRAY(points.size() * 4);
    VRO_FLOAT_ARRAY_SET(jConfidencesArray, 0, points.size() * 4, tempConfidencesArr);

    jlongArray jCloudIdArray = env->NewLongArray(identifiers.size());
    env->SetLongArrayRegion(jCloudIdArray, 0, identifiers.size(), identifiersArr);

    // get constructor, create and return an ARPointCloud Java object
    return VROPlatformConstructHostObject("com/viro/core/ARPointCloud", "([F[J)V", jConfidencesArray, jCloudIdArray);
}

