/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

package com.viro.core;

import java.util.HashMap;
import java.util.Map;

/**
 * Indicates the status of a rotate gesture made on a screen, for use with the {@link GestureRotateListener}.
 */
public enum RotateState {
    /**
     * Two fingers have gone down and have begun a rotation gesture.
     */
    ROTATE_START(1),
    /**
     * The fingers are continuing to rotate.
     */
    ROTATE_MOVE(2),
    /**
     * The fingers have gone up, ending the rotation.
     */
    ROTATE_END(3);

    private final int mTypeId;
    RotateState(int id) {
        mTypeId = id;
    }

    private static Map<Integer, RotateState> map = new HashMap<Integer, RotateState>();
    static {
        for (RotateState status : RotateState.values()) {
            map.put(status.mTypeId, status);
        }
    }

    /**
     * @hide
     */
    public static RotateState valueOf(int id) {
        return map.get(id);
    }

    /**
     * @hide
     */
    public int getTypeId() { return mTypeId; }
}
