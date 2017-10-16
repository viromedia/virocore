//
//  ARPlane_JNI.h
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef ARPlane_JNI_h
#define ARPlane_JNI_h

#include <jni.h>
#include <memory>
#include <VROARPlaneNode.h>
#include "PersistentRef.h"

namespace ARPlane {
    inline jlong jptr(std::shared_ptr<VROARPlaneNode> shared_plane) {
        PersistentRef<VROARPlaneNode> *native_ar_plane = new PersistentRef<VROARPlaneNode>(shared_plane);
        return reinterpret_cast<intptr_t>(native_ar_plane);
    }

    inline std::shared_ptr<VROARPlaneNode> native(jlong ptr) {
        PersistentRef<VROARPlaneNode> *persistentARPlane = reinterpret_cast<PersistentRef<VROARPlaneNode> *>(ptr);
        return persistentARPlane->get();
    }
}

class ARPlaneDelegate : public VROARNodeDelegate {
public:
    ARPlaneDelegate(jobject arNodeObject, JNIEnv *env) {
        _javaObject = reinterpret_cast<jclass>(env->NewGlobalRef(arNodeObject));
        _env = env;
    }

    ~ARPlaneDelegate() {
        _env->DeleteGlobalRef(_javaObject);
    }

    static jlong jptr(std::shared_ptr<ARPlaneDelegate> arNodeDelegate) {
        PersistentRef<ARPlaneDelegate> *persistentDelegate = new PersistentRef<ARPlaneDelegate>(arNodeDelegate);
        return reinterpret_cast<intptr_t>(persistentDelegate);
    }

    static std::shared_ptr<ARPlaneDelegate> native(jlong ptr) {
        PersistentRef<ARPlaneDelegate> *persistentDelegate = reinterpret_cast<PersistentRef<ARPlaneDelegate> *>(ptr);
        return persistentDelegate->get();
    }

    void onARAnchorAttached(std::shared_ptr<VROARAnchor> anchor);
    void onARAnchorUpdated(std::shared_ptr<VROARAnchor> anchor);
    void onARAnchorRemoved();

private:
    jobject _javaObject;
    JNIEnv *_env;
};


#endif