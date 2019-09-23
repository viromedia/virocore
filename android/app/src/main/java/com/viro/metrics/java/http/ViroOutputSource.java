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

import java.io.IOException;
import java.io.OutputStream;

/**
 * Interface to allow writing to an {@link OutputStream} - in particular the output stream
 * of an HTTP request - directly from a source. This avoids having to write into a buffer, pass the
 * buffer to the request handler, and then have the handler copy the buffer to the connection's
 * output stream.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public interface ViroOutputSource {

    /**
     * Writes data to the given {@link OutputStream}. This method is used to send the body
     * of an HTTP request.
     * <p>
     * Implementers of this method should NOT close {@code out}; the calling code will ensure
     * that it is closed.
     *
     * @param out The {@link OutputStream} to which the request body should be written.
     * @throws IOException Implementers may throw an IOException if an error is encountered
     *                     while writing.
     */
    void writeTo(OutputStream out) throws IOException;

}
