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
#include "VROARAnchor.h"
#include "VROARPlaneAnchor.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROTexture.h"
#include "VRODriver.h"
#include "VROConvert.h"
#include "VROScene.h"
#include "VROTextureSubstrate.h"
#include "VROLog.h"
#include <algorithm>
#include "VROPlatformUtil.h"

VROARSessioniOS::VROARSessioniOS(VROTrackingType trackingType, VROWorldAlignment worldAlignment, std::shared_ptr<VRODriver> driver) :
    VROARSession(trackingType, worldAlignment),
    _sessionPaused(true) {
        
    if (@available(iOS 11.0, *)) {
        _session = [[ARSession alloc] init];
    }
    else {
        pabort("ARKit not available on this OS");
    }
    _background = std::make_shared<VROTexture>(VROTextureType::Texture2D, VROTextureInternalFormat::YCBCR);
    _videoTextureCache = std::dynamic_pointer_cast<VROVideoTextureCacheOpenGL>(driver->newVideoTextureCache());
        
    if (getTrackingType() == VROTrackingType::DOF3) {
        _sessionConfiguration = [[AROrientationTrackingConfiguration alloc] init];
        _sessionConfiguration.lightEstimationEnabled = YES;
    }
    else { // DOF6
        ARWorldTrackingConfiguration *config = [[ARWorldTrackingConfiguration alloc] init];
        config.planeDetection = NO;
        config.lightEstimationEnabled = YES;
        switch(getWorldAlignment()) {
            case VROWorldAlignment::Camera:
                config.worldAlignment = ARWorldAlignmentCamera;
                break;
            case VROWorldAlignment::GravityAndHeading:
                config.worldAlignment = ARWorldAlignmentGravityAndHeading;
                break;
            case VROWorldAlignment::Gravity:
            default:
                config.worldAlignment = ARWorldAlignmentGravity;
                break;
        }
        
        _sessionConfiguration = config;
    }
    
    //_trackingHelper = [[VROTrackingHelper alloc] init];
}

VROARSessioniOS::~VROARSessioniOS() {
 
}

#pragma mark - VROARSession implementation

void VROARSessioniOS::run() {
    _sessionPaused = false;
    std::shared_ptr<VROARSessioniOS> shared = shared_from_this();
    _delegateAR = [[VROARKitSessionDelegate alloc] initWithSession:shared];
    _session.delegate = _delegateAR;

    [_session runWithConfiguration:_sessionConfiguration];
}

void VROARSessioniOS::pause() {
    _sessionPaused = true;
    [_session pause];
}

bool VROARSessioniOS::isReady() const {
    return getScene() != nullptr && _currentFrame.get() != nullptr;
}

void VROARSessioniOS::setAnchorDetection(std::set<VROAnchorDetection> types) {
    if (types.find(VROAnchorDetection::PlanesHorizontal) != types.end()) {
        if ([_sessionConfiguration isKindOfClass:[ARWorldTrackingConfiguration class]]) {
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).planeDetection = YES;
        }
    }
    else {
        if ([_sessionConfiguration isKindOfClass:[ARWorldTrackingConfiguration class]]) {
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).planeDetection = NO;
        }
    }

    // apply the configuration
    if (!_sessionPaused) {
        [_session runWithConfiguration:_sessionConfiguration];
    }
}

void VROARSessioniOS::setScene(std::shared_ptr<VROScene> scene) {
    VROARSession::setScene(scene);
}

void VROARSessioniOS::setDelegate(std::shared_ptr<VROARSessionDelegate> delegate) {
    VROARSession::setDelegate(delegate);
    // When we add a new delegate, notify it of all the anchors we've found thus far
    if (delegate) {
        for (auto it = _anchors.begin(); it != _anchors.end();it++) {
            delegate->anchorWasDetected(*it);
        }
    }
}

void VROARSessioniOS::addAnchor(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (!delegate) {
        return;
    }
    
    delegate->anchorWasDetected(anchor);
    _anchors.push_back(anchor);
}

void VROARSessioniOS::removeAnchor(std::shared_ptr<VROARAnchor> anchor) {
    _anchors.erase(std::remove_if(_anchors.begin(), _anchors.end(),
                                 [anchor](std::shared_ptr<VROARAnchor> candidate) {
                                     return candidate == anchor;
                                 }), _anchors.end());
    
    for (auto it = _nativeAnchorMap.begin(); it != _nativeAnchorMap.end();) {
        if (it->second == anchor) {
            // TODO We should remove the anchor from the ARKit session, but unclear
            //      how to do this given just the identifier. Do we create a dummy
            //      ARAnchor and set its identifier?
            //[_session removeAnchor:it->first];
            it = _nativeAnchorMap.erase(it);
        }
        else {
            ++it;
        }
    }
    
    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (delegate) {
        delegate->anchorWasRemoved(anchor);
    }
}

