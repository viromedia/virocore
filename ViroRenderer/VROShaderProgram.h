//
//  VROShaderProgram.h
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROSHADERPROGRAM_H_
#define VROSHADERPROGRAM_H_

#include <stdlib.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#import <GLKit/GLKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

/*
 Attribute indices for this shader program. These are bound to variable names in the program.
 All shaders programs share these same attributes, but not all programs USE them.
 */
enum class VROShaderAttribute : int {
    Verts,
    Tex,
    Color,
    Norm,
    NUM_ATTRIBUTES
};

/*
 Uniforms used by all shaders. The location of each of these uniforms must be populated in
 the xforms array.
 */
enum class VROShaderXForm {
    MVP,      // uniform mat4 mvp_matrix       (model * view * projection)
    Tex,     // uniform mat4 tex_norm_matrix
    Norm,     // uniform mat4 norm_norm_matrix

    NUM_TRANSFORMATIONS
};

/*
 The various types of properties that may be set for a shader.
 */
enum class VROShaderProperty {
    Bool,
    Int,
    Float,
    Vec2,
    Vec3,
    Vec4,
    BVec2,
    BVec3,
    BVec4,
    IVec2,
    IVec3,
    IVec4,
    Mat2,
    Mat3,
    Mat4
} ;

/*
 Bit-mask indicating support for each shader attribute.
 */
enum class VROShaderMask {
    Tex = 1,
    Color = 2,
    Norm = 4,
};

/*
 Supported versions of GLSL.
 */
enum class GLSLVersion {
    V0,
    V3,
};

/*
 Wraps around a uniform variable, holding its location in the shader and the glUniform
 function needed to set its value.
 */
class VROUniform {
public:
    const std::string name;
    const VROShaderProperty type;
    int location;
    const int arraySize;

    VROUniform(const std::string &name, VROShaderProperty type, int arraySize) :
        name(name),
        type(type),
        location(-1),
        arraySize(arraySize) {
    }

    virtual ~VROUniform() {

    }

    virtual void set(void *value) = 0;

    inline void setLocation(int location) {
        this->location = location;
    }

    virtual void reset() {
        //sublcass to reset any cached value
    }
};

class VROUniform1i: public VROUniform {
public:
    VROUniform1i(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::Int, arraySize), curValue(0) {
    }

    void set(void *value) {
        GLint *val = (GLint *) value;

        if (*val != curValue) {
            glUniform1iv(location, arraySize, val);
            curValue = *val;
        }
    }

    void reset() {
        curValue = 0;
    }

private:
    int curValue;

};

class VROUniform2i: public VROUniform {
public:
    VROUniform2i(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::IVec2, arraySize) {
    }

    void set(void *value) {
        glUniform2iv(location, arraySize, (GLint *) value);
    }

};

class VROUniform3i: public VROUniform {
public:
    VROUniform3i(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::IVec3, arraySize) {
    }

    void set(void *value) {
        glUniform3iv(location, arraySize, (GLint *) value);
    }

};

class VROUniform4i: public VROUniform {
public:
    VROUniform4i(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::IVec4, arraySize) {
    }

    void set(void *value) {
        glUniform4iv(location, arraySize, (GLint *) value);
    }

};

class VROUniform1f: public VROUniform {
public:

    VROUniform1f(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::Float, arraySize), curValue(9999) {
    }

    void set(void *value) {
        GLfloat *val = (GLfloat *) value;

        if (arraySize > 1 || *val != curValue) {
            glUniform1f(location, *val);
            curValue = *val;
        }
    }

     void reset() {
        curValue = 9999;
    }

private:
    float curValue;

};

class VROUniform2f: public VROUniform {
public:
    VROUniform2f(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::Vec2, arraySize) {
    }

    void set(void *value) {
        glUniform2fv(location, arraySize, (GLfloat *) value);
    }

};

class VROUniform3f: public VROUniform {
public:
    VROUniform3f(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::Vec3, arraySize) {
        curValue[0] = 0;
        curValue[1] = 0;
        curValue[2] = 0;
    }

    void set(void *value) {
        GLfloat *val = (GLfloat *) value;
        if (arraySize > 1 || memcmp(val, curValue, sizeof(GLfloat) * 3) != 0) {
            glUniform3fv(location, arraySize, val);
            memcpy(curValue, val, sizeof(GLfloat) * 3);
        }
    }

    void getCurrentValue(void *result) {
        memcpy(result, curValue, sizeof(float) * 3);
    }

    void reset() {
        curValue[0] = 0;
        curValue[1] = 0;
        curValue[2] = 0;
    }

private:
    GLfloat curValue[3];

};

class VROUniform4f: public VROUniform {
public:
    VROUniform4f(const std::string &name, int arraySize) :
        VROUniform(name, VROShaderProperty::Vec4, arraySize) {
    }

    void set(void *value) {
        glUniform4fv(location, arraySize, (GLfloat *) value);
    }

};

class VROUniformMat2: public VROUniform {
public:
    VROUniformMat2(const std::string &name) :
        VROUniform(name, VROShaderProperty::Mat2, 1) {
    }

    void set(void *value) {
        glUniformMatrix2fv(location, arraySize, GL_FALSE, (GLfloat *) value);
    }

};

class VROUniformMat3: public VROUniform {
public:
    VROUniformMat3(const std::string &name) :
        VROUniform(name, VROShaderProperty::Mat3, 1) {
    }

    void set(void *value) {
        glUniformMatrix3fv(location, arraySize, GL_FALSE, (GLfloat *) value);
    }

};

