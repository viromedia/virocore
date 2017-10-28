/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.graphics.Color;

import java.util.HashMap;
import java.util.Map;

/**
 * Materials are the set of shading attributes that define the appearance of a geometry's surfaces
 * when rendered. Each {@link Geometry} in a scene can be assigned one or more materials. All UI
 * elements, and most basic 3D models, utilize only one material. Complex 3D objects, represented by
 * {@link Object3D}, typically have multiple materials, one for each defined mesh surface in the 3D
 * object.
 * <p>
 * Whereas a {@link Geometry} defines the <i>structure</i> of a 3D Object, Material defines the
 * appearance of each surface. The final color of each pixel on a surface is determined by both
 * the Material attributes and the parameters of each {@link Light} in the {@link Scene}.
 * <p>
 */
public class Material {

    /**
     * LightingModel defines a formula for combining a material’s diffuse, specular, and other
     * properties with the Lights in the Scene, and the point of view, to create the color
     * of each rendered pixel.
     */
    public enum LightingModel {
        /**
         * The diffuse color (or texture) entirely determines the color of the surface. Lights
         * are not taken into consideration.
         *
         * <tt>color = diffuseMaterial</tt>
         */
        CONSTANT("Constant"),

        /**
         * Lambert’s Law of diffuse reflectance determines the color of the surface. The diffuse
         * color (or texture) and the angle of incidence between each {@link Light} and the surface
         * determines the color of each pixel. The formula is as follows:
         * <p>
         * <tt>color = (∑ambientLight + ∑diffuseLight) * diffuseMaterial</tt>
         * <p>
         * ∑ambientLight: the sum of all ambient lights in the scene, as contributed by
         * ViroAmbientLight objects
         * <p>
         * ∑diffuseLight: the sum of each non-ambient light's diffuse contribution. Each light's
         * contribution is defined as follows
         * <p>
         * <tt>diffuseLight = max(0, dot(N, L))</tt>
         * <p>
         * where N is the surface normal vector at the point being shaded, and L is the normalized
         * vector from the point being shaded to the light source.
         */
        LAMBERT("Lambert"),

        /**
         * The Phong approximation of real-world reflectance. This formula adds a specular
         * contribution to the calculation, taking the point of view of the user into account.
         * <p>
         * <tt>color = (∑ambientLight + ∑diffuseLight) * diffuseMaterial + ∑specularLight *
         * specularTexture</tt>
         * <p>
         * ∑specularLight: the sum of each non-ambient light's specular contribution. Each light's
         * specular contribution is defined as follows
         * <p>
         * <tt>specularLight = pow(max(0, dot(R, E)), shininess)</tt>
         * <p>
         * where E is the normalized vector from the point being shaded to the viewer, R is the
         * reflection of the light vector L across the normal vector N, and shininess is the value
         * of the material's shininess property.
         */
        PHONG("Phong"),

        /**
         * The Blinn-Phong approximation of real-world reflectance. This is similar to
         * Phong, but uses a different formula for the specular contribution.
         * <p>
         * <tt>specularLight = pow(max(0, dot(H, N)), shininess)</tt>
         * <p>
         * where H is the vector halfway between the light vector L and the eye vector E, and
         * shininess is the value of the material's shininess property.
         */
        BLINN("Blinn");

