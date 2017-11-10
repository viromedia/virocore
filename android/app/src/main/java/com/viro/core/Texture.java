/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.core;

import android.graphics.Bitmap;

import com.viro.core.internal.Image;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

/**
 * Texture is an image used to add surface detail and color to a {@link Material}. The most common
 * form of Texture is a diffuse texture, which determines a Material's base color. Other common
 * textures are specular textures, which affect the color of light reflected directly toward the
 * user, and normal textures, which specify the orientation of a surface at each pixel.
 * <p>
 * Textures consist of image data and properties that define how they map to a given surface. These
 * properties include the texture's wrapping mode (e.g. repeat or clamp), and its various filtering
 * modes, which determine how the texture is sampled when it's size on the screen is smaller or
 * larger than its underlying image size.
 */
public class Texture {


    // TODO For bitmaps this is fine. Consider creating a data constructor that takes two
    //      texture format enums: one input and one output. Then we'll have a number of
    //      try-catch blocks that will indicate what happens.
    /**
     * Texture.Format identifies the format of the pixel-data underlying the {@link Texture}.
     */
    public enum Format {
        /**
         * Texture data is stored in standard RGBA8 format.
         */
        RGBA8("RGBA8"),
        /**
         * Texture data is stored with 5 bits red, 6 bits blue, 5 bits green, and no alpha. This
         * provides decent quality for much less memory than RGBA8, but loses the alpha channel.
         */
        RGB565("RGB565"),
        /**
         * Texture data is stored with 9 bits for R, G, and B, and a 5 bit exponent. This format
         * is used for HDR images, as it takes less memory than floating point textures.
         */
        RGB9_E5("RGB9_E5");

        private final String mStringValue;

        public static Format forString(String string) {
            for (Format format : Format.values()) {
                if (format.getStringValue().equalsIgnoreCase(string)) {
                    return format;
                }
            }
            throw new IllegalArgumentException("Invalid texture format [" + string + "]");
        }

        private Format(String value) {
            this.mStringValue = value;
        }
        public String getStringValue() {
            return this.mStringValue;
        }

