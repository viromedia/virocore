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
import org.xmlpull.v1.XmlPullParserException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * Loads the system fonts from the Android device and exposes them. Enables the application to
 * query for a font based on typeface name, style, and weight.
 */
public class SystemFontLoader {

    private static String TAG = "Viro";
    public static boolean DEBUG_FONTS = false;

    /**
     * Maps typeface name to a {@link FontFamily}. Each {@link FontFamily} contains
     * the font with various styles and weights.
     */
    private static Map<String, FontFamily> sTypefaceFontMap;

    /**
     * Maps font family name to a {@link FontFamily}. Not all font families have names.
     */
    private static Map<String, FontFamily> sNameFontMap;

    /**
     * Maps font alias names to specific fonts.
     */
    private static Map<String, FontFamily.Font> sAliasedFonts;

    /**
     * The default font family.
     */
    static FontFamily sDefaultFamily;

    static final String FONTS_CONFIG = "fonts.xml";
    private static File getSystemFontConfigLocation() {
        return new File("/system/etc/");
    }

    /**
     * Find the closest matching font to the given typeface, weight, and style.
     */
    public static FontFamily.Font findFont(String typeface, boolean isItalic, int weight) {
        if (sTypefaceFontMap == null) {
            Log.w(TAG, "Font loader not initialized, default font will be used");
            return null;
        }
        FontFamily family = sTypefaceFontMap.get(typeface);
        if (family == null) {
            // Try searching the 'family name' map in case they specified a group like 'sans-serif-smallcaps'
            family = sNameFontMap.get(typeface);

            if (family == null) {
                // TODO Return a fallback for the desired language
                Log.w(TAG, "Font [" + typeface + "] not found, using default font instead");
                family = sDefaultFamily;
            }
        }

        FontFamily.Font font = family.getFont(isItalic, weight);
        if (font == null) {
            Log.w(TAG, "Font [" + typeface + "] of weight [" + weight + "] and italic [" + isItalic + "] not found, using default font instead");
            return sDefaultFamily.getFont(isItalic, weight);
        }
        else {
            return font;
        }
    }

    private static FontFamily makeFamilyFromParsed(FontConfig.Family family) {
        FontFamily fontFamily = new FontFamily(family.getName(), family.getLanguage(), family.getVariant());
        for (FontConfig.Font font : family.getFonts()) {
            String fullPathName = "/system/fonts/" + font.getFontName();
            fontFamily.addFont(fullPathName, family.getLanguage(), font);
        }
        return fontFamily;
    }

    /**
     * Run only once!
     */
    public static void init() {
        // Load font config and initialize Minikin state
        File systemFontConfigLocation = getSystemFontConfigLocation();
        File configFilename = new File(systemFontConfigLocation, FONTS_CONFIG);
        try {
            FileInputStream fontsIn = new FileInputStream(configFilename);
            FontConfig fontConfig = FontListParser.parse(fontsIn);

            // Iterate through the fonts and organize them by typeface, language, and name
            Map<String, FontFamily> fontsByTypeface = new HashMap<String, FontFamily>();
            Map<String, FontFamily> fontsByLang = new HashMap<String, FontFamily>();
            Map<String, FontFamily> fontsByName = new HashMap<String, FontFamily>();

            for (int i = 0; i < fontConfig.getFamilies().length; i++) {
                FontConfig.Family f = fontConfig.getFamilies()[i];
                FontFamily fontFamily = makeFamilyFromParsed(f);
                if (fontFamily == null || fontFamily.getTypeface() == null) {
                    continue;
                }

                // Check if a typeface for this font-family was already created; if so, add the
                // fonts to the existing family (this occurs if we're using one font family for
                // different languages, for example, as with NotoSansCJK).
                FontFamily existing = fontsByTypeface.get(fontFamily.getTypeface());
                if (existing != null) {
                    existing.addAllFonts(fontFamily);
                    fontFamily = existing;
                }

                if (i == 0) {
                    sDefaultFamily = fontFamily;
                }

                String typefaceName = fontFamily.getTypeface();
                if (typefaceName.endsWith(".ttf") || typefaceName.endsWith(".ttc")) {
                    typefaceName = typefaceName.substring(0, typefaceName.indexOf("."));
                }
                fontsByTypeface.put(typefaceName, fontFamily);

                // The default font for each language has no name
                if (f.getName() != null) {
                    fontsByName.put(fontFamily.getName(), fontFamily);
                }
            }

            // Process aliases: these are alternate names that should point to an
            // existing font (typeface name and, optionally, weight)
            Map<String, FontFamily.Font> fontsByAlias = new HashMap<String, FontFamily.Font>();
            for (FontConfig.Alias alias : fontConfig.getAliases()) {
                FontFamily toFamily = fontsByName.get(alias.getToName());
                int weight = alias.getWeight();

                FontFamily.Font font = toFamily.getFont(false, weight);
                if (font != null) {
                    fontsByAlias.put(alias.getName(), font);
                    if (DEBUG_FONTS) {
                        Log.i(TAG, "Aliased [" + alias.getName() + "] to font " + font.getPath());
                    }
                }
            }

            sTypefaceFontMap = fontsByTypeface;
            sNameFontMap = fontsByName;
            sAliasedFonts = fontsByAlias;

        } catch (RuntimeException e) {
            Log.w(TAG, "Didn't create default family (most likely, non-Minikin build)", e);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "Error opening " + configFilename, e);
        } catch (IOException e) {
            Log.e(TAG, "Error reading " + configFilename, e);
        } catch (XmlPullParserException e) {
            Log.e(TAG, "XML parse exception for " + configFilename, e);
        }
    }
}