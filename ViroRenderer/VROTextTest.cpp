//
//  VROTextTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROTextTest.h"
#include "VROTestUtil.h"
#include "VRODefines.h"
#include "VROTypeface.h"
#include "VROTypefaceCollection.h"

VROTextTest::VROTextTest() :
    VRORendererTest(VRORendererTestType::Text) {
    _textIndex = 0;
    _using3DText = false;
}

VROTextTest::~VROTextTest() {
    
}

void VROTextTest::build(std::shared_ptr<VRORenderer> renderer,
                        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                        std::shared_ptr<VRODriver> driver) {

    _driver = driver;
    std::wstring englishText = L"In older times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face.\n\nClose by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything.";
    
#if VRO_PLATFORM_IOS
    _textSamples.emplace_back(englishText,
                              "", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。",
                              "PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。",
                              "PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::Bold);
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。",
                              "PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::UltraLight);
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ",
                              "Apple SD Gothic Neo", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ",
                              "Apple SD Gothic Neo", 32, VROFontStyle::Normal, VROFontWeight::Heavy);
    _textSamples.emplace_back(L"あ い う え お か き く け こ さ し す せ そ が ぎ ぐ げ ご ぱ ぴ ぷ ぺ ぽ",
                              "Heiti TC", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"अ आ इ ई उ ऊ ए ऐ ओ औ अं अः क ख ग घ ङ च छ ज झ ञ ट ठ ड ढ ण त थ द ध न प फ",
                              "Devanagari Sangam MN", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"ان عدة الشهور عند الله اثنا عشر شهرا في كتاب الله يوم خلق السماوات والارض",
                              "Geeza Pro", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(englishText,
                              "", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(englishText,
                              "", 32, VROFontStyle::Italic, VROFontWeight::Regular);
    _textSamples.emplace_back(englishText,
                              "", 32, VROFontStyle::Normal, VROFontWeight::ExtraBlack);
    _textSamples.emplace_back(L"This is an example of text that is not mixed at all",
                              "", 32, VROFontStyle::Normal, VROFontWeight::Bold);
    _textSamples.emplace_back(L"This is an example of mixed text 他们赋 有理性和良心 that changes between 而自由 (Chinese) and English using two typefaces. 的精神互相对!!!",
                              ", PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::Bold);
    _textSamples.emplace_back(L"Variation sequence test: 葛 (U+845B) may also be represented as 󠄀葛 (U+845B; VS17/U+E0100)",
                              ", PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::Bold);
    
#elif VRO_PLATFORM_ANDROID
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。",
                              "NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。",
                              "NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::Bold);
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。",
                              "NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::UltraLight);
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ",
                              "NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ",
                              "NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::Heavy);
    _textSamples.emplace_back(L"あ い う え お か き く け こ さ し す せ そ が ぎ ぐ げ ご ぱ ぴ ぷ ぺ ぽ",
                              "NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"अ आ इ ई उ ऊ ए ऐ ओ औ अं अः क ख ग घ ङ च छ ज झ ञ ट ठ ड ढ ण त थ द ध न प फ",
                              "NotoSansDevanagari", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(L"ان عدة الشهور عند الله اثنا عشر شهرا في كتاب الله يوم خلق السماوات والارض",
                              "NotoNaskhArabic", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(englishText,
                              "Roboto", 32, VROFontStyle::Normal, VROFontWeight::Regular);
    _textSamples.emplace_back(englishText,
                              "Roboto", 32, VROFontStyle::Italic, VROFontWeight::Regular);
    _textSamples.emplace_back(englishText,
                              "Roboto", 32, VROFontStyle::Normal, VROFontWeight::ExtraBlack);
    _textSamples.emplace_back(L"This is an example of text that is not mixed at all",
                              "Roboto", 32, VROFontStyle::Normal, VROFontWeight::Bold);
    _textSamples.emplace_back(L"This is an example of mixed text 他们赋 有理性和良心 that changes between 而自由 (Chinese) and English using two typefaces. 的精神互相对!!!",
                              "Roboto, NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::Bold);
    _textSamples.emplace_back(L"Variation sequence test: 葛 (U+845B) may also be represented as 󠄀葛 (U+845B; VS17/U+E0100)",
                              "Roboto, NotoSansCJK", 32, VROFontStyle::Normal, VROFontWeight::Bold);
#endif

    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    /*
     KTX texture with text.
     */
    int width = 10;
    int height = 10;

    int fileLength;
    void *fileData = VROTestUtil::loadDataForResource("card_main", "ktx", &fileLength);
    
    VROTextureFormat format;
    int texWidth;
    int texHeight;
    std::vector<uint32_t> mipSizes;
    std::shared_ptr<VROData> texData = VROTextureUtil::readKTXHeader((uint8_t *)fileData, (uint32_t)fileLength,
                                                                     &format, &texWidth, &texHeight, &mipSizes);
    std::vector<std::shared_ptr<VROData>> dataVec = { texData };
    
    std::shared_ptr<VROTexture> texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, format,
                                                                       VROTextureInternalFormat::RGBA8, true,
                                                                       VROMipmapMode::Pregenerated,
                                                                       dataVec, texWidth, texHeight, mipSizes);
    texture->prewarm(driver);
    
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, 0.5, 0.5, 1, 1);
    surface->getMaterials().front()->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    surface->getMaterials().front()->getDiffuse().setTexture(texture);
    
    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setGeometry(surface);
    surfaceNode->setPosition({0, -10, -10.01});
    
    //rootNode->addChildNode(surfaceNode);
    free (fileData);
    
    /*
     Camera
     */
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    camera->setRotationType(VROCameraRotationType::Orbit);
    camera->setOrbitFocalPoint({ 0, 0, -6});
    
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setCamera(camera);
    rootNode->addChildNode(cameraNode);
    
    _pointOfView = cameraNode;
    
    /*
     Actual text.
     */
    _textNode = std::make_shared<VRONode>();
    rootNode->addChildNode(_textNode);
    rotateText();
    
    _eventDelegate = std::make_shared<VROTextEventDelegate>(this);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnPinch, true);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnRotate, true);
    rootNode->setEventDelegate(_eventDelegate);
}

void VROTextTest::rotateText() {
    int width = 5;
    int height = 5;

    VROTextSample &sample = _textSamples[_textIndex];

    VROLineBreakMode linebreakMode = VROLineBreakMode::Justify;
    VROTextClipMode clipMode = VROTextClipMode::ClipToBounds;
    
    float extrusion = _using3DText ? 8 : 0;
    std::shared_ptr<VROText> text = VROText::createText(sample.sample, sample.typefaceNames, sample.fontSize,
                                                        sample.fontStyle, sample.fontWeight, {1.0, 1.0, 1.0, 1.0}, extrusion, width, height,
                                                        VROTextHorizontalAlignment::Left, VROTextVerticalAlignment::Top,
                                                        linebreakMode, clipMode, 0, _driver);
    if (_using3DText) {
        text->getMaterials()[1]->getDiffuse().setColor({ 1.0, 1.0, 1.0, 1.0 });
        text->getMaterials()[2]->getDiffuse().setColor({ 1.0, 0.0, 1.0, 1.0 });
    } else {
        text->setOuterStroke(VROTextOuterStroke::DropShadow, 2, { 0.7, 0.7, 0.7, 1.0 });
    }
    _textNode->setGeometry(text);
    _textNode->setPosition({ 0, 0, -6 });
    _pointOfView->getCamera()->setOrbitFocalPoint({ 0, 0, -6});
    
    if (_textIndex == _textSamples.size() - 1) {
        _using3DText = !_using3DText;
    }
    _textIndex = (_textIndex + 1) % _textSamples.size();
}

void VROTextTest::scaleText(float scaleFactor) {
    _textNode->setScale(VROVector3f(scaleFactor, scaleFactor, scaleFactor));
}

void VROTextEventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                   std::vector<float> position) {
    if (clickState == ClickState::Clicked) {
        _test->rotateText();
    }
}

void VROTextEventDelegate::onPinch(int source, std::shared_ptr<VRONode> node, float scaleFactor, PinchState pinchState) {
    _test->scaleText(scaleFactor);
}

void VROTextEventDelegate::onRotate(int source, std::shared_ptr<VRONode> node, float rotationRadians, RotateState rotateState) {

}
