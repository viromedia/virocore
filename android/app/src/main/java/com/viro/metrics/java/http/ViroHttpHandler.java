package com.viro.metrics.java.http;

import java.io.IOException;

/**
 * Interface which provides an abstraction around making HTTP requests.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public interface ViroHttpHandler {

    /**
     * Executes the given request and returns the received response.
     *
     * @param request   The {@link ViroRequest} to send.
     * @throws IOException If there is an error executing the request or processing the
     * response.
     * @return The {@link ViroResponse} received.
     */
    ViroResponse execute(ViroRequest request) throws IOException;

}