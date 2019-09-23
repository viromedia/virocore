//
//  VROAnimatedTextureOpenGL.cpp
//  ViroKit
//
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROAnimatedTextureOpenGL.h"
#include "VROLog.h"
#include "VROTexture.h"
#include "VROData.h"
#include "VROModelIOUtil.h"
#include "VROTime.h"
#include "VROFrameSynchronizer.h"
#include <giflib/gif_lib.h>
#include "VRODriver.h"
#include "VROTextureSubstrate.h"
#include "VROFrameScheduler.h"
#include "VROStringUtil.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROPlatformUtil.h"

VROAnimatedTextureOpenGL::VROAnimatedTextureOpenGL(VROStereoMode state):
        VROTexture(VROTextureType::Texture2D, VROTextureInternalFormat::RGBA8, state) {
    // No-op
}

VROAnimatedTextureOpenGL::~VROAnimatedTextureOpenGL() {
    // No-op
}

void VROAnimatedTextureOpenGL::onFrameWillRender(const VRORenderContext &context) {
    animateTexture(VROTimeCurrentMillis());
}

void VROAnimatedTextureOpenGL::onFrameDidRender(const VRORenderContext &context) {
    // No-op
}

void VROAnimatedTextureOpenGL::play() {
    if (!_paused || _animatedFrameData.size() < 1) {
        return;
    }
    _paused = false;
    _processedAnimationStartTime = VROTimeCurrentMillis() - _processedTimeWhenPaused;
    _processedTimeWhenPaused = 0;
}

void VROAnimatedTextureOpenGL::pause() {
    if (_paused || _animatedFrameData.size() < 1) {
        return;
    }
    _paused = true;
    _processedTimeWhenPaused = VROTimeCurrentMillis() - _processedAnimationStartTime;
}

void VROAnimatedTextureOpenGL::setLoop(bool loop) {
    _loop = loop;
}

int VROAnimatedTextureOpenGL::getTotalAnimationDurationMs() {
    return _animatedTotalDuration;
}

void VROAnimatedTextureOpenGL::animateTexture(double globalCurrentTimeMs) {
    if (_paused) {
        return;
    }

    double globalEndTimeMs = _processedAnimationStartTime + _animatedTotalDuration;
    if (globalCurrentTimeMs > globalEndTimeMs && !_loop) {
        // Return if we no longer have to animate the texture.
        return;
    } else if (globalCurrentTimeMs > globalEndTimeMs && _loop) {
        // Else, reset the animation if it has ended and is looping.
        _processedAnimationStartTime = globalCurrentTimeMs;
    }

    /*
     Determine how far we have moved forward in the texture animation and
     update the _currentAnimFrame index.
     */
    int i = 0;
    double animationTimeMs = globalCurrentTimeMs - _processedAnimationStartTime;
    for (i = 0; i < _animatedFrameData.size() - 1; i++) {
        if (animationTimeMs > _animatedFrameData[i].timeStamp
            && animationTimeMs < _animatedFrameData[i + 1].timeStamp) {
            break;
        }
    }

    // Return if we have already rendered this frame.
    if (_currentAnimFrame == i) {
        return;
    }

    // Else, render the sub-section of the animated image as needed.
    _currentAnimFrame = i;
    GL( glBindTexture(GL_TEXTURE_2D, _initTexture) );
    GL( glTexSubImage2D(GL_TEXTURE_2D, 0,
                        _animatedFrameData[i].left, _animatedFrameData[i].top,
                        _animatedFrameData[i].width, _animatedFrameData[i].height,
                        GL_RGBA, GL_UNSIGNED_BYTE, _animatedFrameData[i].rawData->getData()) );
    GL( glBindTexture(GL_TEXTURE_2D, 0) );
}

