//
//  VRODriverOpenGLAndroid.h
//  ViroKit
//
//  Created by Raj Advani on 11/8/16.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef ANDROID_VRODRIVEROPENGLANDROID_H
#define ANDROID_VRODRIVEROPENGLANDROID_H

#include <VROSoundGVR.h>
#include "VRODriverOpenGL.h"
#include "VROLog.h"
#include "VROGVRUtil.h"
#include "VROAudioPlayerAndroid.h"
#include "VROTypefaceAndroid.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROPlatformUtil.h"

class VRODriverOpenGLAndroid : public VRODriverOpenGL {

public:

    VRODriverOpenGLAndroid(std::shared_ptr<gvr::AudioApi> gvrAudio) :
        _gvrAudio(gvrAudio) {
    }
    virtual ~VRODriverOpenGLAndroid() { }

    void onFrame(const VRORenderContext &context) {
        _gvrAudio->SetHeadPose(VROGVRUtil::toGVRMat4f(context.getCamera().getLookAtMatrix()));
        _gvrAudio->Update();
    }

    void onPause() {
        _gvrAudio->Pause();
    }

    void onResume() {
        _gvrAudio->Resume();
    }

    std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() {
        pabort("Video texture caches not supported or required on Android");
        return nullptr;
    }

    std::shared_ptr<VROSound> newSound(std::shared_ptr<VROSoundData> data, VROSoundType type) {
        return VROSoundGVR::create(data, _gvrAudio, type);
    }

    std::shared_ptr<VROSound> newSound(std::string path, VROSoundType type, bool local) {
        return VROSoundGVR::create(path, _gvrAudio, type, local);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) {
        return std::make_shared<VROAudioPlayerAndroid>(data);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string fileName, bool isLocal) {
        return std::make_shared<VROAudioPlayerAndroid>(fileName);
    }

    std::shared_ptr<VROTypeface> newTypeface(std::string typefaceName, int size) {
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceAndroid>(typefaceName, size, driver);
        typeface->loadFace();

        return typeface;
    }

    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                              std::string ceilingMaterial, std::string floorMaterial) {
        if (sizeX == 0 && sizeY == 0 && sizeZ == 0) {
            _gvrAudio->EnableRoom(false);
        } else {
            _gvrAudio->EnableRoom(true);
            _gvrAudio->SetRoomProperties(sizeX, sizeY, sizeZ,
                                         VROPlatformParseGVRAudioMaterial(wallMaterial),
                                         VROPlatformParseGVRAudioMaterial(ceilingMaterial),
                                         VROPlatformParseGVRAudioMaterial(floorMaterial));
        }
    }

private:

    std::shared_ptr<gvr::AudioApi> _gvrAudio;
};

#endif //ANDROID_VRODRIVEROPENGLANDROID_H
