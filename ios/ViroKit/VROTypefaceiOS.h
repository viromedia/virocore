//
//  VROTypefaceiOS.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VROTypefaceiOS_h
#define VROTypefaceiOS_h

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <CoreText/CoreText.h>
#include "VROTypeface.h"
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class VRODriver;

class VROTypefaceiOS : public VROTypeface {
    
public:
    
    VROTypefaceiOS(std::string name, int size, VROFontStyle style, VROFontWeight weight,
                   std::shared_ptr<VRODriver> driver);
    virtual ~VROTypefaceiOS();
    
    float getLineHeight() const;
    std::shared_ptr<VROGlyph> loadGlyph(uint32_t charCode, uint32_t variantSelector,
                                        uint32_t outlineWidth, VROGlyphRenderMode renderMode);

protected:
    
    FT_FaceRec_ *loadFTFace();
    
private:

    std::weak_ptr<VRODriver> _driver;
    FT_Face _face;
    
    // Font data must not be deallocated until the typeface is destroyed
    NSData *_fontData;
    NSData *getFontData(CTFontRef font);
    CTFontRef createFont(NSString *family, int size, VROFontStyle style, VROFontWeight weight);
  
};

#endif /* VROTypefaceiOS_h */
