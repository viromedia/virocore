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

/**
 * Interface which provides an abstraction layer around how attempt counts are stored.
 *
 * @author Simon Murtha Smith
 * @since 2.0.2
 */
public interface ViroKeenAttemptCountingEventStore extends ViroKeenEventStore {

    /**
     * Gets the stored attempts String for a given project and collection.
     *
     * @param projectId the project id
     * @param eventCollection the collection name
     * @return a String that was previously stored for this project and collection or null
     * @throws IOException exception
     */
    public String getAttempts(String projectId, String eventCollection) throws IOException;

    /**
     * Set and stores the attempts String for a given project and collection.
     * @param projectId the project id
     * @param eventCollection the collection name
     * @param attemptsString the String to stored for this project and collection
     * @throws IOException exception
     * */
    public void setAttempts(String projectId, String eventCollection, String attemptsString) throws IOException;

}
