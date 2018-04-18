//
//  VROARCameraARCore.cpp
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARCameraARCore.h"
#include "VROViewport.h"
#include "VROMath.h"
#include "VROLog.h"
#include "VROCameraTexture.h"
#include "VROVector3f.h"
#include "VROFieldOfView.h"
#include "VROPlatformUtil.h"
#include "VROMatrix4f.h"
#include "VROARSessionARCore.h"
#include "VROYuvImageConverter.h"

VROARCameraARCore::VROARCameraARCore(arcore::Frame *frame,
                                     std::shared_ptr<VROARSessionARCore> session) :
    _frame(frame),
    _image(nullptr),
    _session(session) {

    float viewMtx[16];
    frame->getViewMatrix(viewMtx);
    VROMatrix4f viewMatrix(viewMtx);
    VROMatrix4f cameraMatrix = viewMatrix.invert();

    _position = { cameraMatrix[12], cameraMatrix[13], cameraMatrix[14] };

    // Remove the translation (this is returned via getPosition()) and we should be
    // left with rotation only
    _rotation = cameraMatrix;
    _rotation[12] = 0;
    _rotation[13] = 0;
    _rotation[14] = 0;
}

VROARCameraARCore::~VROARCameraARCore() {
    if (_image != nullptr) {
        delete (_image);
    }
}

VROARTrackingState VROARCameraARCore::getTrackingState() const {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return VROARTrackingState::Unavailable;
    }

    arcore::TrackingState trackingState = _frame->getTrackingState();
    switch (trackingState) {
        case arcore::TrackingState::NotTracking:
            return VROARTrackingState::Unavailable;
        default:
            return VROARTrackingState::Normal;
    };
}

VROARTrackingStateReason VROARCameraARCore::getLimitedTrackingStateReason() const {
    return VROARTrackingStateReason::None;
}

VROMatrix4f VROARCameraARCore::getRotation() const {
    return _rotation;
}

VROVector3f VROARCameraARCore::getPosition() const {
    return _position;
}

VROMatrix4f VROARCameraARCore::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return {};
    }

    float projectionMtx[16];
    _frame->getProjectionMatrix(near, far, projectionMtx);
    VROMatrix4f projection(projectionMtx);

    float fovX = toDegrees(atan(1.0f / projection[0]) * 2.0);
    float fovY = toDegrees(atan(1.0f / projection[5]) * 2.0);
    *outFOV = VROFieldOfView(fovX / 2.0, fovX / 2.0, fovY / 2.0, fovY / 2.0);

    return projection;
}

bool VROARCameraARCore::isImageDataAvailable() {
    return loadImageData();
}

VROVector3f VROARCameraARCore::getImageSize() {
    return getCroppedImageSize();
}

void VROARCameraARCore::getImageData(uint8_t *outImageData) {
    if (!isImageDataAvailable()) {
        return;
    }
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    VROVector3f rotatedImageSize = getRotatedImageSize();
    int rotatedImageDataLength = (int) rotatedImageSize.x * (int) rotatedImageSize.y * 4;
    uint8_t *rotatedImageData = session->getRotatedCameraImageData(rotatedImageDataLength);

    // Derive the rotated image data from the ARCore _image
    getRotatedImageData(rotatedImageData);

    // Crop the image to match the viewport
    cropImage(rotatedImageData, (int) rotatedImageSize.x, outImageData);
}

VROVector3f VROARCameraARCore::getRotatedImageSize() {
    if (!loadImageData()) {
        return { 0, 0, 0 };
    }
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return { 0, 0, 0 };
    }

    switch (session->getDisplayRotation()) {
        case VROARDisplayRotation::R0:
            return { (float) _image->getHeight(), (float) _image->getWidth(), 0 };
        case VROARDisplayRotation::R90:
            return { (float) _image->getWidth(),  (float) _image->getHeight(), 0 };
        case VROARDisplayRotation::R180:
            return { (float) _image->getHeight(), (float) _image->getWidth(), 0 };
        case VROARDisplayRotation::R270:
            return { (float) _image->getWidth(),  (float) _image->getHeight(), 0 };
    }
}

