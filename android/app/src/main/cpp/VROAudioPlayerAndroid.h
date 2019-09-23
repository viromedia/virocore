//
//  VROAudioPlayerAndroid.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef ANDROID_VROAUDIOPLAYERANDROID_H
#define ANDROID_VROAUDIOPLAYERANDROID_H

#include "VROAudioPlayer.h"
#include "VROAVPlayer.h"
#include <memory>
#include <string>
#include <VROSoundData.h>

class VROAudioPlayerAndroid : public VROAudioPlayer,
                              public VROAVPlayerDelegate,
                              public VROSoundDataDelegate,
                              public std::enable_shared_from_this<VROAudioPlayerAndroid> {

public:

    VROAudioPlayerAndroid(std::string fileName);
    VROAudioPlayerAndroid(std::shared_ptr<VROSoundData> data);
    virtual ~VROAudioPlayerAndroid();

    void setup();

    void setLoop(bool loop);
    void play();
    void pause();
    void setVolume(float volume);
    void setMuted(bool muted);
    void seekToTime(float seconds);
    void setDelegate(std::shared_ptr<VROSoundDelegateInternal> delegate);

    #pragma mark - VROAVPlayerDelegate
    virtual void willBuffer();
    virtual void didBuffer();
    virtual void onPrepared();
    virtual void onFinished();
    virtual void onError(std::string error);

    #pragma mark VROSoundDataDelegate Implementation
    void dataIsReady();
    void dataError(std::string error);

private:

    VROAVPlayer *_player;
    std::shared_ptr<VROSoundData> _data;
    std::string _fileName;

};

#endif //ANDROID_VROAUDIOPLAYERANDROID_H
