//
//  VROARFrame.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARFrame_h
#define VROARFrame_h

#include <memory>
#include <vector>

class VROARCamera;
class VROARAnchor;
class VROMatrix4f;
class VROViewport;
class VROTextureSubstrate;
enum class VROCameraOrientation;

/*
 The continual output of a VROARSession. These frames contain the current camera
 video image, camera parameters, and updated anchors.
 */
class VROARFrame {
public:
    
    VROARFrame() {}
    virtual ~VROARFrame() {}
    
    /*
     Get the timestamp, in seconds.
     */
    virtual double getTimestamp() const = 0;
    
    /*
     Contains information about the camera position, orientation, and imaging
     parameters for this frame.
     */
    virtual const std::shared_ptr<VROARCamera> &getCamera() const = 0;
    
    /*
     Returns the affine transform to apply to the camera background image's 
     texture coordinates. This ensures the camera image maps correctly to the
     current viewport and orientation.
     */
    virtual VROMatrix4f getBackgroundTexcoordTransform() = 0;
    
    /*
     Return the estimated intensity of ambient light in the physical scene.
     */
    virtual float getAmbientLightIntensity() const = 0;
    
    /*
     Get all the anchors representing tracked positions and objects in the
     scene.
     */
    virtual const std::vector<std::shared_ptr<VROARAnchor>> &getAnchors() const = 0;
    
};

#endif /* VROARFrame_h */