void VROAnimatedTextureOpenGL::loadAnimatedSourceAsync(std::string sourcePath,
                                                       std::shared_ptr<VRODriver> driver,
                                                       std::shared_ptr<VROFrameSynchronizer> frameSync,
                                                       std::function<void(bool, std::string)> callback) {
    std::weak_ptr<VROTexture> wAnimTexture = shared_from_this();
    std::weak_ptr<VROFrameSynchronizer> wFrameSync = frameSync;
    std::weak_ptr<VRODriver> wDriver = driver;

    // Retrieve the image resource on the background thread.
    VROPlatformDispatchAsyncBackground([wAnimTexture, wFrameSync, wDriver, sourcePath, callback] {
        std::shared_ptr<VROTexture> currentTexture = wAnimTexture.lock();
        if (currentTexture == nullptr) {
            callback(false, "VROAnimatedTextureOpenGL has been destroyed.");
            return;
        }

        // Persist the animated gif into a temp local directory for processing.
        bool isTemp;
        bool success;
        std::string retrievedResourcePath = VROModelIOUtil::retrieveResource(sourcePath,
                                                                             VROResourceType::URL,
                                                                             &isTemp, &success);

        // Fail fast and return if we've failed to retrieve the resource.
        if (!success) {
            std::string failureMsg = "Failed to retrieve GIF resource texture at " + sourcePath;
            callback(false, failureMsg);
            return;
        }

        // Process raw image data into a VROAnimatedTexture
        std::shared_ptr<VROAnimatedTextureOpenGL> animTexture
                = std::dynamic_pointer_cast<VROAnimatedTextureOpenGL>(currentTexture);
        std::string failureMsg;
        bool parseSuccess = animTexture->parseGIFFile(retrievedResourcePath, failureMsg);
        if (!parseSuccess) {
            callback(false, failureMsg);
            return;
        }

        // Clean up local temp files after processing if needed.
        if (isTemp) {
            VROPlatformDeleteFile(retrievedResourcePath);
        }

        // Finally, initialize the texture and hook up listeners on the render thread.
        VROPlatformDispatchAsyncRenderer([wAnimTexture, wFrameSync, wDriver, callback] {
            std::shared_ptr<VROTexture> currentTexture = wAnimTexture.lock();
            std::shared_ptr<VROFrameSynchronizer> frameSync = wFrameSync.lock();
            std::shared_ptr<VRODriver> driver = wDriver.lock();
            if (currentTexture == nullptr || frameSync == nullptr || driver == nullptr) {
                callback(false, "VROAnimatedTextureOpenGL has been destroyed.");
                return;
            }

            std::shared_ptr<VROAnimatedTextureOpenGL> animTexture
                    = std::dynamic_pointer_cast<VROAnimatedTextureOpenGL>(currentTexture);
            animTexture->init(driver);
            frameSync->removeFrameListener(animTexture);
            if (animTexture->getTotalAnimationDurationMs() > 0) {
                frameSync->addFrameListener(animTexture);
            }
            callback(true, "");
        });
    });
}

