//
//  VRODriverOpenGLWasm.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGLWasm_h
#define VRODriverOpenGLWasm_h

#include "VRODriverOpenGL.h"
#include "VRODisplayOpenGL.h"
#include "VROPlatformUtil.h"
#include "VROTypefaceWasm.h"
#include "VROTypefaceCollection.h"

class VRODriverOpenGLWasm : public VRODriverOpenGL {
    
public:
    
    VRODriverOpenGLWasm() {
    }
    
    virtual ~VRODriverOpenGLWasm() {
        if (_ft != nullptr) {
            FT_Done_FreeType(_ft);
            _ft = nullptr;
        }
    }
    
    void pause() {
       
    }
    
    void resume() {
       
    }
    
    virtual VROColorRenderingMode getColorRenderingMode() {
        return VROColorRenderingMode::Linear;
    }

    std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() {
        return nullptr;
    }

    std::shared_ptr<VROSound> newSound(std::string resource, VROResourceType resourceType, VROSoundType type) {
        return nullptr;
    }

    std::shared_ptr<VROSound> newSound(std::shared_ptr<VROSoundData> data, VROSoundType type) {
        return nullptr;
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string path, bool isLocal) {
        return nullptr;
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) {
        return nullptr;
    }
    
    FT_Library getFreetype() {
        return _ft;
    }
    
    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                      std::string ceilingMaterial, std::string floorMaterial) {}
    
protected:
    
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
            std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceWasm>(VROStringUtil::trim(typefaceName), size, style, weight, driver);
            typeface->loadFace();
            typefaces.push_back(typeface);
        }
        
        return std::make_shared<VROTypefaceCollection>(typefaces);
    }
    
};

#endif /* VRODriverOpenGLWasm_h */
