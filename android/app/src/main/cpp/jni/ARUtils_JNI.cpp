//
//  ARUtils_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "ARUtils_JNI.h"
#include "VROARPointCloud.h"


/**
 * Creates an ARAnchor from the given VROARPlaneAnchor.
 */
jobject ARUtilsCreateJavaARAnchorFromAnchor(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    const char *achorIdArr = anchor->getId().c_str();
    jstring anchorId = env->NewStringUTF(achorIdArr);

    VROMatrix4f transform = anchor->getTransform();
    VROVector3f rotationRads = transform.extractRotation(transform.extractScale()).toEuler();
    jfloatArray positionArray = ARUtilsCreateFloatArrayFromVector3f(transform.extractTranslation());
    jfloatArray rotationArray = ARUtilsCreateFloatArrayFromVector3f( {rotationRads.x,
                                                                      rotationRads.y,
                                                                      rotationRads.z});
    jfloatArray scaleArray = ARUtilsCreateFloatArrayFromVector3f(transform.extractScale());

    std::shared_ptr<VROARPlaneAnchor> plane = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    if (plane) {
        /*
         ARPlaneAnchor's constructor has the following args:
         String anchorId, String type, float[] position, float[] rotation,
         float[] scale, String alignment, float[] extent, float[] center
         */
        jclass cls = env->FindClass("com/viro/core/ARPlaneAnchor");
        jmethodID constructor = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;[F[F[FLjava/lang/String;[F[F)V");

        jstring alignment = ARUtilsCreateStringFromAlignment(plane->getAlignment());
        jfloatArray extentArray = ARUtilsCreateFloatArrayFromVector3f(plane->getExtent());
        jfloatArray centerArray = ARUtilsCreateFloatArrayFromVector3f(plane->getCenter());

        const char *typeArr = "plane";
        jstring type = env->NewStringUTF(typeArr);

        return env->NewObject(cls, constructor, anchorId, type, positionArray, rotationArray, scaleArray,
                                            alignment, extentArray, centerArray);
    }
    else {
        /*
         ARAnchor's constructor has the following args:
         String anchorId, String type, float[] position, float[] rotation, float[] scale
         */
        jclass cls = env->FindClass("com/viro/core/ARAnchor");
        jmethodID constructor = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;[F[F[F)V");

        const char *typeArr = "anchor";
        jstring type = env->NewStringUTF(typeArr);
        return env->NewObject(cls, constructor, anchorId, type, positionArray, rotationArray, scaleArray);
    }
}

jfloatArray ARUtilsCreateFloatArrayFromVector3f(VROVector3f vector) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jfloatArray returnArray = env->NewFloatArray(3);
    jfloat tempArr[3];
    tempArr[0] = vector.x; tempArr[1] = vector.y; tempArr[2] = vector.z;
    env->SetFloatArrayRegion(returnArray, 0, 3, tempArr);
    return returnArray;
}

jfloatArray ARUtilsCreateFloatArrayFromMatrix(VROMatrix4f matrix) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jfloatArray returnArray = env->NewFloatArray(16);

    const float *array = matrix.getArray();
    env->SetFloatArrayRegion(returnArray, 0, 16, array);
    return returnArray;
}

jstring ARUtilsCreateStringFromAlignment(VROARPlaneAlignment alignment) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    const char *strArr;
    if (alignment == VROARPlaneAlignment::Horizontal) {
        strArr = "Horizontal";
    }
    else if (alignment == VROARPlaneAlignment::HorizontalUpwards) {
        strArr = "HorizontalUpward";
    }
    else if (alignment == VROARPlaneAlignment::HorizontalDownwards) {
        strArr = "HorizontalDownward";
    }
    else {
        strArr = "NonHorizontal";
    }
    return env->NewStringUTF(strArr);
}

jobject ARUtilsCreateARHitTestResult(VROARHitTestResult result) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jclass arHitTestResultClass = env->FindClass("com/viro/core/ARHitTestResult");

    jmethodID constructorMethod = env->GetMethodID(arHitTestResultClass, "<init>", "(Ljava/lang/String;[F[F[F)V");
    jstring jtypeString;
    jfloatArray jposition = env->NewFloatArray(3);
    jfloatArray jscale = env->NewFloatArray(3);
    jfloatArray jrotation = env->NewFloatArray(3);

    VROVector3f positionVec = result.getWorldTransform().extractTranslation();
    VROVector3f scaleVec = result.getWorldTransform().extractScale();
    VROVector3f rotationVec = result.getWorldTransform().extractRotation(scaleVec).toEuler();

    float position[3] = {positionVec.x, positionVec.y, positionVec.z};
    float scale[3] = {scaleVec.x, scaleVec.y, scaleVec.z};
    float rotation[3] = {rotationVec.x, rotationVec.y, rotationVec.z};

    env->SetFloatArrayRegion(jposition, 0, 3, position);
    env->SetFloatArrayRegion(jscale, 0, 3, scale);
    env->SetFloatArrayRegion(jrotation, 0, 3, rotation);

    const char* typeString;
    // Note: ARCore currently only supports Plane & FeaturePoint. See VROARFrameARCore::hitTest.
    switch (result.getType()) {
        case VROARHitTestResultType::ExistingPlaneUsingExtent:
            typeString = "ExistingPlaneUsingExtent";
            break;
        default: // FeaturePoint
            typeString = "FeaturePoint";
            break;
    }

    jtypeString = env->NewStringUTF(typeString);
    return env->NewObject(arHitTestResultClass, constructorMethod, jtypeString,
                          jposition, jscale, jrotation);
}

jobject ARUtilsCreateARPointCloud(std::shared_ptr<VROARPointCloud> pointCloud) {
    JNIEnv *env = VROPlatformGetJNIEnv();

    std::vector<VROVector4f> points = pointCloud->getPoints();
    jfloat tempConfidencesArr[points.size() * 4];

    // populate the array with Vector objects
    for (int i = 0; i < points.size(); i++) {
        tempConfidencesArr[i*4] = points[i].x;
        tempConfidencesArr[i*4+1] = points[i].y;
        tempConfidencesArr[i*4+2] = points[i].z;
        tempConfidencesArr[i*4+3] = points[i].w;
    }

    // copy confidence values over to a jfloatArray
    jfloatArray jConfidencesArray = env->NewFloatArray(points.size() * 4);
    env->SetFloatArrayRegion(jConfidencesArray, 0, points.size() * 4, tempConfidencesArr);

    // get constructor, create and return an ARPointCloud Java object
    jclass arPointCloudClass = env->FindClass("com/viro/core/ARPointCloud");
    jmethodID constructorMethod = env->GetMethodID(arPointCloudClass, "<init>", "([F)V");

    return env->NewObject(arPointCloudClass, constructorMethod, jConfidencesArray);
}