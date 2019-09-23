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

import java.io.IOException;
import java.util.List;
import java.util.Map;

/**
 * Interface which provides an abstraction layer around how events are stored in between being
 * queued and being uploaded by a batch post operation.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public interface ViroKeenEventStore {

    /**
     * Stores the given event.
     *
     * @param projectId       The ID of the project in which the event should be stored.
     * @param eventCollection The name of the collection in which the event should be stored.
     * @param event           The serialized JSON for the event to store.
     * @return A handle which can be used to retrieve or remove the event.
     * @throws IOException If there is an error storing the event.
     */
    Object store(String projectId, String eventCollection, String event) throws IOException;

    /**
     * Gets the event corresponding to the given handle.
     *
     * @param handle A handle returned from a previous call to {@link #store(String, String,
     * String)}
     *               or {@link #getHandles(String)}.
     * @return The serialized JSON for the event, or null if the handle is no longer present in
     * the store.
     * @throws IOException If there is an error retrieving the event.
     */
    String get(Object handle) throws IOException;

    /**
     * Removes the specified event from the store.
     *
     * @param handle A handle returned from a previous call to {@link #store(String, String,
     * String)}
     *               or {@link #getHandles(String)}.
     * @throws IOException If there is an error removing the event.
     */
    void remove(Object handle) throws IOException;

    /**
     * Retrieves a map from collection names to lists of handles currently stored under each
     * collection. This will be used by the {@link ViroKeenClient} to retrieve the
     * events to send in a batch to the Keen server, as well as to remove all successfully posted
     * events after processing the response.
     *
     * @param projectId The ID of the project for which to retrieve event handles.
     * @return A map from collection names to lists of handles currently stored under each
     * collection. If there are no events, an empty map will be returned.
     * @throws IOException If there is an error retrieving the handles.
     */
    Map<String, List<Object>> getHandles(String projectId) throws IOException;

}