void VROARSessioniOS::updateAnchor(std::shared_ptr<VROARAnchor> anchor) {
    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (delegate) {
        delegate->anchorWillUpdate(anchor);
    }
    anchor->updateNodeTransform();
    if (delegate) {
        delegate->anchorDidUpdate(anchor);
    }
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

    // Uncomment the below line to enable running image recognition
    // [_trackingHelper processPixelBufferRef:frameiOS->getImage() forceRun:false];

    return _currentFrame;
}

std::unique_ptr<VROARFrame> &VROARSessioniOS::getLastFrame() {
    return _currentFrame;
}

void VROARSessioniOS::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROARSessioniOS::setOrientation(VROCameraOrientation orientation) {
    _orientation = orientation;
}

#pragma mark - Internal Methods

std::shared_ptr<VROARAnchor> VROARSessioniOS::getAnchorForNative(ARAnchor *anchor) {
    auto kv = _nativeAnchorMap.find(std::string([anchor.identifier.UUIDString UTF8String]));
    if (kv != _nativeAnchorMap.end()) {
        return kv->second;
    }
    else {
        return nullptr;
    }
}

void VROARSessioniOS::setFrame(ARFrame *frame) {
    _currentFrame = std::unique_ptr<VROARFrame>(new VROARFrameiOS(frame, _viewport, _orientation, shared_from_this()));
}

void VROARSessioniOS::updateAnchorFromNative(std::shared_ptr<VROARAnchor> vAnchor, ARAnchor *anchor) {
    if ([anchor isKindOfClass:[ARPlaneAnchor class]]) {
        ARPlaneAnchor *planeAnchor = (ARPlaneAnchor *)anchor;
        
        std::shared_ptr<VROARPlaneAnchor> pAnchor = std::dynamic_pointer_cast<VROARPlaneAnchor>(vAnchor);
        pAnchor->setAlignment(VROARPlaneAlignment::Horizontal);
        pAnchor->setCenter(VROConvert::toVector3f(planeAnchor.center));
        pAnchor->setExtent(VROConvert::toVector3f(planeAnchor.extent));
    }
    vAnchor->setTransform(VROConvert::toMatrix4f(anchor.transform));
}

void VROARSessioniOS::addAnchor(ARAnchor *anchor) {
    std::shared_ptr<VROARSessionDelegate> delegate = getDelegate();
    if (!delegate) {
        return;
    }
    
    std::shared_ptr<VROARAnchor> vAnchor;
    if ([anchor isKindOfClass:[ARPlaneAnchor class]]) {
        vAnchor = std::make_shared<VROARPlaneAnchor>();
    }
    else {
        vAnchor = std::make_shared<VROARAnchor>();
    }
    vAnchor->setId(std::string([anchor.identifier.UUIDString UTF8String]));

    updateAnchorFromNative(vAnchor, anchor);
    
    addAnchor(vAnchor);
    _nativeAnchorMap[std::string([anchor.identifier.UUIDString UTF8String])] = vAnchor;
}

void VROARSessioniOS::updateAnchor(ARAnchor *anchor) {
    auto it = _nativeAnchorMap.find(std::string([anchor.identifier.UUIDString UTF8String]));
    if (it != _nativeAnchorMap.end()) {
        std::shared_ptr<VROARAnchor> vAnchor = it->second;
        updateAnchorFromNative(vAnchor, anchor);
        updateAnchor(it->second);
    }
    else {
        pinfo("Anchor %@ not found!", anchor.identifier);
    }
}

void VROARSessioniOS::removeAnchor(ARAnchor *anchor) {
    auto it = _nativeAnchorMap.find(std::string([anchor.identifier.UUIDString UTF8String]));
    if (it != _nativeAnchorMap.end()) {
        removeAnchor(it->second);
    }
}

#pragma mark - VROARKitSessionDelegate

@interface VROARKitSessionDelegate ()

@property (readwrite, nonatomic) std::weak_ptr<VROARSessioniOS> session;

@end

@implementation VROARKitSessionDelegate

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
        VROPlatformDispatchAsyncRenderer([vSession, frame] {
            vSession->setFrame(frame);
        });
    }
}

- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors {
    std::shared_ptr<VROARSessioniOS> vSession = self.session.lock();
    if (vSession) {
        VROPlatformDispatchAsyncRenderer([vSession, anchors] {
            for (ARAnchor *anchor in anchors) {
                vSession->addAnchor(anchor);
            }
        });
    }
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors {
    std::shared_ptr<VROARSessioniOS> vSession = self.session.lock();
    if (vSession) {
        VROPlatformDispatchAsyncRenderer([vSession, anchors] {
            for (ARAnchor *anchor in anchors) {
                vSession->updateAnchor(anchor);
            }
        });
    }
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors {
    std::shared_ptr<VROARSessioniOS> vSession = self.session.lock();
    if (vSession) {
        VROPlatformDispatchAsyncRenderer([vSession, anchors] {
            for (ARAnchor *anchor in anchors) {
                vSession->removeAnchor(anchor);
            }
        });
    }
}

@end

#endif
