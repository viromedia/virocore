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

/**
 * ViroKeenConstants
 *
 * @author dkador
 * @since 1.0.0
 */
public final class ViroKeenConstants {
    private ViroKeenConstants() {}

    static final String SERVER_ADDRESS = "https://api.keen.io";
    static final String API_VERSION = "3.0";

    // Keen API constants

    static final int MAX_EVENT_DEPTH = 1000;
    static final int DEFAULT_MAX_ATTEMPTS = 3;
    static final String NAME_PARAM = "name";
    static final String SUCCESS_PARAM = "success";
    static final String ERROR_PARAM = "error";
    static final String DESCRIPTION_PARAM = "description";
    static final String INVALID_COLLECTION_NAME_ERROR = "InvalidCollectionNameError";
    static final String INVALID_PROPERTY_NAME_ERROR = "InvalidPropertyNameError";
    static final String INVALID_PROPERTY_VALUE_ERROR = "InvalidPropertyValueError";

    // Exported constants

    public static final String KEEN_FAKE_JSON_ROOT = "io.keen.client.java.__fake_root";
}
