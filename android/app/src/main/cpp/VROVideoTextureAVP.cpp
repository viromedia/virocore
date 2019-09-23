//
//  VROVideoTextureAVP.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/16.
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

#include "VROVideoTextureAVP.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROPlatformUtil.h"
#include "VRODriverOpenGL.h"

VROVideoTextureAVP::VROVideoTextureAVP(VROStereoMode stereoMode) :
    VROVideoTexture(VROTextureType::TextureEGLImage, stereoMode),
    _textureId(0)
{

}

VROVideoTextureAVP::~VROVideoTextureAVP() {
    delete (_player);

    std::shared_ptr<VRODriverOpenGL> driver = _driver.lock();
    if (_textureId && driver) {
        driver->deleteTexture(_textureId);
    }
}

void VROVideoTextureAVP::init() {
    _player = new VROAVPlayer();
}

void VROVideoTextureAVP::setDelegate(std::shared_ptr<VROVideoDelegateInternal> delegate) {
    VROVideoTexture::setDelegate(delegate);
    std::shared_ptr<VROAVPlayerDelegate> avDelegate = std::dynamic_pointer_cast<VROAVPlayerDelegate>(shared_from_this());
    _player->setDelegate(avDelegate);
}

void VROVideoTextureAVP::loadVideo(std::string url,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {
    _player->setDataSourceURL(url.c_str());
    _driver = std::dynamic_pointer_cast<VRODriverOpenGL>(driver);

    frameSynchronizer->removeFrameListener(std::dynamic_pointer_cast<VROVideoTexture>(shared_from_this()));
    frameSynchronizer->addFrameListener(std::dynamic_pointer_cast<VROVideoTexture>(shared_from_this()));
}

void VROVideoTextureAVP::loadVideoFromURL(std::string url, std::shared_ptr<VRODriver> driver) {
    _player->setDataSourceURL(url.c_str());
}

void VROVideoTextureAVP::prewarm() {

}

void VROVideoTextureAVP::onFrameWillRender(const VRORenderContext &context) {
    VROVideoTexture::updateVideoTime();
}

void VROVideoTextureAVP::onFrameDidRender(const VRORenderContext &context) { }

void VROVideoTextureAVP::play() {
    _player->play();
}

void VROVideoTextureAVP::pause() {
    _player->pause();
}

bool VROVideoTextureAVP::isPaused() {
    return _player->isPaused();
}

void VROVideoTextureAVP::seekToTime(float seconds) {
    float totalDuration = getVideoDurationInSeconds();

    // Clamp the seek time at a minimum of 0,
    // and a maxiumum of the video's duration period.
    if (seconds > totalDuration){
        seconds = totalDuration;
    } else if (seconds < 0){
        seconds = 0;
    }
    _player->seekToTime(seconds);
}

float VROVideoTextureAVP::getCurrentTimeInSeconds() {
    return _player->getCurrentTimeInSeconds();
}

float VROVideoTextureAVP::getVideoDurationInSeconds() {
    return _player->getVideoDurationInSeconds();
}

void VROVideoTextureAVP::setMuted(bool muted) {
    _player->setMuted(muted);
}

void VROVideoTextureAVP::setVolume(float volume) {
    _player->setVolume(volume);
}

void VROVideoTextureAVP::setLoop(bool loop) {
    _player->setLoop(loop);
}

void VROVideoTextureAVP::bindSurface(std::shared_ptr<VRODriverOpenGL> driver) {
    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureId);

    // Can't do mipmapping with video textures, and clamp to edge is only option
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
            new VROTextureSubstrateOpenGL(GL_TEXTURE_EXTERNAL_OES, _textureId, driver, true));
    setSubstrate(0, std::move(substrate));

    _player->setSurface(_textureId);
}

#pragma mark - VROAVPlayerDelegate

void VROVideoTextureAVP::willBuffer() {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->videoWillBuffer();
    }
}

void VROVideoTextureAVP::didBuffer() {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->videoDidBuffer();
    }
}

void VROVideoTextureAVP::onPrepared() {
    // no-op
}

void VROVideoTextureAVP::onFinished() {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->videoDidFinish();
    }
}

void VROVideoTextureAVP::onError(std::string error) {
    std::shared_ptr<VROVideoDelegateInternal> delegate = _delegate.lock();
    if (delegate) {
        delegate->videoDidFail(error);
    }
}
