/*
 * Copyright © 2016 Viro Media. All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.component.Video360
 * Java JNI Wrapper     : com.viro.renderer.jni.SphereJni.java
 * Cpp JNI wrapper      : Sphere_JNI.cpp
 * Cpp Object           : VROSphere.cpp
 */
package com.viro.renderer.jni;

/**
 * Defines a spherical shape, e.g. where every point on the surface is equidistant from the center.
 * The center of the Sphere is placed at the origin. The surface of the Sphere is approximated
 * by setHeightSegmentCount and setWidthSegmentCount. These define the number of divisions made
 * along the surface to approximate the sphere.
 */
public class Sphere extends Geometry {

    /**
     * Construct a new Sphere. The Sphere will use the default segment approximation and will
     * face outward.
     *
     * @param radius The radius of the Sphere.
     */
    public Sphere(float radius) {
        mNativeRef = nativeCreateSphere(radius);
    }

    /**
     * Construct a new, fully parameterized Sphere.
     *
     * @param radius             The radius of the Sphere.
     * @param widthSegmentCount  The number of segments around the circumference of the Sphere.
     * @param heightSegmentCount The number of segments from the south pole to the north pole of the
     *                           Sphere.
     * @param facesOutward       True to have the Sphere face outward.
     */
    public Sphere(float radius, int widthSegmentCount, int heightSegmentCount, boolean facesOutward) {
        mNativeRef = nativeCreateSphereParameterized(
                radius,
                widthSegmentCount,
                heightSegmentCount,
                facesOutward);
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
     * Release native resources associated with this Sphere.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroySphere(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Set the radius of the Sphere.
     *
     * @param radius The radius.
     */
    public void setRadius(float radius) {
        nativeSetRadius(radius);
    }

    /**
     * Set the number of segments to use to approximate the Sphere across its latitude.
     * <p>
     * This is the number of segments from the Sphere's south pole to its north pole. Higher segment
     * counts more closely resemble a true sphere, but come at a performance and memory cost.
     *
     * @param heightSegmentCount The number of width segments to use.
     */
    public void setHeightSegmentCount(int heightSegmentCount) {
        nativeSetHeightSegmentCount(heightSegmentCount);
    }

    /**
     * Set the number of segments to use to approximate the Sphere across its longitude.
     * <p>
     * This is the number of segments across the Sphere's circumference. Higher segment counts more
     * closely resemble a true sphere, but come at a performance and memory cost.
     *
     * @param widthSegmentCount The number of width segments to use.
     */
    public void setWidthSegmentCount(int widthSegmentCount) {
        nativeSetWidthSegmentCount(widthSegmentCount);
    }

    /**
     * Set whether the Sphere faces outward or inward.
     * <p>
     * Outward is typically used if the Sphere is to be used as a "ball" or similar object in the
     * scene. Inward is used if the Sphere is meant to be a globe enclosing the user (e.g. the user
     * is inside the Sphere).
     *
     * @param facesOutward True to make the Sphere face outward.
     */
    public void setFacesOutward(boolean facesOutward) {
        nativeSetFacesOutward(facesOutward);
    }

    /*
     Native Functions called into JNI
     */
    private native long nativeCreateSphere(float radius);
    private native long nativeCreateSphereParameterized(float radius, int widthSegmentCount, int heightSegementCount, boolean facesOutward);
    private native void nativeSetWidthSegmentCount(int widthSegmentCount);
    private native void nativeSetHeightSegmentCount(int heightSegmentCount);
    private native void nativeSetRadius(float radius);
    private native void nativeSetFacesOutward(boolean facesOutward);
    private native void nativeDestroySphere(long nodeReference);
    private native void nativeSetVideoTexture(long nodeReference, long textureRef);

    /**
     * @hide
     * @param texture
     */
    public void setVideoTexture(VideoTexture texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }

}
