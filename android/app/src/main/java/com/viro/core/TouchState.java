/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

package com.viro.core;

import java.util.HashMap;
import java.util.Map;

/**
 * Indicates the status of a touchpad touch event, for use with {@link TouchpadTouchListener}.
 */
public enum TouchState {
    /**
     * Finger has gone down.
     */
    TOUCH_DOWN(1),
    /**
     * Finger has gone down and has moved.
     */
    TOUCH_DOWN_MOVE(2),
    /**
     * Finger has been released from the touchpad.
     */
    TOUCH_UP(3);

    private final int mTypeId;
    TouchState(int id){
        mTypeId = id;
    }

    private static Map<Integer, TouchState> map = new HashMap<Integer, TouchState>();
    static {
        for (TouchState status : TouchState.values()) {
            map.put(status.mTypeId, status);
        }
    }

    /**
     * @hide
     */
    public static TouchState valueOf(int id) {
        return map.get(id);
    }

    /**
     * @hide
     */
    public int getTypeId() { return mTypeId; }
}