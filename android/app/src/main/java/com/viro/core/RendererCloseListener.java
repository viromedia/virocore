/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Callback runnable that is used for responding to Viro renderer being closed by the user.
 * This listener is used by {@link ViroViewGVR}
 */
public interface RendererCloseListener {

    void onRendererClosed();
}
