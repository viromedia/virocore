package com.viro.metrics.java;

import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.util.Map;

/**
 * Interface for abstracting the tasks of converting an input {@link Reader} into an
 * in-memory object (in the form of a {@code Map&lt;String, Object&gt;}), and for writing that
 * object back out to a {@link Writer}.
 * <p>
 * This interface allows the Keen library to be configured to use different JSON implementations
 * in different environments, depending upon requirements for speed versus size (or other
 * considerations).
 * </p>
 * @author Kevin Litwack (kevin@kevinlitwack.com)
 * @since 2.0.0
 */
public interface ViroKeenJsonHandler {

    /**
     * Reads JSON-formatted data from the provided {@link Reader} and constructs a
     * {@link Map} representing the object described. The keys of the map should
     * correspond to the names of the top-level members, and the values may primitives (Strings,
     * Integers, Booleans, etc.), Maps, or Iterables.
     *
     * @param reader The {@link Reader} from which to read the JSON data.
     * @return The object which was read, held in a {@code Map<String, Object>}.
     * @throws IOException If there is an error reading from the input.
     */
    Map<String, Object> readJson(Reader reader) throws IOException;

    /**
     * Writes the given object (in the form of a {@code Map<String, Object>} to the specified
     * {@link Writer}.
     *
     * @param writer The {@link Writer} to which the JSON data should be written.
     * @param value  The object to write.
     * @throws IOException If there is an error writing to the output.
     */
    void writeJson(Writer writer, Map<String, ?> value) throws IOException;

}