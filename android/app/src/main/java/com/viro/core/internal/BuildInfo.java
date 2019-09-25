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

package com.viro.core.internal;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.Log;

public class BuildInfo {
    private static final String TAG = "Viro";

    /**
     * This method returns whether or not this build is a debug or release build. Usually
     * this is determined by the BuildConfig.DEBUG flag but since we're in the renderer, we
     * can't access the top-level application's BuildConfig so we'll have to do it this way.
     * This method relies on a property being added to the AndroidManifest. Inspired by this
     * blog: https://mobiarch.wordpress.com/2014/02/19/detecting-debug-and-release-builds-in-ios-and-android/
     */
    public static boolean isDebug(Context context) {
        boolean debug = false;
        try {
            debug = (context.getPackageManager().getPackageInfo(
                    getPackageName(context), 0).applicationInfo.flags &
                    ApplicationInfo.FLAG_DEBUGGABLE) != 0;
        } catch (PackageManager.NameNotFoundException e) {
            Log.w(TAG, "Couldn't find the debuggable flag, assuming release");
        }
        return debug;
    }

    public static String getPackageName(Context context) {
        return context.getApplicationContext().getPackageName();
    }
}
