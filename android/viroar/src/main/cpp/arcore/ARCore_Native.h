//
//  ARCore_Native.h
//  Viro
//
//  Created by Raj Advani on 2/20/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ARCORE_NATIVE_h
#define ARCORE_NATIVE_h

#include <jni.h>
#include "ARCore_API.h"
#include "arcore_c_api.h"

namespace arcore {
    class ConfigNative : public Config {
    public:
        ConfigNative(ArConfig *config) : _config(config) {}
        virtual ~ConfigNative();
        virtual void setAugmentedImageDatabase(AugmentedImageDatabase *database);
        ArConfig *_config;
    };

    class AugmentedImageDatabaseNative : public AugmentedImageDatabase {
    public:
        AugmentedImageDatabaseNative(ArAugmentedImageDatabase *database, ArSession *session) :
            _database(database),
            _session(session) {}
        virtual ~AugmentedImageDatabaseNative();
        virtual AugmentedImageDatabaseStatus addImageWithPhysicalSize(const char *image_name, const uint8_t *image_grayscale_pixels,
                                                                      int32_t image_width_in_pixels, int32_t image_height_in_pixels,
                                                                      int32_t image_stride_in_pixels, float image_width_in_meters,
                                                                      int32_t *out_index);
        ArAugmentedImageDatabase *_database;
        ArSession *_session;
    };

    class PoseNative : public Pose {
    public:
        PoseNative(ArPose *pose, ArSession *session) : _pose(pose), _session(session) {}
        virtual ~PoseNative();
        virtual void toMatrix(float *outMatrix);

        ArPose *_pose;
        ArSession *_session;
    };

    class AnchorListNative : public AnchorList {
    public:
        AnchorListNative(ArAnchorList *anchorList, ArSession *session) : _anchorList(anchorList), _session(session) {}
        virtual ~AnchorListNative();
        virtual Anchor *acquireItem(int index);
        virtual int size();

        ArAnchorList *_anchorList;
        ArSession *_session;
    };

    class AnchorNative : public Anchor {
    public:
        AnchorNative(ArAnchor *anchor, ArSession *session);
        virtual ~AnchorNative();
        virtual uint64_t getHashCode();
        virtual uint64_t getId();
        virtual void getPose(Pose *outPose);
        virtual void getTransform(float *outTransform);
        virtual TrackingState getTrackingState();
        virtual void acquireCloudAnchorId(char **outCloudAnchorId);
        virtual CloudAnchorState getCloudAnchorState();
        virtual void detach();

        ArAnchor *_anchor;
        ArSession *_session;
    };

    class TrackableListNative : public TrackableList {
    public:
        TrackableListNative(ArTrackableList *trackableList, ArSession *session) : _trackableList(trackableList), _session(session) {}
        virtual ~TrackableListNative();
        virtual Trackable *acquireItem(int index);
        virtual int size();

        ArTrackableList *_trackableList;
        ArSession *_session;
    };

    class PlaneNative : public Plane {
    public:
        PlaneNative(ArPlane *plane, ArSession *session);
        virtual ~PlaneNative();

        virtual Anchor *acquireAnchor(Pose *pose);
        virtual TrackingState getTrackingState();
        virtual TrackableType getType();

        virtual uint64_t getHashCode();
        virtual void getCenterPose(Pose *outPose);
        virtual float getExtentX();
        virtual float getExtentZ();
        virtual Plane *acquireSubsumedBy();
        virtual PlaneType getPlaneType();
        virtual bool isPoseInExtents(const Pose *pose);
        virtual bool isPoseInPolygon(const Pose *pose);
        virtual float *getPolygon();
        virtual int getPolygonSize();
    private:
        ArTrackable *_trackable;
        ArPlane *_plane;
        ArSession *_session;
    };

    class AugmentedImageNative : public AugmentedImage {
    public:
        AugmentedImageNative(ArAugmentedImage *image, ArSession *session);
        virtual ~AugmentedImageNative();

        virtual Anchor *acquireAnchor(Pose *pose);
        virtual TrackingState getTrackingState();
        virtual TrackableType getType();

        virtual char *getName();
        virtual void getCenterPose(Pose *outPose);
        virtual float getExtentX();
        virtual float getExtentZ();
        virtual int32_t getIndex();
    private:
        ArTrackable *_trackable;
        ArAugmentedImage *_image;
        ArSession *_session;
    };

    class LightEstimateNative : public LightEstimate {
    public:
        LightEstimateNative(ArLightEstimate *lightEstimate, ArSession *session) : _lightEstimate(lightEstimate), _session(session) {}
        virtual ~LightEstimateNative();
        virtual float getPixelIntensity();
        virtual void getColorCorrection(float *outColorCorrection);
        virtual bool isValid();

        ArLightEstimate *_lightEstimate;
        ArSession *_session;
    };

