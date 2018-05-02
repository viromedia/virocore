//
//  VideoDelegate_JNI.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VIDEO_DELEGATE_JNI_H
#define ANDROID_VIDEO_DELEGATE_JNI_H

#include "VROVideoDelegateInternal.h"

class VideoDelegate : public VROVideoDelegateInternal {
    public:
    VideoDelegate(VRO_OBJECT videoJavaObject);
    ~VideoDelegate();

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
