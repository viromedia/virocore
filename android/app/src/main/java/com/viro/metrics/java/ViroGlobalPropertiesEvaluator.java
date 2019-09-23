//
//  Copyright (c) 2018-present, ViroMedia, Inc.
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

package com.viro.metrics.java;

import java.util.Map;

/**
 * An interface to simulate functional programming so that you can tell the {@link ViroKeenClient}
 * how to dynamically return Keen Global Properties based on event collection name.
 *
 * @author dkador
 * @since 1.0.0
 */
public interface ViroGlobalPropertiesEvaluator {

    /**
     * Gets a {@link Map} containing the global properties which should be applied to
     * a new event in the specified collection. This method will be called each time a new event is
     * created.
     *
     * @param eventCollection The name of the collection for which an event is being generated.
     * @return A {@link Map} containing the global properties which should be applied to
     * the event being generated.
     */
    Map<String, Object> getGlobalProperties(String eventCollection);

}
