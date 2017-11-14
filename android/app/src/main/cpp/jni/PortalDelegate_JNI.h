/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */


#ifndef ANDROID_PORTALDELEGATE_JNI_H
#define ANDROID_PORTALDELEGATE_JNI_H

#include <jni.h>
#include <PersistentRef.h>
#include "VROPortalDelegate.h"

class PortalDelegate: public VROPortalDelegate {
    public:
    PortalDelegate(jobject javaObject);
    ~PortalDelegate();

    static jlong jptr(std::shared_ptr<PortalDelegate> shared_node) {
        PersistentRef<PortalDelegate> *portalDelegate = new PersistentRef<PortalDelegate>(shared_node);
        return reinterpret_cast<intptr_t>(portalDelegate);
    }

    static std::shared_ptr<PortalDelegate> native(jlong ptr) {
        PersistentRef<PortalDelegate> *persistentSurface = reinterpret_cast<PersistentRef<PortalDelegate> *>(ptr);
        return persistentSurface->get();
    }

    virtual void onPortalEnter();
    virtual void onPortalExit();
private:
    jobject _javaObject;
};


#endif //ANDROID_PORTALDELEGATE_JNI_H
