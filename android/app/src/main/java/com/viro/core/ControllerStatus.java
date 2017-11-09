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
 * Indicates the availability of a given Controller, for use with the
 * {@link ControllerStatusListener}.
 */
public enum ControllerStatus {
    /**
     * Available is unknown.
     */
    UNKNOWN(1),
    /**
     * Controller is connecting.
     */
    CONNECTING(2),
    /**
     * Controller is connected.
     */
    CONNECTED(3),
    /**
     * Controller is disconnected.
     */
    DISCONNECTED(4),
    /**
     * Controller is in an error state.
     */
    ERROR(5);

    private final int mTypeId;
    ControllerStatus(int id){
        mTypeId = id;
    }

    private static Map<Integer, ControllerStatus> map = new HashMap<Integer, ControllerStatus>();
    static {
        for (ControllerStatus status : ControllerStatus.values()) {
            map.put(status.mTypeId, status);
        }
    }
    public static ControllerStatus valueOf(int id) {
        return map.get(id);
    }
    public int getTypeId() { return mTypeId; }
}