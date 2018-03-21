package com.viro.metrics.java.exceptions;

/**
 * ViroInvalidEventException
 *
 * @author dkador
 * @since 1.0.0
 */
public class ViroInvalidEventException extends ViroKeenException {
    private static final long serialVersionUID = -8714276749665293346L;

    public ViroInvalidEventException() {
        super();
    }

    public ViroInvalidEventException(Throwable cause) {
        super(cause);
    }

    public ViroInvalidEventException(String message) {
        super(message);
    }

    public ViroInvalidEventException(String message, Throwable cause) {
        super(message, cause);
    }
}
