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

class VRODriver;

class VROTypefaceWasm : public VROTypeface {

public:

    VROTypefaceWasm(std::string name, int size, std::shared_ptr<VRODriver> driver);
    virtual ~VROTypefaceWasm();

    std::shared_ptr<VROGlyph> loadGlyph(FT_ULong charCode, bool forRendering);

protected:

    FT_Face loadFace(std::string name, int size, FT_Library ft);

private:

    std::weak_ptr<VRODriver> _driver;
    std::string getFontPath(std::string fontName);

};


#endif //ANDROID_VROTEXTFACEWASM_H
