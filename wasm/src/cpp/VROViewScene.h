//
//  VROViewScene.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/4/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include <memory>
#include "emscripten.h"
#include "emscripten/html5.h"

class VRORenderer;
class VROInputControllerWasm;
class VRODriverOpenGLWasm;
class VRORendererTestHarness;
enum class VRORendererTestType;

class VROViewScene {
public:
    VROViewScene(VRORendererTestType test);
    virtual ~VROViewScene();
    
    void drawFrame();
    void update();
    
    void buildTestScene();
    void onResize();
    void onBlur();
    void onFocus();
    
private:
    
    int _frame;
    int _width, _height;
    float _suspendedNotificationTime;
    
    float _angle;
    
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROInputControllerWasm> _inputController;
    std::shared_ptr<VRODriverOpenGLWasm> _driver;
    VRORendererTestType _testType;
    std::shared_ptr<VRORendererTestHarness> _harness;
    
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _context;
    
};
