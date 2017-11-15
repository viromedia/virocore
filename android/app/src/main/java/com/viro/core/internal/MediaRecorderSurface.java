/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLExt;
import android.opengl.EGLSurface;
import android.util.Log;
import android.view.Surface;

/**
 * Holds state associated with a Surface used for MediaRecorder video input.
 * <p>
 * The constructor takes a Surface obtained from MeadiaRecorder.getSurface(), and uses
 * that to create an EGL window surface. Calls to eglSwapBuffers() then causes a frame of data to
 * be sent to the recorder's video encoder.
 * <p>
 * This object owns the Surface -- releasing this will destroy the Surface too.
 */
public class MediaRecorderSurface {
    private static final int EGL_RECORDABLE_ANDROID = 0x3142;
    private Surface mRecorderSurface;
    private EGLDisplay mEGLDisplay = EGL14.EGL_NO_DISPLAY;
    private EGLSurface mEGLSurface = EGL14.EGL_NO_SURFACE;
    private EGLContext mEGLContext = EGL14.EGL_NO_CONTEXT;
    private EGLContext mSavedEglContext;
    private EGLDisplay mSavedEglDisplay;
    private EGLSurface mSavedEglDrawSurface;
    private EGLSurface mSavedEglReadSurface;

    public MediaRecorderSurface(Surface recorderSurface) {
        mRecorderSurface = recorderSurface;
    }

    public boolean eglSetup() {
        if (mRecorderSurface == null || !mRecorderSurface.isValid()) {
            return false;
        }

        // Firstly, grab the current eglDisplay for this application and setup initial configurations.
        mEGLDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
        if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
            Log.e("Viro","Recording error: unable to get EGL14 display");
            return false;
        }

        int[] version = new int[2];
        if (!EGL14.eglInitialize(mEGLDisplay, version, 0, version, 1)) {
            Log.e("Viro","Recording error: unable to initialize EGL14");
            return false;
        }

        int[] attribList = {
                EGL14.EGL_RED_SIZE, 8,
                EGL14.EGL_GREEN_SIZE, 8,
                EGL14.EGL_BLUE_SIZE, 8,
                EGL14.EGL_ALPHA_SIZE, 8,
                EGL14.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                EGL_RECORDABLE_ANDROID, 1,
                EGL14.EGL_NONE
        };
        EGLConfig[] configs = new EGLConfig[1];
        int[] numConfigs = new int[1];
        EGL14.eglChooseConfig(mEGLDisplay, attribList, 0, configs, 0, configs.length,
                numConfigs, 0);

        if (!checkEglError("eglConfigInit")) {
            return false;
        }

        // Next, create a shared egl context targeting openGl 3.0
        int[] attrib_list = {
                EGL14.EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL14.EGL_NONE
        };
        mEGLContext = EGL14.eglCreateContext(mEGLDisplay, configs[0], EGL14.eglGetCurrentContext(),
                attrib_list, 0);

        if (!checkEglError("eglCreateContext")) {
            return false;
        }

        // Finally, create a window egl surface with the shared eglContext and mRecorderSurface.
        int[] surfaceAttribs = {
                EGL14.EGL_NONE
        };
        mEGLSurface = EGL14.eglCreateWindowSurface(mEGLDisplay, configs[0], mRecorderSurface,
                surfaceAttribs, 0);
        if (!checkEglError("eglCreateWindowSurface")) {
            return false;
        }

        return true;
    }

    public boolean destroy() {
        if (mEGLDisplay != EGL14.EGL_NO_DISPLAY) {
            EGL14.eglDestroySurface(mEGLDisplay, mEGLSurface);
        }
        mRecorderSurface.release();
        mEGLDisplay = EGL14.EGL_NO_DISPLAY;
        mEGLContext = EGL14.EGL_NO_CONTEXT;
        mEGLSurface = EGL14.EGL_NO_SURFACE;
        mRecorderSurface = null;
        return checkEglError("eglRelease");
    }

    public boolean makeCurrent() {
        saveRenderState();
        EGL14.eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext);
        return checkEglError("eglMakeCurrent");
    }

    public boolean saveRenderState() {
        mSavedEglDisplay = EGL14.eglGetCurrentDisplay();
        mSavedEglDrawSurface = EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW);
        mSavedEglReadSurface = EGL14.eglGetCurrentSurface(EGL14.EGL_READ);
        mSavedEglContext = EGL14.eglGetCurrentContext();
        return checkEglError("saveRenderState");
    }

    public boolean restoreRenderState() {
        return EGL14.eglMakeCurrent(mSavedEglDisplay, mSavedEglDrawSurface, mSavedEglReadSurface, mSavedEglContext);
    }

    public boolean swapBuffers() {
        boolean result = EGL14.eglSwapBuffers(mEGLDisplay, mEGLSurface);
        result = result && checkEglError("eglSwapBuffers");
        return result;
    }

    public boolean setPresentationTime(long nsecs) {
        EGLExt.eglPresentationTimeANDROID(mEGLDisplay, mEGLSurface, nsecs);
        return checkEglError("eglPresentationTimeANDROID");
    }

    private boolean checkEglError(String msg) {
        int error;
        if ((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS) {
           Log.e("Viro","Viro internal EGL error at : " + msg + " with : 0x" + Integer.toHexString(error));
            return false;
        }
        return true;
    }
}

