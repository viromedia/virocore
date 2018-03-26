package com.viro.metrics.android;

import android.content.Context;
import android.content.SharedPreferences;

import java.util.UUID;

/**
 * Created by manish on 4/9/18.
 */

public class UniqueID {
    private static String uniqueID = null;
    private static final String PREF_UNIQUE_ID = "PREF_VIRO_UNIQUE_ID";

    public synchronized static String id(Context context) {
        if (uniqueID == null) {
            SharedPreferences sharedPrefs = context.getSharedPreferences(
                    PREF_UNIQUE_ID, Context.MODE_PRIVATE);
            uniqueID = sharedPrefs.getString(PREF_UNIQUE_ID, null);
            if (uniqueID == null) {
                uniqueID = UUID.randomUUID().toString();
                SharedPreferences.Editor editor = sharedPrefs.edit();
                editor.putString(PREF_UNIQUE_ID, uniqueID);
                editor.commit();
            }
        }
        return uniqueID;
    }
}
