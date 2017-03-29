//
//  VROTypefaceAndroid.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROTEXTFACEANDROID_H
#define ANDROID_VROTEXTFACEANDROID_H

#include "VROTypeface.h"
#include <memory>
#include <string>

class VRODriver;

class VROTypefaceAndroid : public VROTypeface {

public:

    VROTypefaceAndroid(std::string name, int size, std::shared_ptr<VRODriver> driver);
    virtual ~VROTypefaceAndroid();

    std::shared_ptr<VROGlyph> loadGlyph(FT_ULong charCode, bool forRendering);

protected:

    FT_Face loadFace(std::string name, int size, FT_Library ft);

private:

    std::weak_ptr<VRODriver> _driver;
    std::string getFontPath(std::string fontName);

};


#endif //ANDROID_VROTEXTFACEANDROID_H
