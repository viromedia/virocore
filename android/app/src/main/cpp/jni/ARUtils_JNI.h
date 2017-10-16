//
//  ARUtils_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ANDROID_ARUTILS_JNI_H
#define ANDROID_ARUTILS_JNI_H


#include <jni.h>
#include <VROARPlaneAnchor.h>

// Helper functions to create a Java ARAnchor object
jobject ARUtilsCreateJavaARAnchorFromAnchor(std::shared_ptr<VROARAnchor> anchor);
jfloatArray ARUtilsCreateFloatArrayFromVector3f(VROVector3f vector);
jstring ARUtilsCreateStringFromAlignment(VROARPlaneAlignment alignment);


#endif //ANDROID_ARUTILS_JNI_H
