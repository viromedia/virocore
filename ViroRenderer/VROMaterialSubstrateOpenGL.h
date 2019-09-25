//
//  VROMaterialSubstrateOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

#ifndef VROMaterialSubstrateOpenGL_h
#define VROMaterialSubstrateOpenGL_h

#include "VROMaterial.h"
#include "VROMaterialSubstrate.h"
#include <map>
#include <memory>
#include "VROOpenGL.h"
#include "VROShaderCapabilities.h"

class VROShaderProgram;
class VRODriverOpenGL;
class VROLight;
class VROMatrix4f;
class VROVector3f;
class VROUniform;
class VROGeometry;
class VROLightingUBO;
class VROInstancedUBO;
class VROBoneUBO;
class VROTextureReference;
class VROMaterialShaderBinding;
enum class VROEyeType;

class VROMaterialSubstrateOpenGL : public VROMaterialSubstrate {
    
public:
    
    VROMaterialSubstrateOpenGL(VROMaterial &material, std::shared_ptr<VRODriverOpenGL> &driver);
    virtual ~VROMaterialSubstrateOpenGL();
    
    /*
     Bind the shader used in this material to the active rendering context.
     This is kept independent of the bindProperties() function because shader changes
     are expensive, so we want to manage them independent of materials in the
     render loop.
     
     The shader used is a function both of the underlying material properties
     and of the desired lighting configuration.
     
     Returns false if the shader could not be bound.
     */
    bool bindShader(int lightsHash,
                    const std::vector<std::shared_ptr<VROLight>> &lights,
                    const VRORenderContext &context,
                    std::shared_ptr<VRODriver> &driver);
    
    /*
     Bind the properties of this material to the active rendering context.
     These properties should be node and geometry independent. The shader
     should always be bound first (via bindShader()).
     */
    void bindProperties(std::shared_ptr<VRODriver> &driver);
    
    /*
     Bind the properties of the given geometry to the active rendering context.
     These are material properties (e.g. shader uniforms) that are dependent
     on properties of the geometry.
     */
    void bindGeometry(float opacity, const VROGeometry &geometry);
    
    /*
     Bind the properties of the view and projection to the active rendering
     context.
     */
    void bindView(VROMatrix4f modelMatrix, VROMatrix4f viewMatrix,
                  VROMatrix4f projectionMatrix, VROMatrix4f normalMatrix,
                  VROVector3f cameraPosition, VROEyeType eyeType);
    
    const std::vector<VROTextureReference> &getTextures() const;
    
    void updateTextures();
    void updateSortKey(VROSortKey &key, const std::vector<std::shared_ptr<VROLight>> &lights,
                       const VRORenderContext &context,
                       std::shared_ptr<VRODriver> driver);

private:

    const VROMaterial &_material;
    VROMaterialShaderCapabilities _materialShaderCapabilities;
    
    /*
     The last used program's binding and its lighting capabilities. This is cached
     to reduce lookups into the _programs map.
     */
    VROMaterialShaderBinding *_activeBinding;
    std::shared_ptr<VROLightingUBO> _lightingUBO;
    
    /*
     The shadow map bound with the lights.
     */
    std::shared_ptr<VROTexture> _shadowMap;
    
    /*
     The shader programs that have been used by this material, each represented as a
     VROMaterialShaderBinding, a binding between this material and a program.
     
     Each program here is capable of rendering materials with _materialShaderCapabilities;
     what makes them unique is they render different lighting configurations (e.g. a
     material can have one shader program it uses when not rendering shadows, and
     another when it is rendering shadows).
     */
    std::map<VROLightingShaderCapabilities, std::unique_ptr<VROMaterialShaderBinding>> _shaderBindings;
    
    /*
     Get the shader program that should be used for the given light configuration.
     Returned as a material-shader binding. If no such binding exists, it is created
     and cached here.
     */
    VROMaterialShaderBinding *getShaderBindingForLights(const std::vector<std::shared_ptr<VROLight>> &lights,
                                                        const VRORenderContext &context,
                                                        std::shared_ptr<VRODriver> driver);

    uint32_t hashTextures(const std::vector<VROTextureReference> &textures) const;
    
};

#endif /* VROMaterialSubstrateOpenGL_h */
