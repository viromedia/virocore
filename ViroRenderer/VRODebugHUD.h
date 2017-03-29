//
//  VRODebugHUD.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/28/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VRODebugHUD_h
#define VRODebugHUD_h

#include <memory>

class VRONode;
class VRORenderContext;
class VRODriver;
class VROText;
class VROTypeface;
enum class VROEyeType;

class VRODebugHUD {
    
public:
    
    VRODebugHUD();
    virtual ~VRODebugHUD();
  
    /*
     Renderer thread initialization.
     */
    void initRenderer(std::shared_ptr<VRODriver> driver);
  
    /*
     Enable or disable the HUD.
     */
    void setEnabled(bool enabled);
  
    /*
     Render-loop functions.
     */
    void prepare(const VRORenderContext &context);
    void renderEye(VROEyeType eye, const VRORenderContext &context, VRODriver &driver);
    
private:
    
    bool _enabled;
    std::shared_ptr<VROTypeface> _typeface;
    std::shared_ptr<VROText> _text;
    std::shared_ptr<VRONode> _node;
    
};

#endif /* VRODebugHUD_h */
