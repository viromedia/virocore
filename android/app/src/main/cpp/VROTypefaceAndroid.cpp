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

VROTypefaceAndroid::VROTypefaceAndroid(std::string name, int size) :
        VROTypeface(name) {
    if (FT_Init_FreeType(&_ft)) {
        pabort("Could not initialize freetype library");
    }

    // TODO replace this, use the font name
    if (FT_New_Face(_ft, "/system/fonts/Roboto-Regular.ttf", 0, &_face)) {
        pabort("Failed to load font");
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