//
//  VROShaderProgram.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROShaderProgram.h"
#include "VROLog.h"
#include "VROVector3f.h"
#include "VROVector4f.h"
#include "VROMatrix4f.h"
#include "VROGeometrySource.h"
#include "VROShaderModifier.h"
#include "VROPlatformUtil.h"
#include "VROGeometryUtil.h"
#include "VROAllocationTracker.h"
#include "VROBoneUBO.h"
#include <atomic>

#define kDebugShaders 0

static std::atomic_int sMaterialId;

std::string loadTextAsset(std::string resource) {
    return VROPlatformLoadResourceAsString(resource, "glsl");
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Initialization
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Initialization

VROShaderProgram::VROShaderProgram(std::string vertexShader, std::string fragmentShader,
                                   const std::vector<std::string> &samplers,
                                   const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                   const std::vector<VROGeometrySourceSemantic> attributes,
                                   std::shared_ptr<VRODriverOpenGL> driver) :
    _shaderId(sMaterialId++),
    _lightingBlockIndex(GL_INVALID_INDEX),
    _bonesBlockIndex(GL_INVALID_INDEX),
    _attributes(0),
    _uniformsNeedRebind(true),
    _shaderName(fragmentShader),
    _program(0),
    _failedToLink(false),
    _samplers(samplers),
    _driver(driver) {

    _vertexSource = loadTextAsset(vertexShader);
    inflateIncludes(_vertexSource);

    _fragmentSource = loadTextAsset(fragmentShader);
    inflateIncludes(_fragmentSource);
    
    inflateVertexShaderModifiers(modifiers, _vertexSource);
    inflateFragmentShaderModifiers(modifiers, _fragmentSource);
        
    passert (!_vertexSource.empty() && !_fragmentSource.empty());
       
    for (VROGeometrySourceSemantic attr : attributes) {
        switch (attr) {
            case VROGeometrySourceSemantic::Texcoord:
                _attributes |= (int)VROShaderMask::Tex;
                break;
                
            case VROGeometrySourceSemantic::Normal:
                _attributes |= (int)VROShaderMask::Norm;
                break;
                
            case VROGeometrySourceSemantic::Color:
                _attributes |= (int)VROShaderMask::Color;
                break;
                
            case VROGeometrySourceSemantic::Tangent:
                _attributes |= (int)VROShaderMask::Tangent;
                break;
                
            case VROGeometrySourceSemantic::BoneIndices:
                _attributes |= (int)VROShaderMask::BoneIndex;
                break;
                
            case VROGeometrySourceSemantic::BoneWeights:
                _attributes |= (int)VROShaderMask::BoneWeight;
                break;
                
            default:
                break;
        }
    }
    _modifiers = modifiers;
    addStandardUniforms();
    addModifierUniforms();
        
    ALLOCATION_TRACKER_ADD(Shaders, 1);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Destruction
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Destruction

VROShaderProgram::~VROShaderProgram() {
    for (VROUniform *uniform : _uniforms) {
        delete (uniform);
    }

    // Ensure we are deleting GL objects with the current GL context
    if (_driver.lock()) {
        glDeleteShader(_program);
    }
    
    ALLOCATION_TRACKER_SUB(Shaders, 1);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Compiling and Linking
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Compiling and Linking

bool VROShaderProgram::hydrate() {
    passert (_program == 0);

#if kDebugShaders
    if (!_shaderName.empty()) {
        pinfo("Compiling shader [%s]", _shaderName.c_str());
    }
    else {
        pinfo("Compiling anonymous shader");
    }
#endif

    if (!compileAndLink()) {
        _failedToLink = true;
        return false;
    }

    return true;
}

bool VROShaderProgram::isHydrated() const {
    return _program != 0;
}

void VROShaderProgram::evict() {
    if (_program != 0) {
        if (_driver.lock()) {
            glDeleteProgram(_program);
        }
    }

    _uniformsNeedRebind = true;
    for (VROUniform *uniform : _uniforms) {
        uniform->reset();
    }

    _program = 0;
}

bool VROShaderProgram::compileShader(GLuint *shader, GLenum type, const char *source) {
    GLint status;
    int len = (int) strlen(source);

    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, &len);
    glCompileShader(*shader);

#if kDebugShaders
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength); // <-- This is broken in qcomm's drivers

    logLength = 4096; // make this "big enough"
    char elog[logLength];
    glGetShaderInfoLog(*shader, logLength, &logLength, elog);

    if (logLength > 1) { // when there are no logs we have just a '\n', don't print that out
        perr("Shader compile log:\n%s", elog);
    }
#endif

    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return false;
    }

    return true;
}

bool VROShaderProgram::linkProgram(GLuint prog) {
    GLint status;
    glLinkProgram(prog);

#if kDebugShaders
    GLint logLength;
    // glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength); // <-- This is broken in qcomm's drivers
    logLength = 4096; // make this "big enough"
    if (logLength > 0) {
        char *elog = (char *) malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, elog);
        perr("Program link log:\n%s", elog);
        free(elog);
    }
