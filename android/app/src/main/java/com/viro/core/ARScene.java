/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

//#IFDEF 'viro_react'
import com.viro.core.internal.ARDeclarativeNode;
//#ENDIF
import android.net.Uri;
import android.util.Log;

import java.util.EnumSet;
import java.util.HashMap;
import java.util.Map;

/**
 * ARScene blends virtual 3D content with the device camera's view of the real world. Similar to a
 * normal {@link Scene}, an ARScene contains a hierarchy of {@link Node} objects representing the
 * virtual 3D world. Behind these objects the camera's live video feed is rendered. ARScene ensures
 * the two worlds -- real and virtual -- stay in sync with a unified coordinate system.
 * <p>
 * There are two ways to add virtual content to the ARScene. The first is to simply place content
 * manually. The origin of the coordinate system is the <i>initial</i> position of the user, and the
 * scale is meters. To add content you only need to set the position of the {@link Node} within this
 * coordinate system, and add it to the scene graph.
 * <p>
 * The second way to add virtual content is to use {@link ARAnchor}. ARAnchors represent features
 * detected in the real-world. You can associate (or "anchor") your content to these features by
 * adding your content to the {@link ARNode} that corresponds to each detected ARAnchor. To do this,
 * implement the ARScene {@link Listener}. The {@link Listener} callbacks are invoked whenever
 * {@link ARAnchor}s are found (or updated, or removed) in the real-world.
 * <p>
 * For a higher level guide on building AR in Viro, refer to the <a
 * href="https://virocore.viromedia.com/docs/augmented-reality-ar">Augmented Reality Guide</a>.
 */
public class ARScene extends Scene {
    /**
     * Callback interface for ARScene events. These include the detection of ambient light and the
     * tracking of real-world features.
     */
    public interface Listener {

        /**
         * Invoked when tracking is initialized and functional.
         *
         * @deprecated Use {@link #onTrackingUpdated(TrackingState, TrackingStateReason)}.
         */
        // TODO VIRO-3172: Remove deprecated onTrackingInitialized callback in future releases.
        @Deprecated
        void onTrackingInitialized();

        /**
         * Invoked when the tracking state of the device changes. {@link TrackingState}
         * indicates how well the device is able to track its position within the real world.
         * Tracking state is subject to lighting conditions, the speed at which the device is
         * moving, and other environmental factors.
         *
         * @param state  The {@link TrackingState} of the device.
         * @param reason Should the tracking state be sub-optimal, the {@link TrackingStateReason}
         *               indicates why.
         */
        void onTrackingUpdated(TrackingState state, TrackingStateReason reason);

        /**
         * Invoked when the AR system's estimation of the current ambient light levels are
         * updated. These values can be pumped into an {@link AmbientLight} to match the
         * lighting used by the virtual world to the lighting observed in the real world.
         *
         * @param intensity The light intensity detected, in lumens.
         * @param color     The color of the light detected in RGB [0, 1].
         */
        void onAmbientLightUpdate(float intensity, Vector color);

        /**
         * Invoked when a real-world {@link ARAnchor} is detected. You can associate virtual
         * content with this new anchor by adding said content to the associated {@link ARNode}.
         * Note that the {@link ARNode} is automatically added to the Scene, and will be
         * continually updated to stay in the sync with the anchor as the anchor's properties,
         * orientation, or position change.
         *
         * @param anchor The detected {@link ARAnchor} representing a real-world feature.
         * @param arNode The virtual world {@link ARNode} associated with the ARAnchor.
         */
        void onAnchorFound(ARAnchor anchor, ARNode arNode);

        /**
         * Invoked when a real-world {@link ARAnchor} is updated. This occurs when the AR tracking
         * system refines its estimation about the size or orientation or position of an ARAnchor,
         * or when other underlying properties of the anchor change.
         *
         * @param anchor The {@link ARAnchor} that was updated.
         * @param arNode The {@link ARNode} corresponding to the anchor.
         */
        void onAnchorUpdated(ARAnchor anchor, ARNode arNode);

        /**
         * Invoked when an {@link ARAnchor} is removed from the world. This occurs if the AR
         * tracking system loses confidence in the ARAnchor and can no longer place it, or when an
         * ARAnchor gets subsumed by another: for example, if the tracking system realizes that two
         * separate horizontal planes are actually one larger plane.
         *
         * @param anchor The {@link ARAnchor} that was removed.
         * @param arNode The corresponding {@link ARNode}, which is also removed from the Scene.
         */
        void onAnchorRemoved(ARAnchor anchor, ARNode arNode);
    }

