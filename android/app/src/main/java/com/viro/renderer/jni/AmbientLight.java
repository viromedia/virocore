/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.graphics.Color;

/**
 * AmbientLight is a {@link Light} that emits ambient light that affects all objects equally, at
 * constant intensity in all directions. The {@link Node} containing the AmbientLight does not
 * impact the light in any way.
 * <p>
 * AmbientLight is used to simulate indirect, constant environmental light.
 * <p>
 * AmbientLight does not cast shadows.
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

}
