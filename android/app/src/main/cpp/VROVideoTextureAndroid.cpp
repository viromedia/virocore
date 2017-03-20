//
//  VROVideoTextureAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTextureSubstrateOpenGL.h"
#include <unistd.h>
#include "VROVideoTextureAndroid.h"
#include "VROFrameSynchronizer.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include "VROPCMAudioPlayer.h"
#include "VRODriverOpenGL.h"

enum {
    kMsgCodecBuffer,
    kMsgPause,
    kMsgResume,
    kMsgPauseAck,
    kMsgDestroy,
    kMsgSeek,
    kMsgLoop,
    kMsgSetLoop,
    kMsgVolume,
    kMsgMute
};

VROVideoTextureAndroid::VROVideoTextureAndroid() :
        VROVideoTexture(VROTextureType::TextureEGLImage),
        _looper(nullptr),
        _paused(true) {

}

VROVideoTextureAndroid::~VROVideoTextureAndroid() {
    killVideo();
}

void VROVideoTextureAndroid::killVideo() {
    if (_looper) {
        _looper->quit();
        delete (_looper);
        _looper = nullptr;
    }
}

ANativeWindow *VROVideoTextureAndroid::createVideoTexture(GLuint *textureId,
                                                          std::shared_ptr<VRODriverOpenGL> driver) {
    glGenTextures(1, textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, *textureId);

    // Can't do mipmapping with video textures, and clamp to edge is only option
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
            new VROTextureSubstrateOpenGL(GL_TEXTURE_EXTERNAL_OES, *textureId, driver, true));
    setSubstrate(std::move(substrate));

    JNIEnv *env = VROPlatformGetJNIEnv();
    jobject jsurface = VROPlatformCreateVideoSink(*textureId);

    return ANativeWindow_fromSurface(env, jsurface);
}

void VROVideoTextureAndroid::loadVideo(std::string url,
                                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                       std::shared_ptr<VRODriver> driver) {

    // Note: the Android implementation does not need to be added as a frame listener

    killVideo();

    AMediaExtractor *extractor = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSource(extractor, url.c_str());
    if (err != AMEDIA_OK) {
        pinfo("[video] error loading video from URL [%s], error [%d]", url.c_str(), err);
        AMediaExtractor_delete(extractor);

        return;
    }

    loadVideo(extractor, driver);
}

void VROVideoTextureAndroid::loadVideoFromAsset(std::string asset,
                                                std::shared_ptr<VRODriver> driver) {
    killVideo();
    AAssetManager *assetMgr = VROPlatformGetAssetManager();

    off_t outStart, outLen;
    int fd = AAsset_openFileDescriptor(AAssetManager_open(assetMgr, asset.c_str(), 0),
                                       &outStart, &outLen);
    if (fd < 0) {
        pinfo("[video] failed to open URL: %s %d (%s)", asset.c_str(), fd, strerror(errno));
        return;
    }

    AMediaExtractor *extractor = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSourceFd(extractor, fd,
                                                         static_cast<off64_t>(outStart),
                                                         static_cast<off64_t>(outLen));
    close(fd);
    if (err != AMEDIA_OK) {
        pinfo("[video] error loading video from asset [%s], error [%d]", asset.c_str(), err);
        AMediaExtractor_delete(extractor);

        return;
    }

    loadVideo(extractor, driver);
}

