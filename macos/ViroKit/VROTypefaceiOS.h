//
//  VROTypefaceiOS.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

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
