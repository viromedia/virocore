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

enum {
    kMsgCodecBuffer,
    kMsgPause,
    kMsgResume,
    kMsgPauseAck,
    kMsgDecodeDone,
    kMsgSeek,
};

VROVideoTextureAndroid::VROVideoTextureAndroid() :
        VROVideoTexture(VROTextureType::TextureEGLImage),
        _looper(nullptr),
        _paused(true),
        _loop(true) {

    _mediaData.fd = -1;
    _mediaData.window = nullptr;
    _mediaData.extractor = nullptr;
    _mediaData.videoCodec = nullptr;
    _mediaData.audioCodec = nullptr;
    _mediaData.renderStart = 0;
    _mediaData.sawInputEOS = false;
    _mediaData.sawOutputEOS = false;
    _mediaData.isPlaying = false;
    _mediaData.renderOnce = false;
}

VROVideoTextureAndroid::~VROVideoTextureAndroid() {
    delete (_looper);
}

void VROVideoTextureAndroid::createVideoTexture() {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);

    // Can't do mipmapping with video textures, and clamp to edge is only option
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
            new VROTextureSubstrateOpenGL(GL_TEXTURE_EXTERNAL_OES, textureId, true));
    setSubstrate(std::move(substrate));

    JNIEnv *env = VROPlatformGetJNIEnv();
    jobject jsurface = VROPlatformCreateVideoSink(textureId);

    _mediaData.window = ANativeWindow_fromSurface(env, jsurface);
}

void VROVideoTextureAndroid::loadVideo(std::string url,
                                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                       VRODriver &driver) {

    frameSynchronizer->addFrameListener(shared_from_this());
    createVideoTexture();

    JNIEnv *env = VROPlatformGetJNIEnv();
    AAssetManager *assetMgr = VROPlatformGetAssetManager();

    off_t outStart, outLen;
    int fd = AAsset_openFileDescriptor(AAssetManager_open(assetMgr, "testfile.mp4", 0),
                                       &outStart, &outLen);
    if (fd < 0) {
        pinfo("[video]: failed to open URL: %s %d (%s)", url.c_str(), fd, strerror(errno));
        return;
    }

    _mediaData.fd = fd;

    VROMediaData *mediaData = &_mediaData;

    AMediaExtractor *extractor = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSourceFd(extractor, mediaData->fd,
                                                         static_cast<off64_t>(outStart),
                                                         static_cast<off64_t>(outLen));
    close(mediaData->fd);
    if (err != AMEDIA_OK) {
        pinfo("[video]: setDataSource error: %d", err);
        return;
    }

    int numTracks = AMediaExtractor_getTrackCount(extractor);

    pinfo("[video]: input has %d tracks", numTracks);
    for (int i = 0; i < numTracks; i++) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(extractor, i);
        const char *s = AMediaFormat_toString(format);
        pinfo("[video]: track %d format: %s", i, s);

        const char *mime;
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            pinfo("[video]: no mime type");
            return;
        }
        else if (!strncmp(mime, "video/", 6)) {
            AMediaExtractor_selectTrack(extractor, i);

            AMediaCodec *codec = AMediaCodec_createDecoderByType(mime);
            AMediaCodec_configure(codec, format, mediaData->window, NULL, 0);
            AMediaCodec_start(codec);

            mediaData->videoCodec = new VROCodec(VROCodecType::Video, codec, i);
        }
        else if (!strncmp(mime, "audio/", 6)) {
            AMediaExtractor_selectTrack(extractor, i);

            AMediaCodec *codec = AMediaCodec_createDecoderByType(mime);
            AMediaCodec_configure(codec, format, NULL, NULL, 0);
            AMediaCodec_start(codec);

            mediaData->audioCodec = new VROCodec(VROCodecType::Audio, codec, i);
        }

        AMediaFormat_delete(format);
    }

    mediaData->extractor = extractor;
    mediaData->renderStart = -1;
    mediaData->sawInputEOS = false;
    mediaData->sawOutputEOS = false;
    mediaData->isPlaying = false;
    mediaData->renderOnce = true;

    _looper = new VROVideoLooper();
    _looper->post(kMsgCodecBuffer, &_mediaData);
}

void VROVideoTextureAndroid::prewarm() {

}

void VROVideoTextureAndroid::onFrameWillRender(const VRORenderContext &context) {
    if (_mediaData.sawOutputEOS) {
        pause();
        if (_loop) {
            seekToTime(0);
            play();
        }
    }
}