void VROVideoTextureAndroid::loadVideo(AMediaExtractor *extractor, std::shared_ptr<VRODriver> driver) {
    /*
     IMPORTANT: quit() must be invoked on the looper before deleting it,
     to perform necessary cleanup on the worker thread.
     */
    _looper = new VROVideoLooper();

    VROMediaData &mediaData = _looper->getData();
    mediaData.window = nullptr;
    mediaData.extractor = nullptr;
    mediaData.videoCodec = nullptr;
    mediaData.audioCodec = nullptr;
    mediaData.renderStart = -1;
    mediaData.sawInputEOS = false;
    mediaData.sawOutputEOS = false;
    mediaData.isPlaying = false;
    mediaData.renderOnce = true;
    mediaData.deviceAudioSampleRate = VROPlatformGetAudioSampleRate();
    mediaData.deviceAudioBufferSize = VROPlatformGetAudioBufferSize();
    mediaData.audioNumChannels = 1;
    mediaData.loop = false;
    mediaData.volume = 1.0;
    mediaData.muted = false;
    mediaData.extractor = extractor;
    mediaData.window = createVideoTexture(&mediaData.textureId,
                                          std::dynamic_pointer_cast<VRODriverOpenGL>(driver));

    int numTracks = AMediaExtractor_getTrackCount(extractor);

    pinfo("[video] input has %d tracks", numTracks);
    for (int i = 0; i < numTracks; i++) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(extractor, i);
        const char *s = AMediaFormat_toString(format);
        pinfo("[video] track %d format: %s", i, s);

        const char *mime;
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            pinfo("[video] track %d has no mime type, ignoring", i);
            return;
        }
        else if (!strncmp(mime, "video/", 6)) {
            AMediaExtractor_selectTrack(extractor, i);
            mediaData.videoCodec = new VROCodec(VROCodecType::Video, mime, format, mediaData.window, i);
        }
        else if (!strncmp(mime, "audio/", 6)) {
            AMediaExtractor_selectTrack(extractor, i);
            mediaData.audioCodec = new VROCodec(VROCodecType::Audio, mime, format, nullptr, i);

            AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &mediaData.sourceAudioSampleRate);
            AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &mediaData.audioNumChannels);

            pinfo("[video] audio track has sample rate %d, num channels %d",
                  mediaData.sourceAudioSampleRate, mediaData.audioNumChannels);
        }

        AMediaFormat_delete(format);
    }

    _looper->post(kMsgCodecBuffer, nullptr);
}

void VROVideoTextureAndroid::prewarm() {

}

void VROVideoTextureAndroid::onFrameWillRender(const VRORenderContext &context) {

}

void VROVideoTextureAndroid::onFrameDidRender(const VRORenderContext &context) {

}

void VROVideoTextureAndroid::pause() {
    if (!_looper) {
        pinfo("[video] pause ignored, no video is loaded");
        return;
    }

    _looper->post(kMsgPause, nullptr);
    _paused = true;
}

void VROVideoTextureAndroid::play() {
    if (!_looper) {
        pinfo("[video] play ignored, no video is loaded");
        return;
    }

    _looper->post(kMsgResume, nullptr);
    _paused = false;
}

bool VROVideoTextureAndroid::isPaused() {
    return _paused;
}

void VROVideoTextureAndroid::seekToTime(float seconds) {
    if (!_looper) {
        pinfo("[video] seek ignored, no video is loaded");
        return;
    }

    VROVideoSeek *seek = (VROVideoSeek *) malloc(sizeof(VROVideoSeek));
    seek->seekTime = seconds * 1000000;

    _looper->post(kMsgSeek, seek);
}

void VROVideoTextureAndroid::setMuted(bool muted) {
    if (!_looper) {
        pinfo("[video] mute ignored, no video is loaded");
        return;
    }

    VROVideoMute *mute = (VROVideoMute *) malloc(sizeof(VROVideoMute));
    mute->muted = muted;

    _looper->post(kMsgMute, mute);
}

void VROVideoTextureAndroid::setVolume(float volume) {
    if (!_looper) {
        pinfo("[video] set volume ignored, no video is loaded");
        return;
    }

    VROVideoVolume *vol = (VROVideoVolume *) malloc(sizeof(VROVideoVolume));
    vol->volume = volume;

    _looper->post(kMsgVolume, vol);
}

void VROVideoTextureAndroid::setLoop(bool loop) {
    if (!_looper) {
        pinfo("[video] set loop ignored, no video is loaded");
        return;
    }

    VROVideoLoop *lp = (VROVideoLoop *) malloc(sizeof(VROVideoLoop));
    lp->loop = loop;

    _looper->post(kMsgSetLoop, lp);
}

VROVideoLooper::~VROVideoLooper() {
    if (_mediaData.window) {
        VROPlatformDestroyVideoSink(_mediaData.textureId);
        ANativeWindow_release(_mediaData.window);
    }
    if (_mediaData.extractor) {
        AMediaExtractor_delete(_mediaData.extractor);
    }
    if (_mediaData.textureId) {
        glDeleteTextures(1, &_mediaData.textureId);
    }

    delete (_mediaData.videoCodec);
    delete (_mediaData.audioCodec);
}

