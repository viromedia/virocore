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
    VROMatrix4f getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV);

    /*
     Get the image data in RGBA8888 format. The passed in data buffer must be large enough to
     fit the data (image width * image height * 4 bytes). These functions are only valid if
     isImageDataAvailable returns true.

     Note these functions return the 'cropped' image data, which matches the dimensions and
     rotation of the current AR viewport.
     */
    bool isImageDataAvailable();
    void getImageData(uint8_t *outImageData);
    VROVector3f getImageSize();
    
private:

    arcore::Frame *_frame;
    arcore::Image *_image;
    std::weak_ptr<VROARSessionARCore> _session;

    VROVector3f _position;
    VROMatrix4f _rotation;

    /*
     Load the image data from ARCore, and stores it in _image.
     */
    bool loadImageData();

    /*
     Retrieve the rotated camera image data in RGBA. The ARCore _image is converted from YCbCr to
     RGBA and rotated to fit the current viewport. It is not cropped to fit the current viewport size.
     */
    void getRotatedImageData(uint8_t *outImageData);
    VROVector3f getRotatedImageSize();

    /*
    Retrieve the cropping rectangle to use on the rotated image data to get the cropped image
    rectangle.
    */
    void getImageCropRectangle(VROARDisplayRotation rotation, int width, int height,
                               int *outLeft, int *outRight, int *outBottom, int *outTop);

    /*
     Crop the AR image given the current rotation, and store it in the given data buffer. The
     buffer must be large enough to fit the data (cropped image width * cropped image height * 4 bytes).
     */
    void cropImage(const uint8_t *image, int imageStride, uint8_t *outImageData);
    VROVector3f getCroppedImageSize();

};

#endif /* VROARCameraARCore_h */
