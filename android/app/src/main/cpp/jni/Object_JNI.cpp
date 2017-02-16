//
// Object_JNI.cpp
// ViroRenderer
//
// Copyright © 2016 Viro Media. All rights reserved.

#include <jni.h>
#include <memory>
#include "VROOBJLoader.h"

#include "VROMaterial.h"
#include "VROGeometry.h"
#include "VROPlatformUtil.h"
#include "PersistentRef.h"
#include "VRONode.h"
#include "Node_JNI.h"
#include "OBJLoaderDelegate_JNI.h"


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_renderer_jni_ObjectJni_##method_name

extern "C" {

JNI_METHOD(jlong, nativeLoadOBJFromFile)(JNIEnv *env,
                                         jobject object,
                                         jstring file) {
    const char *cStrFile = env->GetStringUTFChars(file, NULL);
    std::string strFile(cStrFile);
    std::string objUrlPath = VROPlatformCopyResourceToFile(strFile);
    std::string objUrlBase = objUrlPath.substr(0, objUrlPath.find_last_of('/'));
    env->ReleaseStringUTFChars(file, cStrFile);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(object, env);
    std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromFile(objUrlPath, objUrlBase, true, [delegateRef](std::shared_ptr<VRONode> node, bool success) {
        if (!success) {
            return;
        }
        delegateRef->objLoaded();
    });

    return Node::jptr(objNode);
}

JNI_METHOD(jlong, nativeLoadOBJFromUrl)(JNIEnv *env,
                                         jobject object,
                                         jstring url) {
    const char *cStrUrl = env->GetStringUTFChars(url, NULL);
    std::string objUrlPath(cStrUrl);
    std::string objUrlBase = objUrlPath.substr(0, objUrlPath.find_last_of('/'));
    env->ReleaseStringUTFChars(url, cStrUrl);
    std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(object, env);
    std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromURL(objUrlPath, objUrlBase, true, [delegateRef](std::shared_ptr<VRONode> node, bool success) {
        if (!success) {
            return;
        }
        delegateRef->objLoaded();
    });

    return Node::jptr(objNode);
}
JNI_METHOD(void, nativeDestroyNode)(JNIEnv *env,
                                   jclass clazz,
                                   jlong native_node_ref) {
    delete reinterpret_cast<PersistentRef<VRONode> *>(native_node_ref);
}

JNI_METHOD(void, nativeAttachToNode)(JNIEnv *env,
                                     jclass clazz,
                                     jlong native_object_ref,
                                     jlong native_node_ref) {
    std::shared_ptr<VRONode> nodeWithObj = Node::native(native_object_ref);
    std::shared_ptr<VROGeometry> geometry = nodeWithObj->getGeometry();
    Node::native(native_node_ref)->setGeometry(geometry);
}
} // extern "C"