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
#include "VRODriverOpenGL.h"

static const std::string kSystemFont = "Roboto-Regular";

VROTypefaceAndroid::VROTypefaceAndroid(std::string name, int size,
                                       std::shared_ptr<VRODriver> driver) :
    VROTypeface(name, size),
    _driver(driver) {
    if (FT_Init_FreeType(&_ft)) {
        pabort("Could not initialize freetype library");
    }
}

VROTypefaceAndroid::~VROTypefaceAndroid() {
    FT_Done_Face(_face);
    FT_Done_FreeType(_ft);
}

void VROTypefaceAndroid::loadFace(std::string name, int size) {
    pinfo("Loading font face [%s]", name.c_str());

    if (FT_New_Face(_ft, getFontPath(name, "ttf").c_str(), 0, &_face)) {
        if (FT_New_Face(_ft, getFontPath(name, "ttc").c_str(), 0, &_face)) {
            pinfo("Failed to load font face [%s], defaulting to system font", name.c_str());
            if (FT_New_Face(_ft, getFontPath(kSystemFont, "ttf").c_str(), 0, &_face)) {
                pabort("Failed to load system font %s", kSystemFont.c_str());
            }
        }
    }

    FT_Set_Pixel_Sizes(_face, 0, size);
}

std::shared_ptr<VROGlyph> VROTypefaceAndroid::loadGlyph(FT_ULong charCode, bool forRendering) {
    std::shared_ptr<VROGlyph> glyph = std::make_shared<VROGlyphOpenGL>();
    glyph->load(_face, charCode, forRendering, _driver.lock());

    return glyph;
}

std::string VROTypefaceAndroid::getFontPath(std::string fontName, std::string suffix) {
    std::string prefix = "/system/fonts/";
    return prefix + fontName + "." + suffix;
}

float VROTypefaceAndroid::getLineHeight() const {
    return _face->size->metrics.height >> 6;
}