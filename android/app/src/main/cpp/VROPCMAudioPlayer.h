//
//  VROPCMAudioPlayer.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROPCMAUDIOPLAYER_H
#define ANDROID_VROPCMAUDIOPLAYER_H

#include <unistd.h>
#include <stdint.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

/*
 Plays raw PCM audio data.
 */
class VROPCMAudioPlayer {

public:

    VROPCMAudioPlayer(int sampleRate, SLuint32 numChannels, int bufferSize);
    virtual ~VROPCMAudioPlayer();

    void queueAudio(const char *audio, int size);

private:

    SLObjectItf _audio;
    SLEngineItf _audioEngine;
    SLObjectItf _outputMix;

    SLObjectItf _player;
    SLPlayItf _playState;
    SLAndroidSimpleBufferQueueItf _bufferQueue;
    SLVolumeItf _volume;
    SLmilliHertz _sampleRate;
    int _bufferSize;

    /*
     Upsample the given source sound to this player's sampling rate. Only supports
     simple upsampling (where the sampler's rate is divisible by the source audio's
     rate). Returns nullptr on failure.
     */
    short *upsampleBuffer(const char *source, int sourceSize, uint32_t sourceRate,
                          unsigned *outSize);
};

#endif //ANDROID_VROPCMAUDIOPLAYER_H
