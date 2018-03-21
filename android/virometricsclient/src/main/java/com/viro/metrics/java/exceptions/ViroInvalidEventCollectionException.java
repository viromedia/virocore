package com.viro.metrics.java.exceptions;

/**
 * ViroInvalidEventCollectionException
 *
 * @author dkador
 * @since 1.0.0
 */
public class ViroInvalidEventCollectionException extends ViroKeenException {
    private static final long serialVersionUID = 4340205614523528793L;

    public ViroInvalidEventCollectionException() {
        super();
    }

    public ViroInvalidEventCollectionException(Throwable cause) {
        super(cause);
    }

    public ViroInvalidEventCollectionException(String message) {
        super(message);
    }

    public ViroInvalidEventCollectionException(String message, Throwable cause) {
        super(message, cause);
    }
}