void VROVideoLooper::quit() {
    post(kMsgDestroy, nullptr, true);
    VROLooper::quit();

    delete (_audio);
}

void VROVideoLooper::handle(int what, void *obj) {
    switch (what) {
        case kMsgCodecBuffer:
            doCodecWork();
            break;

        case kMsgDestroy: {
            pinfo("[video] received message [destroy]");

            /*
             This message should always be sent with a flush, since
             we clean up here.
             */
            VROMediaData *d = &_mediaData;
            d->sawInputEOS = true;
            d->sawOutputEOS = true;
        }
            break;

        case kMsgMute: {
            pinfo("[video] received message [mute]");

            VROVideoMute *mute = (VROVideoMute *) obj;

            // If audio is null, muted will be set on kMsgResume
            _mediaData.muted = mute->muted;
            if (_audio) {
                _audio->setMuted(mute->muted);
            }
            free (mute);
        }
        break;

        case kMsgVolume: {
            pinfo("[video] received message [volume]");

            VROVideoVolume *volume = (VROVideoVolume *) obj;

            // If _audio is null, volume will be set on kMsgResume
            _mediaData.volume = volume->volume;
            if (_audio) {
                _audio->setVolume(volume->volume);
            }
            free (volume);
        }
        break;

        case kMsgSetLoop: {
            pinfo("[video] received message [set loop]");

            VROVideoLoop *loop = (VROVideoLoop *) obj;
            _mediaData.loop = loop->loop;

            free (loop);
        }
            break;

        case kMsgSeek: {
            pinfo("[video] received message [seek]");

            VROVideoSeek *seek = (VROVideoSeek *) obj;
            VROMediaData *d = &_mediaData;

            AMediaExtractor_seekTo(d->extractor, seek->seekTime, AMEDIAEXTRACTOR_SEEK_NEXT_SYNC);
            if (d->videoCodec != nullptr) {
                d->videoCodec->flush();
            }
            if (d->audioCodec != nullptr) {
                d->audioCodec->flush();
            }
            d->renderStart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            if (!d->isPlaying) {
                d->renderOnce = true;
                post(kMsgCodecBuffer, d);
            }

            pinfo("[video] seeked to %llu", seek->seekTime);
            free(seek);
        }
            break;

        case kMsgPause: {
            pinfo("[video] received message [pause]");

            VROMediaData *d = &_mediaData;
            if (d->isPlaying) {
                if (d->videoCodec) {
                    d->videoCodec->clear();
                }
                if (d->audioCodec) {
                    d->audioCodec->clear();
                }
                // flush all outstanding codecbuffer messages with a no-op message
                d->isPlaying = false;
                post(kMsgPauseAck, NULL, true);
            }
        }
            break;

        case kMsgResume: {
            pinfo("[video] received message [resume]");

            VROMediaData *d = &_mediaData;
            if (!d->isPlaying) {
                if (!_audio) {
                    _audio = new VROPCMAudioPlayer(d->sourceAudioSampleRate,
                                                   d->audioNumChannels,
                                                   d->deviceAudioBufferSize);
                }
                _audio->setVolume(d->volume);
                _audio->setMuted(d->muted);

                d->renderStart = -1;
                d->isPlaying = true;
                post(kMsgCodecBuffer, d);
            }
        }
            break;

        case kMsgLoop: {
            pinfo("[video] received message [loop]");
            VROMediaData *d = &_mediaData;

            AMediaExtractor_seekTo(d->extractor, 0, AMEDIAEXTRACTOR_SEEK_NEXT_SYNC);
            if (d->videoCodec) {
                d->videoCodec->flush();
            }
            if (d->audioCodec) {
                d->audioCodec->flush();
            }
            d->renderStart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;

            if (!d->isPlaying) {
                d->renderOnce = true;
            }
            post(kMsgCodecBuffer, d);
        }
            break;

    }
}

int64_t systemnanotime() {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000000LL + now.tv_nsec;
}

