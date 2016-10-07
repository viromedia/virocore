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
                                                                     NSURL *url,
                                                                     std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                                                     VRODriver &driver) {
    std::vector<std::shared_ptr<VROGeometrySource>> sources;
    std::vector<std::shared_ptr<VROGeometryElement>> elements;
    VROSurface::buildGeometry(width, height, sources, elements);
    
    
    std::shared_ptr<VROMaterial> material = std::make_shared<VROMaterial>();
    material->setWritesToDepthBuffer(true);
    material->setReadsFromDepthBuffer(true);
    
    std::shared_ptr<VROVideoTexture> texture = std::make_shared<VROVideoTexture>();
    texture->loadVideo(url, frameSynchronizer, driver);
    texture->play();
    material->getDiffuse().setContents(texture);
    
    std::shared_ptr<VROVideoSurface> surface = std::shared_ptr<VROVideoSurface>(new VROVideoSurface(sources, elements, texture));
    surface->getMaterials().push_back(material);
    return surface;
}

VROVideoSurface::VROVideoSurface(std::vector<std::shared_ptr<VROGeometrySource>> &sources,
                                 std::vector<std::shared_ptr<VROGeometryElement>> &elements,
                                 std::shared_ptr<VROVideoTexture> texture) :
    VROSurface(sources, elements),
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

void VROVideoSurface::seekToTime(int seconds) {
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

void VROVideoSurface::setDelegate(id <VROVideoDelegate> delegate) {
    _texture->setDelegate(delegate);
}
