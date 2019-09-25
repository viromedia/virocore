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

    /**
     * @hide
     */
    public static ControllerStatus valueOf(int id) {
        return map.get(id);
    }

    /**
     * @hide
     */
    public int getTypeId() { return mTypeId; }
}