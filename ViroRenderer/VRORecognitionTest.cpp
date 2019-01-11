//
//  VRORecognitionTest.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/10/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VRORecognitionTest.h"
#include "VROTestUtil.h"
#include "VROSphere.h"

#if VRO_PLATFORM_IOS
#include "VROObjectRecognizeriOS.h"
#include "VRODriverOpenGLiOS.h"

static UIColor *colors[14] = {
    [UIColor redColor],
    [UIColor greenColor],
    [UIColor blueColor],
    [UIColor cyanColor],
    [UIColor yellowColor],
    [UIColor magentaColor],
    [UIColor orangeColor],
    [UIColor purpleColor],
    [UIColor brownColor],
    [UIColor blackColor],
    [UIColor darkGrayColor],
    [UIColor lightGrayColor],
    [UIColor whiteColor],
    [UIColor grayColor]
};

#endif

VRORecognitionTest::VRORecognitionTest() :
VRORendererTest(VRORendererTestType::Recognition) {
    
}

VRORecognitionTest::~VRORecognitionTest() {
    
}

void VRORecognitionTest::build(std::shared_ptr<VRORenderer> renderer,
                               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                               std::shared_ptr<VRODriver> driver) {
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    _sceneController->setDelegate(shared_from_this());
    
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    
#if VRO_PLATFORM_IOS
    _view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
    
    std::shared_ptr<VROObjectRecognizeriOS> trackeriOS = std::make_shared<VROObjectRecognizeriOS>();
    trackeriOS->initObjectTracking(VROCameraPosition::Back, driver);
    trackeriOS->startObjectTracking();
    trackeriOS->setDelegate(shared_from_this());
    _objectRecognizer = trackeriOS;
#endif
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
    
    int numClasses = VROObjectRecognizer::getNumClasses();
    
    for (int i = 0; i < numClasses; i++) {
        std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(0.05, 20, 20, 20);
        std::shared_ptr<VRONode> sphereNode = std::make_shared<VRONode>();
        sphereNode->setGeometry(sphere);
        sphereNode->setPosition(VROVector3f(0.0f, -2000.0f, -2000.0f));
        
#if VRO_PLATFORM_IOS
        _objectViews[i] = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 4, 4)];
        _objectViews[i].backgroundColor = colors[i % 14];
        _objectViews[i].clipsToBounds = NO;
        
        UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(7, -3, 100, 8)];
        label.text = [NSString stringWithUTF8String:VROObjectRecognizer::getClassName(i).c_str()];
        label.textColor = colors[i % 14];
        label.font = [UIFont preferredFontForTextStyle:UIFontTextStyleCaption2];
        [_objectViews[i] addSubview:label];
        [_view addSubview:_objectViews[i]];
#endif
    }
}

void VRORecognitionTest::onObjectsFound(const std::map<std::string, VRORecognizedObject> &joints) {
#if VRO_PLATFORM_IOS
    int width  = _view.frame.size.width;
    int height = _view.frame.size.height;
    
    float minAlpha = 0.4;
    float maxAlpha = 1.0;
    float maxConfidence = 0.6;
    float minConfidence = 0.1;
    
    for (int i = 0; i < VROObjectRecognizer::getNumClasses(); i++) {
        _objectViews[i].alpha = 0;
    }
    
    for (auto &kv : joints) {
        int classIndex = VROObjectRecognizer::getIndexOfClass(kv.first);
        VROVector3f point = kv.second.getScreenCoords();
        VROVector3f transformed = { point.x * width, point.y * height, 0 };
        
        _objectViews[classIndex].center = CGPointMake(transformed.x, transformed.y);
        _objectViews[classIndex].alpha = VROMathInterpolate(kv.second.getConfidence(), minConfidence, maxConfidence, minAlpha, maxAlpha);
    }
#endif
}
