package com.viro.metrics.java;

/**
 * An interface to allow callers to receive a callback on success or failure of an operation by the
 * keen client. This is most helpful for asynchronous calls (to detect completion of the
 * operation), but can also be used to be notified of failures, which are normally caught silently
 * by the client to prevent application crashes.
 *
 * @author Kevin Litwack
 * @since 2.0.0
 */
public interface ViroKeenCallback {

    /**
     * Invoked when the requested operation succeeds.
     */
    public void onSuccess();

    /**
     * Invoked when the requested operation fails.
     *
     * @param e An exception indicating the cause of the failure.
     */
    public void onFailure(Exception e);

}
