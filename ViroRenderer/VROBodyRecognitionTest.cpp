//
//  VROBodyRecognitionTest.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/14/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

#include "VROBodyRecognitionTest.h"
#include "VROTestUtil.h"
#include "VROSphere.h"

#if VRO_PLATFORM_IOS
#include "VROBodyTrackeriOS.h"
#include "VRODriverOpenGLiOS.h"

static std::string pointLabels[14] = {
    "top\t\t\t", //0
    "neck\t\t", //1
    "R shoulder\t", //2
    "R elbow\t\t", //3
    "R wrist\t\t", //4
    "L shoulder\t", //5
    "L elbow\t\t", //6
    "L wrist\t\t", //7
    "R hip\t\t", //8
    "R knee\t\t", //9
    "R ankle\t\t", //10
    "L hip\t\t", //11
    "L knee\t\t", //12
    "L ankle\t\t", //13
};

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

VROBodyRecognitionTest::VROBodyRecognitionTest() :
VRORendererTest(VRORendererTestType::BodyRecognition) {
    
}

VROBodyRecognitionTest::~VROBodyRecognitionTest() {
    
}

void VROBodyRecognitionTest::build(std::shared_ptr<VRORenderer> renderer,
                                   std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    _sceneController->setDelegate(shared_from_this());
    
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    
#if VRO_PLATFORM_IOS
    _view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
    
    std::shared_ptr<VROBodyTrackeriOS> trackeriOS = std::make_shared<VROBodyTrackeriOS>();
    trackeriOS->initBodyTracking(VROCameraPosition::Back, driver);
    trackeriOS->startBodyTracking();
    trackeriOS->setDelegate(shared_from_this());
    _bodyTracker = trackeriOS;
#endif
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
    
    _bodyPointsSpheres = std::vector<std::shared_ptr<VRONode>>(20);
    _bodyPointsSpheres.reserve(20);
    
    int endLoop = static_cast<int>(VROBodyJointType::LeftAnkle) + 1;
    for (int i = static_cast<int>(VROBodyJointType::Top); i < endLoop; i++) {
        std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(0.05, 20, 20, 20);
        std::shared_ptr<VRONode> sphereNode = std::make_shared<VRONode>();
        sphereNode->setGeometry(sphere);
        sphereNode->setPosition(VROVector3f(0.0f, -2000.0f, -2000.0f));
        
        _bodyPointsSpheres[i] = sphereNode;
        _arScene->getRootNode()->addChildNode(_bodyPointsSpheres[i]);
        
#if VRO_PLATFORM_IOS
        _bodyViews[i] = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 4, 4)];
        _bodyViews[i].backgroundColor = colors[i];
        _bodyViews[i].clipsToBounds = NO;
        
        UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(7, -3, 100, 8)];
        label.text = [NSString stringWithUTF8String:pointLabels[i].c_str()];
        label.textColor = colors[i];
        label.font = [UIFont preferredFontForTextStyle:UIFontTextStyleCaption2];
        [_bodyViews[i] addSubview:label];
        [_view addSubview:_bodyViews[i]];
#endif
    }
}

void VROBodyRecognitionTest::onBodyJointsFound(const std::map<VROBodyJointType, VROBodyJoint> &joints) {
#if VRO_PLATFORM_IOS
    int width  = _view.frame.size.width;
    int height = _view.frame.size.height;
    
    float minAlpha = 0.4;
    float maxAlpha = 1.0;
    float maxConfidence = 0.6;
    float minConfidence = 0.1;
    
    for (auto &kv : joints) {
        VROVector3f point = kv.second.getScreenCoords();
        VROVector3f transformed = { point.x * width, point.y * height, 0 };
        
        _bodyViews[(int) kv.first].center = CGPointMake(transformed.x, transformed.y);
        _bodyViews[(int) kv.first].alpha = VROMathInterpolate(kv.second.getConfidence(), minConfidence, maxConfidence, minAlpha, maxAlpha);
    }
#endif
}
