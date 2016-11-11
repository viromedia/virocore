//
//  VROVideoTextureAndroid.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROVIDEOTEXTUREANDROID_H
#define ANDROID_VROVIDEOTEXTUREANDROID_H

#include "VROVideoTexture.h"

class VROVideoTextureAndroid : public VROVideoTexture {

public:

    VROVideoTextureAndroid();
    virtual ~VROVideoTextureAndroid();

    virtual void loadVideo(std::string url,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                           VRODriver &driver);

    void prewarm();

    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

    void pause();
    void play();
    bool isPaused();
    void seekToTime(int seconds);

    void setMuted(bool muted);
    void setVolume(float volume);
    void setLoop(bool loop);

};

#endif //ANDROID_VROVIDEOTEXTUREANDROID_H
