//
//  VROTextureReader.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROTextureReader.h"
#include "VROStringUtil.h"
#include "VROFrameSynchronizer.h"
#include "VROData.h"
#include "VRORenderContext.h"

static const int kCoordsPerVertex = 3;
static const int kTexcoordsPerVertex = 2;

static const float kQuadCoords[] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f, +1.0f, 0.0f,
        +1.0f, -1.0f, 0.0f,
        +1.0f, +1.0f, 0.0f,
};

static const float kQuadTexCoords[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
};

static const std::string kQuadRenderingVertexShader =
std::string("// Vertex shader.\n") +
std::string("attribute vec4 a_Position;\n") +
std::string("attribute vec2 a_TexCoord;\n") +
std::string("varying vec2 v_TexCoord;\n") +
std::string("void main() {\n") +
std::string("   gl_Position = a_Position;\n") +
std::string("   v_TexCoord = a_TexCoord;\n") +
std::string("}");

static const std::string kQuadRenderingFragmentShaderRGBA8 =
std::string("// Fragment shader that renders to a RGBA texture.\n") +
std::string("#extension GL_OES_EGL_image_external : require\n") +
std::string("precision mediump float;\n") +
std::string("varying vec2 v_TexCoord;\n") +
std::string("uniform samplerExternalOES sTexture;\n") +
std::string("void main() {\n") +
std::string("    gl_FragColor = texture2D(sTexture, v_TexCoord);\n") +
std::string("}");

static const std::string kQuadRenderingFragmentShaderI8 =
        std::string("// Fragment shader that renders to a grayscale texture.\n") +
std::string("#extension GL_OES_EGL_image_external : require\n") +
std::string("precision mediump float;\n") +
std::string("varying vec2 v_TexCoord;\n") +
std::string("uniform samplerExternalOES sTexture;\n") +
std::string("void main() {\n") +
std::string("    vec4 color = texture2D(sTexture, v_TexCoord);\n") +
std::string("    gl_FragColor.r = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;\n") +
std::string("}");

VROTextureReader::VROTextureReader(int textureId, int textureWidth, int textureHeight,
                                   int outputWidth, int outputHeight,
                                   int framesPerUpdate,
                                   VROTextureReaderOutputFormat format,
                                   std::function<void(std::shared_ptr<VROData>)> callback) :

    _framesPerUpdate(framesPerUpdate),
    _initialized(false),
    _lastFrameAcquired(0),
    _callback(callback),
    _imageFormat(format),
    _textureId(textureId),
    _textureWidth(textureWidth),
    _textureHeight(textureHeight),
    _outputWidth(outputWidth),
    _outputHeight(outputHeight),
    _frontIndex(-1),
    _backIndex(-1) {

    if (_imageFormat == VROTextureReaderOutputFormat::RGBA8) {
        _pixelBufferSize = _outputWidth * _outputHeight * 4;
    } else if (_imageFormat == VROTextureReaderOutputFormat::I8) {
        _pixelBufferSize = _outputWidth * _outputHeight;
    }

    for (int i = 0; i < 8; i++) {
        _texcoords[i] = kQuadTexCoords[i];
    }
}

bool VROTextureReader::init() {
    // Create framebuffers and PBOs
    glGenBuffers(kTextureReaderBufferCount, _pbo);
    glGenFramebuffers(kTextureReaderBufferCount, _frameBuffer);
    glGenTextures(kTextureReaderBufferCount, _texture);

    for (int i = 0; i < kTextureReaderBufferCount; i++) {
        _bufferUsed[i] = false;
        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer[i]);

        glBindTexture(GL_TEXTURE_2D, _texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0,
                     _imageFormat == VROTextureReaderOutputFormat::I8 ? GL_R8 : GL_RGBA,
                     _outputWidth, _outputHeight,
                     0,
                     _imageFormat == VROTextureReaderOutputFormat::I8 ? GL_RED : GL_RGBA,
                     GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture[i], 0);

        int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            pwarn("TextureReader: failed to set up render buffer with status %d", status);
            return false;
        }

        // Setup PBOs
        glBindBuffer(GL_PIXEL_PACK_BUFFER, _pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, _pixelBufferSize, NULL, GL_DYNAMIC_READ);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    // Load shader program
    int vertexShader   = loadGLShader(GL_VERTEX_SHADER, kQuadRenderingVertexShader);
    int fragmentShader = loadGLShader(GL_FRAGMENT_SHADER,
                                      _imageFormat == VROTextureReaderOutputFormat::I8
                                      ? kQuadRenderingFragmentShaderI8
                                      : kQuadRenderingFragmentShaderRGBA8);
    _quadProgram = glCreateProgram();
    glAttachShader(_quadProgram, vertexShader);
    glAttachShader(_quadProgram, fragmentShader);
    glLinkProgram(_quadProgram);
    glUseProgram(_quadProgram);

    _quadPositionAttribute = glGetAttribLocation(_quadProgram, "a_Position");
    _quadTexcoordAttribute = glGetAttribLocation(_quadProgram, "a_TexCoord");
    int texLoc = glGetUniformLocation(_quadProgram, "sTexture");
    glUniform1i(texLoc, 0);
    _initialized = true;

    return true;
}

VROTextureReader::~VROTextureReader() {
    if (_initialized) {
        glDeleteFramebuffers(kTextureReaderBufferCount, _frameBuffer);
        glDeleteTextures(kTextureReaderBufferCount, _texture);
        glDeleteBuffers(kTextureReaderBufferCount, _pbo);
    }
}

