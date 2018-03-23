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
#include <ft2build.h>
#include FT_FREETYPE_H

class VRODriver;

class VROTypefaceAndroid : public VROTypeface {

public:

    VROTypefaceAndroid(std::string name, std::string file, int index, int size,
                       VROFontStyle style, VROFontWeight weight,
                       std::shared_ptr<VRODriver> driver);
    virtual ~VROTypefaceAndroid();

    float getLineHeight() const;
    std::shared_ptr<VROGlyph> loadGlyph(uint32_t charCode, uint32_t variantSelector, bool forRendering);

    // If this typeface was sourced from a TTC file, this will return the number of other faces
    // in the TTC
    int getNumFaces() const { return _numFaces; }

protected:

    void loadFace(std::string name, int size);

private:

    std::weak_ptr<VRODriver> _driver;
    FT_Face _face;
    std::string _file;
    int _index;
    int _numFaces;

    std::string getFontPath(std::string fontName, std::string suffix);

};


#endif //ANDROID_VROTEXTFACEANDROID_H
