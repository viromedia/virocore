/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal.keys;

/**
 * Listener with callback for response from Dyanmo
 */
public interface KeyValidationListener {

    void onResponse(boolean success);
}
