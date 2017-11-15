/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
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
