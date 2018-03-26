package com.viro.metrics.java;

import java.util.Map;

/**
 * An interface to simulate functional programming so that you can tell the {@link ViroKeenClient}
 * how to dynamically return Keen Global Properties based on event collection name.
 *
 * @author dkador
 * @since 1.0.0
 */
public interface ViroGlobalPropertiesEvaluator {

    /**
     * Gets a {@link Map} containing the global properties which should be applied to
     * a new event in the specified collection. This method will be called each time a new event is
     * created.
     *
     * @param eventCollection The name of the collection for which an event is being generated.
     * @return A {@link Map} containing the global properties which should be applied to
     * the event being generated.
     */
    Map<String, Object> getGlobalProperties(String eventCollection);

}
