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

JNI_METHOD(void, nativeLoadOBJFromFile)(JNIEnv *env,
                                        jobject object,
                                        jstring file) {
    const char *cStrFile = env->GetStringUTFChars(file, NULL);
    std::string strFile(cStrFile);
    env->ReleaseStringUTFChars(file, cStrFile);

    jweak weakObj = env->NewWeakGlobalRef(object);

    VROPlatformDispatchAsyncBackground([strFile, weakObj] {
        std::string objUrlPath = VROPlatformCopyResourceToFile(strFile);
        std::string objUrlBase = objUrlPath.substr(0, objUrlPath.find_last_of('/'));

        VROPlatformDispatchAsyncRenderer([objUrlPath, objUrlBase, weakObj] {
            JNIEnv *env = VROPlatformGetJNIEnv();

            jobject localObj = env->NewLocalRef(weakObj);
            if (localObj == NULL) {
                env->DeleteWeakGlobalRef(weakObj);
                return;
            }

            std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(localObj, env);
            std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromFile(objUrlPath, objUrlBase, true, [delegateRef](std::shared_ptr<VRONode> node, bool success) {
                if (!success) {
                    return;
                }
                delegateRef->objLoaded(node);
            });

            env->DeleteLocalRef(localObj);
            env->DeleteWeakGlobalRef(weakObj);
        });
    });
}

JNI_METHOD(void, nativeLoadOBJAndResourcesFromFile)(JNIEnv *env,
                                                     jobject object,
                                                     jstring file,
                                                     jobject resourceMap) {
    const char *cStrFile = env->GetStringUTFChars(file, NULL);
    std::string strFile(cStrFile);
    env->ReleaseStringUTFChars(file, cStrFile);

    jweak weakObj = env->NewWeakGlobalRef(object);
    jweak weakResourceMap = env->NewWeakGlobalRef(resourceMap);

    VROPlatformDispatchAsyncBackground([strFile, weakResourceMap, weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();

        jobject localResourceMap = env->NewLocalRef(weakResourceMap);
        if (localResourceMap == NULL) {
            env->DeleteWeakGlobalRef(weakResourceMap);
            env->DeleteWeakGlobalRef(weakObj);

            return;
        }

        std::string objUrlPath = VROPlatformCopyResourceToFile(strFile);
        std::map<std::string, std::string> cResourceMap = VROPlatformCopyObjResourcesToFile(localResourceMap);

        env->DeleteLocalRef(localResourceMap);
        env->DeleteWeakGlobalRef(weakResourceMap);

        VROPlatformDispatchAsyncRenderer([objUrlPath, cResourceMap, weakObj] {
            JNIEnv *env = VROPlatformGetJNIEnv();

            jobject localObj = env->NewLocalRef(weakObj);
            if (localObj == NULL) {
                env->DeleteWeakGlobalRef(weakObj);
                return;
            }

            std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(localObj, env);
            std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromFileWithResources(objUrlPath, cResourceMap, true,
                [delegateRef](std::shared_ptr<VRONode> node, bool success) {
                    if (!success) {
                        return;
                    }
                    delegateRef->objLoaded(node);
                }
            );

            env->DeleteLocalRef(localObj);
            env->DeleteWeakGlobalRef(weakObj);
        });
    });
}

JNI_METHOD(void, nativeLoadOBJFromUrl)(JNIEnv *env,
                                        jobject object,
                                        jstring url) {
    const char *cStrUrl = env->GetStringUTFChars(url, NULL);
    std::string objUrlPath(cStrUrl);
    std::string objUrlBase = objUrlPath.substr(0, objUrlPath.find_last_of('/'));
    env->ReleaseStringUTFChars(url, cStrUrl);

    jweak weakObj = env->NewWeakGlobalRef(object);

    VROPlatformDispatchAsyncRenderer([objUrlPath, objUrlBase, weakObj] {
        JNIEnv *env = VROPlatformGetJNIEnv();

        jobject localObj = env->NewLocalRef(weakObj);
        if (localObj == NULL) {
            env->DeleteWeakGlobalRef(weakObj);
            return;
        }

        std::shared_ptr<OBJLoaderDelegate> delegateRef = std::make_shared<OBJLoaderDelegate>(localObj, env);
        std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromURL(objUrlPath, objUrlBase, true, [delegateRef](std::shared_ptr<VRONode> node, bool success) {
            if (!success) {
                delegateRef->objFailed("Failed to load OBJ");
            }
            else {
                delegateRef->objLoaded(node);
            }
        });

        env->DeleteLocalRef(localObj);
        env->DeleteWeakGlobalRef(weakObj);
    });
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

    // Need a strong reference to this node since it is not attached
    // to the scene graph, and is immediately destroyed (by ObjectJni)
    // after this function returns
    std::shared_ptr<VRONode> nodeWithObj = Node::native(native_object_ref);
    std::weak_ptr<VRONode> node_w = Node::native(native_node_ref);

    VROPlatformDispatchAsyncRenderer([nodeWithObj, node_w] {
        std::shared_ptr<VRONode> node = node_w.lock();

        if (nodeWithObj && node) {
            std::shared_ptr<VROGeometry> geometry = nodeWithObj->getGeometry();
            node->setGeometry(geometry);
        }
     });
}

} // extern "C"