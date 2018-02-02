//
//  VROTextTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROTextTest.h"
#include "VROTestUtil.h"

VROTextTest::VROTextTest() :
    VRORendererTest(VRORendererTestType::Text) {
        
}

VROTextTest::~VROTextTest() {
    
}

void VROTextTest::build(std::shared_ptr<VRORenderer> renderer,
                        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                        std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->setBackgroundCube(VROTestUtil::loadCloudBackground());
    
    /*
     Create background for text.
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
    surfaceNode->setPosition({0, 0, -10.01});
    
    rootNode->addChildNode(surfaceNode);
    
    /*
     Create text.
     */
    VROLineBreakMode linebreakMode = VROLineBreakMode::Justify;
    VROTextClipMode clipMode = VROTextClipMode::ClipToBounds;
    
    std::shared_ptr<VROTypeface> typeface = driver->newTypeface("SF", 24);
    //std::wstring string = L"Déspacito. This is a test of wrapping a long piece of text, longer than all the previous pieces of text.";
    
    std::wstring string = L"Déspacito In older times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face.\n\nClose by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything.";
    
    VROVector3f size = VROText::getTextSize(string, typeface, width, height, linebreakMode, clipMode, 0);
    pinfo("Estimated size %f, %f", size.x, size.y);
    
    std::shared_ptr<VROText> text = VROText::createText(string, typeface, {1.0, 0.0, 0.0, 1.0}, width, height,
                                                        VROTextHorizontalAlignment::Left, VROTextVerticalAlignment::Top,
                                                        linebreakMode, clipMode);
    
    text->setName("Text");
    pinfo("Realized size %f, %f", text->getRealizedWidth(), text->getRealizedHeight());
    
    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(text);
    textNode->setPosition({0, 0, -10});
    
    rootNode->addChildNode(textNode);
    free (fileData);
}
