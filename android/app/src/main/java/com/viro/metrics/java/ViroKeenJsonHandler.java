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
import java.io.Reader;
import java.io.Writer;
import java.util.Map;

/**
 * Interface for abstracting the tasks of converting an input {@link Reader} into an
 * in-memory object (in the form of a {@code Map&lt;String, Object&gt;}), and for writing that
 * object back out to a {@link Writer}.
 * <p>
 * This interface allows the Keen library to be configured to use different JSON implementations
 * in different environments, depending upon requirements for speed versus size (or other
 * considerations).
 * </p>
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public interface ViroKeenJsonHandler {

    /**
     * Reads JSON-formatted data from the provided {@link Reader} and constructs a
     * {@link Map} representing the object described. The keys of the map should
     * correspond to the names of the top-level members, and the values may primitives (Strings,
     * Integers, Booleans, etc.), Maps, or Iterables.
     *
     * @param reader The {@link Reader} from which to read the JSON data.
     * @return The object which was read, held in a {@code Map<String, Object>}.
     * @throws IOException If there is an error reading from the input.
     */
    Map<String, Object> readJson(Reader reader) throws IOException;

    /**
     * Writes the given object (in the form of a {@code Map<String, Object>} to the specified
     * {@link Writer}.
     *
     * @param writer The {@link Writer} to which the JSON data should be written.
     * @param value  The object to write.
     * @throws IOException If there is an error writing to the output.
     */
    void writeJson(Writer writer, Map<String, ?> value) throws IOException;

}