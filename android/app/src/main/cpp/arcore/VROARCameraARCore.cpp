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

VROMatrix4f VROARCameraARCore::getProjection(VROViewport viewport, float near, float far, VROFieldOfView *outFOV) const {
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

}

void VROARCameraARCore::getImageData(uint8_t *data) {
    if (!loadImageData()) {
        return;
    }
    std::shared_ptr<VROARSessionARCore> session = _session.lock();
    if (!session) {
        return;
    }

    switch (session->getDisplayRotation()) {
        case VROARDisplayRotation::R0:
            pinfo("0 degree rotation");
            VROYuvImageConverter::convertImage90(_image, data);
            return;
        case VROARDisplayRotation::R90:
            pinfo("90 degree rotation");
            VROYuvImageConverter::convertImage(_image, data);
            return;
        case VROARDisplayRotation::R180:
            pinfo("180 degree rotation");
            VROYuvImageConverter::convertImage270(_image, data);
            return;
        case VROARDisplayRotation::R270:
            pinfo("270 degree rotation");
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
    pinfo("Cropping with left %d, right %d, top %d, bottom %d", left, right, top, bottom);

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