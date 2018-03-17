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
        _gvrAudio(gvrAudio),
        _ft(nullptr) {
    }
    virtual ~VRODriverOpenGLAndroid() {
        if (_ft != nullptr) {
            FT_Done_FreeType(_ft);
        }
    }

    virtual VROColorRenderingMode getColorRenderingMode() {
        switch (getGPUType()) {
            case VROGPUType::Adreno330OrOlder:
                return VROColorRenderingMode::NonLinear;
            default:
                // If the GPU doesn't support sRGB framebuffers then use software gamma correction
                return _sRGBFramebuffer ? VROColorRenderingMode::Linear : VROColorRenderingMode::LinearSoftware;
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

    FT_Library getFreetype() {
        return _ft;
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

protected:

    std::shared_ptr<VROTypeface> createTypeface(std::string typefaceName, int size, VROFontStyle style,
                                                VROFontWeight weight) {
        if (_ft == nullptr) {
            if (FT_Init_FreeType(&_ft)) {
                pabort("Could not initialize freetype library");
            }
        }
        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();
        std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceAndroid>(typefaceName, size, style,
                                                                                     weight, driver);
        typeface->loadFace();
        return typeface;
    }

private:

    bool _sRGBFramebuffer;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    FT_Library _ft;


};

#endif //ANDROID_VRODRIVEROPENGLANDROID_H