void VROVideoLooper::doCodecWork() {
    VROMediaData *d = &_mediaData;

    /*
     In this block we read encoded track data from the extractor, and
     write into the codec for decoding.
     */
    if (!d->sawInputEOS) {

        // Get the track index of the current sample
        int trackIndex = AMediaExtractor_getSampleTrackIndex(d->extractor);
        if (trackIndex == -1) {
            /*
             If there are no more samples, then queue one more buffer
             in each codec so we can pass the END_OF_STREAM flag.
             */
            pinfo("[video] no tracks available, end of stream reached");

            if (d->videoCodec) {
                writeToCodec(d->videoCodec, d->extractor, &d->sawInputEOS);
            }
            if (d->audioCodec) {
                writeToCodec(d->audioCodec, d->extractor, &d->sawInputEOS);
            }
        }
        else {
            VROCodec *codec = d->codecForTrack(trackIndex);
            writeToCodec(codec, d->extractor, &d->sawInputEOS);
        }

        // Advance to the next sample
        AMediaExtractor_advance(d->extractor);
    }

    /*
     In this block we read decoded track data from the codecs, and store the
     output buffers that are ready to be written into the audio/video sinks.
     */
    if (!d->sawOutputEOS) {
        if (d->videoCodec) {
            readFromCodec(d->videoCodec, &d->sawOutputEOS);
        }
        if (d->audioCodec) {
            readFromCodec(d->audioCodec, &d->sawOutputEOS);
        }
    }

    /*
     Finally in this block we render any available audio or video
     output buffers, delaying until the first buffer's presentation time.
     */
    if (d->videoCodec && d->audioCodec) {
        writeToSinks(d->videoCodec, d->audioCodec, &d->renderStart);
    }
    else if (d->videoCodec) {
        writeToVideoSink(d->videoCodec, &d->renderStart);
    }
    else if (d->audioCodec) {
        writeToAudioSink(d->audioCodec, &d->renderStart);
    }

    if (d->renderOnce) {
        d->renderOnce = false;
    }

    if (d->isPlaying) {
        /*
         If we're reached the EOS, either loop or pause.
         */
        if (d->sawInputEOS && d->sawOutputEOS) {
            pinfo("[video] end of stream");
            if (d->loop) {
                post(kMsgLoop, d);
            }
            else {
                post(kMsgPause, d);
            }
        }
        /*
         Otherwise keep playing.
         */
        else {
            post(kMsgCodecBuffer, d);
        }
    }
}

void VROVideoLooper::writeToCodec(VROCodec *codec, AMediaExtractor *extractor,
                                  bool *outSawInputEOS) {
    /*
     Retrieve an input buffer from the codec, and read the sample
     data into it.
     */
    ssize_t bufferIndex = codec->dequeueInputBuffer(2000);
    if (bufferIndex >= 0) {
        size_t bufsize;
        uint8_t *buf = codec->getInputBuffer(bufferIndex, &bufsize);

        ssize_t sampleSize = AMediaExtractor_readSampleData(extractor, buf, bufsize);
        if (sampleSize < 0) {
            sampleSize = 0;
            *outSawInputEOS = true;
            pinfo("[video] no samples to extract, end of stream reached");
        }

        /*
         Now that the sample data is in the input buffer, queue it back into the
         codec for processing (decoding). The presentation time is simply passed
         through; we'll get it back on the decoding side.
         */
        int64_t presentationTimeUs = AMediaExtractor_getSampleTime(extractor);
        codec->queueInputBuffer(bufferIndex, 0, sampleSize, presentationTimeUs,
                                *outSawInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
    }
}

void VROVideoLooper::readFromCodec(VROCodec *codec, bool *outSawOutputEOS) {
    AMediaCodecBufferInfo info;
    ssize_t status = codec->dequeueOutputBuffer(&info, 0);
    if (status >= 0) {
        if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
            pinfo("[video] dequeued output buffer at end of stream");
            *outSawOutputEOS = true;
        }

        VROCodecOutputBuffer readyBuffer(status, info);
        codec->pushOutputBuffer(readyBuffer);
    }
    else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
        pinfo("[video] output buffers changed");
    }
    else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        auto format = codec->getOutputFormat();
        pinfo("[video] format changed to: %s", AMediaFormat_toString(format));
        AMediaFormat_delete(format);
    }
    else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
    }
    else {
        pinfo("[video] unexpected info code: %zd", status);
    }
}

