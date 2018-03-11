//
//  ARDeclarativeImageNode_JNI.cpp
//  ViroRenderer
//
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "ARDeclarativeImageNode_JNI.h"
#include "ARImageTarget_JNI.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_viro_core_internal_ARDeclarativeImageNode_##method_name

extern "C" {

JNI_METHOD(jlong, nativeCreateARImageTargetNode)(JNIEnv *env, jclass clazz) {
    std::shared_ptr<VROARDeclarativeImageNode> arImageNode = std::make_shared<VROARDeclarativeImageNode>();
    return ARDeclarativeImageNode::jptr(arImageNode);
}

JNI_METHOD(void, nativeSetARImageTarget)(JNIEnv *env,
                                         jobject jobject,
                                         jlong nativeARImageNode,
                                         jlong nativeARImageTarget) {
    std::shared_ptr<VROARDeclarativeImageNode> arImageNode = ARDeclarativeImageNode::native(nativeARImageNode);
    std::shared_ptr<VROARImageTargetAndroid> arImageTarget = ARImageTarget::native(nativeARImageTarget);
    arImageNode->setImageTarget(arImageTarget);
}

} // extern "C"