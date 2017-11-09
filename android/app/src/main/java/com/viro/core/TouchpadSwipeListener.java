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
 * Callback interface for responding to touchpad swipe events, which occur when a finger is
 * swiped across a touchpad Controller.
 */
public interface TouchpadSwipeListener {

    /**
     * Callback when a swipe was been detected on a touchpad controller while hovering over
     * the given {@link Node}.
     *
     * @param source     The platform specific source ID, which indicates what button or
     *                   component on the Controller triggered the event. See the Controller's
     *                   Guide for information.
     * @param node       The {@link Node} being hovered over at the time of the swipe.
     * @param swipeState The direction of the swipe recorded.
     */
    void onSwipe(int source, Node node, SwipeState swipeState);
}