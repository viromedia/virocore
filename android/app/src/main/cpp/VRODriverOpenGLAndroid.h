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

    virtual VROColorRenderingMode getColorRenderingMode() {
        switch (getGPUType()) {
            case VROGPUType::Adreno330OrOlder:
                return VROColorRenderingMode::NonLinear;
            default:
                // If the GPU doesn't support sRGB framebuffers then disable HDR entirely
                return _sRGBFramebuffer ? VROColorRenderingMode::Linear : VROColorRenderingMode::NonLinear;
        }
    }

    virtual bool isBloomSupported() {
        switch (getGPUType()) {
            case VROGPUType::Adreno330OrOlder:
                return false;
            default:
                return true;
        }
    }

    void willRenderFrame(const VRORenderContext &context) {
        _gvrAudio->SetHeadPose(VROGVRUtil::toGVRMat4f(context.getCamera().getLookAtMatrix()));
        _gvrAudio->Update();

        VRODriverOpenGL::willRenderFrame(context);
    }

    void pause() {
        _gvrAudio->Pause();
    }

    void resume() {
        _gvrAudio->Resume();
    }

    std::shared_ptr<VROVideoTextureCache> newVideoTextureCache() {
        pabort("Video texture caches not supported or required on Android");
        return nullptr;
    }

    std::shared_ptr<VROSound> newSound(std::shared_ptr<VROSoundData> data, VROSoundType type) {
        return VROSoundGVR::create(data, _gvrAudio, type);
    }

    std::shared_ptr<VROSound> newSound(std::string resource, VROResourceType resourceType, VROSoundType type) {
        return VROSoundGVR::create(resource, resourceType, _gvrAudio, type);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::shared_ptr<VROSoundData> data) {
        return std::make_shared<VROAudioPlayerAndroid>(data);
    }

    std::shared_ptr<VROAudioPlayer> newAudioPlayer(std::string fileName, bool isLocal) {
        return std::make_shared<VROAudioPlayerAndroid>(fileName);
    }

    std::shared_ptr<VROTypeface> newTypeface(std::string typefaceName, int size) {
        std::string key = typefaceName + "_" + VROStringUtil::toString(size);
        auto it = _typefaces.find(key);
        if (it == _typefaces.end()) {
            std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
            std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceAndroid>(typefaceName, size, driver);
            typeface->loadFace();

            _typefaces[key] = typeface;
            return typeface;
        }
        else {
            return it->second;
        }
    }

    void setSoundRoom(float sizeX, float sizeY, float sizeZ, std::string wallMaterial,
                              std::string ceilingMaterial, std::string floorMaterial) {
        if (sizeX == 0 && sizeY == 0 && sizeZ == 0) {
            _gvrAudio->EnableRoom(false);
        } else {
            _gvrAudio->EnableRoom(true);
            _gvrAudio->SetRoomProperties(sizeX, sizeY, sizeZ,
                                         (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(wallMaterial),
                                         (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(ceilingMaterial),
                                         (gvr_audio_material_type) VROPlatformParseGVRAudioMaterial(floorMaterial));
        }
    }

    void setSRGBFramebuffer(bool sRGBFramebuffer) {
        _sRGBFramebuffer = sRGBFramebuffer;
    }

private:

    bool _sRGBFramebuffer;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::map<std::string, std::shared_ptr<VROTypeface>> _typefaces;
};

#endif //ANDROID_VRODRIVEROPENGLANDROID_H
