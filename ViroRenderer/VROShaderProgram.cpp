//
//  VROShaderProgram.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROShaderProgram.h"
#include "VROLog.h"

#define kDebugShaders 0

std::string loadTextAsset(const char *resource) {
    NSString *file = [[NSBundle bundleWithIdentifier:@"com.viro.ViroKit"] pathForResource:[NSString stringWithUTF8String:resource] ofType:nil];
    return std::string([[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String]);
}

VROUniform *newUniformForType(const std::string &name, VROShaderProperty type, int arraySize) {

    VROUniform *uniform = nullptr;

    switch (type) {
        case VROShaderProperty::Int:
            uniform = new VROUniform1i(name, arraySize);
            break;

        case VROShaderProperty::IVec2:
            uniform = new VROUniform2i(name, arraySize);
            break;

        case VROShaderProperty::IVec3:
            uniform = new VROUniform3i(name, arraySize);
            break;

        case VROShaderProperty::IVec4:
            uniform = new VROUniform4i(name, arraySize);
            break;

        case VROShaderProperty::Float:
            uniform = new VROUniform1f(name, arraySize);
            break;

        case VROShaderProperty::Vec2:
            uniform = new VROUniform2f(name, arraySize);
            break;

        case VROShaderProperty::Vec3:
            uniform = new VROUniform3f(name, arraySize);
            break;

        case VROShaderProperty::Vec4:
            uniform = new VROUniform4f(name, arraySize);
            break;

        case VROShaderProperty::Mat2:
            uniform = new VROUniformMat2(name);
            break;

        case VROShaderProperty::Mat3:
            uniform = new VROUniformMat3(name);
            break;

        case VROShaderProperty::Mat4:
            uniform = new VROUniformMat4(name);
            break;

        default:
            pabort("Unsupported shader uniform type VROShaderProperty: %d", static_cast<int>(type));
            break;
    }

    return uniform;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Initialize From Embedded Resources
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Initialize From Embedded Resources

VROShaderProgram::VROShaderProgram(const char *name, int cap) :
    capabilities(cap),
    normTransformsSet(false),
    uniformsNeedRebind(true),
    shaderName(name),
    program(0),
    failedToLink(false),
    alphaUniformIndex(-1),
    hasAlphaUniform(true),
    colorUniformIndex(-1),
    hasColorUniform(true),
    offsetUniformIndex(-1),
    hasOffsetUniform(true),
    glslVersion(GLSLVersion::V0) {

    char vertShaderName[200];
    strcpy(vertShaderName, shaderName.c_str());
    strcat(vertShaderName, "_vsh");

    embeddedVertexSource = loadTextAsset(vertShaderName);

    char fragShaderName[200];
    strcpy(fragShaderName, shaderName.c_str());
    strcat(fragShaderName, "_fsh");

    embeddedFragmentSource = loadTextAsset(fragShaderName);
    passert (!embeddedVertexSource.empty() && !embeddedFragmentSource.empty());

    // We don't immediately attempt to compileAndLink here because our
    // uniform and sampler maps are not yet initialized
}

void VROShaderProgram::setUniforms(VROShaderProperty *uniformTypes, const char **names, int count) {
    for (VROUniform *uniform : uniforms) {
        delete (uniform);
    }
    uniforms.clear();

    for (int i = 0; i < count; i++) {
        uniforms.push_back(newUniformForType(names[i], uniformTypes[i], 1));
    }

    uniformsNeedRebind = true;
}

void VROShaderProgram::setSamplers(const char **names, int count) {
    samplers.clear();

    for (int i = 0; i < count; i++) {
        samplers.push_back(std::string(names[i]));
    }
}

void VROShaderProgram::findTransformUniformLocations() {
    xforms[(int)VROShaderXForm::MVP]    = glGetUniformLocation(program, "mvp_matrix");

    if ((capabilities & (int)VROShaderMask::Tex) != 0) {
        xforms[(int)VROShaderXForm::Tex] = glGetUniformLocation(program, "tex_norm_matrix");
    }
    if ((capabilities & (int)VROShaderMask::Norm) != 0) {
        xforms[(int)VROShaderXForm::Norm] = glGetUniformLocation(program, "norm_norm_matrix");
    }
}

void VROShaderProgram::findUniformLocations() {
    for (VROUniform *uniform : uniforms) {
        int location = glGetUniformLocation(program, uniform->name.c_str());
        uniform->setLocation(location);
    }

    int samplerIdx = 0;

    for (std::string &samplerName : samplers) {
        int location = glGetUniformLocation(program, samplerName.c_str());
        glUniform1i(location, samplerIdx);

        ++samplerIdx;
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Add Uniforms / Attributes / Samplers
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Add Uniforms / Attributes / Samplers

void VROShaderProgram::addUniform(VROShaderProperty type, int arraySize, const std::string &name) {
    uniforms.push_back(newUniformForType(name, type, arraySize));
    uniformsNeedRebind = true;
}

void VROShaderProgram::addAttribute(VROShaderAttribute attr) {
    passert (program == 0);

    switch (attr) {
        case VROShaderAttribute::Tex:
            capabilities |= (int)VROShaderMask::Tex;
            break;

        case VROShaderAttribute::Norm:
            capabilities |= (int)VROShaderMask::Norm;
            break;

        case VROShaderAttribute::Color:
            capabilities |= (int)VROShaderMask::Color;
            break;

        default:
            pabort();
    }
}

void VROShaderProgram::addSampler(const std::string &name) {
    samplers.push_back(name);

    uniformsNeedRebind = true;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Destruction
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Destruction

VROShaderProgram::~VROShaderProgram() {
    for (VROUniform *uniform : uniforms) {
        delete (uniform);
    }
    
    glDeleteShader(program);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Compiling and Linking
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Compiling and Linking

bool VROShaderProgram::hydrate() {
    passert (program == 0);

#if kDebugShaders
    if (!shaderName.empty()) {
        pinfo("Compiling [%s], ID %d", shaderName.c_str(), shaderId);
    }
    else {
        pinfo("Compiling anonymous shader with ID %d", shaderId);
    }
#endif

    if (!compileAndLink()) {
        failedToLink = true;
        return false;
    }

    return true;
}

bool VROShaderProgram::isHydrated() {
    return program != 0;
}

void VROShaderProgram::evict() {
    if (program != 0) {
        glDeleteProgram(program);
    }

    normTransformsSet = false;
    alphaUniformIndex = -1;
    hasAlphaUniform = true;
    colorUniformIndex = -1;
    hasColorUniform = true;
    offsetUniformIndex = -1;
    hasOffsetUniform = true;
    uniformsNeedRebind = true;

    for (VROUniform *uniform : uniforms) {
        uniform->reset();
    }

    program = 0;
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
    program = glCreateProgram();

    if (program == 0) {
        if (shaderName.empty()) {
            pinfo("Could not create shader program with glCreateProgram for anonymous shader (do you have an active EGL context?)");
        } else {
            pinfo("Could not create shader program with glCreateProgram for shader with name[%s] (do you have an active EGL context?)", shaderName.c_str());
        }

        // Return true here so we retry the compile later
        return true;
    }

#if kDebugShaders
    if (!shaderName.empty()) {
        pinfo("Compiling and linking shader with name %s, ID %d into GL object %d", shaderName.c_str(), shaderId, program);
    }
    else {
        pinfo("Compiling and linking anonymous shader with ID %d into GL object %d", shaderId, program);
    }
#endif

    /*
     Compile and attach the shaders to the program.
     */
    passert (!embeddedVertexSource.empty());
    passert (!embeddedFragmentSource.empty());

    if (!compileShader(&vertShader, GL_VERTEX_SHADER, embeddedVertexSource.c_str())) {
        pabort("Failed to compile vertex shader \"%s\" with code:\n%s",
               shaderName.c_str(), embeddedVertexSource.c_str());
        return false;
    }

    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, embeddedFragmentSource.c_str())) {
        pabort("Failed to compile fragment shader \"%s\" with code:\n%s",
               shaderName.c_str(), embeddedFragmentSource.c_str());
        return false;
    }

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    /*
     Bind attribute locations.
     */
    glBindAttribLocation(program, (int)VROShaderAttribute::Verts, "position");

    if ((capabilities & (int)VROShaderMask::Tex) != 0) {
        glBindAttribLocation(program, (int)VROShaderAttribute::Tex, "tex");
    }
    if ((capabilities & (int)VROShaderMask::Color) != 0) {
        glBindAttribLocation(program, (int)VROShaderAttribute::Color, "color_attr");
    }
    if ((capabilities & (int)VROShaderMask::Norm) != 0) {
        glBindAttribLocation(program, (int)VROShaderAttribute::Norm, "normal");
    }

    /*
     Link the program.
     */
    if (!linkProgram(program)) {
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (program) {
            glDeleteProgram(program);
            program = 0;
        }
        
        pabort("Failed to link program %d, name %s", program, shaderName.c_str());
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
    pinfo("Finished compiling shader %s", shaderName.c_str());

     if (embeddedVertexSource.c_str() != nullptr) {
         pinfo("Shader vertex source %s", embeddedVertexSource.c_str());
     }
     if (embeddedFragmentSource.c_str() != nullptr) {
         pinfo("Shader frag source %s", embeddedFragmentSource.c_str());
     }
#endif

    return true;
}

bool VROShaderProgram::bind() {
    if (failedToLink) {
        return false;
    }

    passert (isHydrated());
    glUseProgram(program);

    // Bind uniforms locations here, if required.
    if (uniformsNeedRebind) {
        findTransformUniformLocations();
        findUniformLocations();

        uniformsNeedRebind = false;
    }

    return true;
}

void VROShaderProgram::unbind() {
    glUseProgram(0);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Get/Set Uniform and Transform Data
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Set Transform

int VROShaderProgram::getUniformIndex(const std::string &name) {
    int idx = 0;

    for (VROUniform *uniform : uniforms) {
        if (uniform->name == name) {
            return idx;
        }

        ++idx;
    }

    return -1;
}

VROUniform *VROShaderProgram::getUniform(const std::string &name) {
    for (VROUniform *uniform : uniforms) {
        if (uniform->name == name) {
            return uniform;
        }
    }

    return nullptr;
}

VROUniform *VROShaderProgram::getUniform(int index) {
    return uniforms[index];
}

void VROShaderProgram::setUniformValue(void *value, const std::string &name) {
    VROUniform *uniform = getUniform(name);
    if (uniform != nullptr) {
        uniform->set(value);
    }
}

void VROShaderProgram::setVertexTransform(const float *mvpMatrix) {
    if (xforms[(int)VROShaderXForm::MVP] >= 0) {
        glUniformMatrix4fv(xforms[(int)VROShaderXForm::MVP], 1, GL_FALSE, mvpMatrix);
    }
}

void VROShaderProgram::setTexTransform(const float *tex) {
    if ((capabilities & (int)VROShaderMask::Tex) != 0) {
        glUniformMatrix4fv(xforms[(int)VROShaderXForm::Tex], 1, GL_FALSE, tex);
    }
}

void VROShaderProgram::setNormTransform(const float *norm) {
    if (normTransformsSet) {
        return;
    }

    if ((capabilities & (int)VROShaderMask::Norm) != 0) {
        glUniformMatrix4fv(xforms[(int)VROShaderXForm::Norm], 1, GL_FALSE, norm);
    }

    normTransformsSet = true;
}

const std::string &VROShaderProgram::getVertexSource() const {
    return embeddedVertexSource;
}

const std::string &VROShaderProgram::getFragmentSource() const {
    return embeddedFragmentSource;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Fast (Common) Uniform Access
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Fast (Common) Uniform Access

VROUniform1f *VROShaderProgram::getAlphaUniform() {
    int index = getAlphaUniformIndex();
    return index >= 0 ? (VROUniform1f *) uniforms[index] : nullptr;
}

int VROShaderProgram::getAlphaUniformIndex() {
    if (!hasAlphaUniform) {
        return -1;
    }

    if (alphaUniformIndex == -1) {
        alphaUniformIndex = getUniformIndex("alpha");
        if (alphaUniformIndex == -1) {
            hasAlphaUniform = false;
        }
    }

    return alphaUniformIndex;
}

VROUniform3f *VROShaderProgram::getColorUniform() {
    int index = getColorUniformIndex();
    return index >= 0 ? (VROUniform3f *) uniforms[index] : nullptr;
}

int VROShaderProgram::getColorUniformIndex() {
    if (!hasColorUniform) {
        return -1;
    }

    if (colorUniformIndex == -1) {
        colorUniformIndex = getUniformIndex("color");

        if (colorUniformIndex == -1) {
            hasColorUniform = false;
        }
    }

    return colorUniformIndex;
}
