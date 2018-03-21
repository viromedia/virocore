package com.viro.metrics.java;

/**
 * ViroKeenConstants
 *
 * @author dkador
 * @since 1.0.0
 */
public final class ViroKeenConstants {
    private ViroKeenConstants() {}

    static final String SERVER_ADDRESS = "https://api.keen.io";
    static final String API_VERSION = "3.0";

    // Keen API constants

    static final int MAX_EVENT_DEPTH = 1000;
    static final int DEFAULT_MAX_ATTEMPTS = 3;
    static final String NAME_PARAM = "name";
    static final String SUCCESS_PARAM = "success";
    static final String ERROR_PARAM = "error";
    static final String DESCRIPTION_PARAM = "description";
    static final String INVALID_COLLECTION_NAME_ERROR = "InvalidCollectionNameError";
    static final String INVALID_PROPERTY_NAME_ERROR = "InvalidPropertyNameError";
    static final String INVALID_PROPERTY_VALUE_ERROR = "InvalidPropertyValueError";

    // Exported constants

    public static final String KEEN_FAKE_JSON_ROOT = "io.keen.client.java.__fake_root";
}
