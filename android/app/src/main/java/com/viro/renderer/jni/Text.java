/*
 * Copyright © 2016 Viro Media. All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge
 * <p>
 * Android Java Object : com.viromedia.bridge.component.node.control.Text.java
 * Java JNI Wrapper    : com.viro.renderer.jni.TextJni.java
 * Cpp JNI Wrapper     : Text_JNI.cpp
 * Cpp Object          : VROText.cpp
 */
package com.viro.renderer.jni;

import android.graphics.Color;

/**
 * Text is a Geometry that renders strings of text. The properties of the Text determine the style
 * of the rendered string, and the bounds of the Text (width and height) determine the area within
 * which the text is rendered. The bounds are centered at (0,0) in the local coordinate system of
 * the Text. The HorizontalAlignment and VerticalAlignment of the Text determine how the Text is
 * positioned within these bounds.
 */
public class Text extends Geometry {

    public enum HorizontalAlignment {
        LEFT("Left"),
        RIGHT("Right"),
        CENTER("Center");

        private String mStringValue;
        private HorizontalAlignment(String value) {
            this.mStringValue = value;
        }
        public String getStringValue() {
            return mStringValue;
        }
    };

    public enum VerticalAlignment {
        TOP("Top"),
        BOTTOM("Bottom"),
        CENTER("Center");

        private String mStringValue;
        private VerticalAlignment(String value) {
            this.mStringValue = value;
        }
        public String getStringValue() {
            return mStringValue;
        }
    };

    public enum LineBreakMode {
        WORD_WRAP("WordWrap"),
        CHAR_WRAP("CharWrap"),
        JUSTIFY("Justify"),
        NONE("None");

        private String mStringValue;
        private LineBreakMode(String value) {
            this.mStringValue = value;
        }
        public String getStringValue() {
            return mStringValue;
        }
    };

    public enum ClipMode {
        CLIP_TO_BOUNDS("ClipToBounds"),
        NONE("None");

        private String mStringValue;
        private ClipMode(String value) {
            this.mStringValue = value;
        }
        public String getStringValue() {
            return mStringValue;
        }
    };

    private ViroContext mViroContext;
    private String mText;
    private String mFontFamilyName;
    private int mFontSize;
    private long mColor;
    private float mWidth;
    private float mHeight;
    private HorizontalAlignment mHorizontalAlignment;
    private VerticalAlignment mVerticalAlignment;
    private LineBreakMode mLineBreakMode;
    private ClipMode mClipMode;
    private int mMaxLines;

    /**
     * Create a new Text with the given set of minimum parameters: the text to display, and the
     * width and height of the bounds within which to display it. The text will be constrained to
     * the provided bounds, and defaults to wrapping words.
     *
     * @param viroContext The ViroContext is required to render Text.
     * @param text The text string to display.
     * @param width The width of the bounds within which to display the text.
     * @param height The height of the bounds within which to display the text.
     */
    public Text(ViroContext viroContext, String text, float width, float height) {
        this(viroContext, text, "Roboto", 12, Color.WHITE, width, height,
                HorizontalAlignment.CENTER, VerticalAlignment.CENTER, LineBreakMode.WORD_WRAP, ClipMode.CLIP_TO_BOUNDS, 0);
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

}