bool VROAnimatedTextureOpenGL::parseGIFFile(std::string path, std::string &errorOut) {
    int error;
    GifFileType *gifFile = DGifOpenFileName(path.c_str(), &error);
    if (gifFile == NULL){
        errorOut = "Failed to open GIF file [error: " + VROStringUtil::toString(error) + "]";
        return false;
    }

    // Process the source into a GIFLIB format for us to parse through.
    if(DGifSlurp(gifFile) == GIF_ERROR) {
        errorOut = "Invalid GIF file: " + path;
        return false;
    }

    // TODO VIRO-4167: Support Interlaced GIFS.
    if (gifFile->Image.Interlace){
        errorOut = "Interlaced GIFs are not currently supported.";
        return false;
    }
    _height = gifFile->SHeight;
    _width = gifFile->SWidth;

    // Start iterating through each GIF frame and processing its data.
    double totalDuration = 0;
    unsigned int *lastFrameCache = new unsigned int[_width * _height];
    for (int frameIndex = 0; frameIndex < gifFile->ImageCount; frameIndex ++) {
        GraphicsControlBlock gcb;
        int ret = DGifSavedExtensionToGCB(gifFile, frameIndex, &gcb);
        if (ret != GIF_OK){
            errorOut = "Invalid GIF Graphics Control Block for multi-frame animation";
            return false;
        }

        // Process the current timestamp representing this GIF frame.
        int delayMs = gcb.DelayTime * 10;
        if (delayMs == 0){
            delayMs = 100;
        }
        AnimatedFrame animFrame;
        animFrame.timeStamp = totalDuration;
        totalDuration = totalDuration + delayMs;

        // Per frame image data is stored in a color map. Thus, attempt to grab the current
        // local color map, else grab the global one.
        ColorMapObject *colorMap = gifFile->SavedImages[frameIndex].ImageDesc.ColorMap;
        if (colorMap == NULL){
            colorMap = gifFile->SColorMap;
        }

        if (colorMap == NULL){
            errorOut = "Malformed GIF Color Palete detected in image!";
            return false;
        }

        // Finally, with the color map, parse out the current frame's raw image.
        SavedImage *frame = &gifFile->SavedImages[frameIndex];
        int frameWidth   = frame->ImageDesc.Width;
        int frameHeight  = frame->ImageDesc.Height;
        unsigned int *frameColorData = new unsigned int[frameWidth*frameHeight];
        for (int py = 0; py < frameHeight; py++) {
            for (int px = 0; px < frameWidth; px++) {
                unsigned int color = 0;
                int currentPixelIndex = py * frameWidth + px;
                auto clrIndex = frame->RasterBits[currentPixelIndex];
                if (clrIndex <= colorMap->ColorCount) {
                    auto &clrObj = colorMap->Colors[clrIndex];
                    auto r = clrObj.Red;
                    auto g = clrObj.Green;
                    auto b = clrObj.Blue;
                    auto alpha = clrIndex == gcb.TransparentColor ? 0x00 : 0xFF;
                    color = (alpha << 24) | (b << 16) | (g << 8) | r;
                } else {
                    pwarn("Viro: Invalid Color pallete found in Animated Texture.");
                }

                /*
                 In GIFs, individual frames with transparent regions can be used to reveal
                 pixel color data from previously rendered frames. This is often used as an
                 optimization in compressing GIF data. Unfortunately, we currently update frames
                 via glTexSubImage2D that effectively replaces the entire image region's pixel,
                 causing a visual defect when animating gifs (previous color data is lost).

                 To get around this, we simply keep a single "last frame cache" that at first
                 encompases the entire GIF image. We then update this cache with new pixel data
                 with each new parsed GIF frame - if and only if the pixel data of that
                 current sub frame is NOT transparent. We can then use this cache to render
                 future gif-transparent pixels, thereby effectively copying the behavior of
                 rendering previous frames as desired by GIF.
                 */
                int masterIndex = (py + gifFile->SavedImages[frameIndex].ImageDesc.Top) * gifFile->SWidth
                                  + (px + gifFile->SavedImages[frameIndex].ImageDesc.Left);
                if (frameIndex == 0) {
                    lastFrameCache[masterIndex] = color;
                }

                if (clrIndex == gcb.TransparentColor) {
                    if (gcb.DisposalMode == 2) {
                        // Restore to background color.
                        color = clrIndex == gifFile->SBackGroundColor;
                    } else  {
                        // Restore to Previous color.
                        color = lastFrameCache[masterIndex];
                    }
                } else {
                    lastFrameCache[masterIndex] = color;
                }

                frameColorData[currentPixelIndex] = color;
            }
        }

        std::vector<uint32_t> mipSizes;
        std::shared_ptr<VROData> texData = std::make_shared<VROData>((void *)frameColorData,
                                                                     frameWidth*frameHeight*sizeof(unsigned int),
                                                                     VRODataOwnership::Move);
        animFrame.rawData = texData;
        animFrame.top = gifFile->SavedImages[frameIndex].ImageDesc.Top;
        animFrame.left = gifFile->SavedImages[frameIndex].ImageDesc.Left;
        animFrame.width = frameWidth;
        animFrame.height = frameHeight;
        _animatedFrameData.push_back(animFrame);
    }

    _animatedTotalDuration = totalDuration;
    DGifCloseFile(gifFile, &error);
    return true;
}

void VROAnimatedTextureOpenGL::init(std::shared_ptr<VRODriver> driver) {
    _processedAnimationStartTime = 0;
    _currentAnimFrame = 0;
    _processedTimeWhenPaused = 0;
    _loop = true;
    _paused = false;

    // Initialize a single texture substrate for swapping and updating animation frames
    // via it's textureID that will be stored in _initTexture.
    std::vector<uint32_t> mipSizes;
    std::vector<std::shared_ptr<VROData>> data = {_animatedFrameData[0].rawData};
    VROTextureSubstrate *textureSub = driver->newTextureSubstrate(VROTextureType::Texture2D,
                                                                  VROTextureFormat::RGBA8,
                                                                  VROTextureInternalFormat::RGBA8,
                                                                  true,
                                                                  VROMipmapMode::None,
                                                                  data,
                                                                  _width,
                                                                  _height,
                                                                  mipSizes,
                                                                  VROWrapMode::Clamp,
                                                                  VROWrapMode::Clamp,
                                                                  VROFilterMode::Linear,
                                                                  VROFilterMode::Linear,
                                                                  VROFilterMode::Linear);
    _initTexture = ((VROTextureSubstrateOpenGL *) textureSub)->getTexture().second;

    // Configure the substrate to show the first animated frame.
    std::unique_ptr<VROTextureSubstrate> uniqueTextureSub = std::unique_ptr<VROTextureSubstrate>(textureSub);
    setSubstrate(0, std::move(uniqueTextureSub));
}

