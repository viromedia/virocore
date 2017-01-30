/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public interface BaseSoundJni extends NativeSoundDelegate {
    void destroy();
    void play();
    void pause();
    void setVolume(float volume);
    void setMuted(boolean muted);
    void setLoop(boolean loop);
    void seekToTime(float seconds);
    void setDelegate(SoundDelegate delegate);
    void setRotation(float[] rotation);
    void setPosition(float[] position);
    void setDistanceRolloff(String model, float minDistance, float maxDistance);
}
