package com.viro.metrics.java.exceptions;

/**
 * ViroServerException
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public class ViroServerException extends ViroKeenException {
    private static final long serialVersionUID = 3913819084183357142L;

    public ViroServerException() {
        super();
    }

    public ViroServerException(Throwable cause) {
        super(cause);
    }

    public ViroServerException(String message) {
        super(message);
    }

    public ViroServerException(String message, Throwable cause) {
        super(message, cause);
    }
}
