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
#include "VROStringUtil.h"
#include "VRODriverOpenGL.h"
#include <atomic>

#define kDebugShaders 0

static std::atomic_int sMaterialId;
static const int shaderMaxLogLength = 4096;
char shaderLog[shaderMaxLogLength];

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
                                   int attributes,
                                   std::shared_ptr<VRODriverOpenGL> driver) :
    _shaderId(sMaterialId++),
    _lightingFragmentBlockIndex(GL_INVALID_INDEX),
    _lightingVertexBlockIndex(GL_INVALID_INDEX),
    _bonesBlockIndex(GL_INVALID_INDEX),
    _particlesVertexBlockIndex(GL_INVALID_INDEX),
    _particlesFragmentBlockIndex(GL_INVALID_INDEX),
    _attributes(attributes),
    _uniformsNeedRebind(true),
    _shaderName(fragmentShader),
    _program(0),
    _failedToLink(false),
    _samplers(samplers),
    _driver(driver) {
    
    if (VROStringUtil::endsWith(fragmentShader, "_fsh")) {
        _shaderName = fragmentShader.substr(0, fragmentShader.length() - 4);
    }
    _vertexSource = loadTextAsset(vertexShader);
    _fragmentSource = loadTextAsset(fragmentShader);

    // Inflate includes after modifiers (for cases where modifiers have includes)
    inflateVertexShaderModifiers(modifiers, _vertexSource);
    inflateIncludes(_vertexSource);

    inflateFragmentShaderModifiers(modifiers, _fragmentSource);
    inflateIncludes(_fragmentSource);
       
    std::string vertexAssignments = "_geometry.position = position;\n";
    if ((_attributes & (int)VROShaderMask::Tex) != 0) {
        vertexAssignments += "_geometry.texcoord = texcoord;\n";
    }
    if ((_attributes & (int)VROShaderMask::Color) != 0) {
        // Color is not currently supported in the shaders
    }
    if ((_attributes & (int)VROShaderMask::Norm) != 0) {
        vertexAssignments += "_geometry.normal = normal;\n";
    }
    if ((_attributes & (int)VROShaderMask::Tangent) != 0) {
        vertexAssignments += "_geometry.tangent = tangent;\n";
    }
    if ((_attributes & (int)VROShaderMask::BoneIndex) != 0) {
        vertexAssignments += "_geometry.bone_indices = bone_indices;\n";
    }
    if ((_attributes & (int)VROShaderMask::BoneWeight) != 0) {
        vertexAssignments += "_geometry.bone_weights = bone_weights;\n";
    }
    inject("#inject vertex_assignments", vertexAssignments, _vertexSource);
        
    if (driver->getGPUType() == VROGPUType::Adreno330OrOlder) {
        std::map<std::string, std::string> adrenoReplacements;
        adrenoReplacements["_surface."] = "_surface_";
        adrenoReplacements["_vertex."] = "_vertex_";
        adrenoReplacements["_geometry."] = "_geometry_";
        adrenoReplacements["_transforms."] = "_transforms_";
        
        for (auto kv : adrenoReplacements) {
            VROStringUtil::replaceAll(_vertexSource, kv.first, kv.second);
            VROStringUtil::replaceAll(_fragmentSource, kv.first, kv.second);
        }
        pinfo("Inflated Adreno 330 replacements for shader source");
    }
    
    passert (!_vertexSource.empty() && !_fragmentSource.empty());
        
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
    glGetShaderInfoLog(*shader, shaderMaxLogLength, &logLength, shaderLog);
    if (logLength > 1) { // when there are no logs we have just a '\n', don't print that out
        perr("Shader compile log:\n%s", shaderLog);
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
    glGetProgramInfoLog(prog, shaderMaxLogLength, &logLength, shaderLog);
    if (logLength > 1) {
        perr("Program link log:\n%s", shaderLog);
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
    glGetProgramInfoLog(prog, shaderMaxLogLength, &logLength, shaderLog);
    if (logLength > 1) {
        perr("Program validate log:\n%s", shaderLog);
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
        pwarn("Failed to compile vertex shader \"%s\" with code:\n",
               _shaderName.c_str());
        VROStringUtil::printCode(_vertexSource);
        pabort("Failed to compile vertex shader %s", _shaderName.c_str());
        return false;
    }

    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, _fragmentSource.c_str())) {
        pwarn("Failed to compile fragment shader \"%s\" with code:\n",
               _shaderName.c_str());
        VROStringUtil::printCode(_fragmentSource);
        pabort("Failed to compile fragment shader %s", _shaderName.c_str());
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
        bindUniformBlocks();
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
    // The calls to glUniformBlockBinding link the shader's block index to the binding point.
    // Within each Viro UBO class we use glBindBuffer base to then link the actual UBO data
    // the same binding point.

    _lightingFragmentBlockIndex = glGetUniformBlockIndex(_program, "lighting_fragment");
    if (_lightingFragmentBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(_program, _lightingFragmentBlockIndex, sLightingFragmentUBOBindingPoint);
    }
    _lightingVertexBlockIndex = glGetUniformBlockIndex(_program, "lighting_vertex");
    if (_lightingVertexBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(_program, _lightingVertexBlockIndex, sLightingVertexUBOBindingPoint);
    }
    
    _bonesBlockIndex = glGetUniformBlockIndex(_program, kDualQuaternionEnabled ? "bones_dq" : "bones");
    if (_bonesBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(_program, _bonesBlockIndex, sBonesUBOBindingPoint);
    }
    
    _particlesVertexBlockIndex = glGetUniformBlockIndex(_program, "particles_vertex_data");
    if (_particlesVertexBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(_program, _particlesVertexBlockIndex, sParticleVertexUBOBindingPoint);
    }
    _particlesFragmentBlockIndex = glGetUniformBlockIndex(_program, "particles_fragment_data");
    if (_particlesFragmentBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(_program, _particlesFragmentBlockIndex, sParticleFragmentUBOBindingPoint);
    }
}

void VROShaderProgram::addStandardUniforms() {
    addUniform(VROShaderProperty::Mat4, 1, "normal_matrix");
    addUniform(VROShaderProperty::Mat4, 1, "model_matrix");
    addUniform(VROShaderProperty::Mat4, 1, "view_matrix");
    addUniform(VROShaderProperty::Mat4, 1, "projection_matrix");
    addUniform(VROShaderProperty::Vec3, 1, "camera_position");
    addUniform(VROShaderProperty::Float, 1, "eye_type");

    addUniform(VROShaderProperty::Vec4, 1, "material_diffuse_surface_color");
    addUniform(VROShaderProperty::Float, 1, "material_diffuse_intensity");
    addUniform(VROShaderProperty::Float, 1, "material_alpha");
    addUniform(VROShaderProperty::Float, 1, "material_shininess");
    
    addUniform(VROShaderProperty::Float, 1, "material_roughness");
    addUniform(VROShaderProperty::Float, 1, "material_metalness");
    addUniform(VROShaderProperty::Float, 1, "material_ao");
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

void VROShaderProgram::inject(const std::string &directive, const std::string &code, std::string &source) const {
    size_t directiveStart = source.find(directive);
    if (directiveStart == std::string::npos) {
        return;
    }
    size_t directiveEnd = source.find("\n", directiveStart);
    source.replace(directiveStart, directiveEnd - directiveStart, code);
}

void VROShaderProgram::inflateVertexShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                    std::string &source) {
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
        if (modifier->getEntryPoint() != VROShaderEntryPoint::Geometry &&
            modifier->getEntryPoint() != VROShaderEntryPoint::Vertex) {
            continue;
        }
        
        insertModifier(modifier->getBodySource(), modifier->getDirective(VROShaderSection::Body), source);
        insertModifier(modifier->getUniformsSource(), modifier->getDirective(VROShaderSection::Uniforms), source);
        inflateReplacements(modifier->getReplacements(), source);
        
        if (!modifier->getName().empty()) {
            _shaderName.append("_").append(modifier->getName());
        }
    }
}

void VROShaderProgram::inflateFragmentShaderModifiers(const std::vector<std::shared_ptr<VROShaderModifier>> &modifiers,
                                                      std::string &source) {
    
    for (const std::shared_ptr<VROShaderModifier> &modifier : modifiers) {
        if (modifier->getEntryPoint() != VROShaderEntryPoint::Surface &&
            modifier->getEntryPoint() != VROShaderEntryPoint::LightingModel && 
            modifier->getEntryPoint() != VROShaderEntryPoint::Fragment &&
            modifier->getEntryPoint() != VROShaderEntryPoint::Image) {
            continue;
        }
        
        insertModifier(modifier->getBodySource(), modifier->getDirective(VROShaderSection::Body), source);
        insertModifier(modifier->getUniformsSource(), modifier->getDirective(VROShaderSection::Uniforms), source);
        inflateReplacements(modifier->getReplacements(), source);
        
        if (!modifier->getName().empty()) {
            _shaderName.append("_").append(modifier->getName());
        }
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
    source.replace(start, end - start + 1, modifierSource);
}
