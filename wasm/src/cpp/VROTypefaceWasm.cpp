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

}

VROTypefaceWasm::~VROTypefaceWasm() {

}

FT_Face VROTypefaceWasm::loadFace(std::string name, int size, FT_Library ft) {
    FT_Face face;
    if (FT_New_Face(_ft, getFontPath(name).c_str(), 0, &face)) {
        if (FT_New_Face(_ft, getFontPath(kSystemFont).c_str(), 0, &face)) {
            pabort("Failed to load system font %s", kSystemFont.c_str());
        }
    }

    FT_Set_Pixel_Sizes(face, 0, size);
    return face;
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
