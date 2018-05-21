//
//  ARUtils_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "arcore/VROARHitTestResultARCore.h"
#include "arcore/VROARAnchorARCore.h"
#include "VROARImageAnchor.h"
#include "ARUtils_JNI.h"
#include "VROARPointCloud.h"
#include "VROARPlaneAnchor.h"

/**
 * Creates an ARAnchor from the given VROARPlaneAnchor.
 */
VRO_OBJECT ARUtilsCreateJavaARAnchorFromAnchor(std::shared_ptr<VROARAnchor> anchor) {
    VRO_ENV env = VROPlatformGetJNIEnv();
    std::shared_ptr<VROARAnchorARCore> vAnchor = std::dynamic_pointer_cast<VROARAnchorARCore>(anchor);

    const char *achorIdArr = anchor->getId().c_str();
    VRO_STRING anchorId = VRO_NEW_STRING(achorIdArr);

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
        return VROPlatformConstructHostObject("com/viro/core/ARImageAnchor", "(Ljava/lang/String;Ljava/lang/String;[F[F[F)V",
                                              anchorId, type, positionArray, rotationArray,
                                              scaleArray);
    }

    // Otherwise this anchor has no associated trackable: create a normal ARAnchor object and return it
    /*
     ARAnchor's constructor has the following args:
     String anchorId, String type, float[] position, float[] rotation, float[] scale
     */
    const char *typeArr = "anchor";
    VRO_STRING type = VRO_NEW_STRING(typeArr);
    return VROPlatformConstructHostObject("com/viro/core/ARAnchor", "(Ljava/lang/String;Ljava/lang/String;[F[F[F)V",
                                          anchorId, type, positionArray, rotationArray,
                                          scaleArray);
}

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
        strArr = "NonHorizontal";
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
    VRO_FLOAT tempConfidencesArr[points.size() * 4];

    // populate the array with Vector objects
    for (int i = 0; i < points.size(); i++) {
        tempConfidencesArr[i * 4] = points[i].x;
        tempConfidencesArr[i * 4 + 1] = points[i].y;
        tempConfidencesArr[i * 4 + 2] = points[i].z;
        tempConfidencesArr[i * 4 + 3] = points[i].w;
    }

    // copy confidence values over to a VRO_FLOAT_ARRAY
    VRO_FLOAT_ARRAY jConfidencesArray = VRO_NEW_FLOAT_ARRAY(points.size() * 4);
    VRO_FLOAT_ARRAY_SET(jConfidencesArray, 0, points.size() * 4, tempConfidencesArr);

    // get constructor, create and return an ARPointCloud Java object
    return VROPlatformConstructHostObject("com/viro/core/ARPointCloud", "([F)V", jConfidencesArray);
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

