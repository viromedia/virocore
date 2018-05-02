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
            assertFalse("Expected ViroViewARCore to NOT be supported", ViroViewARCore.isDeviceCompatible(mActivity));
            return;
        }
    }
}
