//
//  ARCore_Native_JNI.cpp
//  Viro
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "ARCore_Native.h"
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "Viro"
#endif
#define pinfo(...) \
    do { \
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__); \
    } while (0)

namespace arcore {

#pragma mark - Conversion

    AnchorAcquireStatus convertAnchorStatus(ArStatus status) {
        switch (status) {
            case AR_SUCCESS:
                return AnchorAcquireStatus::Success;
            case AR_ERROR_NOT_TRACKING:
                return AnchorAcquireStatus::ErrorNotTracking;
            case AR_ERROR_SESSION_PAUSED:
                return AnchorAcquireStatus::ErrorSessionPaused;
            case AR_ERROR_RESOURCE_EXHAUSTED:
                return AnchorAcquireStatus::ErrorResourceExhausted;
            case AR_ERROR_DEADLINE_EXCEEDED:
                return AnchorAcquireStatus::ErrorDeadlineExceeded;
            case AR_ERROR_CLOUD_ANCHORS_NOT_CONFIGURED:
                return AnchorAcquireStatus::ErrorCloudAnchorsNotConfigured;
            case AR_ERROR_ANCHOR_NOT_SUPPORTED_FOR_HOSTING:
                return AnchorAcquireStatus::ErrorAnchorNotSupportedForHosting;
            default:
                return AnchorAcquireStatus::ErrorUnknown;
        }
    }

#pragma mark - Config

    ConfigNative::~ConfigNative() {
        ArConfig_destroy(_config);
    }

    void ConfigNative::setAugmentedImageDatabase(AugmentedImageDatabase *database) {
        ArConfig_setAugmentedImageDatabase(((AugmentedImageDatabaseNative *) database)->_session,
                                           _config,
                                           ((AugmentedImageDatabaseNative *) database)->_database);
    }

#pragma mark - AugmentedImageDatabase

    AugmentedImageDatabaseNative::~AugmentedImageDatabaseNative() {
        if (_database) {
            ArAugmentedImageDatabase_destroy(_database);
        }
    }

    AugmentedImageDatabaseStatus
    AugmentedImageDatabaseNative::addImageWithPhysicalSize(const char *image_name,
                                                           const uint8_t *image_grayscale_pixels,
                                                           int32_t image_width_in_pixels,
                                                           int32_t image_height_in_pixels,
                                                           int32_t image_stride_in_pixels,
                                                           float image_width_in_meters,
                                                           int32_t *out_index) {
        ArStatus status = ArAugmentedImageDatabase_addImageWithPhysicalSize(_session, _database,
                                                                            image_name,
                                                                            image_grayscale_pixels,
                                                                            image_width_in_pixels,
                                                                            image_height_in_pixels,
                                                                            image_stride_in_pixels,
                                                                            image_width_in_meters,
                                                                            out_index);

        if (status == AR_ERROR_IMAGE_INSUFFICIENT_QUALITY) {
            return AugmentedImageDatabaseStatus::ImageInsufficientQuality;
        } else {
            return AugmentedImageDatabaseStatus::Success;
        }
    }

#pragma mark - Pose

    PoseNative::~PoseNative() {
        ArPose_destroy(_pose);
    }

    void PoseNative::toMatrix(float *outMatrix) {
        ArPose_getMatrix(_session, _pose, outMatrix);
    }

#pragma mark - AnchorList

    AnchorListNative::~AnchorListNative() {
        ArAnchorList_destroy(_anchorList);
    }

    Anchor *AnchorListNative::acquireItem(int index) {
        ArAnchor *anchor;
        ArAnchorList_acquireItem(_session, _anchorList, index, &anchor);
        return new AnchorNative(anchor, _session);
    }

    int AnchorListNative::size() {
        int size;
        ArAnchorList_getSize(_session, _anchorList, &size);
        return size;
    }

#pragma mark - Anchor

    AnchorNative::AnchorNative(ArAnchor *anchor, ArSession *session) : _anchor(anchor), _session(session) {
    }

    AnchorNative::~AnchorNative() {
        ArAnchor_release(_anchor);
    }

    uint64_t AnchorNative::getHashCode() {
        return reinterpret_cast<uint64_t>(_anchor);
    }

