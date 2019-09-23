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

package com.viromedia.releasetest.tests;

import android.app.Activity;
import android.support.test.rule.ActivityTestRule;
import android.util.Log;

import com.viro.core.ViroView;
import com.viromedia.releasetest.ViroReleaseTestActivity;

/**
 * Created by manish on 11/2/17.
 */

public class ViroActivityTestRule<T extends Activity> extends ActivityTestRule {
    private static final String TAG = ViroActivityTestRule.class.getSimpleName();

    public ViroActivityTestRule(final Class activityClass, final boolean initialTouchMode,
                                final boolean launchActivity) {
        super(activityClass, initialTouchMode, launchActivity);
    }

    @Override
    protected void beforeActivityLaunched() {
        // This is called before onCreate -> onStart -> onResume
        Log.i(TAG, "beforeActivityLaunched");
        super.beforeActivityLaunched();
    }

    @Override
    protected void afterActivityLaunched() {
        // This is called after onCreate -> onStart -> onResume, but before a @Before -> @Test -> @After
        Log.i(TAG, "afterActivityLaunched");
        super.afterActivityLaunched();

        // We start the renderer here, after activity construction; this way we don't block
        // the tests from starting by hogging the event queue for rendering.
        final ViroReleaseTestActivity activity = (ViroReleaseTestActivity) getActivity();
        activity.startRenderer();
    }

    @Override
    protected void afterActivityFinished() {
        Log.i(TAG, "afterActivityFinished");
        super.afterActivityFinished();
        final ViroReleaseTestActivity activity = (ViroReleaseTestActivity) getActivity();
        final ViroView viroView = activity.getViroView();

        // Fake the activity to be destroyed. The new ActivityTestRule does not exeercise
        // the complete Android Lifecycle from onCreate -> onDestroy
        try {
            runOnUiThread(() -> {
                viroView.onActivityStopped(activity);
                viroView.onActivityDestroyed(activity);
            });
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }
}
