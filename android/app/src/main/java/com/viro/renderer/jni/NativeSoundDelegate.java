/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * This is the interface that the JNI Sound objects should implement. This should
 * *NOT* be implemented by the bridge components, those should implement {@link SoundDelegate}.
 * {@link BaseSound} implements this interface.
 *
 * @hide
 */
interface NativeSoundDelegate {
    /**
     * @hide
     */
    void soundIsReady();

    /**
     * @hide
     */
    void soundDidFinish();

    /**
     * @hide
     * @param error
     */
    void soundDidFail(String error);
}