    /**
     * Callback interface for responding to anchor hosting requests. See {@link
     * #hostCloudAnchor(ARAnchor, CloudAnchorHostListener)} for more details.
     */
    public interface CloudAnchorHostListener {

        /**
         * Invoked upon the successful hosting of a cloud anchor. When a local {@link ARAnchor} is
         * hosted, it replaced by a new "cloud" ARAnchor, which is provided in this callback. The
         * cloud anchor has a unique cloud anchor ID which you can retrieve via {@link
         * ARAnchor#getCloudAnchorId()}. Other clients can use this ID with {@link
         * #resolveCloudAnchor(String, CloudAnchorResolveListener)} to pull down this same anchor,
         * creating a shared AR experience.
         * <p>
         * You can also use this callback to add additional content to the given ARNode in response
         * to its successful hosting. The ARNode provided here is the same ARNode that was
         * associated with the local ARAnchor that was hosted: it contains all the content it
         * had previously, except now it belongs to the cloud anchor.
         * <p>
         *
         * @param cloudAnchor The new, successfully hosted, cloud anchor.
         * @param arNode      The ARNode that is attached and synchronized to the cloud anchor, to
         *                    which you can add virtual content. This is the same ARNode that was
         *                    associated with the local ARAnchor prior to its hosting.
         */
        public void onSuccess(ARAnchor cloudAnchor, ARNode arNode);

        /**
         * Invoked when hosting fails. Hosting can fail for a number of reasons: no network access,
         * limited AR tracking, or misconfiguration. The error message is provided in the callback.
         *
         * @param error The error message.
         */
        public void onFailure(String error);
    }

    /**
     * Callback interface for responding to anchor resolution requests. See {@link
     * #resolveCloudAnchor(String, CloudAnchorResolveListener)} for more details.
     */
    public interface CloudAnchorResolveListener {

        /**
         * Invoked upon the successful resolution of a cloud anchor. The provided ARAnchor has
         * been found and synchronized with the cloud, and the given ARNode has been attached
         * to it. You can use the ARNode to start adding content that will remain synchronized
         * with the cloud anchor.
         *
         * @param anchor The new, successfully resolved, cloud anchor.
         * @param arNode The ARNode attached and synchronized to the cloud anchor, to which you
         *               can add virtual content.
         */
        public void onSuccess(ARAnchor anchor, ARNode arNode);

        /**
         * Invoked when the system fails to resolve an anchor. Anchor resolution can fail for a
         * number of reasons: invalid anchor ID, no network access, limited AR tracking, or
         * misconfiguration. The error message is provided in the callback.
         *
         * @param error The error message.
         */
        public void onFailure(String error);
    }

    /**
     * Callback interface for responding to whether or not {@link
     * ARScene#loadARImageDatabase(Uri, LoadARImageDatabaseListener)} was able to access
     * the given {@link Uri}
     */
    public interface LoadARImageDatabaseListener {
        /**
         * Invoked if the given {@link Uri} was successfully downloaded.
         */
        void onSuccess();

        /**
         * Invoked if the given {@link Uri} was invalid.
         *
         * @param error The error message
         */
        void onFailure(String error);
    }

    /**
     * Values representing position tracking quality. You can respond to changes in AR tracking
     * quality through {@link Listener#onTrackingUpdated(TrackingState, TrackingStateReason)}.
     */
    public enum TrackingState {
        /**
         * Tracking is unavailable: the camera's position in the world is not known.
         */
        UNAVAILABLE(1),

        /**
         * Tracking is available, but the camera's position in the world may be inaccurate and
         * should not be used with confidence.
         */
        LIMITED(2),

        /**
         * Camera position tracking is providing optimal results.
         */
        NORMAL(3);

        private int mTypeId;
        TrackingState(int flag){
            mTypeId = flag;
        }
        public int getId(){
            return mTypeId;
        }
    };

    /**
     * Diagnoses to explain cases where we have limited position tracking quality in AR.
     */
    public enum TrackingStateReason {
        /**
         * Camera position tracking is working optimally.
         */
        NONE(1),

        /**
         * The device is moving too fast for accurate position tracking.
         */
        EXCESSIVE_MOTION(2),