void VROARCameraARCore::getImageCropRectangle(VROARDisplayRotation rotation, int width, int height,
                                              int *outLeft, int *outRight, int *outBottom, int *outTop) {

    /*
     The original camera image is rotated 90 degrees (or 270 degrees) when the viewport is
     at 0 degree or 180 degree rotation (in other words the camera image is always landscape,
     while the viewport can vary). Capture the correct image width and height using the
     postRotationWidth and postRotationHeight variables.
     */
    float postRotationWidth  = (float) _image->getWidth();
    float postRotationHeight = (float) _image->getHeight();

    if (rotation == VROARDisplayRotation::R0 || rotation == VROARDisplayRotation::R180) {
        postRotationWidth  = (float) _image->getHeight();
        postRotationHeight = (float) _image->getWidth();
    }

    /*
     To map the camera image to the viewport, we have to scale and crop. First we scale the
     camera image to fit the viewport. We scale the image such by the minimum amount that makes
     both dimensions fit the viewport. Unless this scale is equal for both width and height, by
     doing this we do end up scaling one dimension to be *larger* than the viewport. We crop that
     part off: the excess pixels that we need to crop are captured in excessX and excessY, one of
     which is always zero.
     */
    float scaleX = (float) width  / postRotationWidth;
    float scaleY = (float) height / postRotationHeight;
    float scale = std::max(scaleX, scaleY);
    float excessX = postRotationWidth * (scale / scaleX) - postRotationWidth;
    float excessY = postRotationHeight * (scale / scaleY) - postRotationHeight;

    /*
     Before applying the crop operation, we apply the additional crop that ARCore specifies
     through its texture coordinates. The reason for these coordinates is internal to ARCore.
     The coordinates apply differently depending on the surface rotation.
     */
    float texcoords[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    _frame->getBackgroundTexcoords(texcoords);

    VROVector3f BL, TL, BR, TR;
    BL.x = texcoords[0];
    BL.y = texcoords[1];
    TL.x = texcoords[2];
    TL.y = texcoords[3];
    BR.x = texcoords[4];
    BR.y = texcoords[5];
    TR.x = texcoords[6];
    TR.y = texcoords[7];

    switch (rotation) {
        case VROARDisplayRotation::R0:
            // Image was rotated 90 degrees CC
            *outLeft   = (int) (BR.y * postRotationWidth);
            *outRight  = (int) (TL.y * postRotationWidth);
            *outTop    = (int) (TR.x * postRotationHeight);
            *outBottom = (int) (BL.x * postRotationHeight);

            break;

        case VROARDisplayRotation::R90:
            // No rotation
            *outLeft   = (int) (BL.x * postRotationWidth);
            *outRight  = (int) (BR.x * postRotationWidth);
            *outTop    = (int) (TL.y * postRotationHeight);
            *outBottom = (int) (BL.y * postRotationHeight);

            break;

        case VROARDisplayRotation::R180:
            // Image was rotated 270 degrees CC

            // Note: the values below are unconfirmed, as we don't actually seem to support
            // 180 degree rotation
            *outLeft   = (int) (BR.y * postRotationWidth);
            *outRight  = (int) (TL.y * postRotationWidth);
            *outTop    = (int) (TR.x * postRotationHeight);
            *outBottom = (int) (BL.x * postRotationHeight);

            break;

        case VROARDisplayRotation::R270:
            // Image was rotated 180 degrees CC
            *outLeft   = (int) (BR.x * postRotationWidth);
            *outRight  = (int) (BL.x * postRotationWidth);
            *outTop    = (int) (BL.y * postRotationHeight);
            *outBottom = (int) (TL.y * postRotationHeight);

            break;
    }

    /*
     Finally, apply the cropping require by our single-dimension scale operation.
     */
    *outLeft   += excessX / 2.0;
    *outRight  -= excessX / 2.0;
    *outTop    += excessY / 2.0;
    *outBottom -= excessY / 2.0;
}

void VROARCameraARCore::getRotatedImageData(uint8_t *data) {
    if (!loadImageData()) {
        return;
    }
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    switch (session->getDisplayRotation()) {
        case VROARDisplayRotation::R0:
            VROYuvImageConverter::convertImage90(_image, data);
            return;
        case VROARDisplayRotation::R90:
            VROYuvImageConverter::convertImage(_image, data);
            return;
        case VROARDisplayRotation::R180:
            VROYuvImageConverter::convertImage270(_image, data);
            return;
        case VROARDisplayRotation::R270:
            VROYuvImageConverter::convertImage180(_image, data);
            return;
    }
}

bool VROARCameraARCore::loadImageData() {
    if (_image == nullptr) {
        arcore::ImageRetrievalStatus status = _frame->acquireCameraImage(&_image);
        if (status != arcore::ImageRetrievalStatus::Success) {
            pinfo("Failed to aquire image data: error [%d]", status);
            return false;
        }
    }
    return true;
}

void VROARCameraARCore::cropImage(const uint8_t *image, int imageStride, uint8_t *outImageData) {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    int left, right, bottom, top;
    getImageCropRectangle(session->getDisplayRotation(), session->getWidth(), session->getHeight(),
                          &left, &right, &bottom, &top);
    const uint32_t *source = (const uint32_t *) image;
    uint32_t *dest   = (uint32_t *) outImageData;

    int index = 0;
    for (int y = top; y < bottom; y++) {
        for (int x = left; x < right; x++) {
            dest[index] = source[y * imageStride + x];
            ++index;
        }
    }
}

VROVector3f VROARCameraARCore::getCroppedImageSize() {
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return { 0, 0, 0 };
    }

    int left, right, bottom, top;
    getImageCropRectangle(session->getDisplayRotation(), session->getWidth(), session->getHeight(),
                          &left, &right, &bottom, &top);
    return { (float) (right - left), (float) (bottom - top), 0 };
}