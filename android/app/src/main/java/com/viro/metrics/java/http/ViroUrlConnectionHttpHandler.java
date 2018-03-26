package com.viro.metrics.java.http;

import com.viro.metrics.java.ViroKeenUtils;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;

/**
 * This class provides a default implementation of {@link ViroHttpHandler} using
 * {@link HttpURLConnection}. To use a different HttpURLConnection implementation simply
 * override the {@link ViroUrlConnectionHttpHandler#execute(ViroRequest)}} method.
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public class ViroUrlConnectionHttpHandler implements ViroHttpHandler {

    /**
     * Sends an HTTP request.
     *
     * @param request The {@link ViroRequest} to send.
     * @return A {@link ViroResponse} object describing the response from the server.
     * @throws IOException If there was an error during the connection.
     */
    @Override
    public ViroResponse execute(ViroRequest request) throws IOException {
        HttpURLConnection connection = openConnection(request);
        sendRequest(connection, request);
        return readResponse(connection);
    }

    ///// PROTECTED METHODS /////

    /**
     * Opens a connection based on the URL in the given request.
     *
     * Subclasses can override this method to use a different implementation of
     * {@link HttpURLConnection}.
     *
     * @param request The {@link ViroRequest}.
     * @return A new {@link HttpURLConnection}.
     * @throws IOException If there is an error opening the connection.
     */
    protected HttpURLConnection openConnection(ViroRequest request) throws IOException {
        HttpURLConnection result;
        if (request.proxy != null) {
            result = (HttpURLConnection) request.url.openConnection(request.proxy);
        } else {
            result = (HttpURLConnection) request.url.openConnection();
        }
        result.setConnectTimeout(DEFAULT_CONNECT_TIMEOUT);
        result.setReadTimeout(DEFAULT_READ_TIMEOUT);
        return result;
    }

    /**
     * Sends a request over a given connection.
     *
     * @param connection The connection over which to send the request.
     * @param request The request to send.
     * @throws IOException If there is an error sending the request.
     */
    protected void sendRequest(HttpURLConnection connection, ViroRequest request) throws IOException {
        // Set up the request.
        connection.setRequestMethod(request.method);
        connection.setRequestProperty("Accept", "application/json");
        connection.setRequestProperty("Authorization", request.authorization);

        // If a different ViroHttpHandler is used, we won't get this header. We would need to refactor
        // to a delegation pattern to give the client code's ViroHttpHandler a chance to process the
        // ViroRequest first, then attach our custom headers, which would likely be a breaking change.
        connection.setRequestProperty("Keen-Sdk", "java-" + "5.2.0");

        // If the request has a body, send it. Otherwise just connect.
        if (request.body != null) {
            if (ViroHttpMethods.GET.equals(request.method) ||
                ViroHttpMethods.DELETE.equals(request.method)) {
                throw new IllegalStateException("Trying to send a GET request with a request " +
                                                "body, which would result in sending a POST.");
            }

            connection.setDoOutput(true);
            connection.setRequestProperty("Content-Type", "application/json");
            request.body.writeTo(connection.getOutputStream());
        } else {
            connection.connect();
        }
    }

    /**
     * Reads a {@link ViroResponse} from an existing connection. This method should only be
     * called on a connection for which the entire request has been sent.
     *
     * @param connection The connection that sent the response.
     * @return The {@link ViroResponse}
     * @throws IOException If there is an error reading the response from the connection.
     */
    protected ViroResponse readResponse(HttpURLConnection connection) throws IOException {
        // Try to get the input stream and if that fails, try to get the error stream.
        InputStream in;
        try {
            in = connection.getInputStream();
        } catch (IOException e) {
            in = connection.getErrorStream();
        }

        // If either stream is present, try to read the response body.
        String body = "";
        if (in != null) {
            try {
                body = ViroKeenUtils.convertStreamToString(in);
            } finally {
                ViroKeenUtils.closeQuietly(in);
            }
        }

        // Build and return the HTTP response object.
        return new ViroResponse(connection.getResponseCode(), body);
    }


    ///// PRIVATE CONSTANTS /////

    private static final int DEFAULT_CONNECT_TIMEOUT = 30000;
    private static final int DEFAULT_READ_TIMEOUT = 30000;

}