        /**
         * The scene visible to the camera does not contain enough distinguishable features for
         * optimal position tracking.
         */
        INSUFFICIENT_FEATURES(3);

        private int mTypeId;
        TrackingStateReason(int flag){
            mTypeId = flag;
        }
        public int getId(){
            return mTypeId;
        }
    };

    private Listener mListener = null;
    private LoadARImageDatabaseListener mLoadARImageDatabaseListener;
    private long mNativeARDelegateRef;
    private boolean mHasTrackingInitialized = false;
    private Map<String, CloudAnchorHostListener> mCloudAnchorHostCallbacks = new HashMap<>();
    private Map<String, CloudAnchorResolveListener> mCloudAnchorResolveCallbacks = new HashMap<>();

    /**
     * Construct a new ARScene.
     */
    public ARScene() {
        mNativeARDelegateRef = nativeCreateARSceneDelegate(mNativeRef);
    }

    /**
     * Constructor for the declarative session.
     *
     * @hide
     * @param declarative
     */
    //#IFDEF 'viro_react'
    public ARScene(boolean declarative) {
        super(true); // Invoke the dummy constructor
        long nativeRef = nativeCreateARSceneControllerDeclarative();
        setSceneRef(nativeRef);
        mNativeARDelegateRef = nativeCreateARSceneDelegate(mNativeRef);
    }
    //#ENDIF

