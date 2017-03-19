/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * This is the interface that the JNI Sound objects should implement. This should
 * *NOT* be implemented by the bridge components, those should implement {@link SoundDelegate}
 */
interface NativeSoundDelegate {
    void soundIsReady();
    void soundDidFinish();
    void soundDidFail(String error);
}
