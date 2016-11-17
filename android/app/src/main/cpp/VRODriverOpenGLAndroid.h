//
//  VRODriverOpenGLAndroid.h
//  ViroKit
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef ANDROID_VRODRIVEROPENGLANDROID_H
#define ANDROID_VRODRIVEROPENGLANDROID_H

#include "VRODriverOpenGL.h"
#include "VROLog.h"
#include "VROSoundEffectAndroid.h"
#include "vr/gvr/capi/include/gvr_audio.h"

class VRODriverOpenGLAndroid : public VRODriverOpenGL {

public:

    VRODriverOpenGLAndroid(std::shared_ptr<gvr::AudioApi> gvrAudio) :
        _gvrAudio(gvrAudio) {
    }
    virtual ~VRODriverOpenGLAndroid() { }

    VROVideoTextureCache *newVideoTextureCache() {
        pabort("Video texture caches not supported or required on Android");
        return nullptr;
    }

    std::shared_ptr<VROSoundEffect> newSoundEffect(std::string fileName) {
        auto it = _soundEffectMap.find(fileName);
        if (it == _soundEffectMap.end()) {
            std::shared_ptr<VROSoundEffect> effect = std::make_shared<VROSoundEffectAndroid>(fileName, _gvrAudio);
            _soundEffectMap[fileName] = effect;

            return effect;
        }
        else {
            return it->second;
        }
    }

private:

    std::shared_ptr<gvr::AudioApi> _gvrAudio;

    /*
     Sound effects are cached because we preload them based on filename. This
     way we can unload them when they're destroyed.
     */
    std::map<std::string, std::shared_ptr<VROSoundEffect>> _soundEffectMap;

};

#endif //ANDROID_VRODRIVEROPENGLANDROID_H
