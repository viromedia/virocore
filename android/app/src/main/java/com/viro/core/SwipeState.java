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
 * Indicates the status of a touchpad swipe event, for use with the {@link TouchpadSwipeListener}.
 */
public enum SwipeState {
    /**
     * Swipe upward has been detected.
     */
    SWIPE_UP(1),
    /**
     * Swipe downward has been detected.
     */
    SWIPE_DOWN(2),
    /**
     * Swipe to the left has been detected.
     */
    SWIPE_LEFT(3),
    /**
     * Swipe to the right has been detected.
     */
    SWIPE_RIGHT(4);

    private final int mTypeId;
    SwipeState(int id){
        mTypeId = id;
    }

    private static Map<Integer, SwipeState> map = new HashMap<Integer, SwipeState>();
    static {
        for (SwipeState status : SwipeState.values()) {
            map.put(status.mTypeId, status);
        }
    }
    public static SwipeState valueOf(int id) {
        return map.get(id);
    }
    public int getTypeId() { return mTypeId; }
}