    uint64_t AnchorNative::getId() {
        return reinterpret_cast<uint64_t>(_anchor);
    }

    void AnchorNative::getPose(Pose *outPose) {
        ArAnchor_getPose(_session, _anchor, ((PoseNative *) outPose)->_pose);
    }

    void AnchorNative::getTransform(float *outTransform) {
        ArPose *pose;
        ArPose_create(_session, nullptr, &pose);
        ArAnchor_getPose(_session, _anchor, pose);
        ArPose_getMatrix(_session, pose, outTransform);
        ArPose_destroy(pose);
    }

    TrackingState AnchorNative::getTrackingState() {
        ArTrackingState trackingState;
        ArAnchor_getTrackingState(_session, _anchor, &trackingState);

        if (trackingState == AR_TRACKING_STATE_TRACKING) {
            return TrackingState::Tracking;
        } else {
            return TrackingState::NotTracking;
        }
    }

    void AnchorNative::acquireCloudAnchorId(char **outCloudAnchorId) {
        ArAnchor_acquireCloudAnchorId(_session, _anchor, outCloudAnchorId);
    }

    CloudAnchorState AnchorNative::getCloudAnchorState() {
        ArCloudAnchorState state;
        ArAnchor_getCloudAnchorState(_session, _anchor, &state);
        switch (state) {
            case AR_CLOUD_ANCHOR_STATE_NONE:
                return CloudAnchorState::None;
            case AR_CLOUD_ANCHOR_STATE_TASK_IN_PROGRESS:
                return CloudAnchorState::TaskInProgress;
            case AR_CLOUD_ANCHOR_STATE_SUCCESS:
                return CloudAnchorState::Success;
            case AR_CLOUD_ANCHOR_STATE_ERROR_INTERNAL:
                return CloudAnchorState::ErrorInternal;
            case AR_CLOUD_ANCHOR_STATE_ERROR_NOT_AUTHORIZED:
                return CloudAnchorState::ErrorNotAuthorized;
            case AR_CLOUD_ANCHOR_STATE_ERROR_SERVICE_UNAVAILABLE:
                return CloudAnchorState::ErrorServiceUnavailable;
            case AR_CLOUD_ANCHOR_STATE_ERROR_RESOURCE_EXHAUSTED:
                return CloudAnchorState::ErrorResourceExhausted;
            case AR_CLOUD_ANCHOR_STATE_ERROR_HOSTING_DATASET_PROCESSING_FAILED:
                return CloudAnchorState::ErrorDatasetProcessingFailed;
            case AR_CLOUD_ANCHOR_STATE_ERROR_CLOUD_ID_NOT_FOUND:
                return CloudAnchorState::ErrorCloudIDNotFound;
            case AR_CLOUD_ANCHOR_STATE_ERROR_RESOLVING_LOCALIZATION_NO_MATCH:
                return CloudAnchorState::ErrorResolvingLocalizationNoMatch;
            case AR_CLOUD_ANCHOR_STATE_ERROR_RESOLVING_SDK_VERSION_TOO_OLD:
                return CloudAnchorState::ErrorResolvingSDKVersionTooOld;
            case AR_CLOUD_ANCHOR_STATE_ERROR_RESOLVING_SDK_VERSION_TOO_NEW:
                return CloudAnchorState::ErrorResolvingSDKVersionTooNew;
            default:
                return CloudAnchorState::ErrorInternal;
        }
    }

    void AnchorNative::detach() {
        ArAnchor_detach(_session, _anchor);
    }

#pragma mark - TrackableList

    TrackableListNative::~TrackableListNative() {
        ArTrackableList_destroy(_trackableList);
    }

    Trackable *TrackableListNative::acquireItem(int index) {
        ArTrackable *trackable;
        ArTrackableList_acquireItem(_session, _trackableList, index, &trackable);

        ArTrackableType type;
        ArTrackable_getType(_session, trackable, &type);
        if (type == AR_TRACKABLE_PLANE) {
            return new PlaneNative(ArAsPlane(trackable), _session);
        } else if (type == AR_TRACKABLE_AUGMENTED_IMAGE) {
            return new AugmentedImageNative(ArAsAugmentedImage(trackable), _session);
        } else {
            return nullptr;
        }
    }

