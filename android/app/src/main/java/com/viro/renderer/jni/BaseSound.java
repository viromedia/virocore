/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Top level interface for all sounds.
 *
 * @hide
 */
public interface BaseSound extends NativeSoundDelegate {
    void dispose();
    void play();
    void pause();
    void setVolume(float volume);
    void setMuted(boolean muted);
    void setLoop(boolean loop);
    void seekToTime(float seconds);
}