bool VROTextureReader::start(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer) {
    if (!_initialized) {
        pwarn("Texture reader not initialized, will not start reading frames!");
        return false;
    }
    frameSynchronizer->addFrameListener(shared_from_this());
    return true;
}

void VROTextureReader::stop(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer) {
    frameSynchronizer->removeFrameListener(shared_from_this());
}

void VROTextureReader::onFrameWillRender(const VRORenderContext &context) {
    // No-op
}

void VROTextureReader::onFrameDidRender(const VRORenderContext &context) {
    // Use a double-buffering system, swapping buffers every _framePerUpdate
    // frames. On the first frame, read into the back buffer. After
    // _framesPerUpdate frames, swap and read into the front buffer, and
    // acquire the image in the new front buffer to be sent to the application
    int frame = context.getFrame();
    if (frame >= _lastFrameAcquired + _framesPerUpdate) {
        if (_frontIndex != -1) {
            releaseImage(_frontIndex);
        }

        // Move previous back buffer to front buffer.
        _frontIndex = _backIndex;
        // Submit new request on back buffer
        _backIndex = queueTextureRead();

        // Acquire frame from the new front buffer
        if (_frontIndex != -1) {
            std::shared_ptr<VROData> data = acquireImage(_frontIndex);
            if (data) {
                _callback(data);
            }
        }
        _lastFrameAcquired = frame;
    }
}

int VROTextureReader::queueTextureRead() {
    // Find next buffer.
    int bufferIndex = -1;
    for (int i = 0; i < kTextureReaderBufferCount; i++) {
        if (!_bufferUsed[i]) {
            bufferIndex = i;
            break;
        }
    }
    if (bufferIndex == -1) {
        pwarn("Texture reader: no buffers available for texture read, buffers must be released");
        return -1;
    }

    // Bind both read and write to framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer[bufferIndex]);

    // Save and setup viewport
    int savedViewport[4];
    glGetIntegerv(GL_VIEWPORT, savedViewport);
    glViewport(0, 0, _outputWidth, _outputHeight);

    // Draw texture to framebuffer
    drawTexture();

    // Start reading into PBO
    glBindBuffer(GL_PIXEL_PACK_BUFFER, _pbo[bufferIndex]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, _outputWidth, _outputHeight,
                 _imageFormat == VROTextureReaderOutputFormat::I8 ? GL_RED : GL_RGBA,
                 GL_UNSIGNED_BYTE, 0);

    // Restore viewport.
    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    _bufferUsed[bufferIndex] = true;
    return bufferIndex;
}

std::shared_ptr<VROData> VROTextureReader::acquireImage(int bufferIndex) {
    if (bufferIndex < 0 || bufferIndex >= kTextureReaderBufferCount || !_bufferUsed[bufferIndex]) {
        pwarn("Texture reader: invalid buffer index, failed to acquire output image");
        return nullptr;
    }

    // Bind the current PB and acquire the pixel buffer.
    glBindBuffer(GL_PIXEL_PACK_BUFFER, _pbo[bufferIndex]);

    void *mapped = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, _pixelBufferSize, GL_MAP_READ_BIT);
    if (mapped == nullptr) {
        return nullptr;
    }
    return std::make_shared<VROData>(mapped, _pixelBufferSize, VRODataOwnership::Wrap);
}

bool VROTextureReader::releaseImage(int bufferIndex) {
    if (bufferIndex < 0 || bufferIndex >= kTextureReaderBufferCount || !_bufferUsed[bufferIndex]) {
        pwarn("Texture reader: failed to release buffer, invalid buffer index %d", bufferIndex);
        return false;
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, _pbo[bufferIndex]);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    _bufferUsed[bufferIndex] = false;

    return true;
}

void VROTextureReader::setTextureCoordinates(VROVector3f BL, VROVector3f BR, VROVector3f TL,
                                             VROVector3f TR) {
    _texcoords[0] = BL.x;
    _texcoords[1] = BL.y;
    _texcoords[2] = TL.x;
    _texcoords[3] = TL.y;
    _texcoords[4] = BR.x;
    _texcoords[5] = BR.y;
    _texcoords[6] = TR.x;
    _texcoords[7] = TR.y;
}

void VROTextureReader::drawTexture() {
    // Disable features that we don't use.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glDepthMask(false);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Clear buffers.
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the vertex and texcoord pointers
    glVertexAttribPointer(_quadPositionAttribute, kCoordsPerVertex, GL_FLOAT, false, 0, kQuadCoords);
    glVertexAttribPointer(_quadTexcoordAttribute, kTexcoordsPerVertex, GL_FLOAT, false, 0, _texcoords);

    // Enable vertex arrays
    glEnableVertexAttribArray(_quadPositionAttribute);
    glEnableVertexAttribArray(_quadTexcoordAttribute);

    glUseProgram(_quadProgram);

    // Select input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureId);

    // Draw a quad with texture
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Disable vertex arrays
    glDisableVertexAttribArray(_quadPositionAttribute);
    glDisableVertexAttribArray(_quadTexcoordAttribute);

    // Reset texture binding
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
}

GLuint VROTextureReader::loadGLShader(int type, std::string code) {
    const char *source = code.c_str();
    int sourceLength = code.length();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &sourceLength);
    glCompileShader(shader);

    // Get the compilation status.
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    // If the compilation failed, delete the shader.
    if (compileStatus == 0) {
        pwarn("Error compiling shader");
        VROStringUtil::printCode(code);
        glDeleteShader(shader);
        return 0;
    }

    if (shader == 0) {
        pwarn("Texture reader: failed to create shader");
        VROStringUtil::printCode(code);
        return 0;
    }

    return shader;
}