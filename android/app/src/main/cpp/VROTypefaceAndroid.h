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

class VROTypefaceAndroid : public VROTypeface {

public:

    VROTypefaceAndroid(std::string name, int size);
    virtual ~VROTypefaceAndroid();

    std::unique_ptr<VROGlyph> loadGlyph(FT_ULong charCode);

protected:

    FT_Face loadFace(std::string name, int size, FT_Library ft);

private:

    std::string getFontPath(std::string fontName);

};


#endif //ANDROID_VROTEXTFACEANDROID_H