        private static Map<String, Format> map = new HashMap<String, Format>();
        static {
            for (Format value : Format.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static Format valueFromString(String str) {
            return str == null ? null : map.get(str.toLowerCase());
        }
    }

    /**
     * WrapMode determines what happens when the texture coordinates extend outside the [0.0, 1.0]
     * range when mapping the image.
     */
    public enum WrapMode {
        /**
         * Texture coordinates are clamped between 0.0 and 1.0.
         */
        CLAMP("Clamp"),

        /**
         * Texture coordinates repeat by going back to 0.0 after exceeding 1.0. Essentially this
         * means the renderer only uses the fractional part of the texture coordinates when
         * sampling.
         */
        REPEAT("Repeat"),

        /**
         * Texture coordinates repeat, but the range reverses each time the limit is exceeded. E.g.
         * from 0.0 to 1.0, then 1.0 to 0.0, then 0.0 to 1.0, etc.
         */
        MIRROR("Mirror");

        private String mStringValue;
        private WrapMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, WrapMode> map = new HashMap<String, WrapMode>();
        static {
            for (WrapMode value : WrapMode.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static WrapMode valueFromString(String str) {
            return str == null ? null : map.get(str.toLowerCase());
        }
    };

    /**
     * FilterMode determines how samples are chosen from the {@link Texture} when its rendered
     * size is larger or smaller than the underlying image's actual size.
     */
    public enum FilterMode {
        /**
         * Nearest filtering returns the color from the texel nearest to the coordinates being
         * sampled.
         */
        NEAREST("Nearest"),

        /**
         * Linear filtering samples the texels in the neighborhood of the coordinate and linearly
         * interpolates between them.
         */
        LINEAR("Linear");

        private String mStringValue;
        private FilterMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, FilterMode> map = new HashMap<String, FilterMode>();
        static {
            for (FilterMode value : FilterMode.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static FilterMode valueFromString(String str) {
            return str == null ? null : map.get(str.toLowerCase());
        }
    };


    /**
     * StereoMode is used in VR to render a {@link Texture} in stereo. Stereo images are designed to
     * simulate 3D by rendering slightly different images to each eye, creating a realistic depth
     * illusion. The StereoMode indicates how the stereo image is divided between the left eye and
     * the right eye.
     * <p>
     * The image will be rendered in the given order, the first being the left eye, the next the
     * right eye. For example, LEFT_RIGHT will render the left half of the image to the left eye, and
     * the right half of the image to the right eye. Similarly, TOP_BOTTOM will render the top half
     * of the image to the left eye, and the bottom half of the image to the right eye.
     * <p>
     */
    public enum StereoMode {

        /**
         * Render the left half of the image to the left eye and the right half of the image to the
         * right eye.
         */
        LEFT_RIGHT("leftRight"),

        /**
         * Render the right half of the image to the left eye, and the left half of the image to the
         * right eye.
         */
        RIGHT_LEFT("rightLeft"),

        /**
         * Render the top half of the image to the left eye, and the bottom half of the image to the
         * right eye.
         */
        TOP_BOTTOM("topBottom"),

        /**
         * Render the bottom half of the image to the left eye, and the top half of the image to the
         * right eye.
         */
        BOTTOM_TOP("bottomTop");

        private String mStringValue;
        private StereoMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, StereoMode> map = new HashMap<String, StereoMode>();
        static {
            for (StereoMode value : StereoMode.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static StereoMode valueFromString(String str) {
            return str == null ? null : map.get(str.toLowerCase());
        }
    };


    protected long mNativeRef;
    private int mWidth;
    private int mHeight;
    private WrapMode mWrapS = WrapMode.CLAMP;
    private WrapMode mWrapT = WrapMode.CLAMP;
    private FilterMode mMinificationFilter = FilterMode.LINEAR;
    private FilterMode mMagnificationFilter = FilterMode.LINEAR;
    private FilterMode mMipFilter = FilterMode.LINEAR;

    /**
     * Subclass constructor, creates its own mNativeRef.
     *
     * @hide
     */
    Texture() {

    }

    /**
     * @hide
     */
    public Texture(Image px, Image nx, Image py,
                   Image ny, Image pz, Image nz,
                   Format format) {
        mNativeRef = nativeCreateCubeTexture(px.mNativeRef, nx.mNativeRef,
                                             py.mNativeRef, ny.mNativeRef,
                                             pz.mNativeRef, nz.mNativeRef,
                                             format.getStringValue());
        mWidth = (int) px.getWidth();
        mHeight = (int) px.getHeight();
    }

    /**
     * @hide
     */
    public Texture(Image image, Format format, boolean sRGB, boolean mipmap) {
        mNativeRef = nativeCreateImageTexture(image.mNativeRef, format.getStringValue(), sRGB, mipmap, null);
        mWidth = (int) image.getWidth();
        mHeight = (int) image.getHeight();
    }

    /**
     * @hide
     */
    public Texture(Image image, Format format, boolean sRGB, boolean mipmap, String stereoMode) {
        mNativeRef = nativeCreateImageTexture(image.mNativeRef, format.getStringValue(), sRGB, mipmap, stereoMode);
        mWidth = (int) image.getWidth();
        mHeight = (int) image.getHeight();
    }

    /**
     * Construct a new cube map Texture out of the given images. Cube maps are a group of six
     * images. They are commonly used either as skyboxes (backgrounds that enclose the user), via
     * {@link Scene#setBackgroundCubeTexture(Texture)}, or as reflective or environment
     * maps.
     *
     * @param px            The {@link Bitmap} to use for the right side of the cube.
     * @param nx            The {@link Bitmap} to use for the left side of the cube.
     * @param py            The {@link Bitmap} to use for the top side of the cube.
     * @param ny            The {@link Bitmap} to use for the bottom side of the cube.
     * @param pz            The {@link Bitmap} to use for the far side of the cube.
     * @param nz            The {@link Bitmap} to use for the near side of the cube.
     * @param storageFormat The format in which to store the texture in memory.
     */
    public Texture(Bitmap px, Bitmap nx, Bitmap py,
                   Bitmap ny, Bitmap pz, Bitmap nz,
                   Format storageFormat) {
        if (storageFormat == Format.RGB9_E5) {
            throw new IllegalArgumentException("Bitmaps cannot be stored in RGB9_E5 format");
        }

        mNativeRef = nativeCreateCubeTextureBitmap(px, nx, py, ny, pz, nz, storageFormat.getStringValue());
        mWidth = px.getWidth();
        mHeight = px.getHeight();
    }

    /**
     * Construct a new 2D Texture out of the given image represented as a {@link Bitmap}.
     *
     * @param image           The {@link Bitmap} to turn into a Texture.
     * @param storageFormat   The format in which to store the Texture in memory. Bitmaps can be
     *                        stored as either {@link Format#RGBA8} or {@link
     *                        Format#RGB565}.
     * @param sRGB            True if the image is in a gamma-corrected (sRGB) format. If true, Viro
     *                        will linearize the texture during shading computations. This option
     *                        should generally be set to true for diffuse and specular images, and
     *                        false for non-visual images like normal maps.
     * @param generateMipmaps True if Viro should generate mipmaps for the image and store them in
     *                        the Texture. Mipmapping dramatically improves the visual quality and
     *                        performance of Textures when they are rendered onto small surfaces.
     */
    public Texture(Bitmap image, Format storageFormat, boolean sRGB, boolean generateMipmaps) {
        this(image, storageFormat, sRGB, generateMipmaps, null);
    }

    /**
     * Construct a new <i>stereo</i> 2D Texture out of the given image represented as a {@link
     * Bitmap}.
     *
     * @param image           The {@link Bitmap} to turn into a Texture.
     * @param storageFormat   The format in which to store the Texture in memory. Bitmaps can be
     *                        stored as either {@link Format#RGBA8} or {@link
     *                        Format#RGB565}.
     * @param sRGB            True if the image is in a gamma-corrected (sRGB) format. If true, Viro
     *                        will linearize the texture during shading computations. This option
     *                        should generally be set to true for diffuse and specular images, and
     *                        false for non-visual images like normal maps.
     * @param generateMipmaps True if Viro should generate mipmaps for the image and store them in
     *                        the Texture. Mipmapping dramatically improves the visual quality and
     *                        performance of Textures when they are rendered onto small surfaces.
     * @param stereoMode      The {@link StereoMode} indicating which half of the image to render to
     *                        the left eye, and which to render to the right eye. Null if the image
     *                        is not stereo.
     */
    public Texture(Bitmap image, Format storageFormat, boolean sRGB, boolean generateMipmaps, StereoMode stereoMode) {
        if (storageFormat == Format.RGB9_E5) {
            throw new IllegalArgumentException("Bitmaps cannot be stored in RGB9_E5 format");
        }
        mNativeRef = nativeCreateImageTextureBitmap(image, storageFormat.getStringValue(), sRGB, generateMipmaps,
                                                    stereoMode != null ? stereoMode.getStringValue() : null);
        mWidth = image.getWidth();
        mHeight = image.getHeight();
    }

    /**
     * Construct a new 2D Texture out of the image contained in the given {@link ByteBuffer},
     * stored in the indicated format. Not all input formats are compatible with all storage formats.
     * This method can be used to load images that are not in {@link Bitmap} form: for example,
     * raw images or HDR images.
     *
     * @param data            The raw image data to turn into a Texture. This must be a
     *                        <i>direct</i> {@link ByteBuffer}. The image data must start at
     *                        position 0 in the buffer and end at the buffer's capacity.
     * @param width           The width of the image stored in <tt>data</tt>.
     * @param height          The height of the image stored in <tt>data</tt>.
     * @param inputFormat     The format of the data stored in <tt>data</tt>.
     * @param storageFormat   The format in which to store the Texture in memory.
     * @param sRGB            True if the image is in a gamma-corrected (sRGB) format. If true, Viro
     *                        will linearize the texture during shading computations. This option
     *                        should generally be set to true for diffuse and specular images, and
     *                        false for non-visual images like normal maps.
     * @param generateMipmaps True if Viro should generate mipmaps for the image and store them in
     *                        the Texture. Mipmapping dramatically improves the visual quality and
     *                        performance of Textures when they are rendered onto small surfaces.
     *                        Mipmap generation is not possible for all input formats.
     * @param stereoMode      The {@link StereoMode} indicating which half of the image to render to
     *                        the left eye, and which to render to the right eye. Null if the image
     *                        is not stereo.
     */
    public Texture(ByteBuffer data, int width, int height, Format inputFormat, Format storageFormat,
                   boolean sRGB, boolean generateMipmaps, StereoMode stereoMode) {
        if (!data.isDirect()) {
            throw new IllegalArgumentException("Image data must be stored in a direct ByteBuffer");
        }
        if (inputFormat == Format.RGB9_E5) {
            if (generateMipmaps) {
                throw new IllegalArgumentException("Mipmaps cannot be generated for RGB9_E5 images");
            }
            if (storageFormat != Format.RGB9_E5) {
                throw new IllegalArgumentException("RGB9_E5 images may only be stored in RGB9_E5 format");
            }
        }
        else {
            if (storageFormat == Format.RGB9_E5) {
                throw new IllegalArgumentException("RGB8 and RGB565 images cannot be stored in RGB9_E5 format");
            }
        }

        mNativeRef = nativeCreateImageTextureData(data, width, height, inputFormat.getStringValue(),
                                                  storageFormat.getStringValue(), sRGB, generateMipmaps,
                                                  stereoMode != null ? stereoMode.getStringValue() : null);
        mWidth = width;
        mHeight = height;
    }

    /**
     * Construct a new 2D <i>HDR</i> Texture out of the image contained in VHD format in the
     * given {@link ByteBuffer}.
     *
     * @param data            The raw image data to turn into a Texture. This must be a
     *                        <i>direct</i> {@link ByteBuffer}. The image data must start at
     *                        position 0 in the buffer and end at the buffer's capacity.
     * @param stereoMode      The {@link StereoMode} indicating which half of the image to render to
     *                        the left eye, and which to render to the right eye. Null if the image
     *                        is not stereo.
     */
    public Texture(ByteBuffer data, StereoMode stereoMode) {
        if (!data.isDirect()) {
            throw new IllegalArgumentException("Image data must be stored in a direct ByteBuffer");
        }

        mNativeRef = nativeCreateImageTextureVHD(data, stereoMode != null ? stereoMode.getStringValue() : null);
        mWidth = nativeGetTextureWidth(mNativeRef);
        mHeight = nativeGetTextureHeight(mNativeRef);
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
     * Release native resources associated with this Texture.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyTexture(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Get the width of this Texture in pixels.
     *
     * @return The width of this Texture.
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Get the height of this Texture in pixels.
     *
     * @return The height of the Texture.
     */
    public int getHeight() {
        return mHeight;
    }

    /**
     * Set the horizontal {@link WrapMode}. This determines what happens when the texture
     * coordinates extend outside the [0.0, 1.0] in the X direction of the Texture.
     *
     * @param wrapS The horizontal {@link WrapMode}.
     */
    public void setWrapS(WrapMode wrapS) {
        mWrapS = wrapS;
        nativeSetWrapS(mNativeRef, wrapS.getStringValue());
    }

    /**
     * Get the horizontal {@link WrapMode} used for this Texture.
     *
     * @return The horizontal wrap mode.
     */
    public WrapMode getWrapS() {
        return mWrapS;
    }

    /**
     * Set the vertical {@link WrapMode}. This determines what happens when the texture coordinates
     * extend outside the [0.0, 1.0] in the Y direction of the Texture.
     *
     * @param wrapT The vertical {@link WrapMode}.
     */
    public void setWrapT(WrapMode wrapT) {
        mWrapT = wrapT;
        nativeSetWrapT(mNativeRef, wrapT.getStringValue());
    }

    /**
     * Get the vertical {@link WrapMode} used for this Texture.
     *
     * @return The vertical wrap mode.
     */
    public WrapMode getWrapT() {
        return mWrapT;
    }

    /**
     * Set the minification filter to use for this Texture. The minification filter is used when the
     * texture image needs to be rendered onto a surface <i>smaller</i> than the image. This occurs,
     * for example, when rendering a textured surface very far away from the camera.
     *
     * @param minificationFilter The minification {@link FilterMode} to use for this Texture.
     */
    public void setMinificationFilter(FilterMode minificationFilter) {
        mMinificationFilter = minificationFilter;
        nativeSetMinificationFilter(mNativeRef, minificationFilter.getStringValue());
    }

    /**
     * Get the minification {@link FilterMode} used by this Texture.
     *
     * @return The minification filter.
     */
    public FilterMode getMinificationFilter() {
        return mMinificationFilter;
    }

    /**
     * Set the magnification filter to use for this Texture. The magnification filter is used when
     * the texture image needs to be rendered onto a surface *larger* than the image. This occurs,
     * for example, when rendering a textured surface very close to the camera.
     *
     * @param magnificationFilter The magnification {@link FilterMode} to use for this Texture.
     */
    public void setMagnificationFilter(FilterMode magnificationFilter) {
        mMagnificationFilter = magnificationFilter;
        nativeSetMagnificationFilter(mNativeRef, magnificationFilter.getStringValue());
    }

    /**
     * Get the magnification {@link FilterMode} used by this Texture.
     *
     * @return The magnification filter.
     */
    public FilterMode getMagnificationFilter() {
        return mMagnificationFilter;
    }

    /**
     * Set the filter to use when mipmapping the Texture. Mipmapping increases rendering performance
     * when rendering textures at small sizes. When used, Viro will create scaled down versions of
     * the texture, and during rendering will sample the version that's closest to the size of the
     * surface being rendered.
     * <p>
     * {@link FilterMode#NEAREST} filtering will return the color from the mip-level closest to the
     * size of the rendered surface.
     * <p>
     * {@link FilterMode#LINEAR}' filtering will return the color found by interpolating between the
     * two nearest mip-levels.
     *
     * @param mipFilter The mip {@link FilterMode} to use for this Texture.
     */
    public void setMipFilter(FilterMode mipFilter) {
        mMipFilter = mipFilter;
        nativeSetMipFilter(mNativeRef, mipFilter.getStringValue());
    }

    /**
     * Get the mip {@link FilterMode} used by this Texture.
     *
     * @return The mip filter.
     */
    public FilterMode getMipFilter() {
        return mMipFilter;
    }

    private native long nativeCreateCubeTexture(long px, long nx, long py,
                                                long ny, long pz, long nz,
                                                String format);
    private native long nativeCreateImageTexture(long image, String format, boolean sRGB, boolean mipmap, String stereoMode);
    private native long nativeCreateCubeTextureBitmap(Bitmap px, Bitmap nx, Bitmap py,
                                                      Bitmap ny, Bitmap pz, Bitmap nz,
                                                      String format);
    private native long nativeCreateImageTextureBitmap(Bitmap image, String format, boolean sRGB, boolean mipmap, String stereoMode);
    private native long nativeCreateImageTextureData(ByteBuffer data, int width, int height, String inputFormat, String storageFormat, boolean sRGB, boolean generateMipmaps, String stereoMode);
    private native long nativeCreateImageTextureVHD(ByteBuffer data, String stereoMode);
    private native int nativeGetTextureWidth(long nativeRef);
    private native int nativeGetTextureHeight(long nativeRef);
    private native void nativeSetWrapS(long nativeRef, String wrapS);
    private native void nativeSetWrapT(long nativeRef, String wrapT);
    private native void nativeSetMinificationFilter(long nativeRef, String minificationFilter);
    private native void nativeSetMagnificationFilter(long nativeRef, String magnificationFilter);
    private native void nativeSetMipFilter(long nativeRef, String mipFilter);
    private native void nativeDestroyTexture(long nativeRef);
}
