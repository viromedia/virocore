//
//  ARCore_API.h
//  Viro
//
//  Created by Raj Advani on 2/20/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ARCORE_API_h
#define ARCORE_API_h

#include <stdint.h>

typedef struct AImage AImage;

namespace arcore {

    class Anchor;
    class Trackable;
    class HitResultList;
    class PointCloud;
    class HitResult;

    enum class ConfigStatus {
        Success,
        UnsupportedConfiguration,
        SessionNotPaused,
    };

    enum class ImageRetrievalStatus {
        Success,
        InvalidArgument,
        DeadlineExceeded,
        ResourceExhausted,
        NotYetAvailable,
        UnknownError
    };

    enum class TrackingState {
        NotTracking,
        Tracking
    };

    enum class TrackableType {
        Plane,
        Point
    };

    enum class PlaneType {
        NonHorizontal,
        HorizontalUpward,
        HorizontalDownward,
    };

    enum class LightingMode {
        Disabled,
        AmbientIntensity
    };
    enum class PlaneFindingMode {
        Disabled,
        Horizontal
    };
    enum class UpdateMode {
        Blocking,
        LatestCameraImage
    };

    class Config {
    public:
       virtual ~Config() {}
    };

    class Pose {
    public:
        virtual ~Pose() {}
        virtual void toMatrix(float *outMatrix) = 0;
    };

    class AnchorList {
    public:
        virtual ~AnchorList() {}
        virtual Anchor *acquireItem(int index) = 0;
        virtual int size() = 0;
    };

    class Anchor {
    public:
        virtual ~Anchor() {}
        virtual uint64_t getHashCode() = 0;
        virtual uint64_t getId() = 0;
        virtual void getPose(Pose *outPose) = 0;
        virtual TrackingState getTrackingState() = 0;
        virtual void detach() = 0;
    };

    class TrackableList {
    public:
        virtual ~TrackableList() {}
        virtual Trackable *acquireItem(int index) = 0;
        virtual int size() = 0;
    };

    class Trackable {
    public:
        virtual ~Trackable() {}
        virtual Anchor *acquireAnchor(Pose *pose) = 0;
        virtual TrackingState getTrackingState() = 0;
        virtual TrackableType getType() = 0;
    };

    class Plane : public Trackable {
    public:
        virtual ~Plane() {}
        virtual uint64_t getHashCode() = 0;
        virtual void getCenterPose(Pose *outPose) = 0;
        virtual float getExtentX() = 0;
        virtual float getExtentZ() = 0;
        virtual Plane *acquireSubsumedBy() = 0;
        virtual PlaneType getPlaneType() = 0;
        virtual bool isPoseInExtents(const Pose *pose) = 0;
        virtual bool isPoseInPolygon(const Pose *pose) = 0;
        virtual float *getPolygon() = 0;
        virtual int getPolygonSize() = 0;
    };

    class LightEstimate {
    public:
        virtual ~LightEstimate() {}
        virtual float getPixelIntensity() = 0;
        virtual void getColorCorrection(float *outColorCorrection) = 0;
        virtual bool isValid() = 0;
    };

    class Image {
    public:
        virtual ~Image() {}
        virtual int32_t getWidth() = 0;
        virtual int32_t getHeight() = 0;
        virtual int32_t getFormat() = 0;
        virtual void getCropRect(int *outLeft, int *outRight, int *outBottom, int *outTop) = 0;
        virtual int32_t getNumberOfPlanes() = 0;
        virtual int32_t getPlanePixelStride(int planeIdx) = 0;
        virtual int32_t getPlaneRowStride(int planeIdx) = 0;
        virtual void getPlaneData(int planeIdx, uint8_t **outData, int *outDataLength) = 0;
    };

    class Frame {
    public:
        virtual ~Frame() {}
        virtual void getViewMatrix(float *outMatrix) = 0;
        virtual void getProjectionMatrix(float near, float far, float *outMatrix) = 0;
        virtual TrackingState getTrackingState() = 0;
        virtual void getLightEstimate(LightEstimate *outLightEstimate) = 0;
        virtual bool hasDisplayGeometryChanged() = 0;
        virtual void hitTest(float x, float y, HitResultList *outList) = 0;
        virtual int64_t getTimestampNs() = 0;
        virtual void getUpdatedAnchors(AnchorList *outList) = 0;
        virtual void getUpdatedPlanes(TrackableList *outList) = 0;
        virtual void getBackgroundTexcoords(float *outTexcoords) = 0;
        virtual PointCloud *acquirePointCloud() = 0;
        virtual ImageRetrievalStatus acquireCameraImage(Image **outImage) = 0;
    };

    class PointCloud {
    public:
        virtual ~PointCloud() {}
        virtual const float *getPoints() = 0;
        virtual int getNumPoints() = 0;
    };

    class HitResultList {
    public:
        virtual ~HitResultList() {}
        virtual void getItem(int index, HitResult *outResult) = 0;
        virtual int size() = 0;
    };

    class HitResult {
    public:
        virtual ~HitResult() {}
        virtual float getDistance() = 0;
        virtual void getPose(Pose *outPose) = 0;
        virtual Trackable *acquireTrackable() = 0;
        virtual Anchor *acquireAnchor() = 0;
    };

    class Session {
    public:
        virtual ~Session() {}
        virtual ConfigStatus configure(Config *config) = 0;
        virtual bool checkSupported(Config *config) = 0;
        virtual void setDisplayGeometry(int rotation, int width, int height) = 0;
        virtual void setCameraTextureName(int32_t textureId) = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
        virtual void update(Frame *frame) = 0;

        virtual Config *createConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                     UpdateMode updateMode) = 0;
        virtual Pose *createPose() = 0;
        virtual AnchorList *createAnchorList() = 0;
        virtual TrackableList *createTrackableList() = 0;
        virtual HitResultList *createHitResultList() = 0;
        virtual LightEstimate *createLightEstimate() = 0;
        virtual Frame *createFrame() = 0;
        virtual HitResult *createHitResult() = 0;
    };
}

#endif /* ARCORE_API_h */
