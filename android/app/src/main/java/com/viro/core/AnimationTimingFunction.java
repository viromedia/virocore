/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.core;

import java.util.HashMap;
import java.util.Map;

/**
 * AnimationTimingFunction defines the pacing of an animation.
 */
public enum AnimationTimingFunction {
    /**
     * The animation will occur evenly over its duration.
     */
    Linear("linear"),
    /**
     * The animation will begin slowly and then speed up as it progresses.
     */
    EaseIn("easein"),
    /**
     * The animation will begin quickly and then slow as it progresses.
     */
    EaseOut("easeout"),
    /**
     * The animation will begin slowly, accelerate through the middle of its duration,
     * and then slow again before completing.
     */
    EaseInEaseOut("easeineaseout"),
    /**
     * The animation will begin quickly and <i>overshoot</i> its final position before settling into
     * its final resting place
     */
    Bounce("bounce");

    private String mStringValue;

    private AnimationTimingFunction(String value) {
        this.mStringValue = value;
    }
    /**
     * @hide
     * @return
     */
    public String getStringValue() {
        return mStringValue;
    }

    private static Map<String, AnimationTimingFunction> map = new HashMap<String, AnimationTimingFunction>();
    static {
        for (AnimationTimingFunction value : AnimationTimingFunction.values()) {
            map.put(value.getStringValue().toLowerCase(), value);
        }
    }
    /**
     * @hide
     * @return
     */
    public static AnimationTimingFunction valueFromString(String str) {
        return map.get(str.toLowerCase());
    }
}
