//
//  VideoDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIDEO_DELEGATE_JNI_H
#define ANDROID_VIDEO_DELEGATE_JNI_H

#include <jni.h>
#include <PersistentRef.h>
#include "VROVideoDelegateInternal.h"

class VideoDelegate : public VROVideoDelegateInternal {
    public:
    VideoDelegate(VRO_OBJECT videoJavaObject);
    ~VideoDelegate();

    static VRO_REF jptr(std::shared_ptr<VideoDelegate> shared_node) {
        PersistentRef<VideoDelegate> *native_surface = new PersistentRef<VideoDelegate>(shared_node);
        return reinterpret_cast<intptr_t>(native_surface);
    }

    static std::shared_ptr<VideoDelegate> native(VRO_REF ptr) {
        PersistentRef<VideoDelegate> *persistentSurface = reinterpret_cast<PersistentRef<VideoDelegate> *>(ptr);
        return persistentSurface->get();
    }

    /*
     * Notification mechanism to let the bridge know that the surface has been created
     */
    virtual void onReady();

    /**
     * Video Delegate Callbacks
     */
    virtual void videoWillBuffer();
    virtual void videoDidBuffer();
    virtual void videoDidFinish();
    virtual void onVideoUpdatedTime(float seconds, float duration);
    virtual void videoDidFail(std::string error);

private:
        VRO_OBJECT _javaObject;
};
#endif
