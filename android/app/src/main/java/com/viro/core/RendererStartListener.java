/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Callback interface for responding to the Viro renderer finishing initialization. This listener
 * is used by the subclasses of {@link ViroView}.
 */
public interface RendererStartListener {

    /**
     * Callback invoked when {@link ViroView} has finished initialization. When this is received,
     * the ViroView is ready to begin rendering content.
     */
    void onRendererStart();

    /**
     * Callback invoked when the {@link ViroView} failed to initialize.
     *
     * @param error The error code.
     * @param errorMessage The reason for the failure as a string.
     */
    void onRendererFailed(RendererStartError error, String errorMessage);

}