#endif

    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        return false;
    }

    return true;
}

bool VROShaderProgram::validateProgram(GLuint prog) {
    GLint status;

    glValidateProgram(prog);

#if kDebugShaders
    GLint logLength;
    // glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength); // <-- This is broken in qcomm's drivers
    logLength = 4096; // make this "big enough"
    if (logLength > 0) {
        char *elog = (char *) malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, elog);
        perr("Program validate log:\n%s", elog);
        free(elog);
    }
#endif

    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0) {
        return false;
    }

    return true;
}

bool VROShaderProgram::compileAndLink() {
    GLuint vertShader, fragShader;
    _program = glCreateProgram();

    if (_program == 0) {
        if (_shaderName.empty()) {
            pinfo("Could not create shader program with glCreateProgram for anonymous shader (do you have an active EGL context?)");
        } else {
            pinfo("Could not create shader program with glCreateProgram for shader with name[%s] (do you have an active EGL context?)", _shaderName.c_str());
        }

        // Return true here so we retry the compile later
        return true;
    }

#if kDebugShaders
    if (!_shaderName.empty()) {
        pinfo("Compiling and linking shader with name %s into GL object %d", _shaderName.c_str(), _program);
    }
    else {
        pinfo("Compiling and linking anonymous shader into GL object %d", _program);
    }
#endif

    /*
     Compile and attach the shaders to the program.
     */
    passert (!_vertexSource.empty());
    passert (!_fragmentSource.empty());

    if (!compileShader(&vertShader, GL_VERTEX_SHADER, _vertexSource.c_str())) {
        pabort("Failed to compile vertex shader \"%s\" with code:\n%s",
               _shaderName.c_str(), _vertexSource.c_str());
        return false;
    }

    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, _fragmentSource.c_str())) {
        pabort("Failed to compile fragment shader \"%s\" with code:\n%s",
               _shaderName.c_str(), _fragmentSource.c_str());
        return false;
    }

    glAttachShader(_program, vertShader);
    glAttachShader(_program, fragShader);

    /*
     Bind attribute locations.
     */
    bindAttributes();

    /*
     Link the program.
     */
    if (!linkProgram(_program)) {
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        pabort("Failed to link program %d, name %s", _program, _shaderName.c_str());
        return false;
    }
    
    bindUniformBlocks();

    /*
     Release vertex and fragment shaders.
     */
    if (vertShader) {
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDeleteShader(fragShader);
    }

#if kDebugShaders
    pinfo("Finished compiling shader %s", _shaderName.c_str());

     if (_vertexSource.c_str() != nullptr) {
         pinfo("Shader vertex source %s", _vertexSource.c_str());
     }
     if (_fragmentSource.c_str() != nullptr) {
         pinfo("Shader frag source %s", _fragmentSource.c_str());
     }
#endif

    return true;
}

bool VROShaderProgram::bind() {
    if (_failedToLink) {
        return false;
    }

    passert (isHydrated());
    glUseProgram(_program);

    // Bind uniform locations here, if required.
    if (_uniformsNeedRebind) {
        findUniformLocations();
        _uniformsNeedRebind = false;
    }

    return true;
}

void VROShaderProgram::unbind() {
    glUseProgram(0);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Uniforms
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Uniforms

VROUniform *VROShaderProgram::addUniform(VROShaderProperty type, int arraySize, const std::string &name) {
    VROUniform *uniform = VROUniform::newUniformForType(name, type, arraySize);
    _uniforms.push_back(uniform);
    _uniformsNeedRebind = true;
    
    return uniform;
}

int VROShaderProgram::getUniformIndex(const std::string &name) {
    int idx = 0;

    for (VROUniform *uniform : _uniforms) {
        if (uniform->getName() == name) {
            return idx;
        }

        ++idx;
    }

    return -1;
}

VROUniform *VROShaderProgram::getUniform(const std::string &name) {
    for (VROUniform *uniform : _uniforms) {
        if (uniform->getName() == name) {
            return uniform;
        }
    }

    return nullptr;
}

VROUniform *VROShaderProgram::getUniform(int index) {
    return _uniforms[index];
}

void VROShaderProgram::findUniformLocations() {
    for (VROUniform *uniform : _uniforms) {
        int location = glGetUniformLocation(_program, uniform->getName().c_str());
        uniform->setLocation(location);
    }
    
    int samplerIdx = 0;
    
    for (std::string &samplerName : _samplers) {
        int location = glGetUniformLocation(_program, samplerName.c_str());
        glUniform1i(location, samplerIdx);
        
        ++samplerIdx;
    }
}

void VROShaderProgram::addModifierUniforms() {
    for (const std::shared_ptr<VROShaderModifier> &modifier : _modifiers) {
        std::vector<std::string> uniformNames = modifier->getUniforms();
        
        for (std::string &uniformName : uniformNames) {
            VROUniform *uniform = new VROUniformShaderModifier(uniformName, modifier);
            _uniforms.push_back(uniform);
        }
    }
}

#pragma mark - Standard 3D Shader

void VROShaderProgram::bindAttributes() {
    glBindAttribLocation(_program, (int)VROGeometrySourceSemantic::Vertex, "position");
    
    if ((_attributes & (int)VROShaderMask::Tex) != 0) {
        glBindAttribLocation(_program, VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Texcoord), "texcoord");
    }
    if ((_attributes & (int)VROShaderMask::Color) != 0) {
        glBindAttribLocation(_program, VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Color), "color");
    }
    if ((_attributes & (int)VROShaderMask::Norm) != 0) {
        glBindAttribLocation(_program, VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Normal), "normal");
    }
    if ((_attributes & (int)VROShaderMask::Tangent) != 0) {
        glBindAttribLocation(_program, VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::Tangent), "tangent");
    }
    if ((_attributes & (int)VROShaderMask::BoneIndex) != 0) {
        glBindAttribLocation(_program, VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::BoneIndices), "bone_indices");
    }
    if ((_attributes & (int)VROShaderMask::BoneWeight) != 0) {
        glBindAttribLocation(_program, VROGeometryUtilParseAttributeIndex(VROGeometrySourceSemantic::BoneWeights), "bone_weights");
    }
}

