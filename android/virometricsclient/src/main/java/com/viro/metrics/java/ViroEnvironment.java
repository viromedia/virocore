package com.viro.metrics.java;

/**
 * Exists solely to provide an abstraction around environment variables so we can actually test
 * them.
 *
 * @author Kevin Litwack, masojus
 * @since 1.0.0
 */
class ViroEnvironment {
    /**
     * Gets the Keen Project ID from the system environment.
     *
     * @return The Keen Project ID.
     */
    public String getKeenProjectId() {
        return getValue("KEEN_PROJECT_ID");
    }

    /**
     * Gets the Keen write key from the system environment.
     *
     * @return The Keen write key.
     */
    public String getKeenWriteKey() {
        return getValue("KEEN_WRITE_KEY");
    }

    /**
     * Gets the Keen read key from the system environment.
     *
     * @return The Keen read key.
     */
    public String getKeenReadKey() {
        return getValue("KEEN_READ_KEY");
    }

    /**
     * Gets the Keen read key from the system environment.
     *
     * @return The Keen read key.
     */
    public String getKeenMasterKey() {
        return getValue("KEEN_MASTER_KEY");
    }

    /**
     * Gets the specified property from the system environment.
     *
     * @param name The name of the property to get.
     * @return The value of the property.
     */
    private String getValue(String name) {
        return System.getenv().get(name);
    }
}
