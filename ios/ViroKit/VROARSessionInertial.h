//
//  VROARSessionInertial.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARSessionInertial_h
#define VROARSessionInertial_h

#include "VROARSession.h"

class VRODriver;
class VROARCameraInertial;

class VROARSessionInertial : public VROARSession, public std::enable_shared_from_this<VROARSessionInertial> {
public:
    
    VROARSessionInertial(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver);
    virtual ~VROARSessionInertial();
    
    void run();
    void pause();
    bool isReady() const;
    void resetSession(bool resetTracking, bool removeAnchors);
    
    bool setAnchorDetection(std::set<VROAnchorDetection> types);
    void setCloudAnchorProvider(VROCloudAnchorProvider provider);
    void addARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> target);
    
    void addARObjectTarget(std::shared_ptr<VROARObjectTarget> target) {
        // unsupported
    }
    void removeARObjectTarget(std::shared_ptr<VROARObjectTarget> target) {
        // unsupported
    }
    
    
    void loadARImageDatabase(std::shared_ptr<VROARImageDatabase> arImageDatabase) {
        // unsupported
    }
    
    void unloadARImageDatabase() {
        // unsupported
    }
    
    void addAnchor(std::shared_ptr<VROARAnchor> anchor);
    void removeAnchor(std::shared_ptr<VROARAnchor> anchor);
    void updateAnchor(std::shared_ptr<VROARAnchor> anchor);
    void hostCloudAnchor(std::shared_ptr<VROARAnchor> anchor,
                         std::function<void(std::shared_ptr<VROARAnchor>)> onSuccess,
                         std::function<void(std::string error)> onFailure);
    void resolveCloudAnchor(std::string anchorId,
                            std::function<void(std::shared_ptr<VROARAnchor> anchor)> onSuccess,
                            std::function<void(std::string error)> onFailure);
    
    std::unique_ptr<VROARFrame> &updateFrame();
    std::unique_ptr<VROARFrame> &getLastFrame();
    std::shared_ptr<VROTexture> getCameraBackgroundTexture();
    
    void setViewport(VROViewport viewport);
    void setOrientation(VROCameraOrientation orientation);

    void addAnchorNode(std::shared_ptr<VRONode> node);

    void setNumberOfTrackedImages(int numImages) {
        // no-op
    }
    
    void setWorldOrigin(VROMatrix4f relativeTransform) {
        // no-op
    };
    
    void setAutofocus(bool enabled) {
        // no-op
    };
    
    void setVideoQuality(VROVideoQuality quality) {
        // no-op
    };
    
private:
    
    std::unique_ptr<VROARFrame> _currentFrame;
    std::shared_ptr<VROARCameraInertial> _camera;
    
};

#endif /* VROARSessionInertial_h */