void VROShaderProgram::bindUniformBlocks() {
    _lightingBlockIndex = glGetUniformBlockIndex(_program, "lighting");
    _bonesBlockIndex = glGetUniformBlockIndex(_program, kDualQuaternionEnabled ? "bones_dq" : "bones");
}

void VROShaderProgram::addStandardUniforms() {
    addUniform(VROShaderProperty::Mat4, 1, "normal_matrix");
    addUniform(VROShaderProperty::Mat4, 1, "model_matrix");
    addUniform(VROShaderProperty::Mat4, 1, "modelview_matrix");
    addUniform(VROShaderProperty::Mat4, 1, "modelview_projection_matrix");
    addUniform(VROShaderProperty::Vec3, 1, "camera_position");
    addUniform(VROShaderProperty::Int, 1, "eye_type");
    
    addUniform(VROShaderProperty::Vec4, 1, "material_diffuse_surface_color");
    addUniform(VROShaderProperty::Float, 1, "material_diffuse_intensity");
    addUniform(VROShaderProperty::Float, 1, "material_alpha");
}

#pragma mark - Source Inflation and Shader Modifiers

const std::string &VROShaderProgram::getVertexSource() const {
    return _vertexSource;
}

const std::string &VROShaderProgram::getFragmentSource() const {
    return _fragmentSource;
}

void VROShaderProgram::inflateIncludes(std::string &source) const {
    std::string includeDirective("#include ");
    
    size_t includeStart = source.find(includeDirective);
    if (includeStart == std::string::npos) {
        return;
    }
    
    size_t includeEnd = source.find("\n", includeStart);
    std::string includeFile = source.substr(includeStart + includeDirective.size(),
                                            includeEnd - (includeStart + includeDirective.size()));
    
    std::string includeSource = loadTextAsset(includeFile.c_str());
    source.replace(includeStart, includeEnd - includeStart, includeSource);
    
    // Support recursive includes by invoking this again with the result
    inflateIncludes(source);
}

void VROShaderProgram::inflateVertexShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                    std::string &source) const {
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
        if (modifier->getEntryPoint() != VROShaderEntryPoint::Geometry &&
            modifier->getEntryPoint() != VROShaderEntryPoint::Vertex) {
            continue;
        }
        
        insertModifier(modifier->getBodySource(), modifier->getDirective(VROShaderSection::Body), source);
        insertModifier(modifier->getUniformsSource(), modifier->getDirective(VROShaderSection::Uniforms), source);
        inflateReplacements(modifier->getReplacements(), source);
    }
}

void VROShaderProgram::inflateFragmentShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                      std::string &source) const {
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
        if (modifier->getEntryPoint() != VROShaderEntryPoint::Surface &&
            modifier->getEntryPoint() != VROShaderEntryPoint::Fragment &&
            modifier->getEntryPoint() != VROShaderEntryPoint::Image) {
            continue;
        }
        
        insertModifier(modifier->getBodySource(), modifier->getDirective(VROShaderSection::Body), source);
        insertModifier(modifier->getUniformsSource(), modifier->getDirective(VROShaderSection::Uniforms), source);
        inflateReplacements(modifier->getReplacements(), source);
    }
}

void VROShaderProgram::inflateReplacements(const std::map<std::string, std::string> &replacements, std::string &source) const {
    for (auto key : replacements) {
        const std::string &stringMatching = key.first;
        const std::string &replacementString = key.second;
        
        size_t replaceStart = source.find(stringMatching);
        if (replaceStart != std::string::npos) {
            size_t replaceEnd = source.find("\n", replaceStart);
            source.replace(replaceStart, replaceEnd - replaceStart, replacementString);
        }
    }
}

void VROShaderProgram::insertModifier(std::string modifierSource, std::string directive,
                                      std::string &source) const {

    size_t start = source.find(directive);
    if (start == std::string::npos) {
        return;
    }
    
    size_t end = source.find("\n", start);
    source.replace(start, end - start, modifierSource);
}
