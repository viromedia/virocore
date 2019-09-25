//
//  VRODebugHUD.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/28/17.
//  Copyright © 2017 Viro Media. All rights reserved.
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

#ifndef VRODebugHUD_h
#define VRODebugHUD_h

#include <memory>

class VRONode;
class VRORenderContext;
class VRODriver;
class VROText;
class VROTypefaceCollection;
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
    void renderEye(VROEyeType eye, const VRORenderContext &context, std::shared_ptr<VRODriver> &driver);
    
private:
    
    bool _enabled;
    std::shared_ptr<VROText> _text;
    std::shared_ptr<VRONode> _node;
    
};

#endif /* VRODebugHUD_h */
