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

package com.viro.core.internal.font;

import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * FontFamily contains fonts that all share a given typeface and language, but differ
 * based on weight and style (normal vs. italic). Each font may be in a different file, or
 * they may appear together in a collection, in which case the fonts will have unique
 * TTC indices.
 */
public class FontFamily {

    private static String TAG = "Viro";

    /**
     * The name of the family (may be null).
     */
    private String mName;

    /**
     * The typeface used by the font family.
     */
    private String mTypeface;

    /**
     * The variant. Not yet supported by Viro.
     */
    private int mVariant;

    public static final class Font {
        private String mPath;
        private final int mIndex;
        private final FontVariationAxis[] mAxes;
        private final int mWeight;
        private final boolean mIsItalic;
        private final String mLangTag; // BCP-47 format

        public Font(String path, int ttcIndex, FontVariationAxis[] axes,
                    int weight, boolean isItalic, String langTag) {
            mPath = path;
            mIndex = ttcIndex;
            mAxes = axes;
            mWeight = weight;
            mIsItalic = isItalic;
            mLangTag = langTag;
        }

        public int getIndex() {
            return mIndex;
        }
        public FontVariationAxis[] getAxes() {
            return mAxes;
        }
        public int getWeight() {
            return mWeight;
        }
        public boolean isItalic() {
            return mIsItalic;
        }
        public String getPath() { return mPath; }
        public String getLangTag() { return mLangTag; }
    }

    private List<Font> mItalicFonts = new ArrayList<Font>();
    private List<Font> mNormalFonts = new ArrayList<Font>();

    public FontFamily(String name) {
        mName = name;
        mVariant = 0;
    }

    public FontFamily(String name, String lang, int variant) {
        mName = name;
        mVariant = variant;
        if (SystemFontLoader.DEBUG_FONTS) {
            Log.i(TAG, "Created font family [name: " + name + ", lang: " + lang + "]");
        }
    }

    public String getTypeface() {
        return mTypeface;
    }
    public String getName() {
        return mName;
    }

    public void addFont(String path, String langTag, FontConfig.Font fontConfig) {
        String fontFileName = fontConfig.getFontName();

        // Extract the typeface name from the font's file name
        String typeface = fontFileName;
        int indexOfHyphen = fontFileName.indexOf("-");
        if (indexOfHyphen > 0) {
            typeface = fontFileName.substring(0, indexOfHyphen);
        }

        // Set the typeface of this family to that of the first font added
        if (mTypeface == null) {
            mTypeface = typeface;
        }

        // Otherwise ensure this font is of the right typeface
        else if (!mTypeface.equals(typeface)) {
            Log.w(TAG, "Attempted to add font [file: " + fontFileName + ", typeface: " + typeface +
                    "] to font family with typeface [" + mTypeface +
                    "] -- typefaces do not match, font will be ignored");
            return;
        }

        // Create and add the font
        Font font = new Font(path, fontConfig.getTtcIndex(), fontConfig.getAxes(),
                fontConfig.getWeight(), fontConfig.isItalic(), langTag);
        if (fontConfig.isItalic()) {
            mItalicFonts.add(font);
        }
        else {
            mNormalFonts.add(font);
            if (SystemFontLoader.DEBUG_FONTS) {
                Log.i(TAG, "   Added font " + font.getPath() + " to typeface " + mTypeface);
            }
        }
    }

    /**
     * Get the closest font we have to the given weight and style. If there is no font matching the
     * style, we will return that with the closest weight of the default (normal) style. Returns
     * null if there are no fonts in this family.
     */
    public Font getFont(boolean isItalic, int weight) {
        // Match first by italic vs. not italic
        List<Font> candidates = isItalic ? mItalicFonts : mNormalFonts;
        if (isItalic && mItalicFonts.isEmpty()) {
            candidates = mNormalFonts;
        }

        return findClosestWeightFont(weight, candidates);
    }

    private Font findClosestWeightFont(int weight, List<Font> candidates) {
        Font closestFont = null;
        int closestFontWeightDistance = Integer.MAX_VALUE;

        for (Font font : candidates) {
            int weightDistance = Math.abs(font.getWeight() - weight);
            if (weightDistance < closestFontWeightDistance) {
                closestFont = font;
                closestFontWeightDistance = weightDistance;
            }
        }
        return closestFont;
    }

    /**
     * Add all the fonts from the given family into this one.
     */
    public void addAllFonts(FontFamily other) {
        for (Font font : other.mItalicFonts) {
            mItalicFonts.add(font);
        }
        for (Font font : other.mNormalFonts) {
            mNormalFonts.add(font);
        }
    }

}