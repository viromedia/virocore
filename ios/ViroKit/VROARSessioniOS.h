//
//  VROARSessioniOS.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARSessioniOS_h
#define VROARSessioniOS_h

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
#include "VROARSession.h"
#include "VROViewport.h"
#include <ARKit/ARKit.h>

class VRODriver;
class VROVideoTextureCacheOpenGL;
@class VROARSessionDelegate;

class VROARSessioniOS : public VROARSession, public std::enable_shared_from_this<VROARSessioniOS> {
public:
    
    VROARSessioniOS(VROTrackingType trackingType, std::shared_ptr<VRODriver> driver);
    virtual ~VROARSessioniOS();
    
    void run();
    void pause();
    bool isReady() const;
    
    void addAnchor(std::shared_ptr<VROARAnchor> anchor);
    void removeAnchor(std::shared_ptr<VROARAnchor> anchor);
    
    std::unique_ptr<VROARFrame> &updateFrame();
    std::shared_ptr<VROTexture> getCameraBackgroundTexture();
    
    void setViewport(VROViewport viewport);
    void setOrientation(VROCameraOrientation orientation);
    
    /*
     Internal methods.
     */
    void setFrame(ARFrame *frame);
    
private:
    
    /*
     The ARKit session and its delegate.
     */
    ARSession *_session;
    VROARSessionDelegate *_delegate;
    
    /*
     The last computed ARFrame.
     */
    std::unique_ptr<VROARFrame> _currentFrame;
    
    /*
     The current viewport and camera orientation.
     */
    VROViewport _viewport;
    VROCameraOrientation _orientation;
    
    /*
     Background to be assigned to the VROScene.
     */
    std::shared_ptr<VROTexture> _background;
    
    /*
     Video texture cache used for transferring camera content to OpenGL.
     */
    std::shared_ptr<VROVideoTextureCacheOpenGL> _videoTextureCache;
    
};

/*
 Delegate for ARKit's ARSession.
 */
@interface VROARSessionDelegate : NSObject<ARSessionDelegate>

- (id)initWithSession:(std::shared_ptr<VROARSessioniOS>)session;

@end

#endif
#endif /* VROARSessioniOS_h */
