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
#include "VROStringUtil.h"
#include "VROTypefaceCollection.h"

class VRODriverOpenGLAndroid : public VRODriverOpenGL {

public:

    VRODriverOpenGLAndroid(std::shared_ptr<gvr::AudioApi> gvrAudio) :
        _gvrAudio(gvrAudio),
        _ft(nullptr) {
    }
    virtual ~VRODriverOpenGLAndroid() {
        if (_ft != nullptr) {
            FT_Done_FreeType(_ft);
            _ft = nullptr;
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

    std::shared_ptr<VROTypefaceCollection> createTypefaceCollection(std::string typefaceNames, int size, VROFontStyle style,
                                                                    VROFontWeight weight) {
        if (_ft == nullptr) {
            if (FT_Init_FreeType(&_ft)) {
                pabort("Could not initialize freetype library");
            }
        }

        std::shared_ptr<VRODriverOpenGL> driver = shared_from_this();

        std::vector<std::shared_ptr<VROTypeface>> typefaces;
        std::vector<std::string> typefaceNamesSplit = VROStringUtil::split(typefaceNames, ",", true);

        for (std::string typefaceName : typefaceNamesSplit) {
            typefaceName = VROStringUtil::trim(typefaceName);

            std::pair<std::string, int> fileAndIndex = VROPlatformFindFont(typefaceName, style == VROFontStyle::Italic, (int) weight);
            std::string file = fileAndIndex.first;
            int index = fileAndIndex.second;

            // If we encounter a TTC file, load all the files in the collection
            if (VROStringUtil::endsWith(file, "ttc")) {
                // We have to load the first face in order to derive how many faces there are total
                std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceAndroid>(
                        typefaceName, file, 0, size, style, weight, driver);
                typeface->loadFace();
                typefaces.push_back(typeface);

                int numFaces = std::dynamic_pointer_cast<VROTypefaceAndroid>(typeface)->getNumFaces();
                pinfo("Loaded typeface %s from font collection, total %d faces", typefaceName.c_str(), numFaces);

                for (int i = 1; i < numFaces; i++) {
                    std::shared_ptr<VROTypeface> ttcTypeface = std::make_shared<VROTypefaceAndroid>(
                            typefaceName, file, i, size, style, weight, driver);
                    ttcTypeface->loadFace();
                    typefaces.push_back(ttcTypeface);
                }

            }
            // Otherwise just load the font we received
            else {
                std::shared_ptr<VROTypeface> typeface = std::make_shared<VROTypefaceAndroid>(
                        typefaceName, file, index, size, style, weight, driver);
                typeface->loadFace();
                typefaces.push_back(typeface);
            }
        }

        return std::make_shared<VROTypefaceCollection>(typefaces);
    }

private:

    bool _sRGBFramebuffer;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    FT_Library _ft;


};

#endif //ANDROID_VRODRIVEROPENGLANDROID_H
