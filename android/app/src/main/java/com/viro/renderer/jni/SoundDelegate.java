/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


/**
 * This is the interface that the Sound bridge components should implement.
 */
public interface SoundDelegate {
    void onSoundReady();
    void onSoundFinish();
    void onSoundFail(String error);
}
