/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Callback interface for responding to {@link Object3D} model loading events.
 */
public interface AsyncObject3DListener {

    /**
     * Invoked on main thread after the {@link Object3D} is loaded.
     *
     * @param object The Object3D that was loaded.
     * @param type   The type of Object3D.
     */
    void onObject3DLoaded(Object3D object, Object3D.Type type);

    /**
     * Invoked on the main thread if an {@link Object3D} fails to load.
     *
     * @param error The error message.
     */
    void onObject3DFailed(String error);

}
