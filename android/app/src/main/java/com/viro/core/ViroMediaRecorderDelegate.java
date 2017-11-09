/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.core;

/**
 * Interface delegate to be implemented for responding to success / failures when a
 * video recording or screen capturing task is completed by the ViroMediaRecorder.
 */
public interface ViroMediaRecorderDelegate {
    enum ERROR {
        NONE(-1),
        UNKNOWN(0),
        NO_PERMISSIONS(1),
        INITIALIZATION(2),
        WRITE_TO_FILE(3),
        ALREADY_RUNNING(4),
        ALREADY_STOPPED(5),
        UNSUPPORTED_PLATFORM(6);

        private final int val;

        ERROR(int numVal) {
            val = numVal;
        }

        public int toInt() {
            return val;
        }
    }

    /**
     * Delegate callback that is triggered as a result of a recording or screen capturing
     * operation in the ViroMediaRecorder.
     *
     * @param success - True, if the recording operation associated with this delegate is successful.
     * @param errorCode - Error code representing the result of this operation.
     * @param filePath - If success is true, the absolute file path of the recorded / saved file is
     * provided here, null otherwise.
     */
    void onTaskCompleted(boolean success, ERROR errorCode, String filePath);
}
