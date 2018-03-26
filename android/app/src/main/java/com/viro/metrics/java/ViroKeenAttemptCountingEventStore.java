package com.viro.metrics.java;

import java.io.IOException;

/**
 * Interface which provides an abstraction layer around how attempt counts are stored.
 *
 * @author Simon Murtha Smith
 * @since 2.0.2
 */
public interface ViroKeenAttemptCountingEventStore extends ViroKeenEventStore {

    /**
     * Gets the stored attempts String for a given project and collection.
     *
     * @param projectId the project id
     * @param eventCollection the collection name
     * @return a String that was previously stored for this project and collection or null
     * @throws IOException exception
     */
    public String getAttempts(String projectId, String eventCollection) throws IOException;

    /**
     * Set and stores the attempts String for a given project and collection.
     * @param projectId the project id
     * @param eventCollection the collection name
     * @param attemptsString the String to stored for this project and collection
     * @throws IOException exception
     * */
    public void setAttempts(String projectId, String eventCollection, String attemptsString) throws IOException;

}
