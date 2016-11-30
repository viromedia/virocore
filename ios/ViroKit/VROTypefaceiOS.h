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
#include "VROTypeface.h"
#include <string>

class VROTypefaceiOS : public VROTypeface {
    
public:
    
    VROTypefaceiOS(std::string name, int size);
    virtual ~VROTypefaceiOS();
    
    float getLineHeight();
    std::unique_ptr<VROGlyph> loadGlyph(FT_ULong charCode);
    
protected:
    
    FT_Face loadFace(std::string name, int size, FT_Library ft);
    
private:
    
    NSData *getFontData(CGFontRef cgFont);
    
};

#endif /* VROTypefaceiOS_h */
