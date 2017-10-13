/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge
 * <p>
 * Android Java Object : com.viromedia.bridge.component.node.control.Text.java
 * Java JNI Wrapper    : com.viro.renderer.jni.TextJni.java
 * Cpp JNI Wrapper     : Text_JNI.cpp
 * Cpp Object          : VROText.cpp
 */

public class Text extends Geometry {

    // Save text properties here
    private ViroContext mViroContext;
    private String mTextString;
    private String mFontFamilyName;
    private int mFontSize;
    private long mColor;
    private float mWidth;
    private float mHeight;
    private String mHorizontalAlignment;
    private String mVerticalAlignment;
    private String mLineBreakMode;
    private String mClipMode;
    private int mMaxLines;

    public Text(ViroContext viroContext, String text, String fontFamilyName,
                int size, long color, float width, float height,
                String horizontalAlignment, String verticalAlignment,
                String lineBreakMode, String clipMode, int maxLines) {
        mViroContext = viroContext;
        mTextString = text;
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

        mNativeRef = nativeCreateText(mViroContext.mNativeRef, mTextString, mFontFamilyName, mFontSize, mColor, mWidth,
                mHeight, mHorizontalAlignment, mVerticalAlignment, mLineBreakMode, mClipMode, mMaxLines);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyText(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreateText(long renderContext, String text, String fontFamilyName,
                                         int size, long color, float width, float height,
                                         String horizontalAlignment, String verticalAlignment,
                                         String lineBreakMode, String clipMode, int maxLines);

    private native void nativeDestroyText(long textReference);
}
