package com.viro.metrics.java;

/**
 * Encapsulation of a single Keen project, including read/write/master keys.
 *
 * @author Kevin Litwack, masojus
 * @since 2.0.0
 */
public class ViroKeenProject {

    ///// PUBLIC CONSTRUCTORS /////

    public ViroKeenProject() {
        this(new ViroEnvironment());
    }

    ViroKeenProject(ViroEnvironment env) {
        this(env.getKeenProjectId(), env.getKeenWriteKey(), env.getKeenReadKey());
    }

    /**
     * Construct a Keen project.
     *
     * @param projectId The Keen IO Project ID.
     * @param writeKey  Your Keen IO Write Key. This may be null if this project will only be used
     *                  for reading events.
     * @param readKey   Your Keen IO Read Key. This may be null if this project will only be used
     *                  for writing events.
     */
    public ViroKeenProject(String projectId, String writeKey, String readKey) {
        this(projectId, writeKey, readKey, null);
    }

    /**
     * Construct a Keen project.
     *
     * @param projectId The Keen IO Project ID.
     * @param writeKey  Your Keen IO Write Key. This may be null if this project will only be used
     *                  for reading events.
     * @param readKey   Your Keen IO Read Key. This may be null if this project will only be used
     *                  for writing events.
     * @param masterKey Your Keen IO Master Key. This may be null if not using administrative
     *                  functionality that requires such a key.
     */
    public ViroKeenProject(String projectId, String writeKey, String readKey, String masterKey) {
        if (null == projectId || projectId.trim().isEmpty()) {
            throw new IllegalArgumentException("Invalid project id specified: " + projectId);
        }

        if ((null == writeKey || writeKey.trim().isEmpty()) &&
            (null == readKey || readKey.trim().isEmpty()) &&
            (null == masterKey || masterKey.trim().isEmpty())) {
            throw new IllegalArgumentException("At least one of the keys given must be non-null " +
                                               "and non-empty.");
        }

        this.projectId = projectId;
        this.writeKey = writeKey;
        this.readKey = readKey;
        this.masterKey = masterKey;
    }

    ///// PUBLIC METHODS /////

    /**
     * Getter for the Keen Project Id associated with this project.
     *
     * @return the Keen Project Id
     */
    public String getProjectId() {
        return projectId;
    }

    /**
     * Getter for the Keen Read Key associated with this project.
     *
     * @return the Keen Read Key
     */
    public String getReadKey() {
        return readKey;
    }

    /**
     * Getter for the Keen Write Key associated with this project.
     *
     * @return the Keen Write Key
     */
    public String getWriteKey() {
        return writeKey;
    }

    /**
     * Getter for the Keen Master Key associated with this project.
     *
     * @return the Keen Master Key
     */
    public String getMasterKey() {
        return masterKey;
    }

    ///// PRIVATE FIELDS /////

    private final String projectId;
    private final String readKey;
    private final String writeKey;
    private final String masterKey;
}
