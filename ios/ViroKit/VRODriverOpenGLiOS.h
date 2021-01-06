//
//  VRODriverOpenGLiOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
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

#ifndef VRODriverOpenGLiOS_h
#define VRODriverOpenGLiOS_h

#include "VRODriverOpenGL.h"
#include "VROAudioPlayeriOS.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROTypefaceiOS.h"
#include "VROTypefaceCollection.h"
#include "VRODisplayOpenGLiOS.h"
#include "VROPlatformUtil.h"
#include "VROStringUtil.h"

class VRODriverOpenGLiOS : public VRODriverOpenGL {
    
public:
    
    /*
     If the driver is based off a GLKView, pass that in so we can create the
     display render target with it; otherwise, the display render target will
     be derived from the active framebuffer ID.
     */
    VRODriverOpenGLiOS(GLKView *viewGL, EAGLContext *eaglContext) :
        _viewGL(viewGL),
        _eaglContext(eaglContext),
         _ft(nullptr) {
    }
    
    virtual ~VRODriverOpenGLiOS() {
        if (_ft != nullptr) {
            FT_Done_FreeType(_ft);
            _ft = nullptr;
        }
    }
    void willRenderFrame(const VRORenderContext &context) {
        VRODriverOpenGL::willRenderFrame(context);
    }
    
    void pause() {
    }
    
    void resume() {
    }
    
    virtual VROColorRenderingMode getColorRenderingMode() {
        return VROColorRenderingMode::Linear;
    }
    
    /*
     On iOS the primary framebuffer (display) is tied to a GLKView. To
     bind the display we have to go through the GLKView.
     */
    virtual std::shared_ptr<VRORenderTarget> getDisplay() {
        if (!_display) {
            GLKView *viewGL = _viewGL;
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            _display = std::make_shared<VRODisplayOpenGLiOS>(viewGL, driver);
        }
        return _display;
    }

    std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return std::make_shared<VROVideoTextureCacheOpenGL>(_eaglContext, driver);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string path, bool isLocal) {
        // TODO: VIRO-756 make use of local flag (always assumes it's a web file)
        return std::make_shared<VROAudioPlayeriOS>(path, isLocal);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) {
        return std::make_shared<VROAudioPlayeriOS>(data);
    }
    
    FT_Library getFreetype() {
        return _ft;
    }
    
    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                      std::string ceilingMaterial, std::string floorMaterial);
    
    GLKView *getView() { return _viewGL; }
    
protected:
    
    __weak GLKView *_viewGL;
    EAGLContext *_eaglContext;
    std::map<std::string, std::weak_ptr<VROTypeface>> _typefaces;
    FT_Library _ft;
    
    std::shared_ptr<VROTypefaceCollection> createTypefaceCollection(std::string typefaceNames, int size, VROFontStyle style, VROFontWeight weight) {
        if (_ft == nullptr) {
            if (FT_Init_FreeType(&_ft)) {
                pabort("Could not initialize freetype library");
            }
        }
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        
        std::vector<std::shared_ptr<VROTypeface>> typefaces;
        std::vector<std::string> typefaceNamesSplit = VROStringUtil::split(typefaceNames, ",", true);
        for (std::string typefaceName : typefaceNamesSplit) {
            std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceiOS>(VROStringUtil::trim(typefaceName), size, style, weight, driver);
            typeface->loadFace();
            typefaces.push_back(typeface);
        }
        
        return std::make_shared<VROTypefaceCollection>(typefaces);
    }
    
};

#endif /* VRODriverOpenGLiOS_h */
