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
#include <VROARPlane.h>
#include "PersistentRef.h"

namespace ARPlane {
    inline jlong jptr(std::shared_ptr<VROARPlane> shared_plane) {
        PersistentRef<VROARPlane> *native_ar_plane = new PersistentRef<VROARPlane>(shared_plane);
        return reinterpret_cast<intptr_t>(native_ar_plane);
    }

    inline std::shared_ptr<VROARPlane> native(jlong ptr) {
        PersistentRef<VROARPlane> *persistentARPlane = reinterpret_cast<PersistentRef<VROARPlane> *>(ptr);
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

    // Helper functions to create the ARAnchor
    jobject createJavaARAnchorFromPlane(std::shared_ptr<VROARPlaneAnchor> anchor);
    jfloatArray createFloatArrayFromVector3f(VROVector3f vector);
    jstring createStringFromAlignment(VROARPlaneAlignment alignment);

private:
    jobject _javaObject;
    JNIEnv *_env;
};


#endif