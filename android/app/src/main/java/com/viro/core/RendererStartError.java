/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Errors returned by the {@link RendererStartListener}, in response to the renderer failing to
 * initialize.
 */
public enum RendererStartError {

    /**
     * Indicates ARCore is not supported on the device.
     */
    ARCORE_NOT_SUPPORTED,

    /**
     * Indicates ARCore is supported by the device, but is not installed on the device and no
     * install was requested by the application.
     */
    ARCORE_NOT_INSTALLED,

    /**
     * Indicates ARCore is supported by the device, and an install was requested by the application
     * but declined by the user.
     */
    ARCORE_USER_DECLINED_INSTALL,

    /**
     * Indicates the version of ARCore on the user's device is out of date.
     */
    ARCORE_NEEDS_UPDATE,

    /**
     * Indicates the ARCore SDK used by this app is out of date.
     */
    ARCORE_SDK_NEEDS_UPDATE,

    /**
     * Indicates an unknown error detecting ARCore on the device.
     */
    ARCORE_UNKNOWN,

    /**
     * Indicates AR could not be started because camera permissions were not granted to the
     * application.
     */
    CAMERA_PERMISSIONS_NOT_GRANTED,

};
