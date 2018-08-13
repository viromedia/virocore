//
//  VROTypefaceWasm.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/11/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROTEXTFACEWASM_H
#define ANDROID_VROTEXTFACEWASM_H

#include "VROTypeface.h"
#include <memory>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class VRODriver;

class VROTypefaceWasm : public VROTypeface {

public:

    VROTypefaceWasm(std::string name, int size, VROFontStyle style, VROFontWeight weight,
                    std::shared_ptr<VRODriver> driver);
    virtual ~VROTypefaceWasm();

    float getLineHeight() const;
    std::shared_ptr<VROGlyph> loadGlyph(uint32_t charCode, uint32_t variantSelector,
                                        uint32_t outlineWidth, VROGlyphRenderMode renderMode);

protected:

    FT_FaceRec_ *loadFTFace();

private:

    std::weak_ptr<VRODriver> _driver;
    FT_Face _face;
    
    std::string getFontPath(std::string fontName);

};


#endif //ANDROID_VROTEXTFACEWASM_H
