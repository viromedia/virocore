//
//  VROPCMAudioPlayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/15/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROPCMAudioPlayer.h"
#include "VROLog.h"
#include <cmath>

VROPCMAudioPlayer::VROPCMAudioPlayer(int sampleRate, SLuint32 numChannels, int bufferSize) :
        _audio(nullptr),
        _outputMix(nullptr),
        _player(nullptr),
        _audioEngine(nullptr),
        _playState(nullptr),
        _bufferQueue(nullptr),
        _volume(nullptr),
        _sampleRate(sampleRate * 1000),
        _bufferSize(bufferSize) {

    pinfo("Creating PCM audio player [source audio sample rate: %d, buffer size %d, num channels %d]",
          sampleRate, bufferSize, numChannels);
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

    SLuint32 channelMask = SL_SPEAKER_FRONT_CENTER;
    if (numChannels == 2) {
        channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }

    // Configure the audio source
    SLDataLocator_AndroidSimpleBufferQueue locBufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPCM = {SL_DATAFORMAT_PCM, numChannels, SL_SAMPLINGRATE_8,
                                  SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  channelMask, SL_BYTEORDER_LITTLEENDIAN};

    formatPCM.samplesPerSec = _sampleRate;
    SLDataSource audioSource = {&locBufferQueue, &formatPCM};

    // Configure the audio sink
    SLDataLocator_OutputMix locOutputMix = {SL_DATALOCATOR_OUTPUTMIX, _outputMix};
    SLDataSink audioSink = {&locOutputMix, NULL};

    // Create the audio player
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

    // Set the player's state to playing
    result = (*_playState)->SetPlayState(_playState, SL_PLAYSTATE_PLAYING);
    passert(result == SL_RESULT_SUCCESS);
}

VROPCMAudioPlayer::~VROPCMAudioPlayer() {
    if (_audio) {
        (*_audio)->Destroy(_audio);
    }
    if (_outputMix) {
        (*_outputMix)->Destroy(_outputMix);
    }
    if (_player) {
        (*_player)->Destroy(_player);
    }
}

void VROPCMAudioPlayer::queueAudio(const char *audio, int size) {
    SLresult result = (*_bufferQueue)->Enqueue(_bufferQueue, audio, size);
    if (result != SL_RESULT_SUCCESS) {
        pinfo("[audio] error enqueuing PCM data [result: %d]", result);
    }
}

void VROPCMAudioPlayer::setMuted(bool muted) {
    pinfo("[audio] muted set to %d", muted);
    (*_volume)->SetMute(_volume, muted);
}

void VROPCMAudioPlayer::setVolume(float volume) {
    // Standard gain to DB formula (with cutoff near 0 to effectively mute at low gain)
    float volumeDB = volume < .01 ? -96.0f : 20 * std::log10(volume);

    pinfo("[audio] volume set to %f gain, %f db", volume, volumeDB);
    (*_volume)->SetVolumeLevel(_volume, volumeDB * 100);
}