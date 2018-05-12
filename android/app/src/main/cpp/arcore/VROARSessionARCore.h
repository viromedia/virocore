//
//  VROARSessionARCore.h
//  ViroKit
//
//  Created by Raj Advani on 9/27/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARSessioniOS_h
#define VROARSessioniOS_h

#include "VROARSession.h"
#include "VROARFrameARCore.h"
#include "VROViewport.h"
#include "VROOpenGL.h"
#include "ARCore_API.h"
#include <map>
#include <vector>
#include <VROCameraTexture.h>
#include <VROARPlaneAnchor.h>
#include <VROARTrackingSession.h>

enum class VROARDisplayRotation {
    R0,
    R90,
    R180,
    R270
};

class VRODriverOpenGL;

class VROARSessionARCore : public VROARSession, public VROARTrackingListener, public std::enable_shared_from_this<VROARSessionARCore> {
public:
    
    VROARSessionARCore(std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROARSessionARCore();

    void run();
    void pause();
    bool isReady() const;
    void resetSession(bool resetTracking, bool removeAnchors);

    /*
     Configure this ARCore session with the given modes. Returns true if supported.
     */
    bool configure(arcore::LightingMode lightingMode, arcore::PlaneFindingMode planeFindingMode,
                   arcore::UpdateMode updateMode, arcore::CloudAnchorMode cloudAnchorMode);
    
    void setScene(std::shared_ptr<VROScene> scene);
    void setDelegate(std::shared_ptr<VROARSessionDelegate> delegate);
    bool setAnchorDetection(std::set<VROAnchorDetection> types);
    void setCloudAnchorProvider(VROCloudAnchorProvider provider);
    void addARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void removeARImageTarget(std::shared_ptr<VROARImageTarget> target);
    void addAnchor(std::shared_ptr<VROARAnchor> anchor);
    void removeAnchor(std::shared_ptr<VROARAnchor> anchor);
    void updateAnchor(std::shared_ptr<VROARAnchor> anchor);

    std::unique_ptr<VROARFrame> &updateFrame();
    std::unique_ptr<VROARFrame> &getLastFrame();
    std::shared_ptr<VROTexture> getCameraBackgroundTexture();
    
    void setViewport(VROViewport viewport);
    void setOrientation(VROCameraOrientation orientation);
    void setWorldOrigin(VROMatrix4f relativeTransform);

    void setAutofocus(bool enabled) {
        // no-op on Android
    };

    void setVideoQuality(VROVideoQuality quality) {
        // no-op on Android
    };

    GLuint getCameraTextureId() const;


    // Internal methods

    /*
     Invoked when ARCore is installed on the device: sets the ARCore session implementation.
     This object will own the session.
     */
    void setARCoreSession(arcore::Session *session);

    /*
     Initialize the camera background texture and install it on the ARCore session.
     */
    void initCameraTexture(std::shared_ptr<VRODriverOpenGL> driver);

    std::shared_ptr<VROFrameSynchronizer> _synchronizer;
    void setFrameSynchronizer(std::shared_ptr<VROFrameSynchronizer> synchronizer) {
        _synchronizer = synchronizer;
    }

    std::shared_ptr<VROARAnchor> getAnchorForNative(arcore::Anchor *anchor);

    arcore::Session *getSessionInternal() {
        return _session;
    }

    void setDisplayGeometry(VROARDisplayRotation rotation, int width, int height);
    VROARDisplayRotation getDisplayRotation() const {  return _displayRotation; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

    /*
     This enables/disables image tracking (for debug only!)
     */
    void enableTracking(bool shouldTrack);

    // VROARTrackingListener Implementation
    virtual void onTrackedAnchorFound(std::shared_ptr<VROARAnchor> anchor);
    virtual void onTrackedAnchorUpdated(std::shared_ptr<VROARAnchor> anchor);
    virtual void onTrackedAnchorRemoved(std::shared_ptr<VROARAnchor> anchor);

    /*
     Retrieve the shared rotated camera image data array. The data must be of the
     given size in bytes.
     */
    uint8_t *getRotatedCameraImageData(int size);

private:

    /*
     The ARCore session.
     */
    arcore::Session *_session;

    /*
     Reusable ARCore frame object.
     */
    arcore::Frame *_frame;

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
     Vector of all anchors that have been added to this session.
     */
    std::vector<std::shared_ptr<VROARAnchor>> _anchors;

    arcore::LightingMode _lightingMode;
    arcore::PlaneFindingMode _planeFindingMode;
    arcore::UpdateMode _updateMode;
    arcore::CloudAnchorMode _cloudAnchorMode;

    arcore::AugmentedImageDatabase *_currentARCoreImageDatabase;

    /*
     Map of ARCore anchors ("native" anchors) to their Viro representation.
     Required so we can update VROARAnchors when their ARCore counterparts are
     updated.
     */
    std::map<std::string, std::shared_ptr<VROARAnchor>> _nativeAnchorMap;

    /*
     Background to be assigned to the VROScene.
     */
    std::shared_ptr<VROTexture> _background;

    /*
     The tracking session that handles all the tracking for us
     */
    std::shared_ptr<VROARTrackingSession> _arTrackingSession;

    /*
     The GL_TEXTURE_EXTERNAL_OES texture used for the camera background.
     */
    GLuint _cameraTextureId;

    /*
     The display rotation used by ARCore.
     */
    VROARDisplayRotation _displayRotation;

    /*
     The width and height used by the viewport (and corresponding camera texture).
     */
    int _width;
    int _height;

    int _frameCount;
    bool _hasTrackingSessionInitialized;


    /*
     Stores the RGBA8 rotated camera image data, each frame. This is kept here instead of in
     VROARCameraARCore so that it can be re-used each frame. VROARCameraARCore never exposes this
     to external clients, it only exposes the 'cropped' image data, which matches what's visible
     by the AR viewport.
     */
    int _rotatedImageDataLength;
    uint8_t *_rotatedImageData;

    void initTrackingSession();
    bool updateARCoreConfig();
    void processUpdatedAnchors(VROARFrameARCore *frame);
    void updateAnchorFromARCore(std::shared_ptr<VROARAnchor> anchor, arcore::Anchor *anchorAR);
    void updatePlaneFromARCore(std::shared_ptr<VROARPlaneAnchor> plane, arcore::Plane *planeAR);
    void updateImageAnchorFromARCore(std::shared_ptr<VROARImageAnchor> imageAnchor, arcore::AugmentedImage *imageAR);

    /*
     This is a helper function that synchronously adds the target to the database. This function should not
     be called on the render thread though (as per ARCore guidance).
     */
    void addTargetToDatabase(std::shared_ptr<VROARImageTarget> target,
                             arcore::AugmentedImageDatabase *database);

    /*
     This function rotates the given grayscaleImage so that the image is "Up" based on the given
     orientation. This function sets the given pointers to their new values (keep in mind that
     the caller should free the rotated grayscaleImage when they're done with it).
     */
    void rotateImageForOrientation(uint8_t **grayscaleImage, int *width, int *height, size_t *stride, VROImageOrientation orientation);

    // This is needed for VROTrackingType::ARCore
    std::vector<std::shared_ptr<VROARImageTarget>> _imageTargets;
};

#endif /* VROARSessionARCore_h */
