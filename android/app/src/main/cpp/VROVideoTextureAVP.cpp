//
//  VROVideoTextureAVP.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/18/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoTextureAVP.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROPlatformUtil.h"

VROVideoTextureAVP::VROVideoTextureAVP() :
    VROVideoTexture(VROTextureType::TextureEGLImage),
    _textureId(0) {
    _player = new VROAVPlayer();
}

VROVideoTextureAVP::~VROVideoTextureAVP() {
    delete (_player);

    if (_textureId) {
        glDeleteTextures(1, &_textureId);
    }
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

    frameSynchronizer->removeFrameListener(shared_from_this());
    frameSynchronizer->addFrameListener(shared_from_this());
}

void VROVideoTextureAVP::loadVideoFromURL(std::string url, std::shared_ptr<VRODriver> driver) {
    _player->setDataSourceURL(url.c_str());
}

void VROVideoTextureAVP::loadVideoFromAsset(std::string asset, std::shared_ptr<VRODriver> driver) {
    _player->setDataSourceAsset(asset.c_str());
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

void VROVideoTextureAVP::seekToTime(int seconds) {
    int totalDuration = getVideoDurationInSeconds();

    // Clamp the seek time at a minimum of 0,
    // and a maxiumum of the video's duration period.
    if (seconds > totalDuration){
        seconds = totalDuration;
    } else if (seconds < 0){
        seconds = 0;
    }
    _player->seekToTime(seconds);
}

int VROVideoTextureAVP::getCurrentTimeInSeconds() {
    return _player->getCurrentTimeInSeconds();
}

int VROVideoTextureAVP::getVideoDurationInSeconds() {
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
    setSubstrate(std::move(substrate));

    _player->setSurface(_textureId);
}

#pragma mark - VROAVPlayerDelegate

void VROVideoTextureAVP::onPrepared() {
    // do nothing
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