    int TrackableListNative::size() {
        int size;
        ArTrackableList_getSize(_session, _trackableList, &size);
        return size;
    }

#pragma mark - Plane

    PlaneNative::PlaneNative(ArPlane *plane, ArSession *session) :
            _trackable(ArAsTrackable(plane)), _plane(plane), _session(session) {

    }

    PlaneNative::~PlaneNative() {
        ArTrackable_release(_trackable);
    }

    Anchor *PlaneNative::acquireAnchor(Pose *pose) {
        ArAnchor *anchor;
        ArStatus status = ArTrackable_acquireNewAnchor(_session, _trackable, ((PoseNative *) pose)->_pose, &anchor);
        if (status == AR_SUCCESS) {
            return new AnchorNative(anchor, _session);
        } else {
            return nullptr;
        };
    }

    TrackingState PlaneNative::getTrackingState() {
        ArTrackingState trackingState;
        ArTrackable_getTrackingState(_session, _trackable, &trackingState);

        if (trackingState == AR_TRACKING_STATE_TRACKING) {
            return TrackingState::Tracking;
        } else {
            return TrackingState::NotTracking;
        }
    }

    TrackableType PlaneNative::getType() {
        ArTrackableType type;
        ArTrackable_getType(_session, _trackable, &type);

        if (type == AR_TRACKABLE_PLANE) {
            return TrackableType::Plane;
        } else {
            return TrackableType::Point;
        }
    }

    uint64_t PlaneNative::getHashCode() {
        return reinterpret_cast<uint64_t>(_plane);
    }

    void PlaneNative::getCenterPose(Pose *outPose) {
        ArPlane_getCenterPose(_session, _plane, ((PoseNative *) outPose)->_pose);
    }

    float PlaneNative::getExtentX() {
        float extent;
        ArPlane_getExtentX(_session, _plane, &extent);
        return extent;
    }

    float PlaneNative::getExtentZ() {
        float extent;
        ArPlane_getExtentZ(_session, _plane, &extent);
        return extent;
    }

    Plane *PlaneNative::acquireSubsumedBy() {
        ArPlane *subsumedBy;
        ArPlane_acquireSubsumedBy(_session, _plane, &subsumedBy);
        return subsumedBy != NULL ? new PlaneNative(subsumedBy, _session) : NULL;
    }

    PlaneType PlaneNative::getPlaneType() {
        ArPlaneType type;
        ArPlane_getType(_session, _plane, &type);

        if (type == AR_PLANE_HORIZONTAL_DOWNWARD_FACING) {
            return PlaneType::HorizontalDownward;
        } else if (type == AR_PLANE_HORIZONTAL_UPWARD_FACING) {
            return PlaneType::HorizontalUpward;
        } else if (type == AR_PLANE_VERTICAL) {
            return PlaneType::Vertical;
        } else {
            return PlaneType::HorizontalUpward;
        }
    }

    bool PlaneNative::isPoseInExtents(const Pose *pose) {
        int result;
        ArPlane_isPoseInExtents(_session, _plane, ((PoseNative *) pose)->_pose, &result);
        return (bool) result;
    }

    float *PlaneNative::getPolygon() {
        int32_t polygon_length;
        ArPlane_getPolygonSize(_session, _plane, &polygon_length);

        float *polygonBoundaryVertices = new float[polygon_length];
        if (polygon_length == 0) {
            return polygonBoundaryVertices;
        }

        ArPlane_getPolygon(_session, _plane, polygonBoundaryVertices);
        return polygonBoundaryVertices;
    }

    int PlaneNative::getPolygonSize() {
        int32_t polygon_length;
        ArPlane_getPolygonSize(_session, _plane, &polygon_length);
        return polygon_length;
    }

    bool PlaneNative::isPoseInPolygon(const Pose *pose) {
        int result;
        ArPlane_isPoseInPolygon(_session, _plane, ((PoseNative *) pose)->_pose, &result);
        return (bool) result;
    }

#pragma mark - AugmentedImage

    AugmentedImageNative::AugmentedImageNative(ArAugmentedImage *image, ArSession *session) :
            _trackable(ArAsTrackable(image)), _image(image), _session(session) {
    }

    AugmentedImageNative::~AugmentedImageNative() {
        ArTrackable_release(_trackable);
    }

