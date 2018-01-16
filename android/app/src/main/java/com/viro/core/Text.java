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

/**
 * Text is a Geometry that renders strings of text. The properties of the Text determine the style
 * of the rendered string, and the bounds of the Text (width and height) determine the area within
 * which the text is rendered. The bounds are centered at (0,0) in the local coordinate system of
 * the Text. The HorizontalAlignment and VerticalAlignment of the Text determine how the Text is
 * positioned within these bounds.
 */
public class Text extends Geometry {

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

    private static final String DEFAULT_FONT_FAMILY = "Roboto";
    private static final int DEFAULT_FONT_SIZE = 12;
    private static final long DEFAULT_COLOR = Color.WHITE;
    private static final HorizontalAlignment DEFAULT_HORIZONTAL_ALIGNMENT = HorizontalAlignment.CENTER;
    private static final VerticalAlignment DEFAULT_VERTICAL_ALIGNMENT = VerticalAlignment.CENTER;
    private static final LineBreakMode DEFAULT_LINE_BREAK_MODE = LineBreakMode.WORD_WRAP;
    private static final ClipMode DEFAULT_CLIP_MODE = ClipMode.CLIP_TO_BOUNDS;
    private static final int DEFAULT_MAX_LINES = 0;

    private ViroContext mViroContext;
    private String mText;
    private float mWidth;
    private float mHeight;
    private String mFontFamilyName = DEFAULT_FONT_FAMILY;
    private int mFontSize = DEFAULT_FONT_SIZE;
    private long mColor = DEFAULT_COLOR;
    private HorizontalAlignment mHorizontalAlignment = DEFAULT_HORIZONTAL_ALIGNMENT;
    private VerticalAlignment mVerticalAlignment = DEFAULT_VERTICAL_ALIGNMENT;
    private LineBreakMode mLineBreakMode = DEFAULT_LINE_BREAK_MODE;
    private ClipMode mClipMode = DEFAULT_CLIP_MODE;
    private int mMaxLines = DEFAULT_MAX_LINES;

    /**
     * Create a new Text with the given set of minimum parameters: the text to display, and the
     * width and height of the bounds within which to display it. The text will be constrained to
     * the provided bounds, and defaults to wrapping words.
     *
     * @param viroContext The {@link ViroContext} is required to render Text.
     * @param text The text string to display.
     * @param width The width of the bounds within which to display the text.
     * @param height The height of the bounds within which to display the text.
     */
    public Text(ViroContext viroContext, String text, float width, float height) {
        this(viroContext, text, DEFAULT_FONT_FAMILY, DEFAULT_FONT_SIZE, DEFAULT_COLOR, width, height,
                DEFAULT_HORIZONTAL_ALIGNMENT, DEFAULT_VERTICAL_ALIGNMENT, DEFAULT_LINE_BREAK_MODE,
                DEFAULT_CLIP_MODE, DEFAULT_MAX_LINES);
    }

    /**
     * Create a new fully specified Text. The the given string will be displayed given typeface,
     * constrained to the bounds defined by the provided width and height, and aligned
     * according to the given alignment parameters and linebreak mode.
     * <p>
     * The clip mode determines whether the text is clipped to the given bounds.
     * <p>
     * The maxLines parameter, if set, caps the number of lines; when zero, there is no
     * limit to the number of lines generated.
     *
     * @param viroContext         The ViroContext is required to render Text.
     * @param text                The text string to display.
     * @param fontFamilyName      The name of the font's family name (e.g. 'Roboto' or
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
     */
    public Text(ViroContext viroContext, String text, String fontFamilyName,
                int size, long color, float width, float height,
                HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment,
                LineBreakMode lineBreakMode, ClipMode clipMode, int maxLines) {
        mViroContext = viroContext;
        mText = text;
        mFontFamilyName = fontFamilyName;
        mFontSize = size;
        mColor = color;
        mWidth = width;
        mHeight = height;
        mHorizontalAlignment = horizontalAlignment;
        mVerticalAlignment = verticalAlignment;
        mLineBreakMode = lineBreakMode;
        mClipMode = clipMode;
        mMaxLines = maxLines;

        mNativeRef = nativeCreateText(mViroContext.mNativeRef, mText, mFontFamilyName, mFontSize, mColor, mWidth,
                mHeight, mHorizontalAlignment.getStringValue(), mVerticalAlignment.getStringValue(), mLineBreakMode.getStringValue(),
                mClipMode.getStringValue(), mMaxLines);
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
     * Get the name of the font family used by this Text. For example, 'Roboto' or 'Roboto-Italic'.
     * The font family, in conjunction with the font size, determines the typeface used when
     * rendering the text.
     *
     * @return The name of the font family.
     */
    public String getFontFamilyName() {
        return mFontFamilyName;
    }

    /**
     * Set the name of the font family to use to render this Text. For example, 'Roboto' or
     * 'Roboto-Italic'. The font family, in conjunction with the font size, determines the typeface
     * used when rendering the text.
     *
     * @param fontFamily The name of the font family.
     */
    public void setFontFamilyName(String fontFamily) {
        this.mFontFamilyName = fontFamily;
        nativeSetFont(mViroContext.mNativeRef, mNativeRef, fontFamily, mFontSize);
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
        nativeSetFont(mViroContext.mNativeRef, mNativeRef, mFontFamilyName, fontSize);
    }

    public long getColor() {
        return mColor;
    }

    public void setColor(long color) {
        this.mColor = color;
        nativeSetColor(mNativeRef, color);
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

    private native long nativeCreateText(long viroContext, String text, String fontFamilyName,
                                         int size, long color, float width, float height,
                                         String horizontalAlignment, String verticalAlignment,
                                         String lineBreakMode, String clipMode, int maxLines);
    private native void nativeDestroyText(long textRef);
    private native void nativeSetText(long textRef, String text);
    private native void nativeSetFont(long viroContext, long textRef, String family, int size);
    private native void nativeSetColor(long textRef, long color);
    private native void nativeSetWidth(long textRef, float width);
    private native void nativeSetHeight(long textRef, float height);
    private native void nativeSetHorizontalAlignment(long textRef, String horizontalAlignment);
    private native void nativeSetVerticalAlignment(long textRef, String verticalAlignment);
    private native void nativeSetLineBreakMode(long textRef, String lineBreakMode);
    private native void nativeSetClipMode(long textRef, String clipMode);
    private native void nativeSetMaxLines(long textRef, int maxLines);

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
        private float mWidth;
        private float mHeight;
        private String mFontFamilyName = DEFAULT_FONT_FAMILY;
        private int mFontSize = DEFAULT_FONT_SIZE;
        private long mColor = DEFAULT_COLOR;
        private HorizontalAlignment mHorizontalAlignment = DEFAULT_HORIZONTAL_ALIGNMENT;
        private VerticalAlignment mVerticalAlignment = DEFAULT_VERTICAL_ALIGNMENT;
        private LineBreakMode mLineBreakMode = DEFAULT_LINE_BREAK_MODE;
        private ClipMode mClipMode = DEFAULT_CLIP_MODE;
        private int mMaxLines = DEFAULT_MAX_LINES;

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
         */
        public TextBuilder fontFamilyName(String mFontFamilyName) {
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
         * Refer to {@link Text#setColor(long)}.
         *
         * @return This builder.
         */
        public TextBuilder color(long mColor) {
            this.mColor = mColor;
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
            return new Text(mViroContext, mText, mFontFamilyName,mFontSize, mColor, mWidth, mHeight,
            mHorizontalAlignment, mVerticalAlignment, mLineBreakMode, mClipMode, mMaxLines);
        }

    }
}
