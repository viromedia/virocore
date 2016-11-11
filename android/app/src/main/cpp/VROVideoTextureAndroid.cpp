//
//  VROVideoTextureAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTextureSubstrateOpenGL.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include "VROVideoTextureAndroid.h"
#include "VROFrameSynchronizer.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"

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
    _paused(true),
    _loop(true) {

    _data = {-1, NULL, NULL, NULL, 0, false, false, false, false};
}

VROVideoTextureAndroid::~VROVideoTextureAndroid() {

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

    _data.window = ANativeWindow_fromSurface(env, jsurface);
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

    _data.fd = fd;
    VROVideoData *d = &_data;

    AMediaExtractor *ex = AMediaExtractor_new();
    media_status_t err = AMediaExtractor_setDataSourceFd(ex, d->fd,
                                                         static_cast<off64_t>(outStart),
                                                         static_cast<off64_t>(outLen));
    close(d->fd);
    if (err != AMEDIA_OK) {
        pinfo("[video]: setDataSource error: %d", err);
        return;
    }

    int numtracks = AMediaExtractor_getTrackCount(ex);

    AMediaCodec *codec = NULL;

    pinfo("[video]: input has %d tracks", numtracks);
    for (int i = 0; i < numtracks; i++) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(ex, i);
        const char *s = AMediaFormat_toString(format);
        pinfo("[video]: track %d format: %s", i, s);
        const char *mime;
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            pinfo("no mime type");
            return;
        } else if (!strncmp(mime, "video/", 6)) {
            // Omitting most error handling for clarity.
            // Production code should check for errors.
            AMediaExtractor_selectTrack(ex, i);
            codec = AMediaCodec_createDecoderByType(mime);
            AMediaCodec_configure(codec, format, d->window, NULL, 0);
            d->ex = ex;
            d->codec = codec;
            d->renderstart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            d->isPlaying = false;
            d->renderonce = true;
            AMediaCodec_start(codec);
        }
        AMediaFormat_delete(format);
    }

    _looper = new VROVideoLooper();
    _looper->post(kMsgCodecBuffer, &_data);
}

void VROVideoTextureAndroid::prewarm() {

}

void VROVideoTextureAndroid::onFrameWillRender(const VRORenderContext &context) {
    if (_data.sawOutputEOS) {
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
    _looper->post(kMsgPause, &_data);
    _paused = true;
}

void VROVideoTextureAndroid::play() {
    _looper->post(kMsgResume, &_data);
    _paused = false;
}

bool VROVideoTextureAndroid::isPaused() {
    return _paused;
}

void VROVideoTextureAndroid::seekToTime(int seconds) {
    VROVideoSeek *seek = (VROVideoSeek *) malloc(sizeof(VROVideoSeek));
    seek->data = &_data;
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

void VROVideoLooper::handle(int what, void *obj) {
    switch (what) {
        case kMsgCodecBuffer:
            doCodecWork((VROVideoData *) obj);
            break;

        case kMsgDecodeDone: {
            VROVideoData *d = (VROVideoData *) obj;
            AMediaCodec_stop(d->codec);
            AMediaCodec_delete(d->codec);
            AMediaExtractor_delete(d->ex);
            d->sawInputEOS = true;
            d->sawOutputEOS = true;
        }
            break;

        case kMsgSeek: {
            VROVideoSeek *seek = (VROVideoSeek *)obj;
            VROVideoData *d = (VROVideoData *) seek->data;
            AMediaExtractor_seekTo(d->ex, seek->seekTime, AMEDIAEXTRACTOR_SEEK_NEXT_SYNC);
            AMediaCodec_flush(d->codec);
            d->renderstart = -1;
            d->sawInputEOS = false;
            d->sawOutputEOS = false;
            if (!d->isPlaying) {
                d->renderonce = true;
                post(kMsgCodecBuffer, d);
            }
            pinfo("[video]: seeked to %llu", seek->seekTime);
            free (seek);
        }
            break;

        case kMsgPause: {
            VROVideoData *d = (VROVideoData *) obj;
            if (d->isPlaying) {
                // flush all outstanding codecbuffer messages with a no-op message
                d->isPlaying = false;
                post(kMsgPauseAck, NULL, true);
            }
        }
            break;

        case kMsgResume: {
            VROVideoData *d = (VROVideoData *) obj;
            if (!d->isPlaying) {
                d->renderstart = -1;
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

void VROVideoLooper::doCodecWork(VROVideoData *d) {
    ssize_t bufidx = -1;
    if (!d->sawInputEOS) {
        bufidx = AMediaCodec_dequeueInputBuffer(d->codec, 2000);
        if (bufidx >= 0) {
            size_t bufsize;
            auto buf = AMediaCodec_getInputBuffer(d->codec, bufidx, &bufsize);
            auto sampleSize = AMediaExtractor_readSampleData(d->ex, buf, bufsize);
            if (sampleSize < 0) {
                sampleSize = 0;
                d->sawInputEOS = true;
                pinfo("[video]: EOS");
            }
            auto presentationTimeUs = AMediaExtractor_getSampleTime(d->ex);

            AMediaCodec_queueInputBuffer(d->codec, bufidx, 0, sampleSize, presentationTimeUs,
                                         d->sawInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
            AMediaExtractor_advance(d->ex);
        }
    }

    if (!d->sawOutputEOS) {
        AMediaCodecBufferInfo info;
        auto status = AMediaCodec_dequeueOutputBuffer(d->codec, &info, 0);
        if (status >= 0) {
            if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                pinfo("[video]: output EOS");
                d->sawOutputEOS = true;
            }
            int64_t presentationNano = info.presentationTimeUs * 1000;
            if (d->renderstart < 0) {
                d->renderstart = systemnanotime() - presentationNano;
            }
            int64_t delay = (d->renderstart + presentationNano) - systemnanotime();
            if (delay > 0) {
                usleep(delay / 1000);
            }
            AMediaCodec_releaseOutputBuffer(d->codec, status, info.size != 0);
            if (d->renderonce) {
                d->renderonce = false;
                return;
            }
        } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
            pinfo("[video]: output buffers changed");
        } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            auto format = AMediaCodec_getOutputFormat(d->codec);
            pinfo("[video]: format changed to: %s", AMediaFormat_toString(format));
            AMediaFormat_delete(format);
        } else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
        } else {
            pinfo("[video]: unexpected info code: %zd", status);
        }
    }

    if (!d->sawInputEOS || !d->sawOutputEOS) {
        post(kMsgCodecBuffer, d);
    }
}