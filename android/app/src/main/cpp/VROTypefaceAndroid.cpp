//
//  VROTypefaceAndroid.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTypefaceAndroid.h"
#include "VROLog.h"
#include "VROGlyphOpenGL.h"

static const std::string kSystemFont = "Roboto-Regular";

VROTypefaceAndroid::VROTypefaceAndroid(std::string name, int size) :
        VROTypeface(name) {
    if (FT_Init_FreeType(&_ft)) {
        pabort("Could not initialize freetype library");
    }

    if (FT_New_Face(_ft, getFontPath(name).c_str(), 0, &_face)) {
        pinfo("Failed to load font %s, loading system font", name.c_str());

        if (FT_New_Face(_ft, getFontPath(kSystemFont).c_str(), 0, &_face)) {
            pabort("Failed to load system font %s", kSystemFont.c_str());
        }
    }

    FT_Set_Pixel_Sizes(_face, 0, size);
}

VROTypefaceAndroid::~VROTypefaceAndroid() {
    FT_Done_Face(_face);
    FT_Done_FreeType(_ft);
}

std::unique_ptr<VROGlyph> VROTypefaceAndroid::loadGlyph(FT_ULong charCode) {
    std::unique_ptr<VROGlyph> glyph = std::unique_ptr<VROGlyph>(new VROGlyphOpenGL());
    glyph->load(_face, charCode);

    return glyph;
}

std::string VROTypefaceAndroid::getFontPath(std::string fontName) {
    std::string prefix = "/system/fonts/";
    std::string suffix = ".ttf";

    return prefix + fontName + suffix;
}