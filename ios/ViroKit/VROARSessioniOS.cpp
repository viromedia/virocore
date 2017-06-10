//
//  VROARSessioniOS.cpp
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "Availability.h"
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARSessioniOS.h"
#include "VROARFrameiOS.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROTexture.h"
#include "VRODriver.h"
#include "VROTextureSubstrate.h"
#include "VROLog.h"

VROARSessioniOS::VROARSessioniOS(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver) :
    VROARSession(trackingType) {
        
    if (@available(iOS 11.0, *)) {
        _session = [[ARSession alloc] init];
    }
    else {
        pabort("ARKit not available on this OS");
    }
    _background = std::make_shared<VROTexture>(VROTextureType::Texture2D, VROTextureInternalFormat::YCBCR);
    _videoTextureCache = std::shared_ptr<VROVideoTextureCacheOpenGL>((VROVideoTextureCacheOpenGL *)driver->newVideoTextureCache());
}

VROARSessioniOS::~VROARSessioniOS() {
 
}

void VROARSessioniOS::run() {
    std::shared_ptr<VROARSessioniOS> shared = shared_from_this();
    _delegate = [[VROARSessionDelegate alloc] initWithSession:shared];
    _session.delegate = _delegate;
    
    if (getTrackingType() == VROTrackingType::DOF3) {
        ARSessionConfiguration *config = [[ARSessionConfiguration alloc] init];
        config.lightEstimationEnabled = YES;
        
        [_session runWithConfiguration:config];
    }
    else { // DOF6
        ARWorldTrackingSessionConfiguration *config = [[ARWorldTrackingSessionConfiguration alloc] init];
        config.planeDetection = NO;
        config.lightEstimationEnabled = YES;
        
        [_session runWithConfiguration:config];
    }
}

void VROARSessioniOS::pause() {
    [_session pause];
}

bool VROARSessioniOS::isReady() const {
    return _currentFrame.get() != nullptr;
}

void VROARSessioniOS::addAnchor(std::shared_ptr<VROARAnchor> anchor) {
    // TODO implement
}

void VROARSessioniOS::removeAnchor(std::shared_ptr<VROARAnchor> anchor) {
    // TODO implement
}

std::shared_ptr<VROTexture> VROARSessioniOS::getCameraBackgroundTexture() {
    return _background;
}

std::unique_ptr<VROARFrame> &VROARSessioniOS::updateFrame() {
    VROARFrameiOS *frameiOS = (VROARFrameiOS *)_currentFrame.get();
    
    /*
     Update the background image.
     */
    std::vector<std::unique_ptr<VROTextureSubstrate>> substrates = _videoTextureCache->createYCbCrTextureSubstrates(frameiOS->getImage());
    _background->setSubstrate(0, std::move(substrates[0]));
    _background->setSubstrate(1, std::move(substrates[1]));
    
    return _currentFrame;
}

void VROARSessioniOS::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROARSessioniOS::setOrientation(VROCameraOrientation orientation) {
    _orientation = orientation;
}

void VROARSessioniOS::setFrame(ARFrame *frame) {
    _currentFrame = std::unique_ptr<VROARFrame>(new VROARFrameiOS(frame, _viewport, _orientation));
}

#pragma mark - VROARSessionDelegate

@interface VROARSessionDelegate ()

@property (readwrite, nonatomic) std::weak_ptr<VROARSessioniOS> session;

@end

@implementation VROARSessionDelegate

- (id)initWithSession:(std::shared_ptr<VROARSessioniOS>)session {
    self = [super init];
    if (self) {
        self.session = session;
    }
    return self;
}

- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame {
    std::shared_ptr<VROARSessioniOS> vSession = self.session.lock();
    if (vSession) {
        vSession->setFrame(frame);
    }
}

- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors {
    
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors {
    
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors {
    
}

@end

#endif
