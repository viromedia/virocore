package com.viro.renderer.jni.event;

/**
 * Callback interface for responding to status change events for a Controller. These occur when
 * a Controller is connected, disconnected, or enters an error state.
 */
public interface ControllerStatusListener {

    /**
     * Callback invoked when a Controller's status changes.
     *
     * @param source The platform specific source ID, which indicates what button or component
     *               on the Controller triggered the event. See the Controller's Guide for
     *               information.
     * @param status Indicates the new status of the Controller.
     */
    void onControllerStatus(int source, ControllerStatus status);
}