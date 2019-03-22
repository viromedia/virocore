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
#include "VROARImageTargetiOS.h"
#include "VROARObjectTargetiOS.h"
#include "VROARImageAnchor.h"
#include "VROARObjectAnchor.h"
#include "VROPortal.h"
#include "VROBox.h"
#include "VROProjector.h"
#include "VROARCameraiOS.h"
#include "VROARImageTracker.h"
#include "VROVisionModel.h"
#include "VROTrackingHelper.h"

#pragma mark - Lifecycle and Initialization

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
        
    updateTrackingType(trackingType);
    
#if ENABLE_OPENCV
    _trackingHelper = [[VROTrackingHelper alloc] init];
      
    // Uncomment to run a test screenshot through the OpenCV tracking code.
    [_trackingHelper findInScreenshot];
        
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    // dollar sized
    //std::shared_ptr<VROBox> box = VROBox::createBox(.155956, .066294, .001);
    std::shared_ptr<VROBox> box = VROBox::createBox(.1905f, .001f, 0.24511f);
    boxNode->setGeometry(box);

    _imageTrackingResultNode = std::make_shared<VRONode>();
    _imageTrackingResultNode->addChildNode(boxNode);
    _imageTrackingResultNode->setHidden(true);
        
    _imageResultsContainer = std::make_shared<VRONode>();
    _imageResultsContainer->addChildNode(_imageTrackingResultNode);
#endif /* ENABLE_OPENCV */
}

VROARSessioniOS::~VROARSessioniOS() {

}

void VROARSessioniOS::setTrackingType(VROTrackingType trackingType) {
    if (trackingType == _trackingType) {
        return;
    }
    
    updateTrackingType(trackingType);
    pause();
    run();
}

void VROARSessioniOS::updateTrackingType(VROTrackingType trackingType) {
    _trackingType = trackingType;
    
    if (getTrackingType() == VROTrackingType::DOF3) {
        NSLog(@"DOF3 tracking configuration");

        _sessionConfiguration = [[AROrientationTrackingConfiguration alloc] init];
        _sessionConfiguration.lightEstimationEnabled = YES;
    }
    else {
        NSLog(@"DOF6 tracking configuration");

        // Note that default anchor detection gets overwritten by VROARScene when the
        // session is injected into the scene (the scene will propagate whatever anchor
        // detection setting it has over to this session).
        ARWorldTrackingConfiguration *config = [[ARWorldTrackingConfiguration alloc] init];
        config.planeDetection = ARPlaneDetectionNone;
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
        
        
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
        if (@available(iOS 11.3, *)) {
            _arKitImageDetectionSet = [[NSMutableSet alloc] init];
            config.detectionImages = _arKitImageDetectionSet;
        }
#endif
        
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000
        if (@available(iOS 12.0, *)) {
            _arKitObjectDetectionSet = [[NSMutableSet alloc] init];
            config.detectionObjects = _arKitObjectDetectionSet;
        }
#endif
        
        _sessionConfiguration = config;
    }
}

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

void VROARSessioniOS::resetSession(bool resetTracking, bool removeAnchors) {
    if (_session && (resetTracking || removeAnchors)) {
        NSUInteger options = ((resetTracking ? ARSessionRunOptionResetTracking : 0) | (removeAnchors ? ARSessionRunOptionRemoveExistingAnchors : 0));
        [_session runWithConfiguration:_sessionConfiguration options:options];
    }
}

#pragma mark - Settings

void VROARSessioniOS::setScene(std::shared_ptr<VROScene> scene) {
    VROARSession::setScene(scene);
    
#if ENABLE_OPENCV
    // when we add a scene, make sure that we add this node to it.
    scene->getRootNode()->addChildNode(_imageResultsContainer);
#endif /* ENABLE_OPENCV */
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

void VROARSessioniOS::setAutofocus(bool enabled) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    if (@available(iOS 11.3, *)) {
        if ([_sessionConfiguration isKindOfClass:[ARWorldTrackingConfiguration class]]) {
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).autoFocusEnabled = enabled;
            [_session runWithConfiguration:_sessionConfiguration];
        }
    }
#endif
}

