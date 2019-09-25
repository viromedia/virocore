//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
