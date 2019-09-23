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
 * An interface to allow callers to receive a callback on success or failure of an operation by the
 * keen client. This is most helpful for asynchronous calls (to detect completion of the
 * operation), but can also be used to be notified of failures, which are normally caught silently
 * by the client to prevent application crashes.
 * 
 * This advanced version of the callback adds callbacks that take the event details as parameters.
 * 
 * @author Mike Reinhold
 * @since 2.0.3
 */
public interface ViroViroKeenDetailedCallback extends ViroKeenCallback {

    /**
     * Invoked when the requested operation succeeds.
     *  
     * @param project         The project in which the event was published. If a default project has been set
     *                        on the client, this parameter may be null, in which case the default project
     *                        was used.
     * @param eventCollection The name of the collection in which the event was published
     * @param event           A Map that consists of key/value pairs. Keen naming conventions apply (see
     *                        docs). Nested Maps and lists are acceptable (and encouraged!).
     * @param keenProperties  A Map that consists of key/value pairs to override default properties.
     *                        ex: "timestamp" -&gt; Calendar.getInstance()
     */
    public void onSuccess(ViroKeenProject project, String eventCollection, Map<String, Object> event,
                          Map<String, Object> keenProperties);

    /**
     * Invoked when the requested operation fails.
     *
     * @param project         The project in which the event was published. If a default project has been set
     *                        on the client, this parameter may be null, in which case the default project
     *                        was used.
     * @param eventCollection The name of the collection in which the event was published
     * @param event           A Map that consists of key/value pairs. Keen naming conventions apply (see
     *                        docs). Nested Maps and lists are acceptable (and encouraged!).
     * @param keenProperties  A Map that consists of key/value pairs to override default properties.
     *                        ex: "timestamp" -&gt; Calendar.getInstance()
     * @param e An exception indicating the cause of the failure.
     */
    public void onFailure(ViroKeenProject project, String eventCollection, Map<String, Object> event,
                          Map<String, Object> keenProperties, Exception e);
    
}
