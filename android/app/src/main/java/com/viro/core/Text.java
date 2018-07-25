/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

/*
 * Java JNI wrapper for linking the following classes below across the bridge
 * <p>
 * Android Java Object : com.viromedia.bridge.component.node.control.Text.java
 * Java JNI Wrapper    : com.viro.renderer.jni.TextJni.java
 * Cpp JNI Wrapper     : Text_JNI.cpp
 * Cpp Object          : VROText.cpp
 */
package com.viro.core;

import android.graphics.Color;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Text is a Geometry that renders strings of text. The properties of the Text determine the style
 * of the rendered string, and the bounds of the Text (width and height) determine the area within
 * which the text is rendered. The bounds are centered at (0,0) in the local coordinate system of
 * the Text. The HorizontalAlignment and VerticalAlignment of the Text determine how the Text is
 * positioned within these bounds.
 */
public class Text extends Geometry {

    /**
     * The style of the font used to render {@link Text}.
     */
    public enum FontStyle {
        /**
         * Normal font.
         */
        Normal(0),

        /**
         * Italic font.
         */
        Italic(1);

        private int mIntValue;
        private FontStyle(int value) {
            this.mIntValue = value;
        }
        /**
         * @hide
         * @return
         */
        public int getIntValue() {
            return mIntValue;
        }
    };

    /**
     * The weight of the font used to render {@link Text}. These constants map to numerical
     * weights (100 to 900). Note that not all fonts support all weights: if a weight is not
     * supported, its nearest neighbor will be used as a fallback.
     */
    public enum FontWeight {
        /**
         * Ultra light font weight (numerical value 100).
         */
        UltraLight(100),
        /**
         * Thin font weight (numerical value 200).
         */
        Thin(200),
        /**
         * Light font weight (numerical value 300).
         */
        Light(300),
        /**
         * Regular font weight (numerical value 400).
         */
        Regular(400),
        /**
         * Medium font weight (numerical value 500).
         */
        Medium (500),
        /**
         * Semibold font weight (numerical value 600).
         */
        Semibold(600),
        /**
         * Bold font weight (numerical value 700).
         */
        Bold(700),
        /**
         * Heavy font weight (numerical value 800).
         */
        Heavy(800),
        /**
         * Extra black font weight (numerical value 900).
         */
        ExtraBlack(900);

        private int mIntValue;
        private FontWeight(int value) {
            this.mIntValue = value;
        }
        /**
         * @hide
         * @return
         */
        public int getIntValue() {
            return mIntValue;
        }
    };

    /**
     * Controls the horizontal alignment of Text within its bounds.
     */
    public enum HorizontalAlignment {
        /**
         * Text is aligned with the left boundary of the bounds.
         */
        LEFT("Left"),

        /**
         * Text is aligned to the right boundary of the bounds.
         */
        RIGHT("Right"),

        /**
         * Text is horizontally centered within its bounds.
         */
        CENTER("Center");

        private String mStringValue;
        private HorizontalAlignment(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }
    };

    /**
     * Controls the vertical alignment of Text within its bounds.
     */
    public enum VerticalAlignment {
        /**
         * The top of first line of the text is flush with the top of the Text's bounds.
         */
        TOP("Top"),

        /**
         * The bottom of the last line of text is flush with the bottom of the Text's bounds.
         */
        BOTTOM("Bottom"),

        /**
         * The text is vertically centered within its bounds.
         */
        CENTER("Center");

        private String mStringValue;
        private VerticalAlignment(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }
    };

    /**
     * Controls the way in which text wraps when it exceeds its bound's width.
     */
    public enum LineBreakMode {
        /**
         * Text will wrap on word boundaries; each time a word doesn't fit, Text will move to a new
         * line. If a single word doesn't fit on one line, that word will be placed alone on its own
         * line.
         */
        WORD_WRAP("WordWrap"),

        /**
         * Text will wrap character by character.
         */
        CHAR_WRAP("CharWrap"),

        /**
         * Text will wrap by word boundaries; the newlines and length of each space are chosen to
         * minimize the raggedness of the text edges. If used with {@link HorizontalAlignment#LEFT},
         * this minimizes the raggedness of the right edge; if used with {@link
         * HorizontalAlignment#RIGHT}, this minimizes the raggedness of the left edge; and if used
         * with {@link HorizontalAlignment#CENTER} this will attempt to minimize the raggedness of
         * both edges.
         */
        JUSTIFY("Justify"),

        /**
         * Text does not wrap; everything is kept on one line.
         */
        NONE("None");

