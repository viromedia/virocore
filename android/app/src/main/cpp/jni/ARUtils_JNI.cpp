//
//  ARUtils_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include <VROPlatformUtil.h>
#include "ARUtils_JNI.h"

/**
 * Creates an ARAnchor from the given VROARPlaneAnchor.
 */
jobject ARUtilsCreateJavaARAnchorFromAnchor(std::shared_ptr<VROARAnchor> anchor) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jclass cls = env->FindClass("com/viro/renderer/ARAnchor");

    /*
     ARAnchor's constructor has the following args:

     String anchorId, String type, float[] position, float[] rotation,
     float[] scale, String alignment, float[] extent, float[] center
     */
    jmethodID constructor = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;[F[F[FLjava/lang/String;[F[F)V");

    VROMatrix4f transform = anchor->getTransform();
    VROVector3f rotationRads = transform.extractRotation(transform.extractScale()).toEuler();

    const char *achorIdArr = anchor->getId().c_str();
    jstring anchorId = env->NewStringUTF(achorIdArr);

    // default type to "anchor", override later if necessary
    const char *typeArr = "anchor";

    jfloatArray positionArray = ARUtilsCreateFloatArrayFromVector3f(transform.extractTranslation());
    jfloatArray rotationArray = ARUtilsCreateFloatArrayFromVector3f(
            {toDegrees(rotationRads.x), toDegrees(rotationRads.y), toDegrees(rotationRads.z)});
    jfloatArray scaleArray = ARUtilsCreateFloatArrayFromVector3f(transform.extractScale());

    // plane-only properties
    jstring alignment = NULL;
    jfloatArray extentArray = NULL;
    jfloatArray centerArray = NULL;

    std::shared_ptr<VROARPlaneAnchor> plane = std::dynamic_pointer_cast<VROARPlaneAnchor>(anchor);
    if (plane) {
        typeArr = "plane";
        alignment = ARUtilsCreateStringFromAlignment(plane->getAlignment());
        extentArray = ARUtilsCreateFloatArrayFromVector3f(plane->getExtent());
        centerArray = ARUtilsCreateFloatArrayFromVector3f(plane->getCenter());
    }

    // create the jstring from type after overriding if necessary.
    jstring type = env->NewStringUTF(typeArr);

    jobject javaAnchor = env->NewObject(cls, constructor, anchorId, type, positionArray, rotationArray, scaleArray,
                                        alignment, extentArray, centerArray);

    return javaAnchor;
}

jfloatArray ARUtilsCreateFloatArrayFromVector3f(VROVector3f vector) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    jfloatArray returnArray = env->NewFloatArray(3);
    jfloat tempArr[3];
    tempArr[0] = vector.x; tempArr[1] = vector.y; tempArr[2] = vector.z;
    env->SetFloatArrayRegion(returnArray, 0, 3, tempArr);
    return returnArray;
}

jstring ARUtilsCreateStringFromAlignment(VROARPlaneAlignment alignment) {
    JNIEnv *env = VROPlatformGetJNIEnv();
    const char *strArr;
    if (alignment == VROARPlaneAlignment::Horizontal) {
        strArr = "Horizontal";
    }
    return env->NewStringUTF(strArr);
}