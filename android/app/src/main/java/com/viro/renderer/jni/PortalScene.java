/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.graphics.Bitmap;

import java.lang.ref.WeakReference;

/**
 * PortalScene represents the top-level of an entire subgraph of {@link Node}s that is displayed
 * through a 'window' known as a {@link Portal}. Each PortalScene can represent an entirely new
 * Scene full of child Nodes, geometries, 3D models, and more. Additionally, each PortalScene can
 * have its own background texture. If a PortalScene is set to <tt>passable</tt>, users are able
 * to walk <i>through</i> the {@link Portal} into the PortalScene.
 * <p>
 * PortalScenes are typically used in AR to create an effect where the user walks from the real
 * world into a virtual world.
 *
 * @see Portal
 */
public class PortalScene extends Node {

    /**
     * Callback interface for responding to user's entering or existing a PortalScene.
     */
    public interface Delegate {
        /**
         * Invoked when the user enters this PortalScene.
         *
         * @param portalScene The PortalScene entered.
         */
        void onPortalEnter(PortalScene portalScene);

        /**
         * Invoked when the user exits this PortalScene.
         *
         * @param portalScene The PortalScene exited.
         */
        void onPortalExit(PortalScene portalScene);
    }

    private long mNativeDelegateRef;
    private WeakReference<Delegate> mDelegate = null;
    private boolean mPassable = false;
    private Portal mPortal;

    /**
     * Construct a new PortalScene. To be displayed, a {@link Portal} entrance has to be set for
     * this PortalScene via {@link #setPortalEntrance(Portal)}. The user will then be able to see
     * into this PortalScene through the entrance.
     */
    public PortalScene() {
        super(false);
        setNativeRef(nativeCreatePortalScene());
        attachDelegate();
    }

    /**
     * Used by Scene constructor for the root PortalScene.
     * @hide
     */
    PortalScene(boolean dummyArg) {
        super(dummyArg);
    }

    /**
     * Used by Scene constructor for the root PortalScene.
     * @hide
     */
    void attachDelegate() {
        mNativeDelegateRef = nativeCreatePortalDelegate();
        nativeAttachDelegate(mNativeRef, mNativeDelegateRef);
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
     * Release native resources associated with this PortalScene.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyPortalScene(mNativeRef);
            mNativeRef = 0;
        }

        if (mNativeDelegateRef != 0) {
            nativeDestroyPortalDelegate(mNativeDelegateRef);
            mNativeDelegateRef = 0;
        }
    }

    /**
     * Set this PortalScene to be passable, meaning the user can walk into it. If true, the Portal
     * serves as a 'door' into the world encompassed by this PortalScene. If false, the
     * Portal serves as a 'window' into a world that can't be accessed.
     *
     * @param passable True to make the PortalScene passable.
     */
    public void setPassable(boolean passable) {
        mPassable = passable;
        nativeSetPassable(mNativeRef, passable);
    }

    /**
     * Return true if this PortalScene can be entered.
     *
     * @return True if the PortalScene is passable.
     */
    public boolean isPassable() {
        return mPassable;
    }

    /**
     * Set the {@link Portal} that will serve as an entrance into this PortalScene. The Portal
     * is essentially a {@link Node} containing {@link Geometry} or children Nodes that represent
     * the <i>entrance</i> to this PortalScene. The Portal's Geometry should be aligned with the
     * X axis in its local coordinate system, orthogonal to the Z axis, in order to display properly
     * as a gateway into the PortalScene. The transparent sections of the Portal will render the
     * PortalScene; the non-transparent sections will render normally; and the area outside the
     * Portal will render the current Scene that contains the user.
     *
     * @param portal The {@link Portal} to use as the entrance.
     */
    public void setPortalEntrance(Portal portal) {
        mPortal = portal;
        nativeSetPortalEntrance(mNativeRef, portal.mNativeRef);
    }

    /**
     * Get the {@link Portal} that serves as this PortalScene's entrance, if any.
     *
     * @return The entrance Portal for this PortalScene.
     */
    public Portal getPortalEntrance() {
        return mPortal;
    }

    /**
     * Set the {@link Delegate} to use for responding to the user entering or exiting this
     * {@link PortalScene}. Only used if this PortalScene is set to passable.
     *
     * @param delegate The delegate to use with this PortalScene.
     */
    public void setPortalSceneDelegate(Delegate delegate) {
        mDelegate = new WeakReference<Delegate>(delegate);
    }

    /**
     * Set the background of this PortalScene to display the texture. The provided {@link Texture} should
     * contain a spherical image or spherical video. The image or video will be rendered behind all other
     * content.
     *
     * @param texture The {@link Texture} containing the image, or {@link VideoTexture} containing
     *                     the video.
     */
    public void setBackgroundTexture(Texture texture) {
        nativeSetBackgroundTexture(mNativeRef, texture.mNativeRef);
    }

    /**
     * Rotate the background image or video by the given degrees about the X, Y, and Z axes.
     *
     * @param rotation {@link Vector} containing rotation about the X, Y, and Z axes.
     */
    public void setBackgroundRotation(Vector rotation) {
        nativeSetBackgroundRotation(mNativeRef, rotation.x, rotation.y, rotation.z);
    }

    /**
     * Set the background of this PortalScene to display a cube-map. The provided {@link Texture} should
     * wrap a cube-map image, constructed via {@link Texture#Texture(Bitmap, Bitmap, Bitmap, Bitmap,
     * Bitmap, Bitmap, Texture.TextureFormat)}. The cube-map will be rendered behind all other
     * content.
     *
     * @param cubeTexture The {@link Texture} containing the cube-map.
     */
    public void setBackgroundCubeTexture(Texture cubeTexture) {
        nativeSetBackgroundCubeImageTexture(mNativeRef, cubeTexture.mNativeRef);
    }

    /**
     * Set the background of this PortalScene to display the given color. The color will be rendered
     * behind all other content.
     *
     * @param color The {@link android.graphics.Color}'s int value.
     */
    public void setBackgroundCubeWithColor(long color) {
        nativeSetBackgroundCubeWithColor(mNativeRef, color);
    }

    private native long nativeCreatePortalScene();
    private native long nativeCreatePortalDelegate();
    private native long nativeDestroyPortalScene(long nativeRef);
    private native long nativeDestroyPortalDelegate(long delegateRef);
    private native void nativeSetBackgroundTexture(long nativeRef, long imageRef);
    private native void nativeSetBackgroundCubeImageTexture(long nativeRef, long textureRef);
    private native void nativeSetBackgroundCubeWithColor(long nativeRef, long color);
    private native void nativeSetBackgroundRotation(long nativeRef, float degreeX, float degreeY,
                                                    float degreeZ);
    private native long nativeAttachDelegate(long nativeRef, long delegateRef);
    private native void nativeSetPassable(long nativeRef, boolean passable);
    private native void nativeSetPortalEntrance(long nativeRef, long portalNativeRef);

    /*
     Invoked from JNI.
     */
    /**
     * @hide
     */
    void onPortalEnter() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != 0) {
            mDelegate.get().onPortalEnter(this);
        }
    }

    /**
     * @hide
     */
    void onPortalExit() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != 0) {
            mDelegate.get().onPortalExit(this);
        }
    }
}