class VROUniformMat4: public VROUniform {
public:
    VROUniformMat4(const std::string &name) :
        VROUniform(name, VROShaderProperty::Mat4, 1) {
    }

    void set(void *value) {
        glUniformMatrix4fv(location, arraySize, GL_FALSE, (GLfloat *) value);
    }

};

class VROShaderProgram {
public:

    /*
     The MD5 of this shader. Used to uniquely identify the shader on the server.
     */
    const std::string shaderMD5;

    /*
     Create a new shader program with the given name and capabilities. This constructor assumes that the
     shader code is bundled with the application. The uniforms used by the shader may then be set using the
     various setUniforms(..) and setSamplers(..) methods.
     */
    VROShaderProgram(const char *name, int capabilities);
    void setUniforms(VROShaderProperty *uniformTypes, const char **names, int count);
    void setSamplers(const char **names, int count);

    virtual ~VROShaderProgram();

    /*
     Get the VROUniform setter given a name or index. The name accessors are much slower
     as they iterate through each uniform.
     */
    int getUniformIndex(const std::string &name);
    VROUniform *getUniform(const std::string &name);
    VROUniform *getUniform(int index);
    void setUniformValue(void *value, const std::string &name);

    /*
     Add a new uniform to this shader. Faster to invoke setUniforms(..).
     */
    void addUniform(VROShaderProperty type, int arraySize, const std::string &name);

    /*
     Add a new sampler to this shader. The shader code must reference the sampler by
     the given name.
     
     @ThreadRestricted("Renderer")
     */
    void addSampler(const std::string &name);

    /*
     Add a new attribute to this shader. May only be invoked prior to compile.
     */
    void addAttribute(VROShaderAttribute attribute);

    /*
     Hydration, for shaders, involves compiling and linking the shader program so it can be
     run by the GPU.
     */
    bool hydrate();
    void evict();
    bool isHydrated();

    /*
     Bind this shader program, or unbind any program. Returns NO if the program was already bound.
     */
    bool bind();
    static void unbind();

    /*
     Set the transformation matrices for vertices.
     */
    void setVertexTransform(const float *mvpMatrix);

    /*
     Set the transformation matrices for textures coordinates and normals.
     */
    void setTexTransform(const float *tex);
    void setNormTransform(const float *norm);

    /*
     Retrieve cached, frequently accessed uniform types. Returns nullptr if the shader does not
     support the requested uniform.
     */
    VROUniform1f *getAlphaUniform();
    int getAlphaUniformIndex();

    VROUniform3f *getColorUniform();
    int getColorUniformIndex();

    /*
     Get the vertex and fragment source code for this shader.
     */
    const std::string &getVertexSource() const;
    const std::string &getFragmentSource() const;

    /*
     The GLSL version this shader is written for.
     */
    inline GLSLVersion getGLSLVersion() const {
        return glslVersion;
    }

    inline int getNumUniforms() const {
        return (int) uniforms.size();
    }
    std::vector<VROUniform *> &getUniforms() {
        return uniforms;
    }

    inline const std::string &getName() const {
        return shaderName;
    }

private:

    /*
     VROUniform for each uniform in the shader. The VROUniform holds both the location of the uniform
     and the proper glUniform function to invoke to set its value.
     */
    std::vector<VROUniform *> uniforms;

    /*
     The capabilities of this shader, as defined by the VROShader enum above.
     */
    int capabilities;

    /*
     True if the norm transform matrix has been set (at any time) for this shader.
     */
    bool normTransformsSet;

    /*
     True if the uniforms used by this shader have changed and require a rebind.
     This is always true after a shader is compiled.
     */
    bool uniformsNeedRebind;

    /*
     The name of the shader.
     */
    std::string shaderName;

    /*
     The source code of the shader.
     */
    std::string embeddedVertexSource;
    std::string embeddedFragmentSource;

    /*
     Integer identifying the program.
     */
    GLuint program;

    /*
     True if the shader failed to compile or link.
     */
    bool failedToLink;

    /*
     Transformation matrices corresponding to each possible attribute, except
     for color, which has no transformation. This array links VROShaderAttribute
     types to the location of the corresponding uniform transformation matrix
     in the shader.
     */
    GLint xforms[static_cast<int>(VROShaderXForm::NUM_TRANSFORMATIONS)];

    /*
     List of the names of all samplers used by this shader.
     */
    std::vector<std::string> samplers;

    /*
     Cached alpha uniform index.
     */
    int alphaUniformIndex;
    bool hasAlphaUniform;

    /*
     Cached color uniform index.
     */
    int colorUniformIndex;
    bool hasColorUniform;

    /*
     Cached offset uniform index.
     */
    int offsetUniformIndex;
    bool hasOffsetUniform;

    /*
     The GLSL version this shader is written in.
     */
    GLSLVersion glslVersion;

    /*
     Compile and link the shader. Returns true on success.
     */
    bool compileAndLink();

    /*
     Compile, link, and validate the shader at the given path. Type indicates fragment or vertex.
     */
    bool compileShader(GLuint *shader, GLenum type, const char *source);
    bool linkProgram(GLuint prog);
    bool validateProgram(GLuint prog);

    /*
     Find the location of each transform and populate the xforms array accordingly.
     Requires an EGL context.
     */
    void findTransformUniformLocations();

    /*
     Set the location of each uniform in the uniformMap and each sampler in the samplers
     list. Requires an EGL context and requires that this shader is bound.
     */
    void findUniformLocations();

};

extern void checkGlError(const char* op);

#endif /* VROSHADERPROGRAM_H_ */
