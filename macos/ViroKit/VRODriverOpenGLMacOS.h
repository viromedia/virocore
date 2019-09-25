//
//  VRODriverOpenGLMacOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGLMacOS_h
#define VRODriverOpenGLMacOS_h

#include "VRODriverOpenGL.h"
#include "VROSoundGVR.h"
#include "VROAudioPlayerMacOS.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROTypefaceiOS.h"
#include "VROTypefaceCollection.h"
#include "VROPlatformUtil.h"
#include "VROStringUtil.h"
#include <AppKit/AppKit.h>

class VRODriverOpenGLMacOS : public VRODriverOpenGL {
    
public:
    
    VRODriverOpenGLMacOS(NSOpenGLContext *glContext, NSOpenGLPixelFormat *pixelFormat) :
        _glContext(glContext),
        _pixelFormat(pixelFormat),
         _ft(nullptr) {
    }
    
    virtual ~VRODriverOpenGLMacOS() {
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

    std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        return std::make_shared<VROVideoTextureCacheOpenGL>([_glContext CGLContextObj], [_pixelFormat CGLPixelFormatObj], driver);
    }

    std::shared_ptr<VROSound> newSound(std::string resource, VROResourceType resourceType, VROSoundType type) {
        pabort("Spatial sound not supported on MacOS");
    }

    std::shared_ptr<VROSound> newSound(std::shared_ptr<VROSoundData> data, VROSoundType type) {
        pabort("Spatial sound not supported on MacOS");
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string path, bool isLocal) {
        // TODO: VIRO-756 make use of local flag (always assumes it's a web file)
        return std::make_shared<VROAudioPlayerMacOS>(path, isLocal);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) {
        return std::make_shared<VROAudioPlayerMacOS>(data);
    }
    
    FT_Library getFreetype() {
        return _ft;
    }
    
    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                      std::string ceilingMaterial, std::string floorMaterial) {}
    
    void *getGraphicsContext() {
        return (__bridge void *) _glContext;
    }
    
protected:
    
    NSOpenGLContext *_glContext;
    NSOpenGLPixelFormat *_pixelFormat;
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

#endif /* VRODriverOpenGLMacOS_h */