        private String mStringValue;
        private LineBreakMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }
    };

    /**
     * Controls whether text clips when it exceeds its bounds.
     */
    public enum ClipMode {
        /**
         * Text is clipped (cut off) when it exceeds its bounds.
         */
        CLIP_TO_BOUNDS("ClipToBounds"),

        /**
         * Text is not clipped; it is allowed to exceed its bounds. In this case, the boundary is
         * only used for layout and alignment purposes.
         */
        NONE("None");

        private String mStringValue;
        private ClipMode(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }
    };

    /**
     * The outer stroke which enables effects like outline or drop shadow. Using different
     * outer stroke types can help make text more legible when presented over complex backgrounds in AR.
     */
    public enum OuterStroke {
        /**
         * Text is rendered without an outer stroke.
         */
        NONE("None"),

        /**
         * Text is rendered with an outline.
         */
        OUTLINE("Outline"),

        /**
         * Text is rendered with a drop shadow.
         */
        DROP_SHADOW("DropShadow");

        private String mStringValue;
        private OuterStroke(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, Text.OuterStroke> map = new HashMap<>();
        static {
            for (Text.OuterStroke value : Text.OuterStroke.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         * @return
         */
        public static OuterStroke valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    private static final String DEFAULT_FONT_FAMILY = "Roboto";
    private static final int DEFAULT_FONT_SIZE = 12;
    private static final FontStyle DEFAULT_FONT_STYLE = FontStyle.Normal;
    private static final FontWeight DEFAULT_FONT_WEIGHT = FontWeight.Regular;
    private static final long DEFAULT_COLOR = Color.WHITE;
    private static final HorizontalAlignment DEFAULT_HORIZONTAL_ALIGNMENT = HorizontalAlignment.CENTER;
    private static final VerticalAlignment DEFAULT_VERTICAL_ALIGNMENT = VerticalAlignment.CENTER;
    private static final LineBreakMode DEFAULT_LINE_BREAK_MODE = LineBreakMode.WORD_WRAP;
    private static final ClipMode DEFAULT_CLIP_MODE = ClipMode.CLIP_TO_BOUNDS;
    private static final OuterStroke DEFAULT_OUTER_STROKE = OuterStroke.NONE;
    private static final int DEFAULT_OUTER_STROKE_WIDTH = 2;
    private static final long DEFAULT_OUTER_STROKE_COLOR = Color.DKGRAY;
    private static final int DEFAULT_MAX_LINES = 0;

    private ViroContext mViroContext;
    private String mText;
    private float mWidth;
    private float mHeight;
    private String mFontFamilyName = DEFAULT_FONT_FAMILY;
    private int mFontSize = DEFAULT_FONT_SIZE;
    private FontStyle mFontStyle = DEFAULT_FONT_STYLE;
    private FontWeight mFontWeight = DEFAULT_FONT_WEIGHT;
    private long mColor = DEFAULT_COLOR;
    private HorizontalAlignment mHorizontalAlignment = DEFAULT_HORIZONTAL_ALIGNMENT;
    private VerticalAlignment mVerticalAlignment = DEFAULT_VERTICAL_ALIGNMENT;
    private LineBreakMode mLineBreakMode = DEFAULT_LINE_BREAK_MODE;
    private ClipMode mClipMode = DEFAULT_CLIP_MODE;
    private OuterStroke mOuterStroke = DEFAULT_OUTER_STROKE;
    private int mOuterStrokeWidth = DEFAULT_OUTER_STROKE_WIDTH;
    private long mOuterStrokeColor = DEFAULT_OUTER_STROKE_COLOR;
    private int mMaxLines = DEFAULT_MAX_LINES;
    private float mExtrusionDepth = 0;

    /**
     * Create a new 2D Text with the given set of minimum parameters: the text to display, and the
     * width and height of the bounds within which to display it. The text will be constrained to
     * the provided bounds, and defaults to wrapping words.
     *
     * @param viroContext The {@link ViroContext} is required to render Text.
     * @param text The text string to display.
     * @param width The width of the bounds within which to display the text.
     * @param height The height of the bounds within which to display the text.
     */
    public Text(ViroContext viroContext, String text, float width, float height) {
        this(viroContext, text, width, height, 0);
    }

    /**
     * Create a new Text with the given set of minimum parameters: the text to display, and the
     * width and height of the bounds within which to display it. The text will be constrained to
     * the provided bounds, and defaults to wrapping words. If extrusionDepth is greater than 0,
     * then the Text will be rendered in 3D: it will be vectorized with sides of length
     * <tt>extrusionDepth</tt>. If extrusionDepth is 0, then the Text will be rendered as a Bitmap, which is
     * the equivalent of invoking the basic {@link #Text(ViroContext, String, float, float)}
     * constructor.<p>
     * <p>
     * If the Text is rendered in 3D (if extrusionDepth > 0), then you can apply three
     * materials to the text: Material 0 will represent the front of the Text, Material 1 will represent the
     * back of the Text, and Material 2 will represent the sides of the Text. You can set the
     * colors of these Materials to accordingly change the colors of different parts of the
     * Text. For example:<p><pre>
     * Material frontMaterial = new Material();
     * frontMaterial.setDiffuseColor(Color.WHITE);
     *
     * Material  backMaterial = new Material();
     * backMaterial.setDiffuseColor(Color.BLUE);
     *
     * Material sideMaterial = new Material();
     * sideMaterial.setDiffuseColor(Color.RED);
     * List<Material> materials = Arrays.asList(frontMaterial, backMaterial, sideMaterial);
     * text.setMaterials(materials);
     * </pre><p>
     *
     * @param viroContext The {@link ViroContext} is required to render Text.
     * @param text        The text string to display.
     * @param width       The width of the bounds within which to display the text.
     * @param height      The height of the bounds within which to display the text.
     * @param extrusionDepth The depth of the text along the Z-axis. Set to a value greater than
     *                       zero to create 3D text.
     */
    public Text(ViroContext viroContext, String text, float width, float height, float extrusionDepth) {
        this(viroContext, text, DEFAULT_FONT_FAMILY, DEFAULT_FONT_SIZE, DEFAULT_FONT_STYLE,
                DEFAULT_FONT_WEIGHT, DEFAULT_COLOR, extrusionDepth,
                DEFAULT_OUTER_STROKE, DEFAULT_OUTER_STROKE_WIDTH, DEFAULT_OUTER_STROKE_COLOR,
                width, height,
                DEFAULT_HORIZONTAL_ALIGNMENT, DEFAULT_VERTICAL_ALIGNMENT, DEFAULT_LINE_BREAK_MODE,
                DEFAULT_CLIP_MODE, DEFAULT_MAX_LINES);
    }

    /**
     * Create a new fully specified Text. The the given string will be displayed given typeface,
     * constrained to the bounds defined by the provided width and height, and aligned according to
     * the given alignment parameters and linebreak mode.
     * <p>
     * The clip mode determines whether the text is clipped to the given bounds.
     * <p>
     * The maxLines parameter, if set, caps the number of lines; when zero, there is no limit to the
     * number of lines generated.
     * <p>
     * The fontFamilies string may contain a comma-separated list of typefaces. If so, the best
     * typeface in the list will be chosen for each glyph in the Text. For example, if <tt>fontFamilies</tt>
     * is set to <tt>"Roboto, NotoSansCJK"</tt>, then <tt>Roboto</tt> will be used for all English
     * glyphs and <tt>NotoSansCJK</tt> will be used for all Chinese, Japanese, and Korean glyphs.
     * <p>
     *
     * @param viroContext         The ViroContext is required to render Text.
     * @param text                The text string to display.
     * @param fontFamilies        The name of the font's family name (e.g. 'Roboto' or
     *                            'Roboto-Italic').
     * @param size                The point size of the font.
     * @param color               The color of the text.
     * @param width               The width of the bounds within which to display the text.
     * @param height              The height of the bounds within which to display the text.
     * @param horizontalAlignment The horizontal alignment of the text.
     * @param verticalAlignment   The vertical alignment of the text.
     * @param lineBreakMode       The line-break mode to use when the text breaches the maximum
     *                            width of the bounds.
     * @param clipMode            The clipping mode, which determines behavior when the text
     *                            breaches the maximum width of the bounds (when word-wrapping is
     *                            disabled), and the behavior when the text breaches the maximum
     *                            height of the bounds.
     * @param maxLines            If non-zero, will cap the number of lines. If set to zero, there
     *                            is no limit to the number of lines generated.
     * @deprecated Use the {@link TextBuilder} instead, which also allows you to specify font style
     * and weight.
     */
    @Deprecated
    public Text(ViroContext viroContext, String text, String fontFamilies,
                int size, long color, float width, float height,
                HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment,
                LineBreakMode lineBreakMode, ClipMode clipMode, int maxLines) {

        this(viroContext, text, fontFamilies, size, DEFAULT_FONT_STYLE, DEFAULT_FONT_WEIGHT, color, 0,
                DEFAULT_OUTER_STROKE, DEFAULT_OUTER_STROKE_WIDTH, DEFAULT_OUTER_STROKE_COLOR,
                width, height, horizontalAlignment, verticalAlignment, lineBreakMode, clipMode,
                maxLines);
    }

    private Text(ViroContext viroContext, String text, String fontFamilies,
                 int size, FontStyle fontStyle, FontWeight fontWeight, long color,
                 float extrusionDepth,
                 OuterStroke outerStroke, int outerStrokeWidth, long outerStrokeColor,
                 float width, float height,
                 HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment,
                 LineBreakMode lineBreakMode, ClipMode clipMode, int maxLines) {
        mViroContext = viroContext;
        mText = text;
        mFontFamilyName = fontFamilies;
        mFontSize = size;
        mFontStyle = fontStyle;
        mFontWeight = fontWeight;
        mExtrusionDepth = extrusionDepth;
        mColor = color;
        mOuterStroke = outerStroke;
        mOuterStrokeWidth = outerStrokeWidth;
        mOuterStrokeColor = outerStrokeColor;
        mWidth = width;
        mHeight = height;
        mHorizontalAlignment = horizontalAlignment;
        mVerticalAlignment = verticalAlignment;
        mLineBreakMode = lineBreakMode;
        mClipMode = clipMode;
        mMaxLines = maxLines;

        mNativeRef = nativeCreateText(mViroContext.mNativeRef, mText, mFontFamilyName, mFontSize,
                                      mFontStyle.getIntValue(), mFontWeight.getIntValue(), mColor, mExtrusionDepth,
                                      mOuterStroke.getStringValue(), mOuterStrokeWidth, mOuterStrokeColor, mWidth,
                                      mHeight, mHorizontalAlignment.getStringValue(), mVerticalAlignment.getStringValue(),
                                      mLineBreakMode.getStringValue(), mClipMode.getStringValue(), mMaxLines);
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
     * Release native resources associated with this Text.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyText(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Get the text string displayed by this Text.
     *
     * @return The text string.
     */
    public String getText() {
        return mText;
    }

    /**
     * Set the text string to display with this Text.
     *
     * @param textString The text string.
     */
    public void setText(String textString) {
        this.mText = textString;
        nativeSetText(mNativeRef, textString);
    }

    /**
     * Get the name of the font family used by this Text. The name can be a specific font family
     * like 'Roboto' or a generic group name like 'sans-serif-smallcaps' or 'monospace'.
     *
     * @return The name of the font family.
     * @deprecated Use {@link #getFontFamilies()}
     */
    @Deprecated
    public String getFontFamilyName() {
        return mFontFamilyName;
    }

    /**
     * Get the names of the font families used to render this Text. The names can be specific font
     * families like 'Roboto' or generic group names like 'sans-serif-smallcaps' or 'monospace'.
     *
     * @return The comma-separated names of the font families.
     */
    public String getFontFamilies() {
        return mFontFamilyName;
    }

    /**
     * Set the name of the font family to use to render this Text. For example, 'Roboto' or
     * 'monospace'. The font family, in conjunction with the font size, weight, and style,
     * fully specifies the font used when rendering the text.
     *
     * @param fontFamily The name of the font family.
     * @deprecated Use {@link #setFontFamilies(String)}
     */
    @Deprecated
    public void setFontFamilyName(String fontFamily) {
        this.mFontFamilyName = fontFamily;
        nativeSetFont(mViroContext.mNativeRef, mNativeRef, mFontFamilyName, mFontSize,
                mFontStyle.getIntValue(), mFontWeight.getIntValue());
    }

    /**
     * Set the comma-separated names of the font families that should be used to render this Text.
     * The names can refer to specific font families like 'Roboto' or generic group names like
     * 'sans-serif-smallcaps', 'monospace', or 'cursive'. The font families, in conjunction with the
     * font size, weight, and style, fully specify the font used for each glyph of the Text.
     * <p>
     * The fontFamilies string may contain a comma-separated list of typefaces. If so, the best
     * typeface in the list will be chosen for each glyph in the Text. For example, if <tt>fontFamilies</tt>
     * is set to <tt>"Roboto, NotoSansCJK"</tt>, then <tt>Roboto</tt> will be used for all English
     * glyphs and <tt>NotoSansCJK</tt> will be used for all Chinese, Japanese, and Korean glyphs.
     * <p>
     * If a font family cannot be found, the default system typeface will be used in its place.
     *
     * @param fontFamilies Comma-separated list of font families to use when rendering this Text.
     */
    public void setFontFamilies(String fontFamilies) {
        this.mFontFamilyName = fontFamilies;
        nativeSetFont(mViroContext.mNativeRef, mNativeRef, mFontFamilyName, mFontSize,
                mFontStyle.getIntValue(), mFontWeight.getIntValue());
    }

    /**
     * Get the font size used when rendering this Text.
     *
     * @return The font size of the Text.
     */
    public int getFontSize() {
        return mFontSize;
    }

    /**
     * Set the point size of the font to use when rendering this Text. The font size, in conjunction
     * with the font family, determines the typeface used when rendering the text.
     *
     * @param fontSize The font size.
     */
    public void setFontSize(int fontSize) {
        this.mFontSize = fontSize;
        nativeSetFont(mViroContext.mNativeRef, mNativeRef, mFontFamilyName, mFontSize,
                mFontStyle.getIntValue(), mFontWeight.getIntValue());
    }

    /**
     * Get the font style used when rendering this Text.
     *
     * @return The font style of the Text.
     */
    public FontStyle getFontStyle() {
        return mFontStyle;
    }

    /**
     * Set the style of the font to use when rendering this Text (e.g., italic).
     *
     * @param fontStyle The {@link FontStyle} to use.
     */
    public void setFontStyle(FontStyle fontStyle) {
        this.mFontStyle = fontStyle;
        nativeSetFont(mViroContext.mNativeRef, mNativeRef, mFontFamilyName, mFontSize,
                mFontStyle.getIntValue(), mFontWeight.getIntValue());
    }

    /**
     * Get the font weight used when rendering this Text.
     *
     * @return The {@link FontWeight} of the Text.
     */
    public FontWeight getFontWeight() {
        return mFontWeight;
    }

    /**
     * Set the weight of the font to use when rendering this Text. The values of the {@link FontWeight}
     * enum all map to standard font weight numerical values (100 to 900).
     *
     * @param fontWeight The {@link FontWeight} to use.
     */
    public void setFontWeight(FontWeight fontWeight) {
        this.mFontWeight = fontWeight;
        nativeSetFont(mViroContext.mNativeRef, mNativeRef, mFontFamilyName, mFontSize,
                      mFontStyle.getIntValue(), mFontWeight.getIntValue());
    }

    /**
     * Get the color (as a {@link Color} int) of the Text. For a 3D Text, (one with a non-zero
     * extrusion depth) this is the color of the front face of the Text.
     *
     * @return The color of the Text.
     */
    public long getColor() {
        return mColor;
    }

    /**
     * Set the color of this Text. For a 3D Text, this sets the color of the front face of the Text
     * (it sets the diffuse color the Text's first material). To set the color of the sides or the
     * back, invoke {@link #setMaterials(List)}, and make the diffuse color of the second Material
     * the desired back color and the diffuse color of the third Material the desired side
     * color. For example:<p><pre>
     * Material frontMaterial = new Material();
     * frontMaterial.setDiffuseColor(Color.WHITE);
     *
     * Material  backMaterial = new Material();
     * backMaterial.setDiffuseColor(Color.BLUE);
     *
     * Material sideMaterial = new Material();
     * sideMaterial.setDiffuseColor(Color.RED);
     * List<Material> materials = Arrays.asList(frontMaterial, backMaterial, sideMaterial);
     * text.setMaterials(materials);
     * </pre><p>
     *
     * @param color The color to use for the front faces of the Text.
     */
    public void setColor(long color) {
        this.mColor = color;
        nativeSetColor(mNativeRef, color);
    }

    /**
     * Get the extrusion depth of the text. This is zero for 2D Text. For 3D text, this is the
     * extent of the Text's sides.
     *
     * @return The extrusion depth of the Text, at world coordinate scale (e.g. 1 point equals 1
     * meter).
     */
    public float getExtrusionDepth() {
        return mExtrusionDepth;
    }

    /**
     * Set the extrusion depth of the text. Set this to zero to render 2D Text (e.g. bitmap text).
     * If extrusionDepth is greater than 0, then the Text will be rendered in 3D: it will be
     * vectorized with sides of length <tt>extrusionDepth</tt>.
     *
     * @param extrusionDepth The extrusion depth of the Text, at world coordinate scale (e.g. 1
     *                       point equals 1 meter).
     */
    public void setExtrusionDepth(float extrusionDepth) {
        this.mExtrusionDepth = extrusionDepth;
        nativeSetExtrusionDepth(mNativeRef, extrusionDepth);
    }

    /**
     * Get the width of the bounds to which this Text is constrained.
     *
     * @return The width of the Text's bounds.
     */
    public float getWidth() {
        return mWidth;
    }

    /**
     * Set the width of the bounds used by this Text. The Text is constrained by its bounds,
     * which are specified by this property and {@link #setHeight(float)}. If the Text is long enough
     * that it exceeds this width, it is wrapped to the next line as determined by {@link LineBreakMode}. If
     * the LineBreakMode is set to NONE, then the Text is clipped as per {@link ClipMode}.
     *
     * @param width The width of the bounds used by this Text.
     */
    public void setWidth(float width) {
        this.mWidth = width;
        nativeSetWidth(mNativeRef, width);
    }

    /**
     * Get the height of the bounds to which this Text is constrained.
     *
     * @return The height of the Text's bounds.
     */
    public float getHeight() {
        return mHeight;
    }

    /**
     * Set the height of the bounds used by this Text. The Text is constrained by its bounds,
     * which are specified by this property and {@link #setWidth(float)}. If the Text is long enough
     * that it exceeds this height, it is clipped as determined by the {@link ClipMode}.
     *
     * @param height The height of the bounds used by this Text.
     */
    public void setHeight(float height) {
        this.mHeight = height;
        nativeSetHeight(mNativeRef, height);
    }

    /**
     * Get the {@link HorizontalAlignment} used when rendering this text.
     *
     * @return The {@link HorizontalAlignment} used by this Text.
     */
    public HorizontalAlignment getHorizontalAlignment() {
        return mHorizontalAlignment;
    }

    /**
     * Set the {@link HorizontalAlignment} to use when rendering this text. The VerticalAlignment
     * determines how the text is laid out horizontally relative to its bounds.
     *
     * @param horizontalAlignment The {@link HorizontalAlignment} to use for this text.
     */
    public void setHorizontalAlignment(HorizontalAlignment horizontalAlignment) {
        this.mHorizontalAlignment = horizontalAlignment;
        nativeSetHorizontalAlignment(mNativeRef, horizontalAlignment.getStringValue());
    }

    /**
     * Get the {@link VerticalAlignment} used when rendering this Text.
     *
     * @return The {@link VerticalAlignment} used by this Text.
     */
    public VerticalAlignment getVerticalAlignment() {
        return mVerticalAlignment;
    }

    /**
     * Set the {@link VerticalAlignment} to use when rendering this text. The VerticalAlignment
     * determines how the text is laid out vertically relative to its bounds.
     *
     * @param verticalAlignment The {@link VerticalAlignment} to use for this text.
     */
    public void setVerticalAlignment(VerticalAlignment verticalAlignment) {
        this.mVerticalAlignment = verticalAlignment;
        nativeSetVerticalAlignment(mNativeRef, verticalAlignment.getStringValue());
    }

    /**
     * Get the {@link LineBreakMode} used by this Text.
     *
     * @return The {@link LineBreakMode} used by this Text.
     */
    public LineBreakMode getLineBreakMode() {
        return mLineBreakMode;
    }

    /**
     * Set the {@link LineBreakMode} to use for this Text. Text can be wrapped by words, wrapped by
     * characters, or it can be justified. Justification wraps words to minimize the 'jaggedness' of
     * the edges of the Text.
     *
     * @param lineBreakMode The {@link LineBreakMode} to use for this Text.
     */
    public void setLineBreakMode(LineBreakMode lineBreakMode) {
        this.mLineBreakMode = lineBreakMode;
        nativeSetLineBreakMode(mNativeRef, lineBreakMode.getStringValue());
    }

    /**
     * Get the {@link ClipMode} used by this Text.
     *
     * @return The {@link ClipMode} used by this text.
     */
    public ClipMode getClipMode() {
        return mClipMode;
    }

    /**
     * Set the {@link ClipMode} to use for this Text. If set to <tt>CLIP_TO_BOUNDS</tt>, the text will
     * be clipped if it breaches the bounds vertically or horizontally. If set to <tt>NONE</tt>, the
     * text may exceed its bounds.
     *
     * @param clipMode The {@link ClipMode} to use for the Text.
     */
    public void setClipMode(ClipMode clipMode) {
        this.mClipMode = clipMode;
        nativeSetClipMode(mNativeRef, clipMode.getStringValue());
    }

    /**
     * Get the maximum number of lines allowed for this Text. Zero implies unlimited lines.
     *
     * @return The maximum number of lines allowed for this text.
     */
    public int getMaxLines() {
        return mMaxLines;
    }

    /**
     * Set the maximum number of lines for this Text. If non-zero, this caps the number of lines. If
     * set to zero, there is no limit to the number of lines generated.
     *
     * @param maxLines The maximum of lines to render.
     */
    public void setMaxLines(int maxLines) {
        this.mMaxLines = maxLines;
        nativeSetMaxLines(mNativeRef, maxLines);
    }

    /**
     * Set an optional {@link OuterStroke} type, width, and color to use for this Text. The outer
     * stroke can be used to render an outline surrounding the Text or drop shadow extending to the
     * bottom right of the Text. These are often used to make the Text more legible over busy
     * backgrounds. The width of the outer stroke is given in pixels: a width of 2 displays a fairly
     * standard outline or drop shadow.
     * <p>
     * Note that outer strokes do <i>not</i> apply to 3D text (text with extrusion depth greater than
     * zero).
     * <p>
     * To remove the outer stroke, set the <tt>outerStroke</tt> to {@link OuterStroke#NONE}. In this
     * case the outerStrokeWidth and outerStrokeColor will be ignored.
     * <p>
     *
     * @param outerStroke The {@link OuterStroke} type to render for this text.
     * @param outerStrokeWidth The width of the stroke in pixels.
     * @param outerStrokeColor The color of the Stroke as an Android {@link Color} integer.
     */
    public void setOuterStroke(OuterStroke outerStroke, int outerStrokeWidth, long outerStrokeColor) {
        mOuterStroke = outerStroke;
        mOuterStrokeWidth = outerStrokeWidth;
        mOuterStrokeColor = outerStrokeColor;
        nativeSetOuterStroke(mNativeRef, outerStroke.getStringValue(), outerStrokeWidth, outerStrokeColor);
    }

    /**
     * Get the {@link OuterStroke}, if any, being rendered with this Text. The outer stroke
     * determines if the Text is rendered with an outline or drop shadow.
     *
     * @return The {@link OuterStroke} used to render this Text.
     */
    public OuterStroke getOuterStroke() {
        return mOuterStroke;
    }

    /**
     * Get the width of the {@link OuterStroke} being rendered with this Text, in pixels. This
     * determines how "thick" the outline or drop shadow appears.
     *
     * @return The width of the outer stroke in pixels.
     */
    public int getOuterStrokeWidth() {
        return mOuterStrokeWidth;
    }

    /**
     * Get the color of the {@link OuterStroke} being rendered with this Text. The color is
     * returned as an Android {@link Color} value.
     *
     * @return The outer stroke color.
     */
    public long getOuterStrokeColor() {
        return mOuterStrokeColor;
    }

    private native long nativeCreateText(long viroContext, String text, String fontFamilyName,
                                         int size, int style, int weight, long color, float extrusionDepth,
                                         String outerStroke, int outerStrokeWidth, long outerStrokeColor,
                                         float width, float height,
                                         String horizontalAlignment, String verticalAlignment,
                                         String lineBreakMode, String clipMode, int maxLines);
    private native void nativeDestroyText(long textRef);
    private native void nativeSetText(long textRef, String text);
    private native void nativeSetFont(long viroContext, long textRef, String family, int size, int style, int weight);
    private native void nativeSetColor(long textRef, long color);
    private native void nativeSetOuterStroke(long textRef, String stroke, int width, long color);
    private native void nativeSetWidth(long textRef, float width);
    private native void nativeSetHeight(long textRef, float height);
    private native void nativeSetHorizontalAlignment(long textRef, String horizontalAlignment);
    private native void nativeSetVerticalAlignment(long textRef, String verticalAlignment);
    private native void nativeSetLineBreakMode(long textRef, String lineBreakMode);
    private native void nativeSetClipMode(long textRef, String clipMode);
    private native void nativeSetMaxLines(long textRef, int maxLines);
    private native void nativeSetExtrusionDepth(long textRef, float extrusionDepth);

    /**
     * Builder for creating {@link Text} objects.
     */
    public static TextBuilder builder() {
        return new TextBuilder();
    }

    /**
     * Builder for creating {@link Text} objects.
     */
    public static class TextBuilder {

        private ViroContext mViroContext;
        private String mText;
        private float mExtrusionDepth;
        private float mWidth;
        private float mHeight;
        private String mFontFamilyName = DEFAULT_FONT_FAMILY;
        private int mFontSize = DEFAULT_FONT_SIZE;
        private FontStyle mFontStyle = DEFAULT_FONT_STYLE;
        private FontWeight mFontWeight = DEFAULT_FONT_WEIGHT;
        private long mColor = DEFAULT_COLOR;
        private HorizontalAlignment mHorizontalAlignment = DEFAULT_HORIZONTAL_ALIGNMENT;
        private VerticalAlignment mVerticalAlignment = DEFAULT_VERTICAL_ALIGNMENT;
        private LineBreakMode mLineBreakMode = DEFAULT_LINE_BREAK_MODE;
        private ClipMode mClipMode = DEFAULT_CLIP_MODE;
        private int mMaxLines = DEFAULT_MAX_LINES;
        private OuterStroke mOuterStroke = DEFAULT_OUTER_STROKE;
        private int mOuterStrokeWidth = DEFAULT_OUTER_STROKE_WIDTH;
        private long mOuterStrokeColor = DEFAULT_OUTER_STROKE_COLOR;

        /**
         * Set the {@link ViroContext} to be used while building Text object.
         *
         * @return This builder.
         */
        public TextBuilder viroContext(ViroContext mViroContext) {
            this.mViroContext = mViroContext;
            return this;
        }

        /**
         * Refer to {@link Text#setText(String)}.
         *
         * @return This builder.
         */
        public TextBuilder textString(String mText) {
            this.mText = mText;
            return this;
        }

        /**
         * Refer to {@link Text#setFontFamilyName(String)}.
         *
         * @return This builder.
         * @deprecated Use {@link #fontFamilies(String)}
         */
        @Deprecated
        public TextBuilder fontFamilyName(String mFontFamilyName) {
            this.mFontFamilyName = mFontFamilyName;
            return this;
        }

        /**
         * Refer to {@link Text#setFontFamilies(String)}.
         *
         * @return This builder.
         */
        public TextBuilder fontFamilies(String mFontFamilyName) {
            this.mFontFamilyName = mFontFamilyName;
            return this;
        }

        /**
         * Refer to {@link Text#setFontSize(int)}.
         *
         * @return This builder.
         */
        public TextBuilder fontSize(int mFontSize) {
            this.mFontSize = mFontSize;
            return this;
        }

        /**
         * Refer to {@link Text#setFontStyle(FontStyle)}.
         *
         * @return This builder.
         */
        public TextBuilder fontStyle(FontStyle style) {
            this.mFontStyle = style;
            return this;
        }

        /**
         * Refer to {@link Text#setFontWeight(FontWeight)}.
         *
         * @return This builder.
         */
        public TextBuilder fontWeight(FontWeight weight) {
            this.mFontWeight = weight;
            return this;
        }

        /**
         * Refer to {@link Text#setColor(long)}.
         *
         * @return This builder.
         */
        public TextBuilder color(long mColor) {
            this.mColor = mColor;
            return this;
        }

        /**
         * Refer to {@link Text#setExtrusionDepth(float)}.
         *
         * @return This builder.
         */
        public TextBuilder extrusionDepth(float extrusionDepth) {
            this.mExtrusionDepth = extrusionDepth;
            return this;
        }

        /**
         * Refer to {@link Text#setOuterStroke(OuterStroke, int, long)}.
         *
         * @return This builder.
         */
        public TextBuilder outerStroke(OuterStroke outerStroke) {
            this.mOuterStroke = outerStroke;
            return this;
        }

        /**
         * Refer to {@link Text#setOuterStroke(OuterStroke, int, long)}.
         *
         * @return This builder.
         */
        public TextBuilder outerStrokeWidth(int outerStrokeWidth) {
            this.mOuterStrokeWidth = outerStrokeWidth;
            return this;
        }

        /**
         * Refer to {@link Text#setOuterStroke(OuterStroke, int, long)}.
         *
         * @return This builder.
         */
        public TextBuilder outerStrokeColor(long outerStrokeColor) {
            this.mOuterStrokeColor = outerStrokeColor;
            return this;
        }

        /**
         * Refer to {@link Text#setWidth(float)}.
         *
         * @return This builder.
         */
        public TextBuilder width(float mWidth) {
            this.mWidth = mWidth;
            return this;
        }

        /**
         * Refer to {@link Text#setHeight(float)}.
         *
         * @return This builder.
         */
        public TextBuilder height(float mHeight) {
            this.mHeight = mHeight;
            return this;
        }

        /**
         * Refer to {@link Text#setHorizontalAlignment(HorizontalAlignment)}.
         *
         * @return This builder.
         */
        public TextBuilder horizontalAlignment(HorizontalAlignment mHorizontalAlignment) {
            this.mHorizontalAlignment = mHorizontalAlignment;
            return this;
        }

        /**
         * Refer to {@link Text#setVerticalAlignment(VerticalAlignment)}.
         *
         * @return This builder.
         */
        public TextBuilder verticalAlignment(VerticalAlignment mVerticalAlignment) {
            this.mVerticalAlignment = mVerticalAlignment;
            return this;
        }

        /**
         * Refer to {@link Text#setLineBreakMode(LineBreakMode)}.
         *
         * @return This builder.
         */
        public TextBuilder lineBreakMode(LineBreakMode mLineBreakMode) {
            this.mLineBreakMode = mLineBreakMode;
            return this;
        }

        /**
         * Refer to {@link Text#setClipMode(ClipMode)}.
         *
         * @return This builder.
         */
        public TextBuilder clipMode(ClipMode mClipMode) {
            this.mClipMode = mClipMode;
            return this;
        }

        /**
         * Refer to {@link Text#setMaxLines(int)}.
         *
         * @return This builder.
         */
        public TextBuilder maxLines(int mMaxLines) {
            this.mMaxLines = mMaxLines;
            return this;
        }

        /**
         * Return the built {@link Text}.
         *
         * @return The built Text.
         */
        public Text build() {
            return new Text(mViroContext, mText, mFontFamilyName, mFontSize, mFontStyle, mFontWeight, mColor, mExtrusionDepth,
                    mOuterStroke, mOuterStrokeWidth, mOuterStrokeColor,
                    mWidth, mHeight, mHorizontalAlignment, mVerticalAlignment, mLineBreakMode, mClipMode, mMaxLines);
        }

    }
}