    Anchor *AugmentedImageNative::acquireAnchor(Pose *pose) {
        ArAnchor *anchor;
        ArTrackable_acquireNewAnchor(_session, _trackable, ((PoseNative *) pose)->_pose, &anchor);
        return new AnchorNative(anchor, _session);
    }

    TrackingState AugmentedImageNative::getTrackingState() {
        ArTrackingState trackingState;
        ArTrackable_getTrackingState(_session, _trackable, &trackingState);

        if (trackingState == AR_TRACKING_STATE_TRACKING) {
            return TrackingState::Tracking;
        } else {
            return TrackingState::NotTracking;
        }
    }

    TrackableType AugmentedImageNative::getType() {
        ArTrackableType type;
        ArTrackable_getType(_session, _trackable, &type);

        if (type == AR_TRACKABLE_AUGMENTED_IMAGE) {
            return TrackableType::Image;
        } else {
            return TrackableType::Point;
        }
    }

    char *AugmentedImageNative::getName() {
        char *name;
        ArAugmentedImage_acquireName(_session, _image, &name);
        return name;
    }

    void AugmentedImageNative::getCenterPose(Pose *outPose) {
        ArAugmentedImage_getCenterPose(_session, _image, ((PoseNative *) outPose)->_pose);
    }

    float AugmentedImageNative::getExtentX() {
        float extent;
        ArAugmentedImage_getExtentX(_session, _image, &extent);
        return extent;
    }

    float AugmentedImageNative::getExtentZ() {
        float extent;
        ArAugmentedImage_getExtentZ(_session, _image, &extent);
        return extent;
    }

    int32_t AugmentedImageNative::getIndex() {
        int32_t index;
        ArAugmentedImage_getIndex(_session, _image, &index);
        return index;
    }

#pragma mark - LightEstimate

    LightEstimateNative::~LightEstimateNative() {
        ArLightEstimate_destroy(_lightEstimate);
    }

    float LightEstimateNative::getPixelIntensity() {
        float intensity;
        ArLightEstimate_getPixelIntensity(_session, _lightEstimate, &intensity);
        return intensity;
    }

    void LightEstimateNative::getColorCorrection(float *outCorrection) {
        ArLightEstimate_getColorCorrection(_session, _lightEstimate, outCorrection);
    }

    bool LightEstimateNative::isValid() {
        ArLightEstimateState state;
        ArLightEstimate_getState(_session, _lightEstimate, &state);
        if (state == AR_LIGHT_ESTIMATE_STATE_NOT_VALID) {
            return false;
        } else {
            return true;
        }
    }

#pragma mark - Image

    ImageNative::ImageNative(ArImage *arImage) : _arImage(arImage) {
        ArImage_getNdkImage(_arImage, &_image);
    }

    ImageNative::~ImageNative() {
        ArImage_release(_arImage);
    }

    int32_t ImageNative::getWidth() {
        int32_t width;
        media_status_t status = AImage_getWidth(_image, &width);
        return status == AMEDIA_OK ? width : 0;
    }

    int32_t ImageNative::getHeight() {
        int32_t height;
        media_status_t status = AImage_getHeight(_image, &height);
        return status == AMEDIA_OK ? height : 0;
    }

    int32_t ImageNative::getFormat() {
        int32_t format;
        media_status_t status = AImage_getFormat(_image, &format);
        return status == AMEDIA_OK ? format : 0;
    }

    void ImageNative::getCropRect(int *outLeft, int *outRight, int *outBottom, int *outTop) {
        AImageCropRect rect;
        AImage_getCropRect(_image, &rect);

        *outLeft = rect.left;
        *outRight = rect.right;
        *outBottom = rect.bottom;
        *outTop = rect.top;
    }

    int32_t ImageNative::getNumberOfPlanes() {
        int32_t numPlanes;
        media_status_t status = AImage_getNumberOfPlanes(_image, &numPlanes);
        return status == AMEDIA_OK ? numPlanes : 0;
    }

    int32_t ImageNative::getPlanePixelStride(int planeIdx) {
        int32_t planePixelStride;
        media_status_t status = AImage_getPlanePixelStride(_image, planeIdx, &planePixelStride);
        return status == AMEDIA_OK ? planePixelStride : 0;
    }

