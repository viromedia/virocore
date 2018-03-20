//
//  VROTextureReader.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef ANDROID_VROTEXTUREREADER_H
#define ANDROID_VROTEXTUREREADER_H

#include <memory>
#include <functional>
#include "VROFrameListener.h"
#include "VROOpenGL.h"
#include "VROVector3f.h"

class VROData;
class VROFrameSynchronizer;

static const int kTextureReaderBufferCount = 2;

enum class VROTextureReaderOutputFormat {
    RGBA8,
    I8
};

/*
 VROTexture reader can be used to continuously read any OpenGL texture in an asynchronous way.
 Usage:

 1. Create the and initialize:
    _reader = std::make_shared<VROTextureReader>(textureId,       // The texture to read
                                                 width, height,   // Width and height of the texture
                                                 width, height,   // Width and height of the output image
                                                 N,               // Read the texture every N frames
                                                 VROTextureReaderOutputFormat::RGBA8,  // Output image format
                [width, height, this] (std::shared_ptr<VROData> data) {
                    // Do something with the image data read from the texture. This will be called
                    // every N frames
                });
    _reader->setTextureCoordinates(BL, BR, TL, TR);    // Optional, to read a portion of the texture
    _reader->init();                                   // Initialize the reader (only call once)

 2. Start getting callbacks:

   _reader->start(_renderer->getFrameSynchronizer());

 3. Stop getting callbacks:

   _reader->stop(_renderer->getFrameSynchronizer());
 */
class VROTextureReader : public VROFrameListener, public std::enable_shared_from_this<VROTextureReader> {

public:

    /*
     Construct a texture reader that will read the texture with the given ID every framesPerUpdate
     frames. The given callback will be invoked after each read. This function is asynchronous.
     */
    VROTextureReader(int textureId, int textureWidth, int textureHeight,
                     int outputWidth, int outputHeight,
                     int framesPerUpdate,
                     VROTextureReaderOutputFormat format,
                     std::function<void(std::shared_ptr<VROData>)> callback);

    virtual ~VROTextureReader();

    /*
     Initialize the texture reader on the rendering thread. Returns false on failure.
     */
    bool init();

    /*
     Set the texture coordinates that we should read from the texture.
     */
    void setTextureCoordinates(VROVector3f BL, VROVector3f BR, VROVector3f TL, VROVector3f TR);

    /*
     Start and stop texture reading.
     */
    bool start(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer);
    void stop(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer);

    // Internal
    void onFrameWillRender(const VRORenderContext &context);
    void onFrameDidRender(const VRORenderContext &context);

private:

    /*
     The frequency of updates to the VROTextureReader callback, in frames. If this is set to 3,
     for example, then the callback will receive the image buffer data once every three frames.
     */
    int _framesPerUpdate;
    bool _initialized;
    int _lastFrameAcquired;
    std::function<void(std::shared_ptr<VROData>)> _callback;

    VROTextureReaderOutputFormat _imageFormat;
    GLuint _textureId;
    int _textureWidth, _textureHeight;
    int _outputWidth, _outputHeight;
    int _frontIndex, _backIndex;
    int _pixelBufferSize;
    float _texcoords[8];

    GLuint _pbo[kTextureReaderBufferCount];
    GLuint _frameBuffer[kTextureReaderBufferCount];
    GLuint _texture[kTextureReaderBufferCount];
    bool _bufferUsed[kTextureReaderBufferCount];
    GLuint _quadProgram;
    GLint _quadPositionAttribute;
    GLint _quadTexcoordAttribute;

    /*
     Queue the texture for reading. The texture will be asynchronously read into a buffer,
     and the index to that buffer is immediately returned by this method.
     */
    int queueTextureRead();

    /*
     Get the data for the texture that was submitted via queueRead in a past frame.
     */
    std::shared_ptr<VROData> acquireImage(int bufferIndex);

    /*
     This must be invoked when done with the image data in the buffer.
     */
    bool releaseImage(int bufferIndex);

    /*
     Load the shader with the given code. Returns the GL handle.
     */
    GLuint loadGLShader(int type, std::string code);

    /*
     Draw the texture to the active OpenGL context.
     */
    void drawTexture();

};


#endif //ANDROID_VROTEXTUREREADER_H
