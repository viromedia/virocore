//
//  VROScreen.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#ifndef VROScreen_h
#define VROScreen_h

#include "VRODefines.h"
#if VRO_METAL

#include <stdio.h>
#include <UIKit/UIKit.h>

class VROScreen {
    
public:
    
    VROScreen(UIScreen *screen);
    
    int getWidth() const;
    int getHeight() const;
    
    float getWidthInMeters() const;
    float getHeightInMeters() const;
    
    void setBorderSizeInMeters(float screenBorderSize);
    float getBorderSizeInMeters() const;
    
private:
    
    UIScreen *_screen;
    CGFloat _scale;
    float _xMetersPerPixel;
    float _yMetersPerPixel;
    float _borderSizeMeters;
    
    float pixelsPerInch(UIScreen *screen);
    
};

#endif
#endif /* VROScreen_h */
