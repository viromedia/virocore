//
//  VROTypefaceWasm.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/11/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTypefaceWasm.h"
#include "VROLog.h"
#include "VROGlyphOpenGL.h"
#include "VRODriverOpenGL.h"

static const std::string kSystemFont = "Helvetica";

VROTypefaceWasm::VROTypefaceWasm(std::string name, int size,
                                       std::shared_ptr<VRODriver> driver) :
        VROTypeface(name, size),
        _driver(driver) {

    if (FT_Init_FreeType(&_ft)) {
        pabort("Could not initialize freetype library");
    }
}

VROTypefaceWasm::~VROTypefaceWasm() {
    FT_Done_Face(_face);
    FT_Done_FreeType(_ft);
}

void VROTypefaceWasm::loadFace(std::string name, int size) {
    if (FT_New_Face(_ft, getFontPath(name).c_str(), 0, &_face)) {
        if (FT_New_Face(_ft, getFontPath(kSystemFont).c_str(), 0, &_face)) {
            pabort("Failed to load system font %s", kSystemFont.c_str());
        }
    }

    FT_Set_Pixel_Sizes(_face, 0, size);
}

float VROTypefaceWasm::getLineHeight() const {
    return _face->size->metrics.height >> 6;
}

std::shared_ptr<VROGlyph> VROTypefaceWasm::loadGlyph(FT_ULong charCode, bool forRendering) {
    std::shared_ptr<VROGlyph> glyph = std::make_shared<VROGlyphOpenGL>();
    glyph->load(_face, charCode, forRendering, _driver.lock());

    return glyph;
}

std::string VROTypefaceWasm::getFontPath(std::string fontName) {
    std::string prefix = "/";
    std::string suffix = ".ttc";

    return prefix + fontName + suffix;
}
