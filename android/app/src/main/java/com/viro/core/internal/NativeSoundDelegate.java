/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.core.internal;

import com.viro.core.internal.BaseSound;

/**
 * This is the interface that the JNI Sound objects should implement. This should
 * *NOT* be implemented by the bridge components, those should implement each soundsound's
 * corresponding delegate.
 *
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
