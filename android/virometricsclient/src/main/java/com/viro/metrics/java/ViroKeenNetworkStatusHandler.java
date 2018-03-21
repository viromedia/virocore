package com.viro.metrics.java;

/**
 * This interface holds methods for checking network conditions.
 *
 * @author Simon Murtha Smith
 * @since 2.1.0
 */
public interface ViroKeenNetworkStatusHandler {

    /**
     * Reports on whether there is a network connection
     *
     * @return The object which was read, held in a {@code Map<String, Object>}.
     */
    public boolean isNetworkConnected();

}