void VROARSessioniOS::setVideoQuality(VROVideoQuality quality) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    if (@available(iOS 11.3, *)) {
        if ([_sessionConfiguration isKindOfClass:[ARWorldTrackingConfiguration class]]) {
            NSArray<ARVideoFormat *> *videoFormats = ARWorldTrackingConfiguration.supportedVideoFormats;
            int numberOfSupportedVideoFormats = (int) [videoFormats count];
            // Since iOS 12, ARWorldTrackingConfiguration.supportedVideoFormats started returning 0 ////
            // supportedVideoFormats here, for simulator targets. In that case, we'll skip the following and
            // run session with default videoformat value
            if (numberOfSupportedVideoFormats > 0) {
              if (quality == VROVideoQuality::High) {
                ARVideoFormat *highestFormat;
                float high = 0;
                for (ARVideoFormat *format in videoFormats) {
                  if (format.imageResolution.height > high) {
                    high = format.imageResolution.height;
                    highestFormat = format;
                  }
                }
                ((ARWorldTrackingConfiguration *) _sessionConfiguration).videoFormat = highestFormat;
              } else {
                ARVideoFormat *lowestFormat;
                float low = CGFLOAT_MAX;
                for (ARVideoFormat *format in videoFormats) {
                  if (format.imageResolution.height < low) {
                    low = format.imageResolution.height;
                    lowestFormat = format;
                  }
                }
                ((ARWorldTrackingConfiguration *) _sessionConfiguration).videoFormat = lowestFormat;
              }
            }
        }
        [_session runWithConfiguration:_sessionConfiguration];
    }
#endif
}

void VROARSessioniOS::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROARSessioniOS::setOrientation(VROCameraOrientation orientation) {
    _orientation = orientation;
}

void VROARSessioniOS::setWorldOrigin(VROMatrix4f relativeTransform) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    if (@available(iOS 11.3, *)) {
        if (_session) {
            [_session setWorldOrigin:VROConvert::toMatrixFloat4x4(relativeTransform)];
        }
    }
#endif
}

void VROARSessioniOS::setNumberOfTrackedImages(int numImages) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000
    if (@available(iOS 12.0, *) && _session && _sessionConfiguration) {
        if ([_sessionConfiguration isKindOfClass:[ARWorldTrackingConfiguration class]]) {
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).maximumNumberOfTrackedImages = numImages;
        } else if ([_sessionConfiguration isKindOfClass:[ARImageTrackingConfiguration class]]) {
            ((ARImageTrackingConfiguration *) _sessionConfiguration).maximumNumberOfTrackedImages = numImages;
        }
        [_session runWithConfiguration:_sessionConfiguration];
    }
#endif
}

#pragma mark - Anchors

bool VROARSessioniOS::setAnchorDetection(std::set<VROAnchorDetection> types) {
    if (types.size() == 0){
        if ([_sessionConfiguration isKindOfClass:[ARWorldTrackingConfiguration class]]) {
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).planeDetection = ARPlaneDetectionNone;
        }
    } else {
        if ([_sessionConfiguration isKindOfClass:[ARWorldTrackingConfiguration class]]) {
            NSUInteger detectionTypes = ARPlaneDetectionNone; //default

            if (types.find(VROAnchorDetection::PlanesHorizontal) != types.end()) {
                detectionTypes = detectionTypes | ARPlaneDetectionHorizontal;
            }
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
            else if (@available(iOS 11.3, *) && types.find(VROAnchorDetection::PlanesVertical) != types.end()) {
                detectionTypes = detectionTypes | ARPlaneDetectionVertical;
            }
#endif
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).planeDetection = detectionTypes;

        }
    }

    // apply the configuration
    if (!_sessionPaused) {
        [_session runWithConfiguration:_sessionConfiguration];
    }
    return true;
}

