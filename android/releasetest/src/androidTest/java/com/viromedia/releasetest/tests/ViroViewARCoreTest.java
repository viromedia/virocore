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

import com.google.ar.core.Config;
import com.google.ar.core.Session;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;
import com.viro.core.ViroViewARCore;
import com.viromedia.releasetest.ViroReleaseTestActivity;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class ViroViewARCoreTest {

    @Rule
    public ViroActivityTestRule<ViroReleaseTestActivity> mActivityTestRule
            = new ViroActivityTestRule(ViroReleaseTestActivity.class, true, true);
    protected ViroReleaseTestActivity mActivity;

    @Before
    public void setUp() {
        mActivity = (ViroReleaseTestActivity) mActivityTestRule.getActivity();
    }

    @Test
    public void testViroViewAR_isSupported() {
        Session session;
        Config config;

        try {
            session = new Session(mActivity);
            config = new Config(session);
        } catch (UnavailableException ue) {
            assertFalse("UnavailableException: AR sessino could not be created", true);
            return;
        } catch (UnsatisfiedLinkError error) {
            /**
             * TODO Need to catch this error due to
             * https://github.com/google-ar/arcore-android-sdk/issues/111
             * Remove this once Google fixes the above issue. As of 02/05/2018, the issue is being
             * marked as "fixed in an upcoming release.
             */
            ViroViewARCore.ARCoreAvailability availability = ViroViewARCore.isARSupportedOnDevice(mActivity);
            assertFalse("Expected ViroViewARCore to NOT be supported", availability == ViroViewARCore.ARCoreAvailability.SUPPORTED);
            return;
        }
    }
}
