/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.core.internal;

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