    class ImageNative : public Image {
    public:
        ImageNative(ArImage *image, ArSession *session);
        virtual ~ImageNative();
        virtual int32_t getWidth();
        virtual int32_t getHeight();
        virtual int32_t getFormat();
        virtual void getCropRect(int *outLeft, int *outRight, int *outBottom, int *outTop);
        virtual int32_t getNumberOfPlanes();
        virtual int32_t getPlanePixelStride(int planeIdx);
        virtual int32_t getPlaneRowStride(int planeIdx);
        virtual void getPlaneData(int planeIdx, const uint8_t **outData, int *outDataLength);
    private:
        ArImage *_arImage;
        ArSession *_session;
    };

    class FrameNative : public Frame {
    public:
        FrameNative(ArFrame *frame, ArSession *session) : _frame(frame), _session(session) {}
        virtual ~FrameNative();
        virtual void getViewMatrix(float *outMatrix);
        virtual void getProjectionMatrix(float near, float far, float *outMatrix);
        virtual void getImageIntrinsics(float *outFx, float *outFy, float *outCx, float *outCy);
        virtual TrackingState getTrackingState();
        virtual void getLightEstimate(LightEstimate *outLightEstimate);
        virtual bool hasDisplayGeometryChanged();
        virtual void hitTest(float x, float y, HitResultList *outList);
        virtual void hitTest(float px, float py, float pz, float qx, float qy, float qz, HitResultList *outList);
        virtual int64_t getTimestampNs();
        virtual void getUpdatedAnchors(AnchorList *outList);
        virtual void getUpdatedTrackables(TrackableList *outList, TrackableType type);
        virtual void getBackgroundTexcoords(float *outTexcoords);
        virtual PointCloud *acquirePointCloud();
        virtual ImageRetrievalStatus acquireCameraImage(Image **outImage);

        ArFrame *_frame;
        ArSession *_session;
    };

    class PointCloudNative : public PointCloud {
    public:
        PointCloudNative(ArPointCloud *pointCloud, ArSession *session) : _pointCloud(pointCloud), _session(session) {}
        virtual ~PointCloudNative();
        virtual const float *getPoints();
        virtual int getNumPoints();
        virtual const int *getPointIds();
    private:
        ArPointCloud *_pointCloud;
        ArSession *_session;
    };

    class HitResultListNative : public HitResultList {
    public:
        HitResultListNative(ArHitResultList *hitResultList, ArSession *session) : _hitResultList(hitResultList), _session(session) {}
        virtual ~HitResultListNative();
        virtual void getItem(int index, HitResult *outResult);
        virtual int size();

        ArHitResultList *_hitResultList;
        ArSession *_session;
    };

    class HitResultNative : public HitResult {
    public:
        HitResultNative(ArHitResult *hitResult, ArSession *session) : _hitResult(hitResult), _session(session) {}
        virtual ~HitResultNative();
        virtual float getDistance();
        virtual void getPose(Pose *outPose);
        virtual void getTransform(float *outTransform);
        virtual Trackable *acquireTrackable();
        virtual Anchor *acquireAnchor();

        ArHitResult *_hitResult;
        ArSession *_session;
    };

    class SessionNative : public Session {
    public:
        SessionNative(void *applicationContext, JNIEnv *env);
        virtual ~SessionNative();
        virtual ConfigStatus configure(Config *config);
        virtual void setDisplayGeometry(int rotation, int width, int height);
        virtual void setCameraTextureName(int32_t textureId);
        virtual void pause();
        virtual void resume();
        virtual void update(Frame *frame);

        virtual Config *createConfig(LightingMode lightingMode, PlaneFindingMode planeFindingMode,
                                     UpdateMode updateMode, CloudAnchorMode cloudAnchorMode,
                                     FocusMode focusMode);
        virtual AugmentedImageDatabase *createAugmentedImageDatabase();
        virtual AugmentedImageDatabase *createAugmentedImageDatabase(uint8_t* raw_buffer, int64_t size);
        virtual Pose *createPose();
        virtual Pose *createPose(float px, float py, float pz, float qx, float qy, float qz, float qw);
        virtual AnchorList *createAnchorList();
        virtual TrackableList *createTrackableList();
        virtual HitResultList *createHitResultList();
        virtual LightEstimate *createLightEstimate();
        virtual Frame *createFrame();
        virtual HitResult *createHitResult();
        virtual Anchor *acquireNewAnchor(const Pose *pose);
        virtual Anchor *hostAndAcquireNewCloudAnchor(const Anchor *anchor, AnchorAcquireStatus *status);
        virtual Anchor *resolveAndAcquireNewCloudAnchor(const char *anchorId, AnchorAcquireStatus *status);

    private:
        ArSession *_session;
    };
}

#endif /* ARCORE_Native_h */
