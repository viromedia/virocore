//
//  VRODriverOpenGLiOS.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VRODriverOpenGLiOS_h
#define VRODriverOpenGLiOS_h

#include "VRODriverOpenGL.h"
#include "VROSoundGVR.h"
#include "VROAudioPlayeriOS.h"
#include "VROVideoTextureCacheOpenGL.h"
#include "VROTypefaceiOS.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROPlatformUtil.h"

class VRODriverOpenGLiOS : public VRODriverOpenGL {
    
public:
    
    VRODriverOpenGLiOS(EAGLContext *eaglContext, std::shared_ptr<gvr::AudioApi> gvrAudio) :
    _eaglContext(eaglContext),
    _gvrAudio(gvrAudio) {
    }
    
    virtual ~VRODriverOpenGLiOS() { }
    
    VROVideoTextureCache *newVideoTextureCache() {
        return new VROVideoTextureCacheOpenGL(_eaglContext);
    }

    std::shared_ptr<VROSound> newSound(std::string path, VROSoundType type, bool local) {
        std::shared_ptr<VROSound> sound = VROSoundGVR::create(path, _gvrAudio, type, local);
        return sound;
    }

    std::shared_ptr<VROSound> newSound(std::shared_ptr<VROSoundData> data, VROSoundType type) {
        std::shared_ptr<VROSound> sound = VROSoundGVR::create(data, _gvrAudio, type);
        return sound;
    }

    
    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string path, bool isLocal) {
        // TODO: VIRO-756 make use of local flag (always assumes it's a web file)
        return std::make_shared<VROAudioPlayeriOS>(path, isLocal);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) {
        return VROAudioPlayeriOS::create(data);
    }
    
    std::shared_ptr<VROTypeface> newTypeface(std::string typefaceName, int size) {
        std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceiOS>(typefaceName, size);
        typeface->loadFace();
        
        return typeface;
    }

    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                      std::string ceilingMaterial, std::string floorMaterial) {
        _gvrAudio->SetRoomProperties(sizeX, sizeY, sizeZ,
                                     VROPlatformParseGVRAudioMaterial(wallMaterial),
                                     VROPlatformParseGVRAudioMaterial(ceilingMaterial),
                                     VROPlatformParseGVRAudioMaterial(floorMaterial));
    }
    
private:
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    EAGLContext *_eaglContext;
    
};

#endif /* VRODriverOpenGLiOS_h */
