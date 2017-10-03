/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

/**
 * Implementation of {@link android.opengl.GLSurfaceView.EGLWindowSurfaceFactory} that
 * returns a Viro-compatible surface. Currently this means returning an sRGB-enabled
 * surface.
 */
public class ViroEGLWindowSurfaceFactory implements GLSurfaceView.EGLWindowSurfaceFactory {

    private static final String TAG = "Viro";

    public javax.microedition.khronos.egl.EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display,
                                                                         EGLConfig config, Object nativeWindow) {
        javax.microedition.khronos.egl.EGLSurface result = null;
        try {
            final int EGL_GL_COLORSPACE_KHR = 0x309D;
            final int EGL_GL_COLORSPACE_SRGB_KHR = 0x3089;
            final int[] surfaceAttribs = {
                    EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR,
                    EGL10.EGL_NONE
            };

            result = egl.eglCreateWindowSurface(display, config, nativeWindow, surfaceAttribs);
        } catch (IllegalArgumentException e) {
            // This exception indicates that the surface flinger surface
            // is not valid. This can happen if the surface flinger surface has
            // been torn down, but the application has not yet been
            // notified via SurfaceHolder.Callback.surfaceDestroyed.
            // In theory the application should be notified first,
            // but in practice sometimes it is not. See b/4588890
            Log.e(TAG, "eglCreateWindowSurface", e);
        }
        return result;
    }

    public void destroySurface(EGL10 egl, EGLDisplay display,
                               javax.microedition.khronos.egl.EGLSurface surface) {
        egl.eglDestroySurface(display, surface);
    }
}
