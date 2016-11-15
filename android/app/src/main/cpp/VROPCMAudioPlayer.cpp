//
//  VROPCMAudioPlayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROPCMAudioPlayer.h"
#include "VROLog.h"

// pre-recorded sound clips, both are 8 kHz mono 16-bit signed little endian
static const char hello[] =
#include "hello_clip.h"
;

VROPCMAudioPlayer::VROPCMAudioPlayer(int sampleRate, int bufferSize) :
        _sampleRate(sampleRate * 1000),
        _bufferSize(bufferSize) {

    SLresult result;

    // Create the Open SLES audio and engine interfaces
    result = slCreateEngine(&_audio, 0, NULL, 0, NULL, NULL);
    passert(result == SL_RESULT_SUCCESS);

    result = (*_audio)->Realize(_audio, SL_BOOLEAN_FALSE);
    passert(result == SL_RESULT_SUCCESS);

    result = (*_audio)->GetInterface(_audio, SL_IID_ENGINE, &_audioEngine);
    passert(result == SL_RESULT_SUCCESS);

    // Create and realize the output mix
    result = (*_audioEngine)->CreateOutputMix(_audioEngine, &_outputMix, 0, NULL, NULL);
    passert(result == SL_RESULT_SUCCESS);

    result = (*_outputMix)->Realize(_outputMix, SL_BOOLEAN_FALSE);
    passert(result == SL_RESULT_SUCCESS);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPCM = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_8,
                                  SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    formatPCM.samplesPerSec = _sampleRate;
    SLDataSource audioSource = {&loc_bufq, &formatPCM};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, _outputMix};
    SLDataSink audioSink = {&loc_outmix, NULL};

    /*
     Create the audio player.
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*_audioEngine)->CreateAudioPlayer(_audioEngine, &_player, &audioSource, &audioSink, 2,
                                                ids, req);
    passert(result == SL_RESULT_SUCCESS);

    // Retrieve, the player, play state, buffer queue, and volume interfaces
    result = (*_player)->Realize(_player, SL_BOOLEAN_FALSE);
    passert(result == SL_RESULT_SUCCESS);

    result = (*_player)->GetInterface(_player, SL_IID_PLAY, &_playState);
    passert(result == SL_RESULT_SUCCESS);

    result = (*_player)->GetInterface(_player, SL_IID_BUFFERQUEUE, &_bufferQueue);
    passert(result == SL_RESULT_SUCCESS);

    result = (*_player)->GetInterface(_player, SL_IID_VOLUME, &_volume);
    passert(result == SL_RESULT_SUCCESS);

    // register callback on the buffer queue
    //result = (*_bufferQueue)->RegisterCallback(_bufferQueue, playerCallback, NULL);
    //passert(result == SL_RESULT_SUCCESS);
    //(void)result;

    // Set the player's state to playing
    result = (*_playState)->SetPlayState(_playState, SL_PLAYSTATE_PLAYING);
    passert(result == SL_RESULT_SUCCESS);
}

VROPCMAudioPlayer::~VROPCMAudioPlayer() {

}

void VROPCMAudioPlayer::playClip() {
    unsigned nextSize;
    short *nextBuffer = upsampleBuffer(hello, sizeof(hello), SL_SAMPLINGRATE_8, &nextSize);
    if (!nextBuffer) {
        nextBuffer = (short *) hello;
        nextSize = sizeof(hello);
    }

    if (nextSize > 0) {
        // here we only enqueue one buffer because it is a long clip,
        // but for streaming playback we would typically enqueue at least 2 buffers to start
        SLresult result;
        result = (*_bufferQueue)->Enqueue(_bufferQueue, nextBuffer, nextSize);
    }
}

void VROPCMAudioPlayer::queueAudio(const char *audio, int size) {
    SLresult result;
    result = (*_bufferQueue)->Enqueue(_bufferQueue, audio, size);

    //passert(result == SL_RESULT_SUCCESS);
}

short *VROPCMAudioPlayer::upsampleBuffer(const char *source, int sourceSize, uint32_t sourceRate,
                                            unsigned *outSize) {
    if (_sampleRate == 0) {
        return nullptr;
    }

    // Simple up-sampling, must be divisible
    if (_sampleRate % sourceRate) {
        return nullptr;
    }

    int upSampleRate = _sampleRate / sourceRate;

    int32_t sourceSampleCount = sourceSize >> 1;
    short *src = (short *) source;

    short *resampleBuf = (short *) malloc((sourceSampleCount * upSampleRate) << 1);
    if (resampleBuf == nullptr) {
        return resampleBuf;
    }
    short *workBuf = resampleBuf;
    for (int sample = 0; sample < sourceSampleCount; sample++) {
        for (int dup = 0; dup < upSampleRate; dup++) {
            *workBuf++ = src[sample];
        }
    }

    *outSize = (sourceSampleCount * upSampleRate) << 1;
    return resampleBuf;
}