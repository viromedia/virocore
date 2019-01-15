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

static const int kRecognitionNumColors = 14;

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
    _drawDelegate = [[VRORecognitionDrawDelegate alloc] init];

    [_view setDebugDrawDelegate:_drawDelegate];
    
    std::shared_ptr<VROObjectRecognizeriOS> trackeriOS = std::make_shared<VROObjectRecognizeriOS>();
    trackeriOS->initObjectTracking(VROCameraPosition::Back, driver);
    trackeriOS->startObjectTracking();
    trackeriOS->setDelegate(shared_from_this());
    _objectRecognizer = trackeriOS;
#endif
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
}

void VRORecognitionTest::onObjectsFound(const std::map<std::string, std::vector<VRORecognizedObject>> &objects) {
#if VRO_PLATFORM_IOS
    int viewWidth  = _view.frame.size.width;
    int viewHeight = _view.frame.size.height;
    
    std::vector<VROVector3f> labelPositions;
    std::vector<NSString *> labels;
    std::vector<VROBoundingBox> boxes;
    
    for (auto &kv : objects) {
        for (VRORecognizedObject object : kv.second) {
            VROBoundingBox bounds = object.getBounds();
            
            float x = bounds.getX() * viewWidth;
            float y = bounds.getY() * viewHeight;
            float width = bounds.getSpanX() * viewWidth;
            float height = bounds.getSpanY() * viewHeight;

            VROVector3f labelPosition = { (float) (x - width / 2.0), (float) (y + height / 2.0), 0 };
            VROBoundingBox transformedBox(x - width / 2.0, x + width / 2.0, y - height / 2.0, y + height / 2.0, 0, 0);
            
            NSString *className = [NSString stringWithUTF8String:kv.first.c_str()];
            NSString *classAndConf = [NSString stringWithFormat:@"%@ [%.03f]", className, object.getConfidence()];
            
            boxes.push_back(transformedBox);
            labels.push_back(classAndConf);
            labelPositions.push_back({ labelPosition.x, labelPosition.y, 0 });
        }
    }
    
    [_drawDelegate setBoxes:boxes];
    [_drawDelegate setLabels:labels positions:labelPositions];
#endif
}

@implementation VRORecognitionDrawDelegate {
    std::vector<VROVector3f> _labelPositions;
    std::vector<NSString *> _labels;
    std::vector<VROBoundingBox> _boxes;
}

- (id)init {
    self = [super init];
    if (self) {
        
    }
    return self;
}

- (void)drawRect {
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    UIFont *font = [UIFont boldSystemFontOfSize:16];
    for (int i = 0; i < _labels.size(); i++) {
        VROVector3f point = _labelPositions[i];
        [_labels[i] drawAtPoint:CGPointMake( point.x, point.y ) withAttributes:@{ NSFontAttributeName:font,
                                                                            NSForegroundColorAttributeName : colors[i % kRecognitionNumColors] } ];
    }
    
    for (int i = 0; i < _boxes.size(); i++) {
        VROBoundingBox box = _boxes[i];
        
        CGRect rect = CGRectMake(box.getX() - box.getSpanX() / 2.0, box.getY() - box.getSpanY() / 2.0, box.getSpanX(), box.getSpanY());
        CGContextAddRect(context, rect);
        CGContextSetStrokeColorWithColor(context, [colors[i % kRecognitionNumColors] CGColor]);
        CGContextStrokePath(context);
    }
}

- (void)setLabels:(std::vector<NSString *>)labels positions:(std::vector<VROVector3f>)positions {
    _labels = labels;
    _labelPositions = positions;
}

- (void)setBoxes:(std::vector<VROBoundingBox>)boxes {
    _boxes = boxes;
}

@end