void VROARSessioniOS::setCloudAnchorProvider(VROCloudAnchorProvider provider) {
    // TODO iOS ARCore Cloud Anchor implementation
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
    
    auto it = _nativeAnchorMap.find(anchor->getId());
    _nativeAnchorMap.erase(it);
    
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

void VROARSessioniOS::hostCloudAnchor(std::shared_ptr<VROARAnchor> anchor,
                                      std::function<void(std::shared_ptr<VROARAnchor>)> onSuccess,
                                      std::function<void(std::string error)> onFailure) {
    // Unsupproted
}
void VROARSessioniOS::resolveCloudAnchor(std::string anchorId,
                                         std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                                         std::function<void(std::string error)> onFailure) {
    // Unsupported
}

#pragma mark - Frames

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

    if (_visionModel) {
        _visionModel->update(frameiOS);
    }
    
#if ENABLE_OPENCV
    if (isReady()) {

        // Get and set intrinsic matrix
        std::shared_ptr<VROARCameraiOS> arCameraiOS = std::dynamic_pointer_cast<VROARCameraiOS>(frameiOS->getCamera());
        float* intrinsics = arCameraiOS->getIntrinsics();
        [_trackingHelper setIntrinsics:intrinsics];

        // fetch camera pointer
        std::shared_ptr<VROARCamera> lastCamera = getLastFrame()->getCamera();
        
        [_trackingHelper processPixelBufferRef:frameiOS->getImage()
                                      forceRun:false
                                        camera:arCameraiOS
                                    completion:
         ^(VROTrackingHelperOutput *output) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (_trackerOutputView != nil && [output getImageTrackerOutput].found) {
                    [_trackerOutputView setImage:[output getOutputImage]];
                }
                VROARImageTrackerOutput trackerOutput = [output getImageTrackerOutput];
                if (trackerOutput.found) {
                    
                    VROMatrix4f endTransformation = trackerOutput.worldTransform;
                    
                    _imageTrackingResultNode->setHidden(false);
                    _imageTrackingResultNode->setPosition(endTransformation.extractTranslation());
                    _imageTrackingResultNode->setRotation(endTransformation.extractRotation({1,1,1}));
                    
                    VROVector3f pos = endTransformation.extractTranslation();
                    VROVector3f rot = endTransformation.extractRotation({1,1,1}).toEuler();
                    
                    pinfo("[Viro] the world position was: %f, %f, %f", pos.x, pos.y, pos.z);
                    pinfo("[Viro] the world rotation was: %f, %f, %f", toDegrees(rot.x), toDegrees(rot.y), toDegrees(rot.z));
                    
                    if (_trackerOutputText != nil) {
                        VROVector3f camPos = arCameraiOS->getPosition();
                        NSString *outputText = [NSString stringWithFormat:@"Camera Pos: [%.03f, %.03f, %.03f]\nImage Pos: [%.03f, %.03f, %.03f]\nWorld Pos: [%.03f, %.03f, %.03f]",
                                                camPos.x, camPos.y, camPos.z,
                                                trackerOutput.translation.at<double>(0,0),
                                                - trackerOutput.translation.at<double>(1,0), // because Y and Z axis are flipped in OpenCV.
                                                - trackerOutput.translation.at<double>(2,0),
                                                pos.x, pos.y, pos.z];
                        _trackerOutputText.text = outputText;
                    }
                }
            });
        }];
    }
#endif /* ENABLE_OPENCV */

    return _currentFrame;
}

std::unique_ptr<VROARFrame> &VROARSessioniOS::getLastFrame() {
    return _currentFrame;
}

#pragma mark - Image Targets

void VROARSessioniOS::addARImageTarget(std::shared_ptr<VROARImageTarget> target) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    // we'll get a warning for anything but: if(@available(...))
    if (@available(iOS 11.3, *)) {
        // we only support ARKit for now!
        std::shared_ptr<VROARImageTargetiOS> targetiOS = std::dynamic_pointer_cast<VROARImageTargetiOS>(target);
        if (targetiOS && getImageTrackingImpl() == VROImageTrackingImpl::ARKit && getTrackingType() == VROTrackingType::DOF6) {
            // init the VROARImageTarget so it creates an ARReferenceImage
            targetiOS->initWithTrackingImpl(VROImageTrackingImpl::ARKit);
            ARReferenceImage *refImage = targetiOS->getARReferenceImage();
            
            // add the ARReferenceImage and the VROARImageTarget in a map
            _arKitReferenceImageMap[refImage] = target;
            
            // Add the ARReferenceImage to the set of images for detection, update the config and "run" session.
            // Note, we still need to set the config for the ARSession to start detecting the new target (not
            // just modifying the set). Calling runConfiguration doesn't seem to be necessary in the ARKit 1.5/iOS 11.3
            // preview, but it doesn't hurt and the "examples" that they have do call it, so let's be safe.
            [_arKitImageDetectionSet addObject:refImage];
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).detectionImages = _arKitImageDetectionSet;
            [_session runWithConfiguration:_sessionConfiguration];
        }
    } else {
        pwarn("[Viro] attempting to use ARKit 1.5 features while not on iOS 11.3+");
        return;
    }
