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
