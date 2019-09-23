//
//  Copyright (c) 2018-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.metrics.android;

import android.os.Build;

import com.viro.metrics.java.ViroKeenConstants;
import com.viro.metrics.java.ViroKeenJsonHandler;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

import java.io.IOException;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Implementation of the {@link ViroKeenJsonHandler} interface using the built-in
 * Android JSON library ({@link JSONObject}).
 *
 * @author Kevin Litwack (kevin@kevinlitwack.com), masojus
 * @since 2.0.0
 */
public class ViroAndroidJsonHandlerViro implements ViroKeenJsonHandler {

    ///// ViroKeenJsonHandler METHODS /////

    /**
     * {@inheritDoc}
     */
    @Override
    public Map<String, Object> readJson(Reader reader) throws IOException {
        if (reader == null) {
            throw new IllegalArgumentException("Reader must not be null");
        }

        String json = readerToString(reader);
        try {
            Object jsonObjOrArray = getJsonObjectManager().newTokener(json).nextValue();

            // Issue #99 : Take a look at better dealing with root Map<> vs root List<> in the
            // response.

            Object rootNode = ViroJsonHelper.fromJson(jsonObjOrArray);
            Map<String, Object> rootMap = null;

            if (null == rootNode) {
                throw new IllegalArgumentException("Empty reader or ill-formatted JSON " +
                                                   "encountered.");
            } else if (rootNode instanceof Map) {
                rootMap = (Map)rootNode;
            } else if (rootNode instanceof List) {
                rootMap = new LinkedHashMap<String, Object>();
                rootMap.put(ViroKeenConstants.KEEN_FAKE_JSON_ROOT, rootNode);
            }

            return rootMap;
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void writeJson(Writer writer, Map<String, ?> value) throws IOException {
        if (writer == null) {
            throw new IllegalArgumentException("Writer must not be null");
        }

        JSONObject jsonObject = convertMapToJSONObject(value);
        writer.write(getJsonObjectManager().stringify(jsonObject));
        writer.close();
    }

    /**
     * Sets whether or not this handler should wrap nested maps and collections explicitly. If set
     * to false, maps will be passed directly to JSONObject's constructor without any modification.
     *
     * Wrapping is necessary on older versions of Android due to a bug in the org.json
     * implementation. For details, see:
     *
     * https://code.google.com/p/android/issues/detail?id=55114
     *
     * In general, clients of the SDK should never change the default for this value unless they
     * have done extensive testing and understand the risks.
     *
     * @param value {@code true} to enable wrapping, {@code false} to disable it.
     */
    public void setWrapNestedMapsAndCollections(boolean value) {
        this.isWrapNestedMapsAndCollections = value;
    }

    ///// PROTECTED METHODS /////

    /**
     * Sets the {@link JsonObjectManager} instance to use. By default this class will simply use
     * the normal Android JSONObject methods, but this method may be used to provide a different
     * implementation, such as a stubbed/mocked implementation for unit testing.
     *
     * @param jsonObjectManager The {@link JsonObjectManager} instance to use.
     */
    protected void setJsonObjectManager(JsonObjectManager jsonObjectManager) {
        this.jsonObjectManager = jsonObjectManager;
    }

    ///// PROTECTED INNER CLASSES /////

    /**
     * Interface wrapping usage of JSONObjects.
     */
    protected interface JsonObjectManager {
        String stringify(JSONObject object);
        JSONObject newObject(Map<String, ?> map);
        JSONTokener newTokener(String json);
        JSONArray newArray(Collection<?> collection);
    }

    /**
     * Default implementation of JsonObjectManager which just uses the JSONObject methods directly.
     */
    private static class AndroidJsonObjectManager implements JsonObjectManager {
        @Override
        public String stringify(JSONObject object) {
            return object.toString();
        }

        @Override
        public JSONObject newObject(Map<String, ?> map) {
            return new JSONObject(map);
        }

        @Override
        public JSONTokener newTokener(String json) {
            return new JSONTokener(json);
        }

        @Override
        public JSONArray newArray(Collection<?> collection) {
            return new JSONArray(collection);
        }
    }

    ///// PRIVATE CONSTANTS /////

    /**
     * The size of the buffer to use when copying a reader to a string.
     */
    private static final int COPY_BUFFER_SIZE = 4 * 1024;

    ///// PRIVATE FIELDS /////

    /**
     * Boolean indicating whether or not to wrap maps/collections before passing to JSONObject.
     */
    private boolean isWrapNestedMapsAndCollections = (Build.VERSION.SDK_INT < 19);

    /**
     * Manager for creating JSONObjects and converting them to Strings; used for unit tests.
     */
    private JsonObjectManager jsonObjectManager = null;

    ///// PRIVATE METHODS /////

    /**
     * Get the default jsonObjectManager, or use one that was explicitly specified.
     *
     * @return A default implementation which uses the normal Android library methods, unless a
     * different implementation has been set explicitly.
     */
    private JsonObjectManager getJsonObjectManager() {
        if (jsonObjectManager == null) {
            jsonObjectManager = new AndroidJsonObjectManager();
        }
        return jsonObjectManager;
    }

    /**
     * Converts an input map to a JSONObject, wrapping any values in the map if wrapping is enabled
     * and the map contains wrappable values (i.e. nested maps or collections).
     *
     * @param map The map to convert.
     * @return A JSONObject representing the map.
     * @throws IOException if there is an error creating the JSONObject.
     */
    @SuppressWarnings("unchecked")
    private JSONObject convertMapToJSONObject(Map map) throws IOException {
        Map newMap;

        // Only perform wrapping if it's enabled and the input map requires it.
        if (isWrapNestedMapsAndCollections && requiresWrap(map)) {
            newMap = new HashMap<String, Object>();

            // Iterate through the elements in the input map.
            for (Object key : map.keySet()) {
                Object value = map.get(key);
                Object newValue = value;

                // Perform recursive conversions on maps and collections.
                if (value instanceof Map) {
                    newValue = convertMapToJSONObject((Map) value);
                } else if (value instanceof Collection) {
                    newValue = convertCollectionToJSONArray((Collection) value);
                } else if (value instanceof Object[]) {
                    newValue = convertCollectionToJSONArray(Arrays.asList((Object[]) value));
                }

                // Add the value to the new map.
                newMap.put(key, newValue);
            }
        } else {
            // Use the input map as-is.
            newMap = map;
        }

        // Pass the new map to the JSONObject constructor.
        return getJsonObjectManager().newObject(newMap);
    }

    /**
     * Converts an input collection to a JSONArray, wrapping any values in the collection if
     * wrapping is enabled and the collection contains wrappable values (i.e. maps or nested
     * collections).
     *
     * @param collection The collection to convert.
     * @return A JSONArray representing the collection.
     * @throws IOException if there is an error creating the JSONArray.
     */
    @SuppressWarnings("unchecked")
    private JSONArray convertCollectionToJSONArray(Collection collection) throws IOException {
        Collection newCollection;

        // Only perform wrapping if it's enabled and the input collection requires it.
        if (isWrapNestedMapsAndCollections && requiresWrap(collection)) {
            newCollection = new ArrayList<Object>();

            // Iterate through the elements in the input map.
            for (Object value : collection) {
                Object newValue = value;

                // Perform recursive conversions on maps and collections.
                if (value instanceof Map) {
                    newValue = convertMapToJSONObject((Map) value);
                } else if (value instanceof Collection) {
                    newValue = convertCollectionToJSONArray((Collection) value);
                }

                // Add the value to the new collection.
                newCollection.add(newValue);
            }
        } else {
            // Use the input collection as-is.
            newCollection = collection;
        }

        // Pass the new collection to the JSONArray constructor.
        return getJsonObjectManager().newArray(newCollection);
    }

    /**
     * Converts a Reader to a String by copying the Reader's contents into a StringWriter via a
     * buffer.
     *
     * @param reader The Reader from which to extract a String.
     * @return The String contained in the Reader.
     * @throws IOException If there is an error reading from the input Reader.
     */
    private static String readerToString(Reader reader) throws IOException {
        StringWriter writer = new StringWriter();
        try {
            char[] buffer = new char[COPY_BUFFER_SIZE];
            while (true) {
                int bytesRead = reader.read(buffer);
                if (bytesRead == -1) {
                    break;
                } else {
                    writer.write(buffer, 0, bytesRead);
                }
            }
            return writer.toString();
        } finally {
            reader.close();
        }
    }

    /**
     * Checks whether a map requires any wrapping. This is used to avoid creating a copy of a map
     * if all of its fields can be handled natively.
     *
     * @param map The map to check for values that require wrapping.
     * @return {@code true} if the map contains values that need to be wrapped (i.e. maps or
     * collections), otherwise {@code false}.
     */
    private static boolean requiresWrap(Map map) {
        for (Object value : map.values()) {
            if (value instanceof Collection || value instanceof Map || value instanceof Object[]) {
                return true;
            }
        }
        return false;
    }

    /**
     * Checks whether a collection requires any wrapping. This is used to avoid creating a copy of a
     * collection if all of its fields can be handled natively.
     *
     * @param collection The collection to check for values that require wrapping.
     * @return {@code true} if the collection contains values that need to be wrapped (i.e. maps or
     * collections), otherwise {@code false}.
     */
    private static boolean requiresWrap(Collection collection) {
        for (Object value : collection) {
            if (value instanceof Collection || value instanceof Map) {
                return true;
            }
        }
        return false;
    }

}