#endif
}

void VROARSessioniOS::removeARImageTarget(std::shared_ptr<VROARImageTarget> target) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    // we'll get a warning for anything but: if (@available(...))
    if (@available(iOS 11.3, *)) {
        std::shared_ptr<VROARImageTargetiOS> targetiOS = std::dynamic_pointer_cast<VROARImageTargetiOS>(target);
        if (targetiOS && getImageTrackingImpl() == VROImageTrackingImpl::ARKit && getTrackingType() == VROTrackingType::DOF6) {
            ARReferenceImage *refImage = targetiOS->getARReferenceImage();
            if (refImage) {
                
                // call remove anchor (ARKit should do this IMHO).
                std::shared_ptr<VROARAnchor> anchor = target->getAnchor();
                if (anchor) {
                    removeAnchor(anchor);
                }
                
                // delete the VROARImageTarget from _arKitReferenceImageMap
                for (auto it = _arKitReferenceImageMap.begin(); it != _arKitReferenceImageMap.end();) {
                    if (it->second == target) {
                        it = _arKitReferenceImageMap.erase(it);
                    } else {
                        ++it;
                    }
                }
                
                // delete the ARReferenceImage from the set of images to detect
                [_arKitImageDetectionSet removeObject:refImage];
                
                ((ARWorldTrackingConfiguration *) _sessionConfiguration).detectionImages = _arKitImageDetectionSet;
                [_session runWithConfiguration:_sessionConfiguration];
            }
        }
    } else {
        pwarn("[Viro] attempting to use ARKit 1.5 features while not on iOS 11.3+");
        return;
    }
#endif
}

#if ENABLE_OPENCV
void VROARSessioniOS::outputTextTapped() {
    BOOL tracking = [_trackingHelper toggleTracking];
    _trackerOutputText.text = tracking ? @"Started Tracking!" : @"Not Tracking!";
}
#endif /* ENABLE_OPENCV */

#pragma mark - Object Targets
void VROARSessioniOS::addARObjectTarget(std::shared_ptr<VROARObjectTarget> target) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000
    if (@available(iOS 12.0, *)) {
        std::shared_ptr<VROARObjectTargetiOS> objectTarget = std::dynamic_pointer_cast<VROARObjectTargetiOS>(target);
        
        if (objectTarget && getTrackingType() == VROTrackingType::DOF6) {
            ARReferenceObject *refObject = objectTarget->getARReferenceObject();
            
            // add the ARReferenceObject & VROARObjectTarget to a map
            _arKitReferenceObjectMap[refObject] = target;
            
            // Add the ARReferenceObject to the set of objects for detection, update the config and "run" session.
            // Note, we still need to set the config for the ARSession to start detecting the new target (not
            // just modifying the set).
            [_arKitObjectDetectionSet addObject:refObject];
            ((ARWorldTrackingConfiguration *) _sessionConfiguration).detectionObjects = _arKitObjectDetectionSet;
            [_session runWithConfiguration:_sessionConfiguration];
        }
    }
#endif
}

void VROARSessioniOS::removeARObjectTarget(std::shared_ptr<VROARObjectTarget> target) {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000
    if (@available(iOS 12.0, *)) {
        std::shared_ptr<VROARObjectTargetiOS> objectTarget = std::dynamic_pointer_cast<VROARObjectTargetiOS>(target);
        if (objectTarget && getTrackingType() == VROTrackingType::DOF6) {
            ARReferenceObject *refObject = objectTarget->getARReferenceObject();
            if (refObject) {
                
                // call remove anchor (ARKit should do this IMHO).
                std::shared_ptr<VROARAnchor> anchor = target->getAnchor();
                if (anchor) {
                    removeAnchor(anchor);
                }
                
                // delete the VROARImageTarget from _arKitReferenceImageMap
                for (auto it = _arKitReferenceObjectMap.begin(); it != _arKitReferenceObjectMap.end();) {
                    if (it->second == target) {
                        it = _arKitReferenceObjectMap.erase(it);
                    } else {
                        ++it;
                    }
                }
                
                // delete the ARReferenceImage from the set of images to detect
                [_arKitObjectDetectionSet removeObject:refObject];
                
                ((ARWorldTrackingConfiguration *) _sessionConfiguration).detectionObjects = _arKitObjectDetectionSet;
                [_session runWithConfiguration:_sessionConfiguration];
            }
        }
    }
#endif
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

void VROARSessioniOS::setVisionModel(std::shared_ptr<VROVisionModel> visionModel) {
    _visionModel = visionModel;
}

void VROARSessioniOS::setFrame(ARFrame *frame) {
    _currentFrame = std::unique_ptr<VROARFrame>(new VROARFrameiOS(frame, _viewport, _orientation, shared_from_this()));
}

void VROARSessioniOS::updateAnchorFromNative(std::shared_ptr<VROARAnchor> vAnchor, ARAnchor *anchor) {
    if ([anchor isKindOfClass:[ARPlaneAnchor class]]) {
        ARPlaneAnchor *planeAnchor = (ARPlaneAnchor *)anchor;

        std::shared_ptr<VROARPlaneAnchor> pAnchor = std::dynamic_pointer_cast<VROARPlaneAnchor>(vAnchor);
        pAnchor->setCenter(VROConvert::toVector3f(planeAnchor.center));
        pAnchor->setExtent(VROConvert::toVector3f(planeAnchor.extent));
        pAnchor->setBoundaryVertices(std::vector<VROVector3f>());

        if (planeAnchor.alignment == ARPlaneAnchorAlignmentHorizontal) {
            pAnchor->setAlignment(VROARPlaneAlignment::Horizontal);
        }
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
        else if (@available(iOS 11.3, *) && planeAnchor.alignment == ARPlaneAnchorAlignmentVertical) {
            pAnchor->setAlignment(VROARPlaneAlignment::Vertical);
        }

        if (@available(iOS 11.3, *) && planeAnchor.geometry && planeAnchor.geometry.boundaryVertices && planeAnchor.geometry.boundaryVertexCount > 0) {
            std::vector<VROVector3f> points;
            for (int i = 0; i < planeAnchor.geometry.boundaryVertexCount; i++) {
                vector_float3 vertex = planeAnchor.geometry.boundaryVertices[i];
                SCNVector3 vector3 = SCNVector3FromFloat3(vertex);

                VROVector3f boundaryVertexFromAnchor = VROVector3f(vector3.x, vector3.y, vector3.z);
                VROVector3f boundaryVertexFromCenter = boundaryVertexFromAnchor - pAnchor->getCenter();
                points.push_back(boundaryVertexFromCenter);
            }

            pAnchor->setBoundaryVertices(points);
        }
#endif
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
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110300
    // ignore the warning. The curious thing is that we don't even need the @available() check...
    else if (@available(iOS 11.3, *) && [anchor isKindOfClass:[ARImageAnchor class]]) {
        ARImageAnchor *imageAnchor = (ARImageAnchor *)anchor;
        auto it = _arKitReferenceImageMap.find(imageAnchor.referenceImage);
        if (it != _arKitReferenceImageMap.end()) {
            std::shared_ptr<VROARImageTarget> target = it->second;
            vAnchor = std::make_shared<VROARImageAnchor>(target);
            target->setAnchor(vAnchor);
        }
    }
#endif
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000
    else if(@available(iOS 12.0, *) && [anchor isKindOfClass:[ARObjectAnchor class]]) {
        ARObjectAnchor *objAnchor = (ARObjectAnchor *)anchor;
        auto it = _arKitReferenceObjectMap.find(objAnchor.referenceObject);
        if (it != _arKitReferenceObjectMap.end()) {
            std::shared_ptr<VROARObjectTarget> target = it->second;
            vAnchor = std::make_shared<VROARObjectAnchor>(target);
            target->setAnchor(vAnchor);
        }
    }
#endif
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
    } else {
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
