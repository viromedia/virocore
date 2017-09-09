//
//  VROVideoTexture.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoSurface.h"
#include "VROVideoTexture.h"
#include "VROMaterial.h"
#include <memory>

std::shared_ptr<VROVideoSurface> VROVideoSurface::createVideoSurface(float width, float height,
                                                                     std::string url,
                                                                     std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                                                     std::shared_ptr<VROVideoTexture> texture,
                                                                     std::shared_ptr<VRODriver> driver) {
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    texture->loadVideo(url, frameSynchronizer, driver);
    texture->play();

    material->getDiffuse().setTexture(texture);
    
    std::shared_ptr<VROVideoSurface> surface = std::shared_ptr<VROVideoSurface>(new VROVideoSurface(width, height, texture));
    surface->setMaterials({ material });
    return surface;
}

VROVideoSurface::VROVideoSurface(float width, float height, std::shared_ptr<VROVideoTexture> texture) :
    VROSurface(0, 0, width, height, 0, 0, 1, 1),
    _texture(texture) {

}

VROVideoSurface::~VROVideoSurface() {

}

void VROVideoSurface::play() {
    _texture->play();
}

void VROVideoSurface::pause() {
    _texture->pause();
}

void VROVideoSurface::seekToTime(float seconds) {
    _texture->seekToTime(seconds);
}

void VROVideoSurface::setMuted(bool muted) {
    _texture->setMuted(muted);
}

void VROVideoSurface::setVolume(float volume) {
    _texture->setVolume(volume);
}

void VROVideoSurface::setLoop(bool loop) {
    _texture->setLoop(loop);
}

bool VROVideoSurface::isPaused() {
    return _texture->isPaused();
}

void VROVideoSurface::setDelegate(std::shared_ptr<VROVideoDelegateInternal> delegate) {
    _texture->setDelegate(delegate);
}
