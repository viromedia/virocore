//
//  VROShaderModifier.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include <stdlib.h>
#include <vector>
#include <string>

enum class VROShaderEntryPoint {
    Geometry
};

enum class VROShaderSection {
    Uniforms,
    Body
};

class VROShaderModifier {
    
public:
    
    static uint32_t hashShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers);
    
    VROShaderModifier(VROShaderEntryPoint entryPoint, std::vector<std::string> input);
    virtual ~VROShaderModifier();
    
    int getShaderModifierId() const {
        return _shaderModifierId;
    }
    
    /*
     Get the pragma directive that corresponds to this modifier's entry point and
     the given section within a shader. This is the point in the shader where the
     modifier source will be inserted.
     */
    std::string getDirective(VROShaderSection section) const;
    
    std::string getUniforms() const {
        return _uniforms;
    }
    std::string getBody() const {
        return _body;
    }
    
private:
    
    int _shaderModifierId;
    
    std::string _uniforms;
    std::string _body;
    
    VROShaderEntryPoint _entryPoint;
    
    /*
     Extract the uniforms from the given source string and return them in a new
     string. Mutate the given string, removing the uniforms from the input source.
     */
    std::string extractUniforms(std::string *source);
    void extractNextUniform(std::string *uniforms, std::string *body);
    
};
