package com.viro.metrics.java.exceptions;

/**
 * ViroKeenException
 *
 * @author dkador
 * @since 1.0.0
 */
public abstract class ViroKeenException extends RuntimeException {
    private static final long serialVersionUID = -2830411036279774949L;

    public ViroKeenException() {
        super();
    }

    public ViroKeenException(Throwable cause) {
        super(cause);
    }

    public ViroKeenException(String message) {
        super(message);
    }

    public ViroKeenException(String message, Throwable cause) {
        super(message, cause);
    }
}