    /**
     * Invoked only by the default (no-arg) constructor.
     *
     * @hide
     */
    @Override
    protected long createNativeScene() {
        return nativeCreateARSceneController();
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this ARScene.
     */
    @Override
    public void dispose() {
        super.dispose();
        if (mNativeARDelegateRef != 0) {
            nativeDestroyARSceneDelegate(mNativeARDelegateRef);
            mNativeARDelegateRef = 0;
        }
    }

    /**
     * Set the {@link Listener} to use for responding to AR events. This enables your application
     * to receive {@link ARAnchor} and {@link ARNode} objects as they are detected by the AR
     * tracking system.
     *
     * @param listener The {@link Listener} to use for this ARScene.
     */
    public void setListener(Listener listener) {
        mListener = listener;
    }

    /**
     * Set to true to display a point cloud representing the rough feature points that the AR
     * tracking system is detecting. This is useful for debugging and analysis, but can also be
     * used for rendering effects.
     *
     * @param displayPointCloud True to display the point cloud, false to remove.
     */
    public void displayPointCloud(boolean displayPointCloud) {
        nativeDisplayPointCloud(mNativeRef, displayPointCloud);
    }

    /**
     * Create an {@link ARNode} that will be anchored to the given real-world position. Attaching
     * objects to an anchored node is recommended in AR, because the world coordinate system used by
     * the underlying tracking technology may not be stable. By using an {@link ARNode}, you ensure
     * that said node is continually tracked, maintaining its position in the real world. Normal
     * {@link Node} objects can be added as children to the ARNode, and they too will maintain their
     * position relative to the real-world.
     * <p>
     * <b>If AR tracking is limited, this method will return null.</b>
     * <p>
     * The returned {@link ARNode} is automatically added to the Scene, and will be continually
     * updated to stay in the sync with its underlying anchor as the anchor's properties,
     * orientation, or position change.
     * <p>
     * When finished with this ARNode, you must call {@link ARNode#detach()} to remove it from the
     * system. If you do not detach the ARNode, it will continue to receive tracking updates from
     * the AR subsystem, adversely impacting performance.
     * <p>
     *
     * @return New {@link ARNode} anchored to the given position. Returns null if AR tracking is
     * currently limited.
     */
    public ARNode createAnchoredNode(Vector position) {
        return createAnchoredNode(position, Quaternion.makeIdentity());
    }

    /**
     * Create an {@link ARNode} that will be anchored to the given real-world position and rotation.
     * Attaching objects to an anchored node is recommended in AR, because the world coordinate
     * system used by the underlying tracking technology may not be stable. By using an {@link
     * ARNode}, you ensure that said node is continually tracked, maintaining its position in the
     * real world. Normal {@link Node} objects can be added as children to the ARNode, and they too
     * will maintain their position relative to the real-world.
     * <p>
     * <b>If AR tracking is limited, this method will return null.</b>
     * <p>
     * Note that the returned {@link ARNode} is automatically added to the Scene, and will be
     * continually updated to stay in the sync with its underlying anchor as the anchor's
     * properties, orientation, or position change.
     * <p>
     * When finished with this ARNode, you must call {@link ARNode#detach()} to remove it from the
     * system. If you do not detach the ARNode, it will continue to receive tracking updates from
     * the AR subsystem, adversely impacting performance.
     * <p>
     *
     * @return New {@link ARNode} anchored to the given position, with the given rotation. Returns
     * null if AR tracking is currently limited.
     */
    public ARNode createAnchoredNode(Vector position, Quaternion quaternion) {
        long nodeRef = nativeCreateAnchoredNode(mNativeRef, position.x, position.y, position.z,
                quaternion.x, quaternion.y, quaternion.z, quaternion.w);
        if (nodeRef != 0) {
            return new ARNode(nodeRef);
        } else {
            Log.e("Viro", "Failed to create an anchored node at position " + position);
            return null;
        }
    }

    /**
     * Reset the point cloud surface to the default values. The point cloud surface is the
     * {@link Surface} that will be used to render each point in the point cloud.
     *
     * @deprecated use {@link ARScene#resetPointCloudQuad()} instead
     */
    public void resetPointCloudSurface() {
        nativeResetPointCloudSurface(mNativeRef);
    }

    /**
     * Set the {@link Surface} that should be used to render each point in the rendered
     * AR point cloud. This allows you to customize the appearance of the cloud with a
     * textured (or otherwise colored) Surface.
     *
     * @param surface The {@link Surface} to use for representing each point in the cloud.
     * @deprecated use {@link ARScene#setPointCloudQuad(Quad)} instead
     */
    public void setPointCloudSurface(Surface surface) {
        nativeSetPointCloudSurface(mNativeRef, surface.mNativeRef);
    }

    /**
     * Set the scale factor to use when rendering each point in the AR point cloud.
     *
     * @param scale The scale as a {@link Vector} in X, Y, and Z dimensions.
     * @deprecated use {@link ARScene#setPointCloudQuadScale(Vector)} instead
     */
    public void setPointCloudSurfaceScale(Vector scale) {
        nativeSetPointCloudSurfaceScale(mNativeRef, scale.x, scale.y, scale.z);
    }

    /**
     * Reset the point cloud quad to the default values. The point cloud quad is the
     * {@link Quad} that will be used to render each point in the point cloud.
     */
    public void resetPointCloudQuad() {
        nativeResetPointCloudSurface(mNativeRef);
    }

    /**
     * Set the {@link Quad} that should be used to render each point in the rendered
     * AR point cloud. This allows you to customize the appearance of the cloud with a
     * textured (or otherwise colored) Surface.
     *
     * @param quad The {@link Quad} to use for representing each point in the cloud.
     */
    public void setPointCloudQuad(Quad quad) {
        nativeSetPointCloudSurface(mNativeRef, quad.mNativeRef);
    }

    /**
     * Set the scale factor to use when rendering each point in the AR point cloud.
     *
     * @param scale The scale as a {@link Vector} in X, Y, and Z dimensions.
     */
    public void setPointCloudQuadScale(Vector scale) {
        nativeSetPointCloudSurfaceScale(mNativeRef, scale.x, scale.y, scale.z);
    }

    /**
     * Set the {@link PointCloudUpdateListener} to use for receiving point cloud updates. The
     * provided listener will receive an updated {@link ARPointCloud} as the point cloud updates.
     *
     * @param listener The {@link PointCloudUpdateListener} to use to receive point cloud updates.
     */
    public void setPointCloudUpdateListener(PointCloudUpdateListener listener) {
        if (listener != null) {
            getRootNode().setPointCloudUpdateListener(listener);
        }
    }

    /**
     * Set the maximum number of points to render when drawing the AR point cloud.
     *
     * @param maxPoints The maximum number of points to render.
     */
    public void setPointCloudMaxPoints(int maxPoints) {
        nativeSetPointCloudMaxPoints(mNativeRef, maxPoints);
    }

    /**
     * Get the estimated intensity of the lighting in the real-world, in lumens. This value can
     * be applied to an {@link AmbientLight} to make the ambient lighting of your virtual
     * objects closely resemble the ambient lighting detected in the real-world.
     *
     * @return The estimated intensity of the ambient light, in lumens.
     */
    public float getEstimatedAmbientLightIntensity() {
        return nativeGetAmbientLightIntensity(mNativeRef);
    }

    /**
     * Get the estimated color of the lighting in the real-world, as an RGB vector with values
     * ranging from 0 to 1. The returned color can be applied to an {@link AmbientLight} to make
     * the ambient lighting of your virtual objects closely resemble the ambient lighting detected
     * in the real-world.
     *
     * @return The estimated color of the ambient light, in an RGB [0,1] {@link Vector}.
     */
    public Vector getEstimatedAmbientLightColor() {
        float[] color = nativeGetAmbientLightColor(mNativeRef);
        return new Vector(color[0], color[1], color[2]);
    }

    /**
     * @hide
     */
    public void setAnchorDetectionTypes(EnumSet<ViroViewARCore.AnchorDetectionType> enumTypes) {
        String[] types = new String[enumTypes.size()];
        int i = 0;
        for (ViroViewARCore.AnchorDetectionType type : enumTypes) {
            types[i] = type.getStringValue();
            i++;
        }
        nativeSetAnchorDetectionTypes(mNativeRef, types);
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void addARDeclarativeNode(ARDeclarativeNode node) {
        nativeAddARNode(mNativeRef, node.mNativeRef);
    }
    //#ENDIF

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void updateARDeclarativeNode(ARDeclarativeNode node) {
        nativeUpdateARNode(mNativeRef, node.mNativeRef);
    }
    //#ENDIF

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void removeARDeclarativeNode(ARDeclarativeNode node) {
        nativeRemoveARNode(mNativeRef, node.mNativeRef);
    }
    //#ENDIF

    /**
     * Loads the given {@link com.google.ar.core.AugmentedImageDatabase} uri into
     * the AR subsystem. Once loaded, all the images within will be looked for.
     *
     * @param uri - the uri to the {@link com.google.ar.core.AugmentedImageDatabase} file
     * @param listener - the listener
     */
    public void loadARImageDatabase(Uri uri, LoadARImageDatabaseListener listener) {
        mLoadARImageDatabaseListener = listener;
        nativeLoadARImageDatabase(mNativeRef, uri.toString(), true);
    }

    /**
     * Unloads a previously loaded {@link com.google.ar.core.AugmentedImageDatabase}.
     */
    public void unloadARImageDatabase() {
        nativeUnloadARImageDatabase(mNativeRef, true);
    }

    /**
     * Add an {@link ARImageTarget} to the Scene. Once added, Viro will search for this reference
     * image in the real-world, and alert you when an instance is found by invoking
     * {@link Listener#onAnchorFound(ARAnchor, ARNode)} with an {@link ARImageAnchor} anchor.
     *
     * @param target The reference {@link ARImageTarget} you want to find in the real-world.
     * @see ARImageTarget
     */
    public void addARImageTarget(ARImageTarget target) {
        nativeAddARImageTarget(mNativeRef, target.getNativeRef());
    }

    /**
     * Remove an {@link ARImageTarget} from the Scene. Viro will stop searching for this image.
     *
     * @param target The target to stop searching for in the real-world.
     */
    public void removeARImageTarget(ARImageTarget target) {
        nativeRemoveARImageTarget(mNativeRef, target.getNativeRef());
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void loadARImageDatabaseDeclarative(Uri uri, LoadARImageDatabaseListener listener) {
        mLoadARImageDatabaseListener = listener;
        nativeLoadARImageDatabase(mNativeRef, uri.toString(), false);
    }
    //#ENDIF

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void unloadARImageDatabaseDeclarative() {
        nativeUnloadARImageDatabase(mNativeRef, false);
    }
    //#ENDIF

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void addARImageTargetDeclarative(ARImageTarget target) {
        nativeAddARImageTargetDeclarative(mNativeRef, target.getNativeRef());
    }
    //#ENDIF

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void removeARImageTargetDeclarative(ARImageTarget target) {
        nativeRemoveARImageTargetDeclarative(mNativeRef, target.getNativeRef());
    }
    //#ENDIF

    /**
     * Host the given {@link ARAnchor} to the cloud so it can be shared with other users. The given
     * callback will be invoked if the operation succeeds or fails.
     * <p>
     * The ARAnchor received in the callback upon a successful hosting will contain a cloud anchor
     * ID, accessible via {@link ARAnchor#getCloudAnchorId()}. Other clients can use this ID with
     * {@link #resolveCloudAnchor(String, CloudAnchorResolveListener)} to pull down the hosted
     * anchor, creating a shared AR experience.
     * <p>
     * If the operation fails, an error message will be sent to the callback.
     * <p>
     * Note that to use cloud anchors, you must have a Google AR API key in the application section
     * of your AndroidManifest. This is of the form:
     * <p>
     * <pre>
     * &#60;meta-data android:name="com.google.android.ar.API_KEY"
     *            android:value="Your-API-Key" /&#62;
     * </pre>
     * <p>
     *
     * @param anchor   The {@link ARAnchor} to host. You can retrieve an ARAnchor to host either
     *                 from the ARScene {@link Listener}, in response to detected planes, images, or
     *                 other real-world features, or from any {@link ARNode} through {@link
     *                 ARNode#getAnchor()}.
     * @param callback The callback to invoke on success or failure.
     */
    public void hostCloudAnchor(ARAnchor anchor, CloudAnchorHostListener callback) {
        if (mCloudAnchorHostCallbacks.containsKey(anchor.getAnchorId())) {
            Log.e("Viro", "Ignoring redundant cloud anchor hosting request: we are already processing anchor ["
                    + anchor.getAnchorId() + "]");
            return;
        }
        mCloudAnchorHostCallbacks.put(anchor.getAnchorId(), callback);
        nativeHostCloudAnchor(mNativeRef, anchor.getAnchorId());
    }

    // Called by native
    void onHostSuccess(String originalAnchorId, ARAnchor cloudAnchor, int arNodeId) {
        CloudAnchorHostListener callback = mCloudAnchorHostCallbacks.get(originalAnchorId);
        if (callback != null) {
            ARNode node = ARNode.getARNodeWithID(arNodeId);
            if (node == null) {
                node = new ARNode(arNodeId);
            }
            callback.onSuccess(cloudAnchor, node);
        } else {
            Log.e("Viro", "Cloud anchor host successful, but no callback found to invoke [anchor ID: "
                    + originalAnchorId + "]");
        }
    }

    // Called by native
    void onHostFailure(String originalAnchorId, String error) {
        CloudAnchorHostListener callback = mCloudAnchorHostCallbacks.get(originalAnchorId);
        if (callback != null) {
            callback.onFailure(error);
        } else {
            Log.e("Viro", "Cloud anchor host failed, and no callback found to invoke [anchor ID: "
                    + originalAnchorId + "]");
        }
    }

    /**
     * Resolve the {@link ARAnchor} with the given cloud identifier. If the given anchor is
     * successfully found in the cloud and synchronized with this client, it will be returned in the
     * provided callback. The ARAnchor received will be associated with a new {@link ARNode}, to
     * which you can add virtual content. That content will remain synchronized with the location
     * and pose of the cloud anchor.
     * <p>
     * If the operation fails, an error message will be sent to the callback.
     * <p>
     * Note that to use cloud anchors, you must have a Google AR API key in the application section
     * of your AndroidManifest. This is of the form:
     * <p>
     * <pre>
     * &#60;meta-data android:name="com.google.android.ar.API_KEY"
     *            android:value="Your-API-Key" /&#62;
     * </pre>
     * <p>
     *
     * @param cloudAnchorId The unique cloud identifier of the anchor to resolve. This ID can be
     *                      retrieved from {@link ARAnchor#getCloudAnchorId()}, but some form of
     *                      remote data transmission is typically required to move that ID from
     *                      client to client.
     * @param callback      The callback to invoke on success or failure.
     */
    public void resolveCloudAnchor(String cloudAnchorId, CloudAnchorResolveListener callback) {
        if (mCloudAnchorResolveCallbacks.containsKey(cloudAnchorId)) {
            Log.e("Viro", "Ignoring redundant cloud anchor resolve request: we are already processing anchor ["
                    + cloudAnchorId + "]");
            return;
        }
        mCloudAnchorResolveCallbacks.put(cloudAnchorId, callback);
        nativeResolveCloudAnchor(mNativeRef, cloudAnchorId);
    }

    // Called by native
    void onResolveSuccess(String cloudAnchorId, ARAnchor cloudAnchor, int arNodeId) {
        if (!cloudAnchorId.equals(cloudAnchor.getCloudAnchorId())) {
            throw new IllegalStateException("Resolved cloud anchor ID [" + cloudAnchor.getCloudAnchorId() +
                    "] does not match requested ID [" + cloudAnchorId + "]!");
        }

        CloudAnchorResolveListener callback = mCloudAnchorResolveCallbacks.get(cloudAnchorId);
        if (callback != null) {
            ARNode node = ARNode.getARNodeWithID(arNodeId);
            if (node == null) {
                node = new ARNode(arNodeId);
            }
            callback.onSuccess(cloudAnchor, node);
        } else {
            Log.e("Viro", "Cloud anchor resolve successful, but no callback found to invoke [anchor ID: "
                    + cloudAnchorId + "]");
        }
    }

    // Called by native
    void onResolveFailure(String originalAnchorId, String error) {
        CloudAnchorResolveListener callback = mCloudAnchorResolveCallbacks.get(originalAnchorId);
        if (callback != null) {
            callback.onFailure(error);
        } else {
            Log.e("Viro", "Cloud anchor resolve failed, and no callback found to invoke [anchor ID: "
                    + originalAnchorId + "]");
        }
    }

    private native long nativeCreateARSceneController();
    private native long nativeCreateARSceneControllerDeclarative();
    private native long nativeCreateARSceneDelegate(long sceneControllerRef);
    private native void nativeDestroyARSceneDelegate(long delegateRef);
    private native void nativeSetAnchorDetectionTypes(long sceneControllerRef, String[] types);
    private native void nativeAddARNode(long sceneControllerRef, long arPlaneRef);
    private native void nativeUpdateARNode(long sceneControllerRef, long arPlaneRef);
    private native void nativeRemoveARNode(long sceneControllerRef, long arPlaneRef);
    private native void nativeDisplayPointCloud(long sceneControllerRef, boolean displayPointCloud);
    private native void nativeResetPointCloudSurface(long sceneControllerRef);
    private native void nativeSetPointCloudSurface(long sceneControllerRef, long surfaceRef);
    private native void nativeSetPointCloudSurfaceScale(long sceneControllerRef, float scaleX, float scaleY, float scaleZ);
    private native void nativeSetPointCloudMaxPoints(long sceneControllerRef, int maxPoints);
    private native void nativeLoadARImageDatabase(long sceneControllerRef, String uri, boolean useImperative);
    private native void nativeUnloadARImageDatabase(long sceneControllerRef, boolean useImperative);
    private native void nativeAddARImageTarget(long sceneControllerRef, long arImageTargetRef);
    private native void nativeRemoveARImageTarget(long sceneControllerRef, long arImageTargetRef);
    private native void nativeAddARImageTargetDeclarative(long sceneControllerRef, long arImagetTargetRef);
    private native void nativeRemoveARImageTargetDeclarative(long sceneControllerRef, long arImageTargetRef);
    private native void nativeHostCloudAnchor(long sceneControllerRef, String anchorId);
    private native void nativeResolveCloudAnchor(long sceneControllerRef, String cloudAnchorId);
    private native float nativeGetAmbientLightIntensity(long sceneControllerRef);
    private native long nativeCreateAnchoredNode(long sceneControllerRef, float px, float py, float pz,
                                                 float qx, float qy, float qz, float qw);
    private native float[] nativeGetAmbientLightColor(long sceneControllerRef);

    // Called by JNI

    /**
     * @hide
     */
    void onTrackingUpdated(int stateInt, int reasonInt) {
        if (mListener == null) {
            return;
        }
        TrackingState state = getTrackingState(stateInt);
        if (state == TrackingState.NORMAL && !mHasTrackingInitialized){
            mHasTrackingInitialized = true;
            mListener.onTrackingInitialized();
        }

        mListener.onTrackingUpdated(getTrackingState(stateInt), getTrackingStateReason(reasonInt));
    }

    /**
     * @hide
     */
    private static TrackingState getTrackingState(int stateInt){
        switch(stateInt){
            case 1:
                return TrackingState.UNAVAILABLE;
            case 2:
                return TrackingState.LIMITED;
            case 3:
                return TrackingState.NORMAL;
        }

        return TrackingState.UNAVAILABLE;
    }

    /**
     * @hide
     */
    private static TrackingStateReason getTrackingStateReason(int stateInt){
        switch(stateInt){
            case 1:
                return TrackingStateReason.NONE;
            case 2:
                return TrackingStateReason.EXCESSIVE_MOTION;
            case 3:
                return TrackingStateReason.INSUFFICIENT_FEATURES;
        }

        return TrackingStateReason.NONE;
    }

    /**
     * @hide
     */
    void onAmbientLightUpdate(float intensity, float r, float g, float b) {
        Listener delegate;
        if (mListener != null) {
            mListener.onAmbientLightUpdate(intensity, new Vector(r, g, b));
        }
    }
    /**
     * @hide
     */
    void onAnchorFound(ARAnchor anchor, long nodeNativeRef) {
        // We still process the Node even if no listener is installed, so that the Node gets
        // added to ARNode.nodeARMap
        ARNode node = null;
        if (nodeNativeRef != 0) {
            node = new ARNode(nodeNativeRef);
        }
        if (mListener != null) {
            mListener.onAnchorFound(anchor, node);
        }
    }
    /**
     * @hide
     */
    void onAnchorUpdated(ARAnchor anchor, int nodeId) {
        if (mListener != null) {
            ARNode node = null;
            if (nodeId != 0) {
                node = ARNode.getARNodeWithID(nodeId);
            }
            mListener.onAnchorUpdated(anchor, node);
        }
    }
    /**
     * @hide
     */
    /* Called by Native */
    void onAnchorRemoved(ARAnchor anchor, int nodeId) {
        // We still process the Node even if no listener is installed, so that the Node gets
        // removed from ARNode.nodeARMap
        ARNode node = null;
        if (nodeId != 0) {
            node = ARNode.removeARNodeWithID(nodeId);
        }
        if (mListener != null) {
            mListener.onAnchorRemoved(anchor, node);
        }
    }

    /**
     * @hide
     */
    /* Called by Native */
    void onLoadARImageDatabaseSuccess() {
        if (mLoadARImageDatabaseListener != null) {
            mLoadARImageDatabaseListener.onSuccess();
        }
    }

    /**
     * @hide
     */
    /* Called by Native */
    void onLoadARImageDatabaseFailure(String message) {
        if (mLoadARImageDatabaseListener != null) {
            mLoadARImageDatabaseListener.onFailure(message);
        }
    }

    /**
     * Creates a builder for building complex {@link ARScene} objects.
     *
     * @return {@link ARSceneBuilder} object.
     */
    public static ARSceneBuilder<? extends Scene, ? extends SceneBuilder> builder() {
        return new ARSceneBuilder<>();
    }
// +---------------------------------------------------------------------------+
// | ARSceneBuilder class for ARScene
// +---------------------------------------------------------------------------+

    /**
     * ARSceneBuilder for building {@link ARScene}.
     */
    public static class ARSceneBuilder<R extends ARScene, B extends ARSceneBuilder<R, B>> extends SceneBuilder<R, B> {
        private R aRScene;

        /**
         * Constructor for SceneBuilder class.
         */
        public ARSceneBuilder() {
            this.aRScene = (R) new ARScene();
        }

        /**
         * Refer to {@link ARScene#setListener(Listener)}.
         *
         * @return This builder.
         */
        public ARSceneBuilder listener(Listener listener) {
            aRScene.setListener(listener);
            return (B) this;
        }

        /**
         * Refer to {@link ARScene#displayPointCloud(boolean)}.
         *
         * @return This builder.
         */
        public ARSceneBuilder displayPointCloud(boolean displayPointCloud) {
            aRScene.displayPointCloud(displayPointCloud);
            return (B) this;
        }

        /**
         * Refer to {@link ARScene#setPointCloudSurface(Surface)}.
         *
         * @return This builder.
         */
        public ARSceneBuilder pointCloudSurface(Surface pointCloudSurface) {
            aRScene.setPointCloudSurface(pointCloudSurface);
            return (B) this;
        }

        /**
         * Refer to {@link ARScene#setPointCloudSurfaceScale(Vector)}.
         *
         * @return This builder.
         */
        public ARSceneBuilder pointCloudSurfaceScale(Vector pointCloudSurfaceScale) {
            aRScene.setPointCloudSurfaceScale(pointCloudSurfaceScale);
            return (B) this;
        }

        /**
         * Refer to {@link ARScene#setPointCloudMaxPoints(int)}.
         *
         * @return This builder.
         */
        public ARSceneBuilder pointCloudMaxPoints(Integer pointCloudMaxPoints) {
            aRScene.setPointCloudMaxPoints(pointCloudMaxPoints);
            return (B) this;
        }
    }
}
