/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 *
 * Java JNI wrapper for linking the following classes below across the bridge
 *
 * Android Java Object : com.viromedia.bridge.component.node.control.Text.java
 * Java JNI Wrapper    : com.viro.renderer.jni.TextJni.java
 * Cpp JNI Wrapper     : Text_JNI.cpp
 * Cpp Object          : VROText.cpp
 *
 */

public class TextJni extends BaseGeometry {
    private long mNativeRef;
    private long mNodeRef;

    // Save text properties here
    private RenderContextJni mRenderContext;
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
    private boolean mTextCreated = false;
    private boolean mDestroyOnTextCreation = false;

    public TextJni(RenderContextJni renderContext, String text, String fontFamilyName,
                   int size, long color, float width, float height,
                   String horizontalAlignment, String verticalAlignment,
                   String lineBreakMode, String clipMode, int maxLines) {
        mRenderContext = renderContext;
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
    }

    public void destroy() {
        if (mTextCreated) {
            nativeDestroyText(mNativeRef);
            mTextCreated = false;
            mDestroyOnTextCreation = false;
        } else {
            // destroy() called before textDidFinishCreation() was called from the renderer thread.
            // set flag here so that destroy is called when textDidFinishCreation() gets called to
            // prevent memory leak
            mDestroyOnTextCreation = true;
        }
    }


    @Override
    public void attachToNode(NodeJni node) {
        mNodeRef = node.mNativeRef;
        // createText
        nativeCreateText(mNodeRef, mRenderContext.mNativeRef, mTextString, mFontFamilyName, mFontSize, mColor, mWidth,
                mHeight, mHorizontalAlignment, mVerticalAlignment, mLineBreakMode, mClipMode, mMaxLines);
    }

    // Called from TextDelegate_JNI.cpp on successfully text creation on renderer thread
    public void textDidFinishCreation(long nativeTextRef) {
        // Save this for manipulation in the future
        mNativeRef = nativeTextRef;
        mTextCreated = true;

        // destroy called before text creation finished.
        if (mDestroyOnTextCreation) {
            destroy();
        }
    }

    private native void nativeCreateText(long nodeRef, long renderContext, String text, String fontFamilyName,
                                          int size, long color, float width, float height,
                                          String horizontalAlignment, String verticalAlignment,
                                          String lineBreakMode, String clipMode, int maxLines);

    private native void nativeDestroyText(long textReference);
}
