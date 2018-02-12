/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * AmbientLight is a {@link Light} that emits ambient light that affects all objects equally, at
 * constant intensity in all directions. The {@link Node} containing the AmbientLight does not
 * impact the light in any way.
 * <p>
 * AmbientLight is used to simulate indirect, constant environmental light.
 * <p>
 * AmbientLight does not cast shadows.
 * <p>
 * For an extended discussion on Lights, refer to the <a href="https://virocore.viromedia.com/docs/3d-scene-lighting">Lighting
 * and Materials Guide</a>.
 */
public class AmbientLight extends Light {

    /**
     * Construct a new AmbientLight with default values: white color, normal intensity.
     */
    public AmbientLight() {
        mNativeRef = nativeCreateAmbientLight(mColor, mIntensity);
    }

    /**
     * Construct a new AmbientLight with the given color and intensity.
     *
     * @param color The {@link android.graphics.Color} of the light.
     * @param intensity The intensity, where 1000 is normal intensity.
     */
    public AmbientLight(long color, float intensity) {
        mColor = color;
        mIntensity = intensity;
        mNativeRef = nativeCreateAmbientLight(color, intensity);
    }

    private native long nativeCreateAmbientLight(long color, float intensity);

    /**
     * Builder for creating {@link AmbientLight} objects.
     */
    public static AmbientLightBuilder<? extends Light, ? extends LightBuilder> builder() {
        return new AmbientLightBuilder<>();
    }

    /**
     * Builder for creating {@link AmbientLight} objects.
     */
    public static class AmbientLightBuilder<R extends Light, B extends LightBuilder<R, B>> {
        private AmbientLight light;

        /**
         * Constructor for AmbientLightBuilder objects.
         */
        public AmbientLightBuilder() {
            light = new AmbientLight();
        }

    }
}