void VROVideoTextureAndroid::onFrameDidRender(const VRORenderContext &context) {

}

void VROVideoTextureAndroid::pause() {
    _looper->post(kMsgPause, &_mediaData);
    _paused = true;
}

void VROVideoTextureAndroid::play() {
    _looper->post(kMsgResume, &_mediaData);
    _paused = false;
}

bool VROVideoTextureAndroid::isPaused() {
    return _paused;
}

void VROVideoTextureAndroid::seekToTime(int seconds) {
    VROVideoSeek *seek = (VROVideoSeek *) malloc(sizeof(VROVideoSeek));
    seek->data = &_mediaData;
    seek->seekTime = seconds * 1000000;

    _looper->post(kMsgSeek, seek);
}

void VROVideoTextureAndroid::setMuted(bool muted) {

}

void VROVideoTextureAndroid::setVolume(float volume) {

}

void VROVideoTextureAndroid::setLoop(bool loop) {
    _loop = loop;
}

VROVideoLooper::~VROVideoLooper() {
    delete (_audio);
}

void VROVideoLooper::handle(int what, void *obj) {
    if (_audio == nullptr) {
        _audio = new VROPCMAudioPlayer(48000, 960);
    }
    switch (what) {
        case kMsgCodecBuffer:
            doCodecWork((VROMediaData *) obj);
            break;

        case kMsgDecodeDone: {
            VROMediaData *d = (VROMediaData *) obj;

            if (d->videoCodec != nullptr) {
                d->videoCodec->stop();
            }
            if (d->audioCodec != nullptr) {
                d->audioCodec->stop();
            }

            AMediaExtractor_delete(d->extractor);
            d->sawInputEOS = true;
            d->sawOutputEOS = true;
        }
            break;

        case kMsgSeek: {
            VROVideoSeek *seek = (VROVideoSeek *) obj;
            VROMediaData *d = (VROMediaData *) seek->data;
            AMediaExtractor_seekTo(d->extractor, seek->seekTime, AMEDIAEXTRACTOR_SEEK_NEXT_SYNC);

            if (d->videoCodec != nullptr) {
                d->videoCodec->flush();
            }
            if (d->audioCodec != nullptr) {
                d->videoCodec->flush();
            }
            d->renderStart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            if (!d->isPlaying) {
                d->renderOnce = true;
                post(kMsgCodecBuffer, d);
            }

            pinfo("[video]: seeked to %llu", seek->seekTime);
            free(seek);
        }
            break;

        case kMsgPause: {
            VROMediaData *d = (VROMediaData *) obj;
            if (d->isPlaying) {
                // flush all outstanding codecbuffer messages with a no-op message
                d->isPlaying = false;
                post(kMsgPauseAck, NULL, true);
            }
        }
            break;

        case kMsgResume: {
            VROMediaData *d = (VROMediaData *) obj;
            if (!d->isPlaying) {
                d->renderStart = -1;
                d->isPlaying = true;
                post(kMsgCodecBuffer, d);
            }
        }
            break;
    }
}

int64_t systemnanotime() {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000000000LL + now.tv_nsec;
}

void VROVideoLooper::doCodecWork(VROMediaData *d) {
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
            pinfo("[video]: track index -1, end of stream reached");

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
        return;
    }

    if (!d->sawInputEOS || !d->sawOutputEOS) {
        post(kMsgCodecBuffer, d);
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
            pinfo("[video]: sample size 0, end of stream reached");
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
            pinfo("[video]: output EOS");
            *outSawOutputEOS = true;
        }

        VROCodecOutputBuffer readyBuffer(status, info);
        codec->pushOutputBuffer(readyBuffer);
    }
    else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
        pinfo("[video]: output buffers changed");
    }
    else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
        auto format = codec->getOutputFormat();
        pinfo("[video]: format changed to: %s", AMediaFormat_toString(format));
        AMediaFormat_delete(format);
    }
    else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
    }
    else {
        pinfo("[video]: unexpected info code: %zd", status);
    }
}

void VROVideoLooper::writeToSinks(VROCodec *videoCodec, VROCodec *audioCodec, int64_t *renderStart) {
    if (!videoCodec->hasOutputBuffer() && !audioCodec->hasOutputBuffer()) {
        return;
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

    /*
     Otherwise both are available, so wait until the first's presentation time and
     render it, or render both if both are ready.
     */
    else {
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
