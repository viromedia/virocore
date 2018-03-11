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

class VRODriverOpenGLWasm : public VRODriverOpenGL {
    
public:
    
    VRODriverOpenGLWasm() {
    }
    
    virtual ~VRODriverOpenGLWasm() { }
    
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
    
    std::shared_ptr<VROTypeface> newTypeface(std::string typefaceName, int size) {
        std::string key = typefaceName + "_" + VROStringUtil::toString(size);
        auto it = _typefaces.find(key);
        if (it == _typefaces.end()) {
            std::shared_ptr<VROTypeface> typeface = createTypeface(typefaceName, size);
            _typefaces[key] = typeface;
            return typeface;
        }
        else {
            std::shared_ptr<VROTypeface> typeface = it->second.lock();
            if (typeface) {
                return typeface;
            }
            else {
                typeface = createTypeface(typefaceName, size);
                _typefaces[key] = typeface;
                return typeface;
            }
        }
    }
    
    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                      std::string ceilingMaterial, std::string floorMaterial) {}
    
protected:
    
    std::map<std::string, std::weak_ptr<VROTypeface>> _typefaces;
    
    std::shared_ptr<VROTypeface> createTypeface(std::string typefaceName, int size) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceWasm>(typefaceName, size, driver);
        typeface->loadFace();
        return typeface;
    }
    
};

#endif /* VRODriverOpenGLWasm_h */
