/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.graphics.Color;

/**
 * Light is a light source that illuminates Nodes in the {@link Scene}. Lights can be added to Nodes
 * so they are part of the overall scene graph. The area each Light illuminates depends on its type,
 * which is determined by the subclass of Light used. {@link Spotlight}, for example, illuminates a
 * cone-shaped area; AmbientLight, on the other hand, illuminates everything in the Scene with a
 * constant intensity.
 * <p>
 * The way in which a Light interacts with each surface of a {@link Geometry} to produce a color
 * depends on the materials used by the Geometry, along with the properties of the Light itself.
 * For more information see the Lighting and Materials guide.
 * <p>
 * Lights are performance intensive so it is recommended to use as few as possible, and only to
 * light dynamic objects in the Scene. For static objects, it is better to use pre-generated
 * lighting (e.g. baking the lighting into textures).
 */
public abstract class Light {
    protected long mNativeRef;
    protected long mColor = Color.WHITE;
    protected float mIntensity = 1000;
    private int mInfluenceBitMask = 1;

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this Light.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyLight(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Set the {@link Color} of this Light.
     *
     * @param color The color.
     */
    public void setColor(long color) {
        mColor = color;
        nativeSetColor(mNativeRef, color);
    }

    /**
     * Get the {@link Color} of this Light.
     *
     * @return The color.
     */
    public long getColor() {
        return mColor;
    }

    /**
     * Set the intensity of this Light. Set to 1000 for normal intensity. Lower intensities (below
     * 1000) will decrease the brightness of the light, and higher intensities (above 1000) will
     * increase the brightness of the light.
     *
     * @param intensity The intensity of the Light.
     */
    public void setIntensity(float intensity) {
        mIntensity = intensity;
        nativeSetIntensity(mNativeRef, intensity);
    }

    /**
     * Return the intensity of this Light, which measures its brightness.
     *
     * @return The intensity of the light.
     */
    public float getIntensity() {
        return mIntensity;
    }

    /**
     * This property is used to make Lights apply to specific Nodes. Lights and Nodes in the {@link
     * Scene} can be assigned bit-masks to determine how each {@link Light} influences each {@link
     * Node}.
     * <p>
     * During rendering, Viro compares each Light's influenceBitMask with each node's
     * lightReceivingBitMask and shadowCastingBitMask. The bit-masks are compared using a bitwise
     * AND operation:
     * <p>
     * If (influenceBitMask & lightReceivingBitMask) != 0, then the Light will illuminate the Node
     * (and the Node will receive shadows cast from objects occluding the Light).
     * <p>
     * If (influenceBitMask & shadowCastingBitMask) != 0, then the Node will cast shadows from the
     * Light.
     * <p>
     * The default mask is 0x1.
     *
     * @param bitMask The bit mask to set.
     */
    public void setInfluenceBitMask(int bitMask) {
        mInfluenceBitMask = bitMask;
        nativeSetInfluenceBitMask(mNativeRef, bitMask);
    }

    /**
     * Get the influence bit mask for this Light, which determines how it interacts with
     * individual {@link Node}s. See {@link #setInfluenceBitMask(int)} for more information.
     *
     * @return The influence bit mask.
     */
    public int getInfluenceBitMask() {
        return mInfluenceBitMask;
    }

    private native void nativeDestroyLight(long nativeRef);
    private native void nativeSetColor(long lightRef, long color);
    private native void nativeSetIntensity(long lightRef, float intensity);
    private native void nativeSetInfluenceBitMask(long lightRef, int bitMask);

}
