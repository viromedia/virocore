package com.viro.metrics.java;

/**
 * This class implements the ViroKeenNetworkStatusHandler. It always returns true.
 *
 * @author Simon Murtha Smith
 * @since 2.1.0
 */
public class ViroAlwaysConnectedNetworkStatusHandlerViro implements ViroKeenNetworkStatusHandler {

    /**
     * Default implementation of the isNetworkConnected method.
     *
     * @return true, always
     */
    public boolean isNetworkConnected() {
        return true;
    }

}