    int32_t ImageNative::getPlaneRowStride(int planeIdx) {
        int32_t planeRowStride;
        media_status_t status = AImage_getPlaneRowStride(_image, planeIdx, &planeRowStride);
        return status == AMEDIA_OK ? planeRowStride : 0;
    }

    void ImageNative::getPlaneData(int planeIdx, uint8_t **outData, int *outDataLength) {
        AImage_getPlaneData(_image, planeIdx, outData, outDataLength);
    }

#pragma mark - Frame

    FrameNative::~FrameNative() {
        ArFrame_destroy(_frame);
    }

    void FrameNative::getViewMatrix(float *outMatrix) {
        ArCamera *camera;
        ArFrame_acquireCamera(_session, _frame, &camera);
        ArCamera_getViewMatrix(_session, camera, outMatrix);
        ArCamera_release(camera);
    }

    void FrameNative::getProjectionMatrix(float near, float far, float *outMatrix) {
        ArCamera *camera;
        ArFrame_acquireCamera(_session, _frame, &camera);
        float matrix[16];
        ArCamera_getProjectionMatrix(_session, camera, near, far, outMatrix);
        ArCamera_release(camera);
    }

    void FrameNative::getImageIntrinsics(float *outFx, float *outFy, float *outCx, float *outCy) {
        ArCamera *camera;
        ArFrame_acquireCamera(_session, _frame, &camera);
        ArCameraIntrinsics *outIntrinsics;
        ArCameraIntrinsics_create(_session, &outIntrinsics);
        ArCamera_getImageIntrinsics(_session, camera, outIntrinsics);
        ArCameraIntrinsics_getFocalLength(_session, outIntrinsics, outFx, outFy);
        ArCameraIntrinsics_getPrincipalPoint(_session, outIntrinsics, outCx, outCy);
        ArCamera_release(camera);
        ArCameraIntrinsics_destroy(outIntrinsics);
    }

    TrackingState FrameNative::getTrackingState() {
        ArCamera *camera;
        ArFrame_acquireCamera(_session, _frame, &camera);

        ArTrackingState trackingState;
        ArCamera_getTrackingState(_session, camera, &trackingState);
        ArCamera_release(camera);

        if (trackingState == AR_TRACKING_STATE_PAUSED ||
            trackingState == AR_TRACKING_STATE_STOPPED) {
            return TrackingState::NotTracking;
        } else {
            return TrackingState::Tracking;
        }
    }

    void FrameNative::getLightEstimate(LightEstimate *outLightEstimate) {
        ArFrame_getLightEstimate(_session, _frame,
                                 ((LightEstimateNative *) outLightEstimate)->_lightEstimate);
    }

    int64_t FrameNative::getTimestampNs() {
        int64_t timestamp;
        ArFrame_getTimestamp(_session, _frame, &timestamp);
        return timestamp;
    }

    void FrameNative::getUpdatedAnchors(AnchorList *outList) {
        ArFrame_getUpdatedAnchors(_session, _frame, ((AnchorListNative *) outList)->_anchorList);
    }

    void FrameNative::getUpdatedTrackables(TrackableList *outList, TrackableType type) {
        if (type == TrackableType::Plane) {
            ArFrame_getUpdatedTrackables(_session, _frame, AR_TRACKABLE_PLANE,
                                         ((TrackableListNative *) outList)->_trackableList);
        } else if (type == TrackableType::Image) {
            ArFrame_getUpdatedTrackables(_session, _frame, AR_TRACKABLE_AUGMENTED_IMAGE,
                                         ((TrackableListNative *) outList)->_trackableList);
        }
    }

    bool FrameNative::hasDisplayGeometryChanged() {
        int changed;
        ArFrame_getDisplayGeometryChanged(_session, _frame, &changed);
        return (bool) changed;
    }

    void FrameNative::hitTest(float x, float y, HitResultList *outList) {
        ArFrame_hitTest(_session, _frame, x, y, ((HitResultListNative *) outList)->_hitResultList);
    }

    void FrameNative::hitTest(float px, float py, float pz, float qx, float qy, float qz, HitResultList *outList) {
        float origin[3] = { px, py, pz };
        float dest[3] = {qx, qy, qz};
        ArFrame_hitTestRay(_session, _frame, origin, dest, ((HitResultListNative *) outList)->_hitResultList);
    }