void VROVideoLooper::writeToSinks(VROCodec *videoCodec, VROCodec *audioCodec, int64_t *renderStart) {
    /*
     If both are available, wait until the first's presentation time and
     render it, or render both if both are ready.
     */
    if (videoCodec->hasOutputBuffer() && audioCodec->hasOutputBuffer()) {
        VROCodecOutputBuffer videoBuffer = videoCodec->nextOutputBuffer();
        int64_t videoDelay = computeDelayToRender(videoBuffer, renderStart);

        VROCodecOutputBuffer audioBuffer = audioCodec->nextOutputBuffer();
        int64_t audioDelay = computeDelayToRender(audioBuffer, renderStart);

        if (videoDelay > 0 && audioDelay > 0) {
            if (videoDelay < audioDelay) {
                usleep(videoDelay / 1000);
                renderVideo(videoBuffer, videoCodec);
                videoCodec->popOutputBuffer();
            }
            else {
                usleep(audioDelay / 1000);
                renderAudio(audioBuffer, audioCodec);
                audioCodec->popOutputBuffer();
            }
        }
        else {
            if (videoDelay < 0) {
                renderVideo(videoBuffer, videoCodec);
                videoCodec->popOutputBuffer();
            }
            if (audioDelay < 0) {
                renderAudio(audioBuffer, audioCodec);
                audioCodec->popOutputBuffer();
            }
        }
    }

    /*
     If there's only a video buffer available, wait until its presentation time
     and render it.
     */
    else if (videoCodec->hasOutputBuffer()) {
        writeToVideoSink(videoCodec, renderStart);
    }

    /*
     If there's only an audio buffer available, same thing.
     */
    else if (audioCodec->hasOutputBuffer()) {
        writeToAudioSink(audioCodec, renderStart);
    }
}


void VROVideoLooper::writeToVideoSink(VROCodec *codec, int64_t *renderStart) {
    if (!codec->hasOutputBuffer()) {
        return;
    }
    VROCodecOutputBuffer outputBuffer = codec->nextOutputBuffer();

    int64_t delay = computeDelayToRender(outputBuffer, renderStart);
    if (delay > 0) {
        usleep(delay / 1000);
    }
    renderVideo(outputBuffer, codec);
    codec->popOutputBuffer();
}

void VROVideoLooper::writeToAudioSink(VROCodec *codec, int64_t *renderStart) {
    if (!codec->hasOutputBuffer()) {
        return;
    }
    VROCodecOutputBuffer outputBuffer = codec->nextOutputBuffer();

    int64_t delay = computeDelayToRender(outputBuffer, renderStart);
    if (delay > 0) {
        usleep(delay / 1000);
    }

    renderAudio(outputBuffer, codec);
    codec->popOutputBuffer();
}

int64_t VROVideoLooper::computeDelayToRender(VROCodecOutputBuffer buffer, int64_t *renderStart) {
    /*
     If the render start time is not yet initialized, then we're going
     to render this frame now. So set the render start time to the
     past: current time minus the presentation time of this frame. This
     way,

     renderStart + presentationNano = currentTime

     meaning renderStart will represent the time at which we started the
     video.
     */
    int64_t presentationNano = buffer.info.presentationTimeUs * 1000;
    int64_t currentTime = systemnanotime();
    if (*renderStart < 0) {
        *renderStart = currentTime - presentationNano;
        pinfo("[video] render start reset to %lld", (*renderStart / 1000));
    }

    /*
     See how long we have to wait to render this frame.
     */
    int64_t delay = (*renderStart + presentationNano) - currentTime;
    return delay;
}

void VROVideoLooper::renderVideo(VROCodecOutputBuffer buffer, VROCodec *codec) {
    /*
     This call performs the actual rendering of the buffer to the
     provided surface (by setting the third param to true).
     */
    codec->releaseOutputBuffer(buffer.index, buffer.info.size != 0);
}

void VROVideoLooper::renderAudio(VROCodecOutputBuffer buffer, VROCodec *codec) {
    /*
     Render audio by writing the PCM data to our PCM audio player.
     */
    size_t bufSize;
    uint8_t *audioBuffer = codec->getOutputBuffer(buffer.index, &bufSize);
    _audio->queueAudio((char *) audioBuffer, buffer.info.size);

    codec->releaseOutputBuffer(buffer.index, false);
}
