//
//  VROARCameraARCore.h
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARCameraARCore_h
#define VROARCameraARCore_h

#include "VROARCamera.h"
#include "ARCore_API.h"
#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROARSessionARCore.h"
#include <memory>

class VROARSessionARCore;
class VROARFrameARCore;
enum class VROCameraOrientation;

class VROARCameraARCore : public VROARCamera {
public:
    
    VROARCameraARCore(arcore::Frame *frame,
                      std::shared_ptr<VROARSessionARCore> session);
    virtual ~VROARCameraARCore();
    
    VROARTrackingState getTrackingState() const;
    VROARTrackingStateReason getLimitedTrackingStateReason() const;
    
    VROMatrix4f getRotation() const;
    VROVector3f getPosition() const;
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const;

    /*
     Get the image data in RGBA8888 format. The passed in data buffer must be large enough to
     fit the data (image width * image height * 4 bytes). These functions are only valid if
     isImageDataAvailable returns true.
     */
    bool isImageDataAvailable();
    void getImageData(uint8_t *outImageData);
    VROVector3f getImageSize();

    /*
     Crop the AR image givne the current rotation, and store it in the given data buffer. The
     buffer must be large enough to fit the data (cropped image width * cropped image height * 4 bytes).
     */
    void cropImage(const uint8_t *image, int imageStride, uint8_t *outImageData);
    VROVector3f getCroppedImageSize();
    
private:

    arcore::Frame *_frame;
    arcore::Image *_image;
    std::weak_ptr<VROARSessionARCore> _session;

    VROVector3f _position;
    VROMatrix4f _rotation;

    bool loadImageData();
    void getImageCropRectangle(VROARDisplayRotation rotation, int width, int height,
                               int *outLeft, int *outRight, int *outBottom, int *outTop);

};

#endif /* VROARCameraARCore_h */