    void FrameNative::getBackgroundTexcoords(float *outTexcoords) {
        // BL, TL, BR, TR
        const float source[8] = {0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0};
        ArFrame_transformCoordinates2d(_session, _frame, AR_COORDINATES_2D_VIEW_NORMALIZED, 4, source, AR_COORDINATES_2D_TEXTURE_NORMALIZED, outTexcoords);
    }

    PointCloud *FrameNative::acquirePointCloud() {
        ArPointCloud *cloud;
        ArFrame_acquirePointCloud(_session, _frame, &cloud);
        return new PointCloudNative(cloud, _session);
    }

    ImageRetrievalStatus FrameNative::acquireCameraImage(Image **outImage) {
        ArImage *arImage;
        ArStatus status = ArFrame_acquireCameraImage(_session, _frame, &arImage);
        if (status == AR_ERROR_INVALID_ARGUMENT) {
            return ImageRetrievalStatus::InvalidArgument;
        } else if (status == AR_ERROR_DEADLINE_EXCEEDED) {
            return ImageRetrievalStatus::DeadlineExceeded;
        } else if (status == AR_ERROR_RESOURCE_EXHAUSTED) {
            return ImageRetrievalStatus::ResourceExhausted;
        } else if (status == AR_ERROR_NOT_YET_AVAILABLE) {
            return ImageRetrievalStatus::NotYetAvailable;
        } else if (status == AR_SUCCESS) {
            *outImage = new ImageNative(arImage);
            return ImageRetrievalStatus::Success;
        } else {
            return ImageRetrievalStatus::UnknownError;
        }
    }

#pragma mark - PointCloud

    PointCloudNative::~PointCloudNative() {
        ArPointCloud_release(_pointCloud);
    }

    const float *PointCloudNative::getPoints() {
        const float *points;
        ArPointCloud_getData(_session, _pointCloud, &points);
        return points;
    }

    int PointCloudNative::getNumPoints() {
        int numPoints;
        ArPointCloud_getNumberOfPoints(_session, _pointCloud, &numPoints);
        return numPoints;
    }

    const int * PointCloudNative::getPointIds() {
        const int* point_cloud_ids;
        ArPointCloud_getPointIds(_session, _pointCloud, &point_cloud_ids);
        return point_cloud_ids;
    }

#pragma mark - HitResultList

    HitResultListNative::~HitResultListNative() {
        ArHitResultList_destroy(_hitResultList);
    }

    void HitResultListNative::getItem(int index, HitResult *outResult) {
        ArHitResultList_getItem(_session, _hitResultList, index,
                                ((HitResultNative *) outResult)->_hitResult);
    }

    int HitResultListNative::size() {
        int size;
        ArHitResultList_getSize(_session, _hitResultList, &size);
        return size;
    }

#pragma mark - HitResult

    HitResultNative::~HitResultNative() {
        ArHitResult_destroy(_hitResult);
    }

    float HitResultNative::getDistance() {
        float distance;
        ArHitResult_getDistance(_session, _hitResult, &distance);
        return distance;
    }

    void HitResultNative::getPose(Pose *outPose) {
        ArHitResult_getHitPose(_session, _hitResult, ((PoseNative *) outPose)->_pose);
    }

    void HitResultNative::getTransform(float *outTransform) {
        ArPose *pose;
        ArPose_create(_session, nullptr, &pose);
        ArHitResult_getHitPose(_session, _hitResult, pose);
        ArPose_getMatrix(_session, pose, outTransform);
        ArPose_destroy(pose);
    }

    Trackable *HitResultNative::acquireTrackable() {
        ArTrackable *trackable;
        ArHitResult_acquireTrackable(_session, _hitResult, &trackable);

        ArTrackableType type;
        ArTrackable_getType(_session, trackable, &type);
        if (type == AR_TRACKABLE_PLANE) {
            return new PlaneNative(ArAsPlane(trackable), _session);
        } else {
            return nullptr;
        }
    }

    Anchor *HitResultNative::acquireAnchor() {
        ArAnchor *anchor;
        ArStatus status = ArHitResult_acquireNewAnchor(_session, _hitResult, &anchor);
        if (status == AR_SUCCESS) {
            return new AnchorNative(anchor, _session);
        } else {
            return nullptr;
        };
    }

#pragma mark - Session

