package com.viromedia.releasetest.tests;

import com.google.ar.core.Config;
import com.google.ar.core.Session;
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

        if (session.isSupported(config)) {
            assertTrue("Expected ViroViewARCore to be supported", ViroViewARCore.isDeviceCompatible(mActivity));
        } else {
            assertFalse("Expected ViroViewARCore to NOT be supported", ViroViewARCore.isDeviceCompatible(mActivity));
        }
    }
}
