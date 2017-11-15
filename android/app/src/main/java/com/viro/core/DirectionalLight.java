/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * DirectionalLight is a {@link Light} that illuminates all objects in the {@link Scene} from the
 * same direction with constant intensity. Because the intensity is constant, there is no
 * attenuation of a DirectionalLight and the position of the {@link Node} it is in has no impact.
 * <p>
 * DirectionalLight is meant to emulate powerful, far-away light sources like the sun.
 * <p>
 * DirectionalLight can cast shadows. These shadows are cast using an orthographic projection (e.g.
 * the size of the shadows does not change as the shadow casters' distance from the light changes).
 * This best simulates shadows cast from light sources like the sun.
 */
public class DirectionalLight extends Light {

    private Vector mDirection = new Vector(0, 0, -1);
    private boolean mCastsShadow = false;
    private float mShadowOrthographicSize = 20;
    private Vector mShadowOrthographicPosition = new Vector(0, 0, 0);
    private int mShadowMapSize = 1024;
    private float mShadowBias = 0.005f;
    private float mShadowNearZ = 0.1f;
    private float mShadowFarZ = 20;
    private float mShadowOpacity = 1.0f;

    /**
     * Construct a new DirectionalLight with default values: white color, pointing in the
     * negative Z direction, and normal intensity (1000).
     */
    public DirectionalLight() {
        mNativeRef = nativeCreateDirectionalLight(mColor, mIntensity, mDirection.x, mDirection.y, mDirection.z);
    }

    /**
     * Construct a new DirectionalLight with the given color, intensity, and direction.
     *
     * @param color The {@link android.graphics.Color} of the light.
     * @param intensity The intensity, where 1000 is normal intensity.
     * @param direction The direction of the light as a {@link Vector}.
     */
    public DirectionalLight(long color, float intensity, Vector direction) {
        mColor = color;
        mIntensity = intensity;
        mDirection = direction;
        mNativeRef = nativeCreateDirectionalLight(color, intensity, direction.x, direction.y, direction.z);
    }

    /**
     * Set the direction of the DirectionalLight.
     *
     * @param direction The direction as a {@link Vector}.
     */
    public void setDirection(Vector direction) {
        mDirection = direction;
        nativeSetDirection(mNativeRef, direction.x, direction.y, direction.z);
    }

    /**
     * Get the direction of this DirectionalLight.
     *
     * @return The direction as a {@link Vector}.
     */
    public Vector getDirection() {
        return mDirection;
    }

    /**
     * Set to true to make this DirectionalLight cast shadows. Shadow construction is
     * performance intensive; it is recommended to cast shadows from as few lights as
     * possible to preserve frame-rate.
     *
     * @param castsShadow True to cast shadows, false to not cast shadows from the light.
     */
    public void setCastsShadow(boolean castsShadow) {
        mCastsShadow = castsShadow;
        nativeSetCastsShadow(mNativeRef, castsShadow);
    }

    /**
     * Returns true if this DirectionalLight casts shadows.
     *
     * @return True if this light casts shadows.
     */
    public boolean getCastsShadow() {
        return mCastsShadow;
    }

    /**
     * The orthographic size determines the width and height of the area, centered at
     * shadowOrthographicPosition, for which shadows will be computed. More specifically, it
     * determines the size of the area that is rendered to the shadow map.
     * <p>
     * A larger value means more of the scene will be shadowed, but at lower resolution.
     * <p>
     * The shadow bounds constructed from this property, shadowFarZ, and shadowOrthographicSize
     * should be kept as tight as possible to maximize the resolution of shadows.
     * <p>
     * The default value is 20.
     *
     * @param orthographicSize The orthographic size for the shadow projection.
     */
    public void setShadowOrthographicSize(float orthographicSize) {
        mShadowOrthographicSize = orthographicSize;
        nativeSetShadowOrthographicSize(mNativeRef, orthographicSize);
    }

    /**
     * Return the size of the area to be shadowed by this light.
     *
     * @return The shadow orthographic size.
     */
    public float getShadowOrthographicSize() {
        return mShadowOrthographicSize;
    }

    /**
     * Set the center of the shadow map created by this directional light. Although directional
     * lights have no center, the shadow map must have a center.
     *
     * @param position The center of the shadow projection.
     */
    public void setShadowOrthographicPosition(Vector position) {
        mShadowOrthographicPosition = position;
        nativeSetShadowOrthographicPosition(mNativeRef, position.x, position.y, position.z);
    }

    /**
     * Return the center of the area to be shadowed by this light.
     *
     * @return The center of the shadow projection.
     */
    public Vector getShadowOrthographicPosition() {
        return mShadowOrthographicPosition;
    }

    /**
     * Set the size of the shadow map used to cast shadows for this light.
     * <p>
     * Shadows are created by rendering the silhouettes of scene geometry onto a 2D image from the
     * point of view of the light, then projecting that image onto the final view.
     * <p>
     * Larger shadow maps result in higher resolution shadows, but can have a higher memory and
     * performance cost. Smaller shadow maps are faster but result in pixelated edges.
     * <p>
     * The default value is 1024.
     *
     * @param shadowMapSize The size of the shadow map.
     */
    public void setShadowMapSize(int shadowMapSize) {
        mShadowMapSize = shadowMapSize;
        nativeSetShadowMapSize(mNativeRef, shadowMapSize);
    }

    /**
     * Return the size of the texture map to which the shadowed portion of the scene will be
     * rendered when computing shadows.
     *
     * @return The size of the shadow map.
     */
    public int getShadowMapSize() {
        return mShadowMapSize;
    }

    /**
     * Set the amount of bias to apply to the Z coordinate when performing the shadow depth
     * comparison. This reduces shadow acne, but large biases can cause "peter panning".
     * <p>
     * The default value is 0.005.
     *
     * @param shadowBias The shadow bias value.
     */
    public void setShadowBias(float shadowBias) {
        mShadowBias = shadowBias;
        nativeSetShadowBias(mNativeRef, shadowBias);
    }

    /**
     * Return the amount of bias that is applied to the Z coordinate when performing shadow
     * depth comparisons.
     *
     * @return The shadow bias value.
     */
    public float getShadowBias() {
        return mShadowBias;
    }

    /**
     * Set the near clipping plane to use when rendering shadows. Shadows are only cast by and on
     * surfaces further away than this plane.
     * <p>
     * This value defines the units away the near clipping plane is from the
     * shadowOrthographicPosition in the direction of the light.
     * <p>
     * The shadow bounds constructed from this property, shadowFarZ, and shadowOrthographicSize
     * should be kept as tight as possible to maximize the resolution of shadows.
     * <p>
     * The default value is 0.1.
     *
     * @param shadowNearZ
     */
    public void setShadowNearZ(float shadowNearZ) {
        mShadowNearZ = shadowNearZ;
        nativeSetShadowNearZ(mNativeRef, shadowNearZ);
    }

    /**
     * Get the near clipping plane used when rendering shadows. See {@link #setShadowNearZ(float)}
     * for discussion.
     *
     * @return The shadow near clipping plane.
     */
    public float getShadowNearZ() {
        return mShadowNearZ;
    }

    /**
     * Set the far clipping plane to use when rendering shadows. Shadows are only cast by and on
     * surfaces closer than this plane.
     * <p>
     * This value defines the units away the far clipping plane is from the
     * shadowOrthographicPosition in the direction of the light.
     * <p>
     * The shadow bounds constructed from this property, shadowNearZ, and shadowOrthographicSize
     * should be kept as tight as possible to maximize the resolution of shadows.
     * <p>
     * The default value is 20.
     *
     * @param shadowFarZ
     */
    public void setShadowFarZ(float shadowFarZ) {
        mShadowFarZ = shadowFarZ;
        nativeSetShadowFarZ(mNativeRef, shadowFarZ);
    }

    /**
     * Get the far clipping plane used when rendering shadows. See {@link #setShadowFarZ(float)}
     * for discussion.
     *
     * @return The shadow far clipping plane.
     */
    public float getShadowFarZ() {
        return mShadowFarZ;
    }

    /**
     * The opacity of the shadow. 1.0 creates a pitch black shadow. Lower opacities (approaching
     * 0.0), make the shadow fainter and fainter.
     *
     * @param opacity The shadow opacity.
     */
    public void setShadowOpacity(float opacity) {
        mShadowOpacity = opacity;
        nativeSetShadowOpacity(mNativeRef, opacity);
    }

    /**
     * Return the opacity of shadows cast from this Light.
     *
     * @return The shadow opacity.
     */
    public float getShadowOpacity() {
        return mShadowOpacity;
    }

    private native long nativeCreateDirectionalLight(long color, float intensity,
                                                     float directionX, float directionY, float directionZ);
    private native void nativeSetDirection(long lightRef, float directionX, float directionY, float directionZ);
    private native void nativeSetCastsShadow(long lightRef, boolean castsShadow);
    private native void nativeSetShadowOrthographicSize(long lightRef, float size);
    private native void nativeSetShadowOrthographicPosition(long lightRef, float posX, float posY, float posZ);
    private native void nativeSetShadowMapSize(long lightRef, int shadowMapSize);
    private native void nativeSetShadowBias(long lightRef, float shadowBias);
    private native void nativeSetShadowNearZ(long lightRef, float shadowNearZ);
    private native void nativeSetShadowFarZ(long lightRef, float shadowFarZ);
    private native void nativeSetShadowOpacity(long lightRef, float opacity);
}
