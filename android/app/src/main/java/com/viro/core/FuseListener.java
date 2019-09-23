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

import com.viro.core.Node;

/**
 * Callback interface for responding to fuse events, which occur when the user hovers the
 * Controller's pointer over a {@link Node} for a set period of time.
 */
public interface FuseListener {

    /**
     * Callback when the given {@link Node} has fused, meaning the Controller
     * has hovered over the Node for <i>time to fuse</i> milliseconds. Time to fuse can be
     * set via {@link Node#setTimeToFuse(float)}.
     *
     * @param source The platform specific source ID, which indicates what button or component on
     *               the Controller triggered the event. See the Controller's Guide for
     *               information.
     * @param node   The {@link Node} being hovered over (the Node that is "fusing").
     */
    void onFuse(int source, Node node);
}