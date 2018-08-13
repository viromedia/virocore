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
#include "VRODriverOpenGLWasm.h"

static const std::string kSystemFont = "Helvetica";

VROTypefaceWasm::VROTypefaceWasm(std::string name, int size, VROFontStyle style, VROFontWeight weight,
                                 std::shared_ptr<VRODriver> driver) :
        VROTypeface(name, size, style, weight),
        _driver(driver),
        _face(nullptr) {
}

VROTypefaceWasm::~VROTypefaceWasm() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (driver && _face != nullptr) {
        // FT crashes if we delete a face after the freetype library has been deleted
        if (std::dynamic_pointer_cast<VRODriverOpenGLWasm>(driver)->getFreetype() != nullptr) {
            FT_Done_Face(_face);
        }
    }
}

FT_FaceRec_ *VROTypefaceWasm::loadFTFace() {
    std::shared_ptr<VRODriver> driver = _driver.lock();
    if (!driver) {
        return nullptr;
    }
    
    FT_Library ft = std::dynamic_pointer_cast<VRODriverOpenGLWasm>(driver)->getFreetype();
    if (FT_New_Face(ft, getFontPath(getName()).c_str(), 0, &_face)) {
        if (FT_New_Face(ft, getFontPath(kSystemFont).c_str(), 0, &_face)) {
            pabort("Failed to load system font %s", kSystemFont.c_str());
        }
    }

    FT_Set_Pixel_Sizes(_face, 0, getSize());
    return _face;
}

float VROTypefaceWasm::getLineHeight() const {
    return _face->size->metrics.height >> 6;
}

std::shared_ptr<VROGlyph> VROTypefaceWasm::loadGlyph(uint32_t charCode, uint32_t variantSelector,
                                                     uint32_t outlineWidth, VROGlyphRenderMode renderMode) {
    std::shared_ptr<VROGlyph> glyph = std::make_shared<VROGlyphOpenGL>();
    std::shared_ptr<VRODriverOpenGLWasm> driver = std::dynamic_pointer_cast<VRODriverOpenGLWasm>(_driver.lock());
    if (!driver) {
        return glyph;
    }

    if (renderMode == VROGlyphRenderMode::None) {
        glyph->loadMetrics(_face, charCode, variantSelector);
    } else if (renderMode == VROGlyphRenderMode::Bitmap) {
        glyph->loadBitmap(_face, charCode, variantSelector, &_glyphAtlases, driver);
        if (outlineWidth > 0) {
            glyph->loadOutlineBitmap(driver->getFreetype(), _face, charCode, variantSelector, outlineWidth,
                                     &_outlineAtlases[outlineWidth], driver);
        }
    } else {
        glyph->loadVector(_face, charCode, variantSelector);
    }

    return glyph;
}

std::string VROTypefaceWasm::getFontPath(std::string fontName) {
    std::string prefix = "/";
    std::string suffix = ".ttc";

    return prefix + fontName + suffix;
}
