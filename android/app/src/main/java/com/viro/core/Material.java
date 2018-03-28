/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

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
 * appearance of each surface. The final color of each pixel on a surface is determined by both the
 * Material attributes and the parameters of each {@link Light} in the {@link Scene}.
 * <p>
 * For an extended discussion on Material properties, refer to the <a
 * href="https://virocore.viromedia.com/docs/3d-scene-lighting">Lighting and Materials Guide</a>.
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
        BLINN("Blinn"),

        /**
         * Physically-based shading, or PBR, is a set of algorithms that more accurately render
         * environments while simplifying the modeling process for artists. Use this LightingModel
         * to render with PBR. PBR should be combined with HDR rendering for best results: you can
         * enable HDR via {@link ViroView#setHDREnabled(boolean)}. In addition, PBR must itself be
         * enabled via {@link ViroView#setPBREnabled(boolean)}.
         * <p>
         * PBR relies on the following properties when rendering:
         * <p>
         * <ul><li> The diffuse or <i>albedo</i> texture defines the base color of the material.
         * This can be set by {@link #setDiffuseTexture(Texture)}</li> <li>The metalness value or
         * texture defines how "metallic" the surface is, which influences the degree to which light
         * refracts and reflects off the surface, the sharpness of the reflections, and more. This
         * can be set by {@link #setMetalnessMap(Texture)}, or, if a uniform value is preferred,
         * {@link #setMetalness(float)}.</li> <li>The roughness value or texture defines the
         * roughness of the microfacets on the surface. The rougher a surface is, the more light
         * will scatter along completely different directions, resulting in larger and more muted
         * specular reflections. Smoother surfaces, meanwhile, exhibit a sharper specular reflection
         * as light rays are more likely to reflect in a uniform direction. Roughness can be set by
         * {@link #setRoughnessMap(Texture)}, or, if a uniform value is preferred, {@link
         * #setRoughness(float)} </li> <li>The ambient occlusion value or texture approximates how
         * exposed the surface is to ambient lighting. This has no effect on direct lights (it does
         * not result in clear shadows) but it darkens enclosed and sheltered areas. These textures
         * are typically authored using modeling tools along with roughness and metalness. Ambient
         * occlusion can be set by {@link #setAmbientOcclusionMap(Texture)}.</li> <li>The lighting
         * environment is a {@link Texture} (typically an HDR equirectangular image) that acts as a
         * global light source, illuminating surfaces with diffuse and specular ambient light. We
         * treat each pixel in the lighting environment as a light emitter, thereby capturing the
         * environment's global lighting and general feel. This gives objects a sense of belonging
         * to their environment. For this reason it is common to use the scene's background texture
         * as the lighting environment, but this is not necessary. To set the lighting environment
         * use {@link Scene#setLightingEnvironment(Texture)}. </li> </ul>
         * <p>
         * PBR rendering also takes into account normal maps to add more surface detail. However,
         * PBR will <i>ignore</i> some properties used by the basic lighting models. Specifically,
         * the specular texture ({@link #setSpecularTexture(Texture)}) and the shininess property
         * ({@link #setShininess(float)}) are ignored when rendering physically-based surfaces.
         */
        PHYSICALLY_BASED("PBR");

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
        /**
         * @hide
         * @return
         */
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
        ADD("Add"),

        /**
         * The source color is subtracted from the destination color.
         */
        SUBTRACT("Subtract"),

        /**
         * The source color is multiplied by the destination color. This results in colors that are
         * at the same brightness or darker than either the source or destination color.
         */
        MULTIPLY("Multiply"),

        /**
         * The inverse of the source color is multiplied by the inverse of the destination color.
         * This results in colors that are the same brightness or lighter than either the source or
         * destination color.
         */
        SCREEN("Screen");

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
        /**
         * @hide
         * @return
         */
        public static BlendMode valueFromString(String str) {
            return str == null ? null : map.get(str.toLowerCase());
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
        /**
         * @hide
         * @return
         */
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
        /**
         * @hide
         * @return
         */
        public static TransparencyMode valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    /**
     * ShadowMode defines in what form a {@link Material} receives shadows. Shadows for this
     * surface can either be disabled, rendered normally through shading, or rendered set to a special AR mode for
     * use on transparent surfaces.
     */
    public enum ShadowMode {
        /**
         * Shadows are disabled for this material. Surfaces using this material will be illuminated
         * by lights as though there are no occluding objects.
         */
        DISABLED("Disabled"),

        /**
         * Shadows are enabled and behave normally. Surfaces using this material will be illuminated
         * by lights <i>except</i> over areas where the light is occluded.
         */
        NORMAL("Normal"),

        /**
         * Shadows are enabled, but behave by setting the <i>alpha</i> channel of the surface.
         * Surfaces using this material are set to <i>black</i> and <i>transparent</i>. Over areas
         * where the surface is occluded from the light, the alpha value will be be greater than
         * zero. Over areas where the surface is in full view of the light, the alpha value is
         * zero.
         * <p>
         * This setting is used primarily in AR, in cases where you wish to have virtual objects
         * cast shadows on real-world surfaces. You can achieve this by creating a transparent
         * surface and aligning it with the real-world surface you wish to receive shadows. Then set
         * the transparent surface's material to this shadow mode.
         */
        TRANSPARENT("Transparent");

        private String mStringValue;
        private ShadowMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, ShadowMode> map = new HashMap<String, ShadowMode>();
        static {
            for (ShadowMode value : ShadowMode.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static ShadowMode valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    long mNativeRef;
    private boolean mWritesToDepthBuffer = true;
    private boolean mReadsFromDepthBuffer = true;
    private LightingModel mLightingModel = LightingModel.CONSTANT;
    private ShadowMode mShadowMode = ShadowMode.NORMAL;
    private Texture mDiffuseTexture;
    private int mDiffuseColor = Color.WHITE;
    private float mDiffuseIntensity = 1.0f;
    private Texture mSpecularTexture;
    private float mShininess = 2.0f;
    private float mFresnelExponent = 1.0f;
    private Texture mNormalMap;
    private Texture mMetalnessMap;
    private Texture mRoughnessMap;
    private float mRoughness = 0.484529f;
    private float mMetalness = 0.0f;
    private Texture mAmbientOcclusionMap;
    private CullMode mCullMode = CullMode.BACK;
    private TransparencyMode mTransparencyMode = TransparencyMode.A_ONE;
    private BlendMode mBlendMode = BlendMode.ALPHA;
    private float mBloomThreshold = -1.0f;
    private String mName ="";

    /**
     * Construct a new Material. The material defaults to a flat white color with a constant
     * lighting model.
     */
    public Material() {
        mNativeRef = nativeCreateMaterial();
    }

    /**
     * Construct a new Material with a passed in ref.
     *
     * @hide
     */
    Material(long ref){
        mNativeRef = ref;
    }

    /**
     * The bridge creates immutable materials on the UI thread, which are then copied to individual
     * components. It therefore needs this constructor, because the setters on this constructor
     * all dispatch to the rendering thread -- we can't do that for the bridge because those setters
     * will end up running after the materials have already been copied and assigned to components.
     * Hence this constructor that does it all at once on the UI thread.
     * @hide
     */
    //#IFDEF 'viro_react'
    public Material(LightingModel lightingModel, int diffuseColor, Texture diffuseTexture, float diffuseIntensity, Texture specularTexture,
                    float shininess, float fresnelExponent, Texture normalMap, CullMode cullMode,
                    TransparencyMode transparencyMode, BlendMode blendMode, float bloomThreshold,
                    boolean writesToDepthBuffer, boolean readsFromDepthBuffer) {

        mWritesToDepthBuffer = writesToDepthBuffer;
        mReadsFromDepthBuffer = readsFromDepthBuffer;
        mLightingModel = lightingModel;
        mDiffuseTexture = diffuseTexture;
        mDiffuseColor = diffuseColor;
        mDiffuseIntensity = diffuseIntensity;
        mSpecularTexture = specularTexture;
        mShininess = shininess;
        mFresnelExponent = fresnelExponent;
        mNormalMap = normalMap;
        mCullMode = cullMode;
        mTransparencyMode = transparencyMode;
        mBlendMode = blendMode;
        mBloomThreshold = bloomThreshold;
        mNativeRef = nativeCreateImmutableMaterial(lightingModel.getStringValue(),
                diffuseColor,
                diffuseTexture != null ? diffuseTexture.mNativeRef : 0,
                diffuseIntensity,
                specularTexture != null ? specularTexture.mNativeRef : 0,
                shininess,
                fresnelExponent,
                normalMap != null ? normalMap.mNativeRef : 0,
                cullMode.getStringValue(),
                transparencyMode.getStringValue(),
                blendMode.getStringValue(),
                bloomThreshold, writesToDepthBuffer, readsFromDepthBuffer);
    }
    //#ENDIF
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
        long nativeRef = (texture != null) ? texture.mNativeRef : -1;
        nativeSetTexture(mNativeRef, nativeRef, "diffuseTexture");
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
    //#IFDEF 'viro_react'
    public void setDiffuseIntensity(float diffuseIntensity) {
        mDiffuseIntensity = diffuseIntensity;
        nativeSetDiffuseIntensity(mNativeRef, diffuseIntensity);
    }
    //#ENDIF
    /**
     * @hide
     * @return
     */
    //#IFDEF 'viro_react'
    public float getDiffuseIntensity() {
        return mDiffuseIntensity;
    }
    //#ENDIF
    /**
     * Set the specular {@link Texture} to use with this Material. The specular Texture defines
     * the amount of light that is reflected by the Material toward the user, resulting in a
     * bright, shiny highlight. Specular effects are point of view dependent, and only have an
     * effect when using the Blinn or Phong lighting models. See {@link LightingModel} for more
     * details on how the specular highlights are computed.
     * <p>
     * This property is ignored if the Material is using {@link LightingModel#PHYSICALLY_BASED}.
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
     * @param normalMap The normal map to use for this Material.
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
     * Set the metalness {@link Texture} to use for this Material. The metalness map defines how
     * "metallic" the surface is at each texel, which influences the degree to which light
     * reflects off the surface, the sharpness of the reflections, and more.
     *<p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting.
     *
     * @param metalnessMap The metalness map to use for this Material.
     */
    public void setMetalnessMap(Texture metalnessMap) {
        mMetalnessMap = metalnessMap;
        nativeSetTexture(mNativeRef, metalnessMap.mNativeRef, "metalnessTexture");
    }

    /**
     * Get the metalness {@link Texture} used by this Material. The metalness map defines how
     * "metallic" the surface is at each texel, which influences the degree to which light
     * reflects off the surface, the sharpness of the reflections, and more.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting.
     *
     * @return The metalness map used by this Material. Null if no metalness map is installed.
     */
    public Texture getMetalnessMap() {
        return mMetalnessMap;
    }

    /**
     * Set a uniform metalness value to use for this Material. Metalness defines how
     * "metallic" the surface is at each pixel, which influences the degree to which light
     * reflects off the surface, the sharpness of the reflections, and more.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting. If a
     * metalness map is set (by {@link #setMetalnessMap(Texture)}, then this value is ignored, and
     * the values derived from the metalness map are used instead.
     *
     * @param metalness The uniform metalness value to use for this Material.
     */
    public void setMetalness(float metalness) {
        mMetalness = metalness;
        nativeSetFloat(mNativeRef, metalness, "metalness");
    }

    /**
     * Set a uniform metalness value to use for this Material. Metalness defines how
     * "metallic" the surface is at each pixel, which influences the degree to which light
     * reflects off the surface, the sharpness of the reflections, and more.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting. If a
     * metalness map is set (by {@link #setMetalnessMap(Texture)}, then this value is ignored, and
     * the values derived from the metalness map are used instead.
     *
     * @return metalness The uniform metalness value used by this Material.
     */
    public float getMetalness() {
        return mMetalness;
    }

    /**
     * Set the roughness {@link Texture} to use for this Material. The roughness map defines the
     * roughness of the surface's microfacets at each texel. The rougher a surface is (roughness
     * approaching 1.0), the more light sill scatter along completely different directions,
     * resulting in larger and more muted specular reflections. Smoother surfaces (roughness
     * approaching 0.0), meanwhile, exhibit a sharper specular reflection as light rays are more
     * likely to reflect in a uniform direction.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting.
     *
     * @param roughnessMap The roughness map to use for this Material.
     */
    public void setRoughnessMap(Texture roughnessMap) {
        mRoughnessMap = roughnessMap;
        nativeSetTexture(mNativeRef, roughnessMap.mNativeRef, "roughnessTexture");
    }

    /**
     * Set the roughness {@link Texture} to use for this Material. The roughness map defines the
     * roughness of the surface's microfacets at each texel. The rougher a surface is (roughness
     * approaching 1.0), the more light sill scatter along completely different directions,
     * resulting in larger and more muted specular reflections. Smoother surfaces (roughness
     * approaching 0.0), meanwhile, exhibit a sharper specular reflection as light rays are more
     * likely to reflect in a uniform direction.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting.
     *
     * @return The roughness map used by this Material. Null if no roughness map is used.
     */
    public Texture getRoughnessMap() {
        return mRoughnessMap;
    }

    /**
     * Set a uniform roughness value to use for this Material. Roughness defines the roughness of
     * the surface's microfacets, at each pixel. The rougher a surface is (roughness approaching
     * 1.0), the more light sill scatter along completely different directions, resulting in larger
     * and more muted specular reflections. Smoother surfaces (roughness approaching 0.0),
     * meanwhile, exhibit a sharper specular reflection as light rays are more likely to reflect in
     * a uniform direction.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting. If a
     * roughness map is set (by {@link #setRoughnessMap(Texture)}, then this value is ignored, and
     * the values derived from the roughness map are used instead.
     *
     * @param roughness The uniform roughness value to use for this Material.
     */
    public void setRoughness(float roughness) {
        mRoughness = roughness;
        nativeSetFloat(mNativeRef, roughness, "roughness");
    }

    /**
     * Set a uniform roughness value to use for this Material. Roughness defines the roughness of
     * the surface's microfacets, at each pixel. The rougher a surface is (roughness approaching
     * 1.0), the more light sill scatter along completely different directions, resulting in larger
     * and more muted specular reflections. Smoother surfaces (roughness approaching 0.0),
     * meanwhile, exhibit a sharper specular reflection as light rays are more likely to reflect in
     * a uniform direction.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting. If a
     * roughness map is set (by {@link #setRoughnessMap(Texture)}, then this value is ignored, and
     * the values derived from the roughness map are used instead.
     *
     * @return The uniform roughness value used by this Material.
     */
    public float getRoughness() {
        return mRoughness;
    }

    /**
     * Set the ambient occlusion {@link Texture} to use for this Material. The ambient occlusion map
     * approximates how exposed the surface is to ambient lighting, at each texel. This has no
     * effect on direct lights (it does not result in clear shadows) but it darkens enclosed and
     * sheltered areas. These textures are typically authored using modeling tools along with
     * roughness and metalness.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting.
     *
     * @param ambientOcclusionMap The ambient occlusion map to use for this Material.
     */
    public void setAmbientOcclusionMap(Texture ambientOcclusionMap) {
        mAmbientOcclusionMap = ambientOcclusionMap;
        nativeSetTexture(mNativeRef, ambientOcclusionMap.mNativeRef, "ambientOcclusionTexture");
    }

    /**
     * Get the ambient occlusion {@link Texture} used by this Material. The ambient occlusion map
     * approximates how exposed the surface is to ambient lighting, at each texel.
     * <p>
     * This property is only used under {@link LightingModel#PHYSICALLY_BASED} lighting.
     *
     * @return ambientOcclusionMap The ambient occlusion map used by this Material. Null if no
     * ambient occlusion map is installed.
     */
    public Texture getAmbientOcclusionMap() {
        return mAmbientOcclusionMap;
    }

    /**
     * Set the sharpness of specular highlights. This only applies if this Material has a specular
     * texture. See {@link LightingModel} for the specifics of how the shininess value is applied.
     * <p>
     * Defaults to 2.0.
     * <p>
     * This property is ignored if the Material is using {@link LightingModel#PHYSICALLY_BASED}.
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
    //#IFDEF 'viro_react'
    public void setFresnelExponent(float fresnelExponent) {
        mFresnelExponent = fresnelExponent;
        nativeSetFresnelExponent(mNativeRef, fresnelExponent);
    }
    //#ENDIF
    /**
     * @hide
     * @return
     */
    //#IFDEF 'viro_react'
    public float getFresnelExponent() {
        return mFresnelExponent;
    }
    //#ENDIF
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

    /**
     * Set the {@link ShadowMode} for this Material, which defines how surfaces using this
     * Material render shadows. See {@link ShadowMode} for details. The default value is
     * {@link ShadowMode#NORMAL}.
     *
     * @param shadowMode The {@link ShadowMode} to use for this Material.
     */
    public void setShadowMode(ShadowMode shadowMode) {
        mShadowMode = shadowMode;
        nativeSetShadowMode(mNativeRef, shadowMode.getStringValue());
    }

    /**
     * Get the {@link ShadowMode} used by this Material, which defines how surfaces using
     * this material render shadows. See {@link ShadowMode} for details.
     *
     * @return The {@link ShadowMode} used by this Material.
     */
    public ShadowMode getShadowMode() {
        return mShadowMode;
    }

    /**
     * Sets a (non-unique) name to represent this Material object. Names may be automatically set
     * when loaded from OBJ or FBX files.
     */
    public void setName(String name){
        mName = name;
        nativeSetName(mNativeRef, mName);
    }

    /**
     * Get the (non-unique) name that can be optionally used to represent this Material. Material
     * names are either set by {@link #setName(String)}, or they are automatically set when
     * loading OBJ or FBX files. Null if no name is set.
     *
     * @return name The string value representing the name used by this Material.
     */
    public String getName(){
        return mName;
    }

    private native long nativeCreateMaterial();
    private native long nativeCreateImmutableMaterial(String lightingModel, long diffuseColor, long diffuseTexture, float diffuseIntensity, long specularTexture,
                                                      float shininess, float fresnelExponent, long normalMap, String cullMode,
                                                      String transparencyMode, String blendMode, float bloomThreshold,
                                                      boolean writesToDepthBuffer, boolean readsFromDepthBuffer);
    private native void nativeSetWritesToDepthBuffer(long nativeRef, boolean writesToDepthBuffer);
    private native void nativeSetReadsFromDepthBuffer(long nativeRef, boolean readsFromDepthBuffer);
    private native void nativeSetTexture(long nativeRef, long textureRef, String materialPropertyName);
    private native void nativeSetColor(long nativeRef, long color, String materialPropertyName);
    private native void nativeSetFloat(long nativeRef, float value, String materialPropertyName);
    private native void nativeSetShininess(long nativeRef, double shininess);
    private native void nativeSetFresnelExponent(long nativeRef, double fresnelExponent);
    private native void nativeSetLightingModel(long nativeRef, String lightingModelName);
    private native void nativeSetBlendMode(long nativeRef, String blendModeName);
    private native void nativeSetTransparencyMode(long nativeRef, String transparencyModeName);
    private native void nativeSetCullMode(long nativeRef, String cullModeName);
    private native void nativeSetDiffuseIntensity(long nativeRef, float diffuseIntensity);
    private native void nativeDestroyMaterial(long nativeRef);
    private native void nativeSetBloomThreshold(long nativeRef, float bloomThreshold);
    private native void nativeSetShadowMode(long nativeRef, String shadowMode);
    private native void nativeSetName(long nativeRef, String name);

    /**
     * Builder for creating {@link Material} objects.
     */
    public static MaterialBuilder builder() {
        return new MaterialBuilder();
    }

    /**
     * Builder for creating {@link Material} objects.
     */
    public static class MaterialBuilder {
        private Material material;

        /**
         * Constructor for {@link MaterialBuilder}.
         */
        public MaterialBuilder() {
            material = new Material();
        }

        /**
         * Refer to {@link Material#setWritesToDepthBuffer(boolean)}.
         *
         * @return This builder.
         */
        public MaterialBuilder writesToDepthBuffer(boolean writesToDepthBuffer) {
            material.setWritesToDepthBuffer(writesToDepthBuffer);
            return this;
        }

        /**
         * Refer to {@link Material#setReadsFromDepthBuffer(boolean)}.
         *
         * @return This builder.
         */
        public MaterialBuilder readsFromDepthBuffer(boolean readsFromDepthBuffer) {
            material.setReadsFromDepthBuffer(readsFromDepthBuffer);
            return this;
        }

        /**
         * Refer to {@link Material#setDiffuseTexture(Texture)}.
         *
         * @return This builder.
         */
        public MaterialBuilder diffuseTexture(Texture texture) {
            material.setDiffuseTexture(texture);
            return this;
        }

        /**
         * Refer to {@link Material#setDiffuseColor(int)}.
         *
         * @return This builder.
         */
        public MaterialBuilder diffuseColor(int color) {
            material.setDiffuseColor(color);
            return this;
        }

        /**
         * Refer to {@link Material#setDiffuseIntensity(float)}.
         *
         * @hide
         * @return This builder.
         */
        //#IFDEF 'viro_react'
        public MaterialBuilder diffuseIntensity(float diffuseIntensity) {
            material.setDiffuseIntensity(diffuseIntensity);
            return this;
        }
        //#ENDIF
        /**
         * Refer to {@link Material#setSpecularTexture(Texture)}.
         *
         * @return This builder.
         */
        public MaterialBuilder specularTexture(Texture texture) {
            material.setSpecularTexture(texture);
            return this;
        }

        /**
         * Refer to {@link Material#setNormalMap(Texture)}.
         *
         * @return This builder.
         */
        public MaterialBuilder normalMap(Texture normalMap) {
            material.setNormalMap(normalMap);
            return this;
        }

        /**
         * Refer to {@link #setMetalnessMap(Texture)}.
         *
         * @return This builder.
         */
        public MaterialBuilder metalnessMap(Texture metalnessMap) {
            material.setMetalnessMap(metalnessMap);
            return this;
        }

        /**
         * Refer to {@link #setMetalness(float)}.
         *
         * @return This builder.
         */
        public MaterialBuilder metalness(float metalness) {
            material.setMetalness(metalness);
            return this;
        }

        /**
         * Refer to {@link #setRoughnessMap(Texture)}.
         *
         * @return This builder.
         */
        public MaterialBuilder roughnessMap(Texture roughnessMap) {
            material.setRoughnessMap(roughnessMap);
            return this;
        }

        /**
         * Refer to {@link #setRoughness(float)}.
         *
         * @return This builder.
         */
        public MaterialBuilder roughness(float roughness) {
            material.setRoughness(roughness);
            return this;
        }

        /**
         * Refer to {@link Material#setAmbientOcclusionMap(Texture)}.
         *
         * @return This builder.
         */
        public MaterialBuilder ambientOcclusionMap(Texture ambientOcclusionMap) {
            material.setAmbientOcclusionMap(ambientOcclusionMap);
            return this;
        }

        /**
         * Refer to {@link Material#setShininess(float)}.
         *
         * @return This builder.
         */
        public MaterialBuilder shininess(float shininess) {
            material.setShininess(shininess);
            return this;
        }

        /**
         * Refer to {@link Material#setFresnelExponent(float)}.
         *
         * @hide
         * @return This builder.
         */
        //#IFDEF 'viro_react'
        public MaterialBuilder fresnelExponent(float fresnelExponent) {
            material.setFresnelExponent(fresnelExponent);
            return this;
        }
        //#ENDIF
        /**
         * Refer to {@link Material#setLightingModel(LightingModel)}.
         *
         * @return This builder.
         */
        public MaterialBuilder lightingModel(LightingModel lightingModel) {
            material.setLightingModel(lightingModel);
            return this;
        }

        /**
         * Refer to {@link Material#setBlendMode(BlendMode)}.
         *
         * @return This builder.
         */
        public MaterialBuilder blendMode(BlendMode blendMode) {
            material.setBlendMode(blendMode);
            return this;
        }

        /**
         * Refer to {@link Material#setTransparencyMode(TransparencyMode)}.
         *
         * @return This builder.
         */
        public MaterialBuilder transparencyMode(TransparencyMode transparencyMode) {
            material.setTransparencyMode(transparencyMode);
            return this;
        }

        /**
         * Refer to {@link Material#setCullMode(CullMode)}.
         *
         * @return This builder.
         */
        public MaterialBuilder cullMode(CullMode cullMode) {
            material.setCullMode(cullMode);
            return this;
        }

        /**
         * Refer to {@link Material#setBloomThreshold(float)}.
         *
         * @return This builder.
         */
        public MaterialBuilder bloomThreshold(float bloomThreshold) {
            material.setBloomThreshold(bloomThreshold);
            return this;
        }

        /**
         * Refer to {@link Material#setShadowMode(ShadowMode)}.
         *
         * @return This builder.
         */
        public MaterialBuilder shadowMode(ShadowMode shadowMode) {
            material.setShadowMode(shadowMode);
            return this;
        }

        /**
         * Refer to {@link Material#setName(String)}}.
         *
         * @return This builder.
         */
        public MaterialBuilder setName(String name) {
            material.setName(name);
            return this;
        }

        /**
         * Returns the built {@link Material}.
         *
         * @return The built Material.
         */
        public Material build() {
            return material;
        }
    }
}
