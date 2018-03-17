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

VROTextTest::VROTextTest() :
    VRORendererTest(VRORendererTestType::Text) {
    _textIndex = 0;
}

VROTextTest::~VROTextTest() {
    
}

void VROTextTest::build(std::shared_ptr<VRORenderer> renderer,
                        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                        std::shared_ptr<VRODriver> driver) {

    std::wstring englishText = L"In older times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face.\n\nClose by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything.";
    
#if VRO_PLATFORM_IOS
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。", driver->newTypeface("PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。", driver->newTypeface("PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::Bold));
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。", driver->newTypeface("PingFang HK", 32, VROFontStyle::Normal, VROFontWeight::UltraLight));
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ", driver->newTypeface("Apple SD Gothic Neo", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ", driver->newTypeface("Apple SD Gothic Neo", 32, VROFontStyle::Normal, VROFontWeight::Heavy));
    _textSamples.emplace_back(L"あ い う え お か き く け こ さ し す せ そ が ぎ ぐ げ ご ぱ ぴ ぷ ぺ ぽ", driver->newTypeface("Heiti TC", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"अ आ इ ई उ ऊ ए ऐ ओ औ अं अः क ख ग घ ङ च छ ज झ ञ ट ठ ड ढ ण त थ द ध न प फ", driver->newTypeface("Devanagari Sangam MN", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"ان عدة الشهور عند الله اثنا عشر شهرا في كتاب الله يوم خلق السماوات والارض", driver->newTypeface("Geeza Pro", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(englishText, driver->newTypeface("SF Mono", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(englishText, driver->newTypeface("Helvetica", 32, VROFontStyle::Italic, VROFontWeight::Regular));
    _textSamples.emplace_back(englishText, driver->newTypeface("Helvetica", 32, VROFontStyle::Normal, VROFontWeight::ExtraBlack));

#elif VRO_PLATFORM_ANDROID
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。", driver->newTypeface("NotoSansCJK-Regular", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。", driver->newTypeface("NotoSansCJK-Regular", 32, VROFontStyle::Normal, VROFontWeight::Bold));
    _textSamples.emplace_back(L"人人生而自由,在尊严和权利上一律平等。他们赋 有理性和良心,并应以兄弟关系的精神互相对待。", driver->newTypeface("NotoSansCJK-Regular", 32, VROFontStyle::Normal, VROFontWeight::UltraLight));
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ", driver->newTypeface("NotoSansCJK-Regular", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"ㄱ, ㄴ, ㄷ, ㄹ, ㅁ, ㅂ, ㅅ, ㅇ, ㅈ, ㄲ, ㄸ, ㅃ, ㅆ, ㅉ, ㅊ, ㅋ, ㅌ, ㅍ, ㅎ", driver->newTypeface("NotoSansCJK-Regular", 32, VROFontStyle::Normal, VROFontWeight::Heavy));
    _textSamples.emplace_back(L"あ い う え お か き く け こ さ し す せ そ が ぎ ぐ げ ご ぱ ぴ ぷ ぺ ぽ", driver->newTypeface("NotoSansCJK-Regular", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"अ आ इ ई उ ऊ ए ऐ ओ औ अं अः क ख ग घ ङ च छ ज झ ञ ट ठ ड ढ ण त थ द ध न प फ", driver->newTypeface("NotoSansDevanagari-Regular", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(L"ان عدة الشهور عند الله اثنا عشر شهرا في كتاب الله يوم خلق السماوات والارض", driver->newTypeface("NotoNaskhArabic-Regular", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(englishText, driver->newTypeface("Roboto", 32, VROFontStyle::Normal, VROFontWeight::Regular));
    _textSamples.emplace_back(englishText, driver->newTypeface("Roboto", 32, VROFontStyle::Italic, VROFontWeight::Regular));
    _textSamples.emplace_back(englishText, driver->newTypeface("Roboto", 32, VROFontStyle::Normal, VROFontWeight::ExtraBlack));
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
    
    rootNode->addChildNode(surfaceNode);
    free (fileData);
    
    /*
     Actual text.
     */
    _textNode = std::make_shared<VRONode>();
    _textNode->setPosition({0, 0, -5});
    rootNode->addChildNode(_textNode);
    rotateText();
    
    _eventDelegate = std::make_shared<VROTextEventDelegate>(this);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    rootNode->setEventDelegate(_eventDelegate);
}

void VROTextTest::rotateText() {
    int width = 5;
    int height = 5;
    std::wstring &string = _textSamples[_textIndex].sample;
    std::shared_ptr<VROTypeface> typeface = _textSamples[_textIndex].typeface;
    VROLineBreakMode linebreakMode = VROLineBreakMode::Justify;
    VROTextClipMode clipMode = VROTextClipMode::ClipToBounds;
    std::shared_ptr<VROText> text = VROText::createText(string, typeface, {1.0, 1.0, 1.0, 1.0}, width, height,
                                                        VROTextHorizontalAlignment::Left, VROTextVerticalAlignment::Top,
                                                        linebreakMode, clipMode);
    _textNode->setGeometry(text);
    
    _textIndex = (_textIndex + 1) % _textSamples.size();
}

void VROTextEventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                   std::vector<float> position) {
    if (clickState == ClickState::Clicked) {
        _test->rotateText();
    }
}
