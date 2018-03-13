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

    VROTypefaceWasm(std::string name, int size, std::shared_ptr<VRODriver> driver);
    virtual ~VROTypefaceWasm();

    float getLineHeight() const;
    std::shared_ptr<VROGlyph> loadGlyph(FT_ULong charCode, bool forRendering);

protected:

    void loadFace(std::string name, int size);

private:

    std::weak_ptr<VRODriver> _driver;
    FT_Library _ft;
    FT_Face _face;
    
    std::string getFontPath(std::string fontName);

};


#endif //ANDROID_VROTEXTFACEWASM_H
