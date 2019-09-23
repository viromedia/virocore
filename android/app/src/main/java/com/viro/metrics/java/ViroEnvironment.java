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
 * Exists solely to provide an abstraction around environment variables so we can actually test
 * them.
 *
 * @author Kevin Litwack, masojus
 * @since 1.0.0
 */
class ViroEnvironment {
    /**
     * Gets the Keen Project ID from the system environment.
     *
     * @return The Keen Project ID.
     */
    public String getKeenProjectId() {
        return getValue("KEEN_PROJECT_ID");
    }

    /**
     * Gets the Keen write key from the system environment.
     *
     * @return The Keen write key.
     */
    public String getKeenWriteKey() {
        return getValue("KEEN_WRITE_KEY");
    }

    /**
     * Gets the Keen read key from the system environment.
     *
     * @return The Keen read key.
     */
    public String getKeenReadKey() {
        return getValue("KEEN_READ_KEY");
    }

    /**
     * Gets the Keen read key from the system environment.
     *
     * @return The Keen read key.
     */
    public String getKeenMasterKey() {
        return getValue("KEEN_MASTER_KEY");
    }

    /**
     * Gets the specified property from the system environment.
     *
     * @param name The name of the property to get.
     * @return The value of the property.
     */
    private String getValue(String name) {
        return System.getenv().get(name);
    }
}
