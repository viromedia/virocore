//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core;

import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

/**
 * Implementation of {@link android.opengl.GLSurfaceView.EGLWindowSurfaceFactory} that
 * returns a Viro-compatible surface. Currently this means returning an sRGB-enabled
 * surface.
 *
 * @hide
 */
public class ViroEGLWindowSurfaceFactory implements GLSurfaceView.EGLWindowSurfaceFactory {

    private static final String TAG = "Viro";

    @Override
    public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display, EGLConfig config, java.lang.Object nativeWindow) {
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
