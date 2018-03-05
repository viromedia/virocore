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
        return nullptr;
    }
    
    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                      std::string ceilingMaterial, std::string floorMaterial) {}
    
protected:
    
};

#endif /* VRODriverOpenGLWasm_h */
