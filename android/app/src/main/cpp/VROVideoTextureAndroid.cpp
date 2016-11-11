//
//  VROVideoTextureAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/10/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoTextureAndroid.h"
#include "VROFrameSynchronizer.h"

VROVideoTextureAndroid::VROVideoTextureAndroid() {

}
VROVideoTextureAndroid::~VROVideoTextureAndroid() {

}

void VROVideoTextureAndroid::loadVideo(std::string url,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       VRODriver &driver) {

    frameSynchronizer->addFrameListener(shared_from_this());
}

void VROVideoTextureAndroid::prewarm() {

}

void VROVideoTextureAndroid::onFrameWillRender(const VRORenderContext &context) {

}
void VROVideoTextureAndroid::onFrameDidRender(const VRORenderContext &context) {

}

void VROVideoTextureAndroid::pause() {

}
void VROVideoTextureAndroid::play() {

}
bool VROVideoTextureAndroid::isPaused() {

}
void VROVideoTextureAndroid::seekToTime(int seconds) {

}

void VROVideoTextureAndroid::setMuted(bool muted) {

}
void VROVideoTextureAndroid::setVolume(float volume) {

}
void VROVideoTextureAndroid::setLoop(bool loop) {

}
