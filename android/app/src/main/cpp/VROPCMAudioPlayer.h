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

    /*
     Create a new audio for playing PCM audio with the given properties.
     The sample rate and channels are derived from the source audio; the
     buffer size can be retrieved from the device (see VROPlatformUtils).
     */
    VROPCMAudioPlayer(int sampleRate, SLuint32 numChannels, int bufferSize);
    virtual ~VROPCMAudioPlayer();

    void setMuted(bool muted);
    void setVolume(float volume);

    /*
     Queue the given raw PCM data to be played.
     */
    void queueAudio(const char *audio, int size);

private:

    /*
     Allocated audio engine components. Must be deallocated.
     */
    SLObjectItf _audio;
    SLObjectItf _outputMix;
    SLObjectItf _player;

    /*
     Audio interfaces.
     */
    SLEngineItf _audioEngine;
    SLPlayItf _playState;
    SLAndroidSimpleBufferQueueItf _bufferQueue;
    SLVolumeItf _volume;
    SLmilliHertz _sampleRate;
    int _bufferSize;

};

#endif //ANDROID_VROPCMAUDIOPLAYER_H