    SessionNative::SessionNative(void *applicationContext, JNIEnv *env) {
        ArSession_create(env, applicationContext, &_session);
    }

    SessionNative::~SessionNative() {
        ArSession_destroy(_session);
    }

    ConfigStatus SessionNative::configure(Config *config) {
        ArStatus status = ArSession_configure(_session, ((ConfigNative *) config)->_config);
        if (status == AR_SUCCESS) {
            return ConfigStatus::Success;
        } else if (status == AR_ERROR_UNSUPPORTED_CONFIGURATION) {
            return ConfigStatus::UnsupportedConfiguration;
        } else if (status == AR_ERROR_SESSION_NOT_PAUSED) {
            return ConfigStatus::SessionNotPaused;
        }
        return ConfigStatus::UnsupportedConfiguration;
    }

    void
    SessionNative::setDisplayGeometry(int rotation, int width, int height) {
        ArSession_setDisplayGeometry(_session, rotation, width, height);
    }

    void SessionNative::setCameraTextureName(int32_t textureId) {
        ArSession_setCameraTextureName(_session, textureId);
    }

    void SessionNative::pause() {
        ArSession_pause(_session);
    }

    void SessionNative::resume() {
        ArSession_resume(_session);
    }

    void SessionNative::update(Frame *frame) {
        ArSession_update(_session, ((FrameNative *) frame)->_frame);
    }

