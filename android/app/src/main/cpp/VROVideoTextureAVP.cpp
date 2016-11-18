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
    bindSurface();
}

VROVideoTextureAVP::~VROVideoTextureAVP() {
    delete (_player);

    if (_textureId) {
        glDeleteTextures(1, &_textureId);
    }
}

void VROVideoTextureAVP::loadVideo(std::string url,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       VRODriver &driver) {

    // TODO
}

void VROVideoTextureAVP::loadVideoFromAsset(std::string asset, VRODriver &driver) {
    _player->setDataSourceAsset(asset.c_str());
}

void VROVideoTextureAVP::prewarm() { }
void VROVideoTextureAVP::onFrameWillRender(const VRORenderContext &context) { }
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
    _player->seekToTime(seconds);
}

void VROVideoTextureAVP::setMuted(bool muted) {
    // TODO
}

void VROVideoTextureAVP::setVolume(float volume) {
    _player->setVolume(volume);
}

void VROVideoTextureAVP::setLoop(bool loop) {
    _player->setLoop(loop);
}

void VROVideoTextureAVP::bindSurface() {
    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureId);

    // Can't do mipmapping with video textures, and clamp to edge is only option
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
            new VROTextureSubstrateOpenGL(GL_TEXTURE_EXTERNAL_OES, _textureId, true));
    setSubstrate(std::move(substrate));

    _player->setSurface(_textureId);
}