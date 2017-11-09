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
 * Callback interface for responding to click events, which occur when any Controller button is
 * clicked.
 */
public interface ClickListener {

    /**
     * Callback when a click event is registered over the {@link Node} receiving this
     * event.
     *
     * @param source     The platform specific source ID, which indicates what button or
     *                   component on the Controller triggered the event. See the Controller's
     *                   Guide for information.
     * @param node       The {@link Node} that was clicked.
     * @param clickState The status of the click event.
     * @param location   The location of the event in world coordinates.
     */
    void onClick(int source, Node node, ClickState clickState, Vector location);
}
