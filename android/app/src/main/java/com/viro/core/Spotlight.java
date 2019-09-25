//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core;

/**
 * Spotlight is a {@link Light} that illuminates objects within a cone-shaped area, in a given
 * direction, with decreasing intensity over distance.
 * <p>
 * Spotlight can cast shadows. These shadows are cast using an perspective projection (e.g.
 * the size of the shadows changes as the shadow casters' distance from the light changes).
 * <p>
 * For an extended discussion on Lights, refer to the <a href="https://virocore.viromedia.com/docs/3d-scene-lighting">Lighting
 * and Materials Guide</a>.
 */
public class Spotlight extends Light {

    private Vector mDirection = new Vector(0, 0, -1);
    private Vector mPosition = new Vector(0, 0, 0);
    private float mInnerAngle = 0;
    private float mOuterAngle = (float) Math.PI / 4;
    private float mAttenuationStartDistance = 2.0f;
    private float mAttenuationEndDistance = 10f;
    private boolean mCastsShadow = false;
    private int mShadowMapSize = 1024;
    private float mShadowBias = 0.005f;
    private float mShadowNearZ = 0.1f;
    private float mShadowFarZ = 20;
    private float mShadowOpacity = 1.0f;

    /**
     * Construct a new Spotlight with default values: white color, normal intensity, pointing
     * in the negative Z direction and positioned at the origin of its parent {@link Node}.
     */
    public Spotlight() {
        mNativeRef = nativeCreateSpotLight(mColor, mIntensity, mAttenuationStartDistance, mAttenuationEndDistance,
                mPosition.x, mPosition.y, mPosition.z, mDirection.x, mDirection.y, mDirection.z,
                mInnerAngle, mOuterAngle);
    }

    /**
     * @hide
     * @param color
     * @param intensity
     * @param attenuationStartDistance
     * @param attenuationEndDistance
     * @param position
     * @param direction
     * @param innerAngle
     * @param outerAngle
     */
    //#IFDEF 'viro_react'
    public Spotlight(long color, float intensity, float attenuationStartDistance,
                     float attenuationEndDistance, Vector position, Vector direction,
                     float innerAngle, float outerAngle) {
        mColor = color;
        mIntensity = intensity;
        mAttenuationStartDistance = attenuationStartDistance;
        mAttenuationEndDistance = attenuationEndDistance;
        mPosition = position;
        mDirection = direction;
        mInnerAngle = innerAngle;
        mOuterAngle = outerAngle;
        mNativeRef = nativeCreateSpotLight(color, intensity, attenuationStartDistance, attenuationEndDistance,
                position.x, position.y, position.z, direction.x, direction.y, direction.z,
                innerAngle, outerAngle);
    }
    //#ENDIF

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

    /**
     * Set the inner angle, which is the angle from edge to edge of the 'full strength' light cone.
     * The lighting is at maximum intensity within this cone, and begins to attenuate outside of
     * it.
     * <p>
     * The default is set to 0, which means only objects hitting the center of the spotlight will
     * receive the light's full illumination. The illumination declines from innerAngle until
     * reaching outerAngle.
     *
     * @param innerAngle The inner angle in radians.
     */
    public void setInnerAngle(float innerAngle) {
        mInnerAngle = innerAngle;
        nativeSetInnerAngle(mNativeRef, innerAngle);
    }

    /**
     * Get the inner angle, which is the angle from edge to edge of the 'full strength' light cone.
     *
     * @return The inner angle in radians.
     */
    public float getInnerAngle() {
        return mInnerAngle;
    }

    /**
     * Set the outer angle, which is the angle from edge to edge of the 'attenuated' light cone. The
     * lighting declines in strength between the inner angle and outer angle. Outside of the outer
     * angle the light attenuates to zero, resulting in no light.
     * <p>
     * The default is set to PI / 4 radians (45 degrees).
     *
     * @param outerAngle The outer angle in radians.
     */
    public void setOuterAngle(float outerAngle) {
        mOuterAngle = outerAngle;
        nativeSetOuterAngle(mNativeRef, outerAngle);
    }

    /**
     * Get the outer angle, which is the angle from edge to edge of the 'attenuated' light cone.
     *
     * @return The outer angle in radians.
     */
    public float getOuterAngle() {
        return mOuterAngle;
    }

    /**
     * Set the position of this Spotlight within the coordinate system of its parent {@link Node}.
     *
     * @param position The position of as a {@link Vector}.
     */
    public void setPosition(Vector position) {
        mPosition = position;
        nativeSetPosition(mNativeRef, position.x, position.y, position.z);
    }

    /**
     * Get the position of this Spotlight.
     *
     * @return The positio as a {@link Vector}.
     */
    public Vector getPosition() {
        return mPosition;
    }

    /**
     * Set the direction of the Spotlight.
     *
     * @param direction The direction as a {@link Vector}.
     */
    public void setDirection(Vector direction) {
        mDirection = direction;
        nativeSetDirection(mNativeRef, direction.x, direction.y, direction.z);
    }

    /**
     * Get the direction of this Spotlight.
     *
     * @return The direction as a {@link Vector}.
     */
    public Vector getDirection() {
        return mDirection;
    }

    /**
     * Set to true to make this Spotlight cast shadows. Shadow construction is
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
     * Returns true if this Spotlight casts shadows.
     *
     * @return True if this light casts shadows.
     */
    public boolean getCastsShadow() {
        return mCastsShadow;
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
     * This value defines the units away the near clipping plane is from the light's position, in
     * the direction of the light.
     * <p>
     * The shadow bounds constructed from this property, shadowNearZ, and the light's cone
     * (innerAngle and outerAngle) should be kept as tight as possible to maximize the resolution of
     * shadows.
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
     * This value defines the units away the far clipping plane is from the light's position, in the
     * direction of the light.
     * <p>
     * The shadow bounds constructed from this property, shadowNearZ, and the light's cone
     * (innerAngle and outerAngle) should be kept as tight as possible to maximize the resolution of
     * shadows.
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

    private native long nativeCreateSpotLight(long color, float intensity,
                                              float attenuationStartDistance,
                                              float attenuationEndDistance,
                                              float positionX, float positionY, float positionZ,
                                              float directionX, float directionY, float directionZ,
                                              float innerAngle, float outerAngle);
    private native void nativeSetAttenuationStartDistance(long lightRef, float attenuationStartDistance);
    private native void nativeSetAttenuationEndDistance(long lightRef, float attenuationEndDistance);
    private native void nativeSetPosition(long lightRef, float positionX, float positionY, float positionZ);
    private native void nativeSetDirection(long lightRef, float directionX, float directionY, float directionZ);
    private native void nativeSetInnerAngle(long lightRef, float innerAngleRadians);
    private native void nativeSetOuterAngle(long lightRef, float outerAngleRadians);
    private native void nativeSetShadowMapSize(long lightRef, int shadowMapSize);
    private native void nativeSetShadowBias(long lightRef, float shadowBias);
    private native void nativeSetShadowNearZ(long lightRef, float shadowNearZ);
    private native void nativeSetShadowFarZ(long lightRef, float shadowFarZ);
    private native void nativeSetCastsShadow(long lightRef, boolean castsShadow);
    private native void nativeSetShadowOpacity(long lightRef, float opacity);

    /**
     * Builder for building {@link Spotlight} objects.
     */
    public static SpotlightBuilder<? extends Light, ? extends LightBuilder> builder() {
        return new SpotlightBuilder<>();
    }

    /**
     * Builder for creating {@link Spotlight} objects.
     */
    public static class SpotlightBuilder<R extends Light, B extends LightBuilder<R, B>> {
        private Spotlight light;

        /**
         * Constructor for SpotLightBuilder.
         */
        public SpotlightBuilder() {
            light = new Spotlight();
        }

        /**
         * Refer to {@link Spotlight#setPosition(Vector)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder position(Vector position) {
            light.setPosition(position);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setAttenuationStartDistance(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder attenuationStartDistance(float attenuationStartDistance) {
            light.setAttenuationStartDistance(attenuationStartDistance);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setAttenuationEndDistance(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder attenuationEndDistance(float attenuationEndDistance) {
            light.setAttenuationEndDistance(attenuationEndDistance);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setInnerAngle(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder innerAngle(float innerAngle) {
            light.setInnerAngle(innerAngle);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setOuterAngle(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder outerAngle(float outerAngle) {
            light.setOuterAngle(outerAngle);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setDirection(Vector)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder direction(Vector direction) {
            light.setDirection(direction);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setCastsShadow(boolean)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder castsShadow(boolean castsShadow) {
            light.setCastsShadow(castsShadow);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setShadowMapSize(int)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder shadowMapSize(int shadowMapSize) {
            light.setShadowMapSize(shadowMapSize);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setShadowBias(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder shadowBias(float shadowBias) {
            light.setShadowBias(shadowBias);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setShadowNearZ(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder shadowNearZ(float shadowNearZ) {
            light.setShadowNearZ(shadowNearZ);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setShadowFarZ(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder shadowFarZ(float shadowFarZ) {
            light.setShadowFarZ(shadowFarZ);
            return this;
        }

        /**
         * Refer to {@link Spotlight#setShadowOpacity(float)}.
         *
         * @return This builder.
         */
        public SpotlightBuilder shadowOpacity(float opacity) {
            light.setShadowOpacity(opacity);
            return this;
        }
    }
}

