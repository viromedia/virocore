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
 * Indicates that status of a pinch gesture made on a screen, for use with the {@link GesturePinchListener}.
 */
public enum PinchState {
    /**
     * Two fingers have gone down and have pinched inward or outward, beginning the pinch.
     */
    PINCH_START(1),
    /**
     * The pinch is continuing to move.
     */
    PINCH_MOVE(2),
    /**
     * The fingers have gone up, ending the pinch.
     */
    PINCH_END(3);

    private final int mTypeId;
    PinchState(int id) {
        mTypeId = id;
    }

    private static Map<Integer, PinchState> map = new HashMap<Integer, PinchState>();
    static {
        for (PinchState status : PinchState.values()) {
            map.put(status.mTypeId, status);
        }
    }

    /**
     * @hide
     */
    public static PinchState valueOf(int id) {
        return map.get(id);
    }
    public int getTypeId() { return mTypeId; }
}
