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

import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.util.logging.StreamHandler;

/**
 * ViroKeenLogging is a wrapper around a logging module and provides, well, logging for the Keen Java SDK.
 * Logging is disabled by default so as not to clutter up your development experience.
 *
 * @author dkador
 * @since 1.0.0
 */
public class ViroKeenLogging {

    private static final Logger LOGGER;
    private static final StreamHandler HANDLER;

    static {
        LOGGER = Logger.getLogger(ViroKeenLogging.class.getName());
        HANDLER = new StreamHandler(System.out, new SimpleFormatter());
        LOGGER.addHandler(HANDLER);
        disableLogging();
    }

    static void log(String msg) {
        if (isLoggingEnabled()) {
            LOGGER.log(Level.FINER, msg);
            HANDLER.flush();
        }
    }

    /**
     * Call this to enable logging.
     */
    public static void enableLogging() {
        setLogLevel(Level.FINER);
    }

    /**
     * Call this to disable logging.
     */
    public static void disableLogging() {
        setLogLevel(Level.OFF);
    }
    
    /**
     * Disable the default log handler installed by the Keen Client to allow 
     * log management by standard j.u.l mechanisms.
     */
    public static void disableDefaultLogHandler(){
    	LOGGER.removeHandler(HANDLER);	//returns silently if given handler is null or not found
    }

    /**
     * Whether or not logging is enabled.
     *
     * @return a boolean saying whether or not logging is enabled
    */
    public static boolean isLoggingEnabled() {
        return LOGGER.getLevel() == Level.FINER;
    }

    private static void setLogLevel(Level newLevel) {
        LOGGER.setLevel(newLevel);
        for (Handler handler : LOGGER.getHandlers()) {
            try {
                handler.setLevel(newLevel);
            } catch (Throwable t) {
                // Ignore.
            }
        }
    }
}
