/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
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
