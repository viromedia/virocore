/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * OmniLight is a {@link Light} that emits light in all directions from a given position, with
 * decreasing intensity over distance. This is also commonly referred to as a point light.
 * <p>
 * OmniLight does not cast shadows.
 * <p>
 * For an extended discussion on Lights, refer to the <a href="https://virocore.viromedia.com/docs/3d-scene-lighting">Lighting
 * and Materials Guide</a>.
 */
public class OmniLight extends Light {

    private Vector mPosition = new Vector(0, 0, 0);
    private float mAttenuationStartDistance = 2.0f;
    private float mAttenuationEndDistance = 10f;

    /**
     * Construct a new OmniLight with default values: white color, normal intensity, and positioned
     * at the origin of its parent {@link Node}.
     */
    public OmniLight() {
        mNativeRef = nativeCreateOmniLight(mColor, mIntensity, mAttenuationStartDistance,
                mAttenuationEndDistance, mPosition.x, mPosition.y, mPosition.z);
    }

    /**
     * @hide
     * @param color
     * @param intensity
     * @param attenuationStartDistance
     * @param attenuationEndDistance
     * @param position
     */
    public OmniLight(long color, float intensity, float attenuationStartDistance,
                     float attenuationEndDistance, Vector position) {
        mColor = color;
        mIntensity = intensity;
        mAttenuationStartDistance = attenuationStartDistance;
        mAttenuationEndDistance = attenuationEndDistance;
        mPosition = position;
        mNativeRef = nativeCreateOmniLight(color, intensity, attenuationStartDistance,
                attenuationEndDistance, position.x, position.y, position.z);
    }

    /**
     * Set the position of this OmniLight within the coordinate system of its parent {@link Node}.
     *
     * @param position The position of as a {@link Vector}.
     */
    public void setPosition(Vector position) {
        mPosition = position;
        nativeSetPosition(mNativeRef, position.x, position.y, position.z);
    }

    /**
     * Get the position of this OmniLight.
     *
     * @return The positio as a {@link Vector}.
     */
    public Vector getPosition() {
        return mPosition;
    }

    /**
     * Set the attenuation start distance, which determines when the light begins to attenuate.
     * Objects positioned closer to the light than the attenuation start distance will receive the
     * light's full illumination.
     * <p>
     * Objects positioned between the start and end distance will receive a proportion of the lights
     * illumination, transitioning from full illumination to no illumination the further out from
     * the lights position the object is.
     * <p>
     * The default value is 2.
     *
     * @param attenuationStartDistance The distance from the light at which the light begins to
     *                                 attenuate.
     */
    public void setAttenuationStartDistance(float attenuationStartDistance) {
        mAttenuationStartDistance = attenuationStartDistance;
        nativeSetAttenuationStartDistance(mNativeRef, attenuationStartDistance);
    }

    /**
     * Get the attenuation start distance, which determines when the light starts attenuating.
     *
     * @return The attenuation start distance.
     */
    public float getAttenuationStartDistance() {
        return mAttenuationStartDistance;
    }

    /**
     * Set the attenuation end distance, which determines when the light drops to zero illumination.
     * Objects positioned at a distance greater than the attenuation end distance from the light's
     * position will receive no illumination from this light.
     * <p>
     * The default value is Float.MAX_VALUE.
     *
     * @param attenuationEndDistance The distance from the light at which no illumination will be
     *                               received.
     */
    public void setAttenuationEndDistance(float attenuationEndDistance) {
        mAttenuationEndDistance = attenuationEndDistance;
        nativeSetAttenuationEndDistance(mNativeRef, attenuationEndDistance);
    }

    /**
     * Get the attenuation end distance, which is the distance from the light at which no
     * illumination will be received.
     *
     * @return The attenuation end distance.
     */
    public float getAttenuationEndDistance() {
        return mAttenuationEndDistance;
    }

    private native long nativeCreateOmniLight(long color, float intensity,
                                              float attenuationStartDistance,
                                              float attenuationEndDistance,
                                              float positionX, float positionY, float positionZ);
    private native void nativeSetAttenuationStartDistance(long lightRef, float attenuationStartDistance);
    private native void nativeSetAttenuationEndDistance(long lightRef, float attenuationEndDistance);
    private native void nativeSetPosition(long lightRef, float positionX, float positionY, float positionZ);

    /**
     * Builder for creating {@link OmniLight} objects.
     */
    public static OmniLightBuilder<? extends Light, ? extends LightBuilder> builder() {
        return new OmniLightBuilder<>();
    }

    /**
     * Builder for creating {@link OmniLight} objects.
     */
    public static class OmniLightBuilder<R extends Light, B extends LightBuilder<R, B>> {
        private OmniLight light;

        /**
         * Constructor for OmniLightBuilder.
         */
        public OmniLightBuilder() {
            light = new OmniLight();
        }

        /**
         * Refer to {@link OmniLight#setPosition(Vector)}
         */
        public OmniLightBuilder position(Vector position) {
            light.setPosition(position);
            return this;
        }

        /**
         * Refer to {@link OmniLight#setAttenuationStartDistance(float)}.
         *
         * @return This builder.
         */
        public OmniLightBuilder attenuationStartDistance(float attenuationStartDistance) {
            light.setAttenuationStartDistance(attenuationStartDistance);
            return this;
        }

        /**
         * Refer to {@link OmniLight#setAttenuationEndDistance(float)}.
         *
         * @return This builder.
         */
        public OmniLightBuilder attenuationEndDistance(float attenuationEndDistance) {
            light.setAttenuationEndDistance(attenuationEndDistance);
            return this;
        }

    }
}
