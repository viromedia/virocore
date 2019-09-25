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

package com.viro.metrics.java.http;

/**
 * Encapsulates an HTTP response.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public final class ViroResponse {

    ///// PROPERTIES /////

    public final int statusCode;
    public final String body;

    ///// PUBLIC CONSTRUCTORS /////

    public ViroResponse(int statusCode, String body) {
        this.statusCode = statusCode;
        this.body = body;
    }

    ///// PUBLIC METHODS /////

    public boolean isSuccess() {
        return isSuccessCode(statusCode);
    }

    ///// PRIVATE STATIC METHODS /////

    /**
     * Checks whether an HTTP status code indicates success.
     *
     * @param statusCode The HTTP status code.
     * @return {@code true} if the status code indicates success (2xx), otherwise {@code false}.
     */
    private static boolean isSuccessCode(int statusCode) {
        return (statusCode / 100 == 2);
    }

}