    Config *
    SessionNative::createConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                UpdateMode updateMode, CloudAnchorMode cloudAnchorMode, FocusMode focusMode) {
        ArConfig *config;
        ArConfig_create(_session, &config);

        // Set light estimation mode
        ArLightEstimationMode arLightingMode;
        switch (lightingMode) {
            case LightingMode::Disabled: {
                arLightingMode = AR_LIGHT_ESTIMATION_MODE_DISABLED;
                break;
            }
            case LightingMode::AmbientIntensity: {
                arLightingMode = AR_LIGHT_ESTIMATION_MODE_AMBIENT_INTENSITY;
                break;
            }
        }
        ArConfig_setLightEstimationMode(_session, config, arLightingMode);

        // Set plane finding mode
        ArPlaneFindingMode arPlaneFindingMode;
        switch (planeFindingMode) {
            case PlaneFindingMode::Disabled:
                arPlaneFindingMode = AR_PLANE_FINDING_MODE_DISABLED;
                break;
            case PlaneFindingMode::Horizontal:
                arPlaneFindingMode = AR_PLANE_FINDING_MODE_HORIZONTAL;
                break;
            case PlaneFindingMode::HorizontalAndVertical:
                arPlaneFindingMode = AR_PLANE_FINDING_MODE_HORIZONTAL_AND_VERTICAL;
                break;
            case PlaneFindingMode::Vertical:
                arPlaneFindingMode = AR_PLANE_FINDING_MODE_VERTICAL;
                break;
        }
        ArConfig_setPlaneFindingMode(_session, config, arPlaneFindingMode);

        // Set update mode
        ArUpdateMode arUpdateMode;
        switch (updateMode) {
            case UpdateMode::Blocking: {
                arUpdateMode = AR_UPDATE_MODE_BLOCKING;
                break;
            }
            case UpdateMode::LatestCameraImage: {
                arUpdateMode = AR_UPDATE_MODE_LATEST_CAMERA_IMAGE;
                break;
            }
        }
        ArConfig_setUpdateMode(_session, config, arUpdateMode);

        // Set cloud anchor mode
        ArCloudAnchorMode arCloudAnchorMode;
        switch (cloudAnchorMode) {
            case CloudAnchorMode::Disabled:
                arCloudAnchorMode = AR_CLOUD_ANCHOR_MODE_DISABLED;
                break;
            case CloudAnchorMode::Enabled:
                arCloudAnchorMode = AR_CLOUD_ANCHOR_MODE_ENABLED;
                break;
        }
        ArConfig_setCloudAnchorMode(_session, config, arCloudAnchorMode);

        // Set Camera focus mode
        ArFocusMode  arFocusMode;
        switch (focusMode) {
            case FocusMode::FIXED_FOCUS:
                arFocusMode = AR_FOCUS_MODE_FIXED;
                break;
            case FocusMode::AUTO_FOCUS:
                arFocusMode = AR_FOCUS_MODE_AUTO;
                break;
        }
        ArConfig_setFocusMode(_session, config, arFocusMode);
        return new ConfigNative(config);
    }

    AugmentedImageDatabase *SessionNative::createAugmentedImageDatabase() {
        ArAugmentedImageDatabase *database;
        ArAugmentedImageDatabase_create(_session, &database);

        return new AugmentedImageDatabaseNative(database, _session);
    }

    AugmentedImageDatabase *SessionNative::createAugmentedImageDatabase(uint8_t* raw_buffer, int64_t size) {
        ArAugmentedImageDatabase *database;

        ArStatus status = ArAugmentedImageDatabase_deserialize(_session, raw_buffer, size, &database);

        if (status != AR_SUCCESS) {
            pinfo("[Viro] Failed to load AugmentedImageDatabase, error: %d", status);
        }

        return new AugmentedImageDatabaseNative(database, _session);
    }

    Pose *SessionNative::createPose() {
        ArPose *pose;
        ArPose_create(_session, nullptr, &pose);
        return new PoseNative(pose, _session);
    }

    Pose *SessionNative::createPose(float px, float py, float pz, float qx, float qy, float qz, float qw) {
        ArPose *pose;
        float poseRaw[7] = { qx, qy, qz, qw, px, py, pz };
        ArPose_create(_session, poseRaw, &pose);
        return new PoseNative(pose, _session);
    }

    AnchorList *SessionNative::createAnchorList() {
        ArAnchorList *list;
        ArAnchorList_create(_session, &list);
        return new AnchorListNative(list, _session);
    }

    TrackableList *SessionNative::createTrackableList() {
        ArTrackableList *list;
        ArTrackableList_create(_session, &list);
        return new TrackableListNative(list, _session);
    }

    HitResultList *SessionNative::createHitResultList() {
        ArHitResultList *list;
        ArHitResultList_create(_session, &list);
        return new HitResultListNative(list, _session);
    }

    LightEstimate *SessionNative::createLightEstimate() {
        ArLightEstimate *estimate;
        ArLightEstimate_create(_session, &estimate);
        return new LightEstimateNative(estimate, _session);
    }

    HitResult *SessionNative::createHitResult() {
        ArHitResult *result;
        ArHitResult_create(_session, &result);
        return new HitResultNative(result, _session);
    }

    Frame *SessionNative::createFrame() {
        ArFrame *frame;
        ArFrame_create(_session, &frame);
        return new FrameNative(frame, _session);
    }

    Anchor *SessionNative::acquireNewAnchor(const Pose *pose) {
        ArAnchor *anchor;
        ArStatus status = ArSession_acquireNewAnchor(_session, ((PoseNative *)pose)->_pose, &anchor);
        if (status == AR_SUCCESS) {
            return new AnchorNative(anchor, _session);
        } else {
            return nullptr;
        };
    }

    Anchor *SessionNative::hostAndAcquireNewCloudAnchor(const Anchor *anchor, AnchorAcquireStatus *status_v) {
        ArAnchor *cloudAnchor;
        ArStatus status = ArSession_hostAndAcquireNewCloudAnchor(_session, ((AnchorNative *) anchor)->_anchor,
                                                                 &cloudAnchor);
        if (status == AR_SUCCESS) {
            *status_v = AnchorAcquireStatus::Success;
            return new AnchorNative(cloudAnchor, _session);
        } else {
            *status_v = convertAnchorStatus(status);
            return nullptr;
        };
    }

    Anchor *SessionNative::resolveAndAcquireNewCloudAnchor(const char *anchorId, AnchorAcquireStatus *status_v) {
        ArAnchor *cloudAnchor;
        ArStatus status = ArSession_resolveAndAcquireNewCloudAnchor(_session, anchorId, &cloudAnchor);
        if (status == AR_SUCCESS) {
            *status_v = AnchorAcquireStatus::Success;
            return new AnchorNative(cloudAnchor, _session);
        } else {
            *status_v = convertAnchorStatus(status);
            return nullptr;
        };
    }

}