        private String mStringValue;
        private LightingModel(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, LightingModel> map = new HashMap<String, LightingModel>();
        static {
            for (LightingModel value : LightingModel.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        public static LightingModel valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    /**
     * BlendMode determines how a pixel's color, as it is being rendered, interacts with the color
     * of the pixel <i>already</i> in the framebuffer. The former pixel is called the "source"
     * pixel, and the latter the "destination" pixel.
     */
    public enum BlendMode {

        /**
         * Disables blending: the incoming (source) pixel completely overwrites any existing
         * (destination) pixel.
         */
        NONE("None"),

        /**
         * Blend based on the incoming pixel's alpha value. The source pixel is multiplied by its
         * alpha value and the destination pixel is multiplied by 1.0 minus the source pixel's
         * alpha value.
         */
        ALPHA("Alpha"),

        /**
         * The source and destination pixel colors are added together. This is useful for creating
         * a 'glow' effect.
         */
        ADD("Add");

        private String mStringValue;
        private BlendMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, BlendMode> map = new HashMap<String, BlendMode>();
        static {
            for (BlendMode value : BlendMode.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        public static BlendMode valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };


    /**
     * CullMode determines whether we render front faces, back faces, or both. Each 3D surface in
     * Viro has a front face and a back face. The front-face is that which has a counter-clockwise
     * <i>winding order</i>.
     */
    public enum CullMode {

        /**
         * Cull (do not render) the back-facing faces. This is the default. For convex 3D models,
         * back-faces typically represent the "interior", and therefore invisible, surfaces of the
         * model.
         */
        BACK("Back"),

        /**
         * Cull (do not render) the front-facing faces.
         */
        FRONT("Front"),

        /**
         * Do not cull anything: render both front and back faces. This is useful if you want the
         * user to be able to see both sides of a concave model.
         */
        NONE("None");

        private String mStringValue;
        private CullMode(String value) {
            this.mStringValue = value;
        }

        /**
         * @return
         * @hide
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, CullMode> map = new HashMap<String, CullMode>();
        static {
            for (CullMode value : CullMode.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        public static CullMode valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    }

    /**
     * TransparencyMode determines how the opacity of pixels is computed.
     */
    public enum TransparencyMode {

        /**
         * Transparency is derived from the alpha channel of each color, where 0.0 is fully transparent
         * and 1.0 is opaque. This is the default.
         */
        A_ONE("AOne"),

        /**
         * Transparency is derived from the luminance of the colors.
         */
        RGB_ZERO("RGBZero");

        private String mStringValue;
        private TransparencyMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, TransparencyMode> map = new HashMap<String, TransparencyMode>();
        static {
            for (TransparencyMode value : TransparencyMode.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        public static TransparencyMode valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    long mNativeRef;
    private boolean mWritesToDepthBuffer = true;
    private boolean mReadsFromDepthBuffer = true;
    private LightingModel mLightingModel = LightingModel.CONSTANT;
    private Texture mDiffuseTexture;
    private int mDiffuseColor = Color.WHITE;
    private float mDiffuseIntensity = 1.0f;
    private Texture mSpecularTexture;
    private float mShininess = 2.0f;
    private float mFresnelExponent = 1.0f;
    private Texture mNormalMap;
    private CullMode mCullMode = CullMode.BACK;
    private TransparencyMode mTransparencyMode = TransparencyMode.A_ONE;
    private BlendMode mBlendMode = BlendMode.ALPHA;
    private float mBloomThreshold = -1.0f;

    /**
     * Construct a new Material. The material defaults to a flat white color with a constant
     * lighting model.
     */
    public Material() {
        mNativeRef = nativeCreateMaterial();
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
     * Release native resources associated with this Material.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyMaterial(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * @hide
     * @return
     */
    public long getNativeRef() {
        return mNativeRef;
    }

    /**
     * Set whether this Material writes to the depth buffer. Viro tracks the depth of each object in
     * the scene with a depth buffer. This way the renderer can determine which surfaces occlude
     * others from the point of view of the user.
     * <p>
     * Set this to true to write the depth buffer. By doing so, any surfaces that *read* from the
     * depth buffer will only appear if they are at a shallower depth than this material (e.g. if
     * they are closer to the user). Defaults to true.
     * <p>
     * Set to false for advanced usage only. Defaults to true.
     *
     * @param writesToDepthBuffer True to write to the depth buffer.
     */
    public void setWritesToDepthBuffer(boolean writesToDepthBuffer) {
        mWritesToDepthBuffer = writesToDepthBuffer;
        nativeSetWritesToDepthBuffer(mNativeRef, writesToDepthBuffer);
    }

    /**
     * Return true if this Material writes to the depth buffer. See {@link
     * #setWritesToDepthBuffer(boolean)} for discussion.
     *
     * @return True if writing is enabled.
     */
    public boolean getWritesToDepthBuffer() {
        return mWritesToDepthBuffer;
    }

    /**
     * Set whether this Material reads from the depth buffer. Viro tracks the depth of each object
     * in the scene with a depth buffer. This way the renderer can determine which surfaces occlude
     * others from the point of view of the user.
     * <p>
     * Set this to true to read from the depth buffer. By doing so, the surface first checks if any
     * other surface is closer to the user: if so, the surface will not appear (it is occluded).
     * <p>
     * Set to false for advanced usage only. Defaults to true.
     *
     * @param readsFromDepthBuffer True to read from the depth buffer.
     */
    public void setReadsFromDepthBuffer(boolean readsFromDepthBuffer) {
        mReadsFromDepthBuffer = readsFromDepthBuffer;
        nativeSetReadsFromDepthBuffer(mNativeRef, readsFromDepthBuffer);
    }

    /**
     * Return true if this Material reads from the depth buffer. See {@link
     * #setReadsFromDepthBuffer(boolean)} for discussion.
     *
     * @return True if reading is enabled.
     */
    public boolean getReadsFromDepthBuffer() {
        return mReadsFromDepthBuffer;
    }

    /**
     * Set the diffuse {@link Texture} to use for this Material. The diffuse texture, if
     * specified, defines the "base" color of the surfaces using this Material. More specifically,
     * the diffuse Texture defines for each pixel the light that is reflected from the surface,
     * independent of point of view.
     * <p>
     * The color from the diffuse Texture will be modulated (multiplied) by the diffuse color.
     *
     * @param texture The diffuse Texture to use for this Material.
     */
    public void setDiffuseTexture(Texture texture) {
        mDiffuseTexture = texture;
        nativeSetTexture(mNativeRef, texture.mNativeRef, "diffuseTexture");
    }

    /**
     * Get the diffuse {@link Texture} used by this Material, which defines its base color. See
     * {@link #setDiffuseTexture(Texture)} for more details.
     *
     * @return The diffuse {@link Texture} used by this Material.
     */
    public Texture getDiffuseTexture() {
        return mDiffuseTexture;
    }

    /**
     * Set the diffuse {@link Color} to use for this Material. The diffuse color, if specified,
     * defines the "base" color of the surfaces using this Material. More specifically,
     * the diffuse color defines for each pixel the light that is reflected from the surface,
     * independent of point of view.
     * <p>
     * If a diffuse {@link Texture} is also defined, it will be modulated (multiplied) by this
     * color.
     * <p>
     * Defaults to white.
     *
     * @param color The diffuse {@link Color} to use for this Material.
     */
    public void setDiffuseColor(int color) {
        mDiffuseColor = color;
        nativeSetColor(mNativeRef, color, "diffuseColor");
    }

    /**
     * Get the diffuse {@link Color} used by this Material, which defines its base color. See {@link
     * #setDiffuseColor(int)} for more details.
     *
     * @return The diffuse
     */
    public int getDiffuseColor() {
        return mDiffuseColor;
    }

    /**
     * @hide
     * @param diffuseIntensity
     */
    public void setDiffuseIntensity(float diffuseIntensity) {
        mDiffuseIntensity = diffuseIntensity;
        nativeSetDiffuseIntensity(mNativeRef, diffuseIntensity);
    }

    /**
     * @hide
     * @return
     */
    public float getDiffuseIntensity() {
        return mDiffuseIntensity;
    }

    /**
     * Set the specular {@link Texture} to use with this Material. The specular Texture defines
     * the amount of light that is reflected by the Material toward the user, resulting in a
     * bright, shiny highlight. Specular effects are point of view dependent, and only have an
     * effect when using the Blinn or Phong lighting models. See {@link LightingModel} for more
     * details on how the specular highlights are computed.
     *
     * @param texture The specular Texture to use for this Material.
     */
    public void setSpecularTexture(Texture texture) {
        mSpecularTexture = texture;
        nativeSetTexture(mNativeRef, texture.mNativeRef, "specularTexture");
    }

    /**
     * Get the specular {@link Texture} used by this Material. The specular Texture defines the
     * amount of light that is reflected by the Material toward the user. See {@link
     * #setSpecularTexture(Texture)} for more details.
     *
     * @return The specular Texture used by this Material.
     */
    public Texture getSpecularTexture() {
        return mSpecularTexture;
    }

    /**
     * Set the normal map {@link Texture} to use for this Material. Normal maps define the
     * orientation of the surface used by this Material for each <i>pixel</i>, thus enabling
     * the fine-grained responses to light that are necessary to simulate rough surfaces.
     * <p>
     * For each pixel of the normal map, the RGB colors are interpreted by the renderer as XYZ
     * components of a surface normal vector.
     *
     * @param normalMap
     */
    public void setNormalMap(Texture normalMap) {
        mNormalMap = normalMap;
        nativeSetTexture(mNativeRef, normalMap.mNativeRef, "normalTexture");
    }

    /**
     * Get the normap map {@link Texture} used by this Material. The normal map defines a normal
     * vector for each point on the surface.
     *
     * @return The normal map Texture used by this material.
     */
    public Texture getNormalMap() {
        return mNormalMap;
    }

    /**
     * Set the sharpness of specular highlights. This only applies if this Material has a specular
     * texture. See {@link LightingModel} for the specifics of how the shininess value is applied.
     * <p>
     * Defaults to 2.0.
     *
     * @param shininess The shininess value.
     */
    public void setShininess(float shininess) {
        mShininess = shininess;
        nativeSetShininess(mNativeRef, shininess);
    }

    /**
     * Get the shininess value, which defines the sharpness of specular highlights.
     *
     * @return The shininess value.
     */
    public float getShininess() {
        return mShininess;
    }

    /**
     * @hide
     * @param fresnelExponent
     */
    public void setFresnelExponent(float fresnelExponent) {
        mFresnelExponent = fresnelExponent;
        nativeSetFresnelExponent(mNativeRef, fresnelExponent);
    }

    /**
     * @hide
     * @return
     */
    public float getFresnelExponent() {
        return mFresnelExponent;
    }

    /**
     * Set the {@link LightingModel} to use for this Material. LightingModel defines a formula for
     * combining a material’s diffuse, specular, and other properties with the Lights in the Scene,
     * and the point of view, to create the color of each rendered pixel.
     * <p>
     * Defaults to {@link LightingModel#CONSTANT}.
     *
     * @param lightingModel The LightingModel to use for this Material.
     */
    public void setLightingModel(LightingModel lightingModel) {
        mLightingModel = lightingModel;
        nativeSetLightingModel(mNativeRef, lightingModel.getStringValue());
    }

    /**
     * Get the {@link LightingModel} used by this Material. See {@link LightingModel} for more details.
     *
     * @return The LightingModel used by this Material.
     */
    public LightingModel getLightingModel() {
        return mLightingModel;
    }

    /**
     * Set the {@link BlendMode} to use for this Material. BlendMode determines how a pixel's
     * color, as it is being rendered, interacts with the color of the pixel <i>already</i> in the
     * framebuffer.
     *
     * @param blendMode The BlendMode to use for this Material.
     */
    public void setBlendMode(BlendMode blendMode) {
        this.mBlendMode = blendMode;
        nativeSetBlendMode(mNativeRef, blendMode.getStringValue());
    }

    /**
     * Get the {@link BlendMode} used by this Material. See {@link BlendMode} for more details.
     *
     * @return The BlendMode used by this Material.
     */
    public BlendMode getBlendMode() {
        return mBlendMode;
    }

    /**
     * Set the {@link TransparencyMode} to use for this Material. TransparencyMode determines how
     * the opacity of pixels is computed.
     *
     * @param transparencyMode The TransparencyMode used by this material.
     */
    public void setTransparencyMode(TransparencyMode transparencyMode) {
        mTransparencyMode = transparencyMode;
        nativeSetTransparencyMode(mNativeRef, transparencyMode.getStringValue());
    }

    /**
     * Get the {@link TransparencyMode} to use for Material. See {@link TransparencyMode} for more
     * details.
     *
     * @return The TransparencyMode used by this Material.
     */
    public TransparencyMode getTransparencyMode() {
        return mTransparencyMode;
    }

    /**
     * Set the {@link CullMode} to use for this Material. CullMode determines whether we render
     * front faces, back faces, or both.
     *
     * @param cullMode The CullMode to use for this Material.
     */
    public void setCullMode(CullMode cullMode) {
        mCullMode = cullMode;
        nativeSetCullMode(mNativeRef, cullMode.getStringValue());
    }

    /**
     * Get the {@link CullMode} used by this Material. See {@link CullMode} for more details.
     *
     * @return The CullMode used by this Material.
     */
    public CullMode getCullMode() {
        return mCullMode;
    }

    /**
     * Set the brightness value at which this Material will begin to bloom.
     * <p>
     * Bloom is an effect that makes surfaces appear to glow by applying a Gaussian blur and
     * additive blend.
     * <p>
     * This value specifies at what 'brightness' the pixels of the surfaces using this material
     * should start to bloom. Brightness is effectively the magnitude of the final color of a pixel
     * (modified for the human eye: specifically it is the dot product of the final color with
     * (0.2126, 0.7152, 0.0722)).
     * <p>
     * For example, if this property is set to 0.0, then all surfaces using this material will
     * bloom. If this property is set to 1.0, then only those pixels of the surface whose brightness
     * exceeds 1.0 (after lights are applied) will bloom.
     * <p>
     * Default is -1.0, which disables bloom.
     *
     * @param bloomThreshold The bloom threshold value to set. Set to less than zero to disable
     *                       bloom entirely.
     */
    public void setBloomThreshold(float bloomThreshold) {
        mBloomThreshold = bloomThreshold;
        nativeSetBloomThreshold(mNativeRef, bloomThreshold);
    }

    /**
     * Get the bloom threshold, the brightness at which this Material will begin to bloom. See
     * {@link #setBloomThreshold(float)} for more details.
     *
     * @return The bloom threshold.
     */
    public float getBloomThreshold() {
        return mBloomThreshold;
    }

    private native long nativeCreateMaterial();
    private native void nativeSetWritesToDepthBuffer(long nativeRef, boolean writesToDepthBuffer);
    private native void nativeSetReadsFromDepthBuffer(long nativeRef, boolean readsFromDepthBuffer);
    private native void nativeSetTexture(long nativeRef, long textureRef, String materialPropertyName);
    private native void nativeSetColor(long nativeRef, long color, String materialPropertyName);
    private native void nativeSetShininess(long nativeRef, double shininess);
    private native void nativeSetFresnelExponent(long nativeRef, double fresnelExponent);
    private native void nativeSetLightingModel(long nativeRef, String lightingModelName);
    private native void nativeSetBlendMode(long nativeRef, String blendModeName);
    private native void nativeSetTransparencyMode(long nativeRef, String transparencyModeName);
    private native void nativeSetCullMode(long nativeRef, String cullModeName);
    private native void nativeSetDiffuseIntensity(long nativeRef, float diffuseIntensity);
    private native void nativeDestroyMaterial(long nativeRef);
    private native void nativeSetBloomThreshold(long nativeRef, float bloomThreshold);
}
