//
//  VROSceneRendererOVR.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSceneRendererOVR.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>					// for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/native_window_jni.h>	// for native window JNI
#include <android/input.h>
#include "VROPlatformUtil.h"
#include "VRODriverOpenGLAndroidOVR.h"
#include "VROAllocationTracker.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <VrApi.h>
#include <VrApi_Types.h>
#include <VROTime.h>

#pragma mark - OVR System

#if !defined( EGL_OPENGL_ES3_BIT_KHR )
#define EGL_OPENGL_ES3_BIT_KHR		0x0040
#endif

#if !defined( GL_EXT_multisampled_render_to_texture )
typedef void (GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
#endif

#if !defined( GL_OVR_multiview )
static const int GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR       = 0x9630;
static const int GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR = 0x9632;
static const int GL_MAX_VIEWS_OVR                                      = 0x9631;
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
#endif

#if !defined( GL_OVR_multiview_multisampled_render_to_texture )
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei samples, GLint baseViewIndex, GLsizei numViews);
#endif

// Must use EGLSyncKHR because the VrApi still supports OpenGL ES 2.0
#define EGL_SYNC

#if defined EGL_SYNC
// EGL_KHR_reusable_sync
PFNEGLCREATESYNCKHRPROC			eglCreateSyncKHR;
PFNEGLDESTROYSYNCKHRPROC		eglDestroySyncKHR;
PFNEGLCLIENTWAITSYNCKHRPROC		eglClientWaitSyncKHR;
PFNEGLSIGNALSYNCKHRPROC			eglSignalSyncKHR;
PFNEGLGETSYNCATTRIBKHRPROC		eglGetSyncAttribKHR;
#endif

#include "VrApi.h"
#include "VrApi_Helpers.h"
#include "VrApi_SystemUtils.h"
#include "VROInputControllerOVR.h"

#define DEBUG 1

#define ALOGE(...) __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__ )
#if DEBUG
#define ALOGV(...) __android_log_print( ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__ )
#else
#define ALOGV(...)
#endif

static const int CPU_LEVEL			= 2;
static const int GPU_LEVEL			= 3;
static const int NUM_MULTI_SAMPLES	= 4;

/*
================================================================================

OVR to VRO Utility Functions

================================================================================
*/

static VROMatrix4f toMatrix4f(ovrMatrix4f in)
{
    float m[16] = {
            in.M[0][0], in.M[1][0], in.M[2][0], in.M[3][0],
            in.M[0][1], in.M[1][1], in.M[2][1], in.M[3][1],
            in.M[0][2], in.M[1][2], in.M[2][2], in.M[3][2],
            in.M[0][3], in.M[1][3], in.M[2][3], in.M[3][3],
    };

    return VROMatrix4f(m);
}

/*
================================================================================

OpenGL-ES Utility Functions

================================================================================
*/

typedef struct
{
    bool multi_view;			// GL_OVR_multiview, GL_OVR_multiview2
} OpenGLExtensions_t;

OpenGLExtensions_t glExtensions;

static void EglInitExtensions()
{
#if defined EGL_SYNC
    eglCreateSyncKHR		= (PFNEGLCREATESYNCKHRPROC)			eglGetProcAddress( "eglCreateSyncKHR" );
    eglDestroySyncKHR		= (PFNEGLDESTROYSYNCKHRPROC)		eglGetProcAddress( "eglDestroySyncKHR" );
    eglClientWaitSyncKHR	= (PFNEGLCLIENTWAITSYNCKHRPROC)		eglGetProcAddress( "eglClientWaitSyncKHR" );
    eglSignalSyncKHR		= (PFNEGLSIGNALSYNCKHRPROC)			eglGetProcAddress( "eglSignalSyncKHR" );
    eglGetSyncAttribKHR		= (PFNEGLGETSYNCATTRIBKHRPROC)		eglGetProcAddress( "eglGetSyncAttribKHR" );
#endif

    // get extension pointers
    const char * allExtensions = (const char *)glGetString( GL_EXTENSIONS );
    if ( allExtensions != NULL )
    {
        glExtensions.multi_view = strstr( allExtensions, "GL_OVR_multiview2" ) &&
                                  strstr( allExtensions, "GL_OVR_multiview_multisampled_render_to_texture" );
    }
}

static const char * EglErrorString( const EGLint error )
{
    switch ( error )
    {
        case EGL_SUCCESS:				return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:		return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:			return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:				return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:			return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT:			return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG:			return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE:	return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:			return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE:			return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH:				return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER:			return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP:		return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:		return "EGL_BAD_NATIVE_WINDOW";
        case EGL_CONTEXT_LOST:			return "EGL_CONTEXT_LOST";
        default:						return "unknown";
    }
}

static const char * GlFrameBufferStatusString( GLenum status )
{
    switch ( status )
    {
        case GL_FRAMEBUFFER_UNDEFINED:						return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        case GL_FRAMEBUFFER_UNSUPPORTED:					return "GL_FRAMEBUFFER_UNSUPPORTED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        default:											return "unknown";
    }
}

/*
================================================================================

ovrEgl

================================================================================
*/

typedef struct
{
    EGLint		MajorVersion;
    EGLint		MinorVersion;
    EGLDisplay	Display;
    EGLConfig	Config;
    EGLSurface	TinySurface;
    EGLSurface	MainSurface;
    EGLContext	Context;
} ovrEgl;

static void ovrEgl_Clear( ovrEgl * egl )
{
    egl->MajorVersion = 0;
    egl->MinorVersion = 0;
    egl->Display = 0;
    egl->Config = 0;
    egl->TinySurface = EGL_NO_SURFACE;
    egl->MainSurface = EGL_NO_SURFACE;
    egl->Context = EGL_NO_CONTEXT;
}

static void ovrEgl_CreateContext( ovrEgl * egl, const ovrEgl * shareEgl )
{
    if ( egl->Display != 0 )
    {
        return;
    }

    egl->Display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    ALOGV( "        eglInitialize( Display, &MajorVersion, &MinorVersion )" );
    eglInitialize( egl->Display, &egl->MajorVersion, &egl->MinorVersion );
    // Do NOT use eglChooseConfig, because the Android EGL code pushes in multisample
    // flags in eglChooseConfig if the user has selected the "force 4x MSAA" option in
    // settings, and that is completely wasted for our warp target.
    const int MAX_CONFIGS = 1024;
    EGLConfig configs[MAX_CONFIGS];
    EGLint numConfigs = 0;
    if ( eglGetConfigs( egl->Display, configs, MAX_CONFIGS, &numConfigs ) == EGL_FALSE )
    {
        ALOGE( "        eglGetConfigs() failed: %s", EglErrorString( eglGetError() ) );
        return;
    }
    const EGLint configAttribs[] =
            {
                    EGL_RED_SIZE,		8,
                    EGL_GREEN_SIZE,		8,
                    EGL_BLUE_SIZE,		8,
                    EGL_ALPHA_SIZE,		8, // need alpha for the multi-pass timewarp compositor
                    EGL_DEPTH_SIZE,		0,
                    EGL_STENCIL_SIZE,	0,
                    EGL_SAMPLES,		0,
                    EGL_NONE
            };
    egl->Config = 0;
    for ( int i = 0; i < numConfigs; i++ )
    {
        EGLint value = 0;

        eglGetConfigAttrib( egl->Display, configs[i], EGL_RENDERABLE_TYPE, &value );
        if ( ( value & EGL_OPENGL_ES3_BIT_KHR ) != EGL_OPENGL_ES3_BIT_KHR )
        {
            continue;
        }

        // The pbuffer config also needs to be compatible with normal window rendering
        // so it can share textures with the window context.
        eglGetConfigAttrib( egl->Display, configs[i], EGL_SURFACE_TYPE, &value );
        if ( ( value & ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT ) ) != ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT ) )
        {
            continue;
        }

        int	j = 0;
        for ( ; configAttribs[j] != EGL_NONE; j += 2 )
        {
            eglGetConfigAttrib( egl->Display, configs[i], configAttribs[j], &value );
            if ( value != configAttribs[j + 1] )
            {
                break;
            }
        }
        if ( configAttribs[j] == EGL_NONE )
        {
            egl->Config = configs[i];
            break;
        }
    }
    if ( egl->Config == 0 )
    {
        ALOGE( "        eglChooseConfig() failed: %s", EglErrorString( eglGetError() ) );
        return;
    }
    EGLint contextAttribs[] =
            {
                    EGL_CONTEXT_CLIENT_VERSION, 3,
                    EGL_NONE
            };
    ALOGV( "        Context = eglCreateContext( Display, Config, EGL_NO_CONTEXT, contextAttribs )" );
    egl->Context = eglCreateContext( egl->Display, egl->Config, ( shareEgl != NULL ) ? shareEgl->Context : EGL_NO_CONTEXT, contextAttribs );
    if ( egl->Context == EGL_NO_CONTEXT )
    {
        ALOGE( "        eglCreateContext() failed: %s", EglErrorString( eglGetError() ) );
        return;
    }
    const EGLint surfaceAttribs[] =
            {
                    EGL_WIDTH, 16,
                    EGL_HEIGHT, 16,
                    EGL_NONE
            };
    ALOGV( "        TinySurface = eglCreatePbufferSurface( Display, Config, surfaceAttribs )" );
    egl->TinySurface = eglCreatePbufferSurface( egl->Display, egl->Config, surfaceAttribs );
    if ( egl->TinySurface == EGL_NO_SURFACE )
    {
        ALOGE( "        eglCreatePbufferSurface() failed: %s", EglErrorString( eglGetError() ) );
        eglDestroyContext( egl->Display, egl->Context );
        egl->Context = EGL_NO_CONTEXT;
        return;
    }
    ALOGV( "        eglMakeCurrent( Display, TinySurface, TinySurface, Context )" );
    if ( eglMakeCurrent( egl->Display, egl->TinySurface, egl->TinySurface, egl->Context ) == EGL_FALSE )
    {
        ALOGE( "        eglMakeCurrent() failed: %s", EglErrorString( eglGetError() ) );
        eglDestroySurface( egl->Display, egl->TinySurface );
        eglDestroyContext( egl->Display, egl->Context );
        egl->Context = EGL_NO_CONTEXT;
        return;
    }
}

static void ovrEgl_DestroyContext( ovrEgl * egl )
{
    if ( egl->Display != 0 )
    {
        ALOGE( "        eglMakeCurrent( Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT )" );
        if ( eglMakeCurrent( egl->Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT ) == EGL_FALSE )
        {
            ALOGE( "        eglMakeCurrent() failed: %s", EglErrorString( eglGetError() ) );
        }
    }
    if ( egl->Context != EGL_NO_CONTEXT )
    {
        ALOGE( "        eglDestroyContext( Display, Context )" );
        if ( eglDestroyContext( egl->Display, egl->Context ) == EGL_FALSE )
        {
            ALOGE( "        eglDestroyContext() failed: %s", EglErrorString( eglGetError() ) );
        }
        egl->Context = EGL_NO_CONTEXT;
    }
    if ( egl->TinySurface != EGL_NO_SURFACE )
    {
        ALOGE( "        eglDestroySurface( Display, TinySurface )" );
        if ( eglDestroySurface( egl->Display, egl->TinySurface ) == EGL_FALSE )
        {
            ALOGE( "        eglDestroySurface() failed: %s", EglErrorString( eglGetError() ) );
        }
        egl->TinySurface = EGL_NO_SURFACE;
    }
    if ( egl->Display != 0 )
    {
        ALOGE( "        eglTerminate( Display )" );
        if ( eglTerminate( egl->Display ) == EGL_FALSE )
        {
            ALOGE( "        eglTerminate() failed: %s", EglErrorString( eglGetError() ) );
        }
        egl->Display = 0;
    }
}

/*
================================================================================

ovrFramebuffer

================================================================================
*/

struct ovrFramebuffer
{
    int						Width;
    int						Height;
    int						Multisamples;
    int						TextureSwapChainLength;
    int						TextureSwapChainIndex;
    bool					UseMultiview;
    ovrTextureSwapChain *	ColorTextureSwapChain;
    GLuint *				DepthBuffers;
    GLuint *				FrameBuffers;
};

static void ovrFramebuffer_Clear( ovrFramebuffer * frameBuffer )
{
    frameBuffer->Width = 0;
    frameBuffer->Height = 0;
    frameBuffer->Multisamples = 0;
    frameBuffer->TextureSwapChainLength = 0;
    frameBuffer->TextureSwapChainIndex = 0;
    frameBuffer->UseMultiview = false;
    frameBuffer->ColorTextureSwapChain = NULL;
    frameBuffer->DepthBuffers = NULL;
    frameBuffer->FrameBuffers = NULL;
}

static bool ovrFramebuffer_Create( ovrFramebuffer * frameBuffer, const bool useMultiview, const ovrTextureFormat colorFormat, const int width, const int height, const int multisamples )
{
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT =
            (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress( "glRenderbufferStorageMultisampleEXT" );
    PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT =
            (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress( "glFramebufferTexture2DMultisampleEXT" );

    PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR =
            (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC) eglGetProcAddress( "glFramebufferTextureMultiviewOVR" );
    PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glFramebufferTextureMultisampleMultiviewOVR =
            (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC) eglGetProcAddress( "glFramebufferTextureMultisampleMultiviewOVR" );

    frameBuffer->Width = width;
    frameBuffer->Height = height;
    frameBuffer->Multisamples = multisamples;
    frameBuffer->UseMultiview = ( useMultiview && ( glFramebufferTextureMultiviewOVR != NULL ) ) ? true : false;

    frameBuffer->ColorTextureSwapChain = vrapi_CreateTextureSwapChain( frameBuffer->UseMultiview ? VRAPI_TEXTURE_TYPE_2D_ARRAY : VRAPI_TEXTURE_TYPE_2D, colorFormat, width, height, 1, true );
    frameBuffer->TextureSwapChainLength = vrapi_GetTextureSwapChainLength( frameBuffer->ColorTextureSwapChain );
    frameBuffer->DepthBuffers = (GLuint *)malloc( frameBuffer->TextureSwapChainLength * sizeof( GLuint ) );
    frameBuffer->FrameBuffers = (GLuint *)malloc( frameBuffer->TextureSwapChainLength * sizeof( GLuint ) );



    ALOGV( "        frameBuffer->UseMultiview = %d", frameBuffer->UseMultiview );

    for ( int i = 0; i < frameBuffer->TextureSwapChainLength; i++ )
    {
        // Create the color buffer texture.
        const GLuint colorTexture = vrapi_GetTextureSwapChainHandle( frameBuffer->ColorTextureSwapChain, i );
        GLenum colorTextureTarget = frameBuffer->UseMultiview ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
        GL( glBindTexture( colorTextureTarget, colorTexture ) );
        GL( glTexParameteri( colorTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
        GL( glTexParameteri( colorTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );
        GL( glTexParameteri( colorTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
        GL( glTexParameteri( colorTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
        GL( glBindTexture( colorTextureTarget, 0 ) );

        if ( frameBuffer->UseMultiview )
        {
            // Create the depth buffer texture.
            GL( glGenTextures( 1, &frameBuffer->DepthBuffers[i] ) );
            GL( glBindTexture( GL_TEXTURE_2D_ARRAY, frameBuffer->DepthBuffers[i] ) );
            GL( glTexStorage3D( GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, width, height, 2 ) );
            GL( glBindTexture( GL_TEXTURE_2D_ARRAY, 0 ) );

            // Create the frame buffer.
            GL( glGenFramebuffers( 1, &frameBuffer->FrameBuffers[i] ) );
            GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[i] ) );
            if ( multisamples > 1 && ( glFramebufferTextureMultisampleMultiviewOVR != NULL ) )
            {
                GL( glFramebufferTextureMultisampleMultiviewOVR( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, frameBuffer->DepthBuffers[i], 0 /* level */, multisamples /* samples */, 0 /* baseViewIndex */, 2 /* numViews */ ) );
                GL( glFramebufferTextureMultisampleMultiviewOVR( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTexture, 0 /* level */, multisamples /* samples */, 0 /* baseViewIndex */, 2 /* numViews */ ) );
            }
            else
            {
                GL( glFramebufferTextureMultiviewOVR( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, frameBuffer->DepthBuffers[i], 0 /* level */, 0 /* baseViewIndex */, 2 /* numViews */ ) );
                GL( glFramebufferTextureMultiviewOVR( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTexture, 0 /* level */, 0 /* baseViewIndex */, 2 /* numViews */ ) );
            }

            GL( GLenum renderFramebufferStatus = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER ) );
            GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
            if ( renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE )
            {
                ALOGE( "Incomplete frame buffer object: %s", GlFrameBufferStatusString( renderFramebufferStatus ) );
                return false;
            }
        }
        else
        {
            if ( multisamples > 1 && glRenderbufferStorageMultisampleEXT != NULL && glFramebufferTexture2DMultisampleEXT != NULL )
            {
                // Create multisampled depth buffer.
                GL( glGenRenderbuffers( 1, &frameBuffer->DepthBuffers[i] ) );
                GL( glBindRenderbuffer( GL_RENDERBUFFER, frameBuffer->DepthBuffers[i] ) );
                GL( glRenderbufferStorageMultisampleEXT( GL_RENDERBUFFER, multisamples, GL_DEPTH_COMPONENT24, width, height ) );
                GL( glBindRenderbuffer( GL_RENDERBUFFER, 0 ) );

                // Create the frame buffer.
                // NOTE: glFramebufferTexture2DMultisampleEXT only works with GL_FRAMEBUFFER.
                GL( glGenFramebuffers( 1, &frameBuffer->FrameBuffers[i] ) );
                GL( glBindFramebuffer( GL_FRAMEBUFFER, frameBuffer->FrameBuffers[i] ) );
                GL( glFramebufferTexture2DMultisampleEXT( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0, multisamples ) );
                GL( glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->DepthBuffers[i] ) );
                GL( GLenum renderFramebufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER ) );
                GL( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
                if ( renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE )
                {
                    ALOGE( "Incomplete frame buffer object: %s", GlFrameBufferStatusString( renderFramebufferStatus ) );
                    return false;
                }
            }
            else
            {
                // Create depth buffer.
                GL( glGenRenderbuffers( 1, &frameBuffer->DepthBuffers[i] ) );
                GL( glBindRenderbuffer( GL_RENDERBUFFER, frameBuffer->DepthBuffers[i] ) );
                GL( glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height ) );
                GL( glBindRenderbuffer( GL_RENDERBUFFER, 0 ) );

                // Create the frame buffer.
                GL( glGenFramebuffers( 1, &frameBuffer->FrameBuffers[i] ) );
                GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[i] ) );
                GL( glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->DepthBuffers[i] ) );
                GL( glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0 ) );
                GL( GLenum renderFramebufferStatus = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER ) );
                GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
                if ( renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE )
                {
                    ALOGE( "Incomplete frame buffer object: %s", GlFrameBufferStatusString( renderFramebufferStatus ) );
                    return false;
                }
            }
        }
    }

    return true;
}

static void ovrFramebuffer_Destroy( ovrFramebuffer * frameBuffer )
{
    GL( glDeleteFramebuffers( frameBuffer->TextureSwapChainLength, frameBuffer->FrameBuffers ) );
    if ( frameBuffer->UseMultiview )
    {
        GL( glDeleteTextures( frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers ) );
    }
    else
    {
        GL( glDeleteRenderbuffers( frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers ) );
    }
    vrapi_DestroyTextureSwapChain( frameBuffer->ColorTextureSwapChain );

    free( frameBuffer->DepthBuffers );
    free( frameBuffer->FrameBuffers );

    ovrFramebuffer_Clear( frameBuffer );
}

static void ovrFramebuffer_SetCurrent( ovrFramebuffer * frameBuffer )
{
    GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[frameBuffer->TextureSwapChainIndex] ) );
}

static void ovrFramebuffer_SetNone()
{
    GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
}

static void ovrFramebuffer_Resolve( ovrFramebuffer * frameBuffer )
{
    // Discard the depth buffer, so the tiler won't need to write it back out to memory.
    const GLenum depthAttachment[1] = { GL_DEPTH_ATTACHMENT };
    glInvalidateFramebuffer( GL_DRAW_FRAMEBUFFER, 1, depthAttachment );

    // Flush this frame worth of commands.
    glFlush();
}

static void ovrFramebuffer_Advance( ovrFramebuffer * frameBuffer )
{
    // Advance to the next texture from the set.
    frameBuffer->TextureSwapChainIndex = ( frameBuffer->TextureSwapChainIndex + 1 ) % frameBuffer->TextureSwapChainLength;
}

/*
================================================================================================================================

ovrFence

================================================================================================================================
*/

typedef struct
{
#if defined( EGL_SYNC )
    EGLDisplay	Display;
    EGLSyncKHR	Sync;
#else
    GLsync		Sync;
#endif
} ovrFence;

static void ovrFence_Create( ovrFence * fence )
{
#if defined( EGL_SYNC )
    fence->Display = 0;
    fence->Sync = EGL_NO_SYNC_KHR;
#else
    fence->Sync = 0;
#endif
}

static void ovrFence_Destroy( ovrFence * fence )
{
#if defined( EGL_SYNC )
    if ( fence->Sync != EGL_NO_SYNC_KHR )
    {
        if ( eglDestroySyncKHR( fence->Display, fence->Sync ) ==  EGL_FALSE )
        {
            ALOGE( "eglDestroySyncKHR() : EGL_FALSE" );
            return;
        }
        fence->Display = 0;
        fence->Sync = EGL_NO_SYNC_KHR;
    }
#else
    if ( fence->Sync != 0 )
	{
		glDeleteSync( fence->Sync );
		fence->Sync = 0;
	}
#endif
}

static void ovrFence_Insert( ovrFence * fence )
{
    ovrFence_Destroy( fence );

#if defined( EGL_SYNC )
    fence->Display = eglGetCurrentDisplay();
    fence->Sync = eglCreateSyncKHR( fence->Display, EGL_SYNC_FENCE_KHR, NULL );
    if ( fence->Sync == EGL_NO_SYNC_KHR )
    {
        ALOGE( "eglCreateSyncKHR() : EGL_NO_SYNC_KHR" );
        return;
    }
    // Force flushing the commands.
    // Note that some drivers will already flush when calling eglCreateSyncKHR.
    if ( eglClientWaitSyncKHR( fence->Display, fence->Sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0 ) == EGL_FALSE )
    {
        ALOGE( "eglClientWaitSyncKHR() : EGL_FALSE" );
        return;
    }
#else
    // Create and insert a new sync object.
	fence->Sync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
	// Force flushing the commands.
	// Note that some drivers will already flush when calling glFenceSync.
	glClientWaitSync( fence->Sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0 );
#endif
}

/*
================================================================================

ovrRenderer

================================================================================
*/

static int MAX_FENCES = 4;

typedef struct
{
    ovrFramebuffer	FrameBuffer[VRAPI_FRAME_LAYER_EYE_MAX];
    ovrFramebuffer  HUDFrameBuffer[VRAPI_FRAME_LAYER_EYE_MAX];
    ovrFence *		Fence;
    int				NumBuffers;
    int             FenceIndex;
} ovrRenderer;

static void ovrRenderer_Clear(ovrRenderer *renderer) {
    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        ovrFramebuffer_Clear( &renderer->FrameBuffer[eye] );
    }
    renderer->NumBuffers = VRAPI_FRAME_LAYER_EYE_MAX;
    renderer->FenceIndex = 0;
}

static void ovrRenderer_Create(ovrRenderer *renderer, const ovrJava *java, const bool useMultiview) {
    renderer->NumBuffers = useMultiview ? 1 : VRAPI_FRAME_LAYER_EYE_MAX;

    // Create the frame buffers.
    for ( int eye = 0; eye < renderer->NumBuffers; eye++ )
    {
        ovrFramebuffer_Create( &renderer->FrameBuffer[eye], useMultiview,
                               VRAPI_TEXTURE_FORMAT_8888,
                               vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH),
                               vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT),
                               NUM_MULTI_SAMPLES);

        ovrFramebuffer_Create( &renderer->HUDFrameBuffer[eye], useMultiview,
                               VRAPI_TEXTURE_FORMAT_8888,
                               vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH),
                               vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT),
                               NUM_MULTI_SAMPLES);

        renderer->Fence = (ovrFence *) malloc( MAX_FENCES * sizeof( ovrFence ) );
        for ( int i = 0; i < MAX_FENCES; i++ )
        {
            ovrFence_Create( &renderer->Fence[i] );
        }
    }
}

static void ovrRenderer_Destroy( ovrRenderer * renderer )
{
    for ( int eye = 0; eye < renderer->NumBuffers; eye++ ) {
        ovrFramebuffer_Destroy(&renderer->FrameBuffer[eye]);
    }

    for ( int i = 0; i < MAX_FENCES; i++ ) {
            ovrFence_Destroy( &renderer->Fence[i] );
    }
    free( renderer->Fence );
}

static void ovrRenderer_clearBorder(ovrFramebuffer *frameBuffer) {
    // Clear to fully opaque black.
    GL( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
    // bottom
    GL( glScissor( 0, 0, frameBuffer->Width, 1 ) );
    GL( glClear( GL_COLOR_BUFFER_BIT ) );
    // top
    GL( glScissor( 0, frameBuffer->Height - 1, frameBuffer->Width, 1 ) );
    GL( glClear( GL_COLOR_BUFFER_BIT ) );
    // left
    GL( glScissor( 0, 0, 1, frameBuffer->Height ) );
    GL( glClear( GL_COLOR_BUFFER_BIT ) );
    // right
    GL( glScissor( frameBuffer->Width - 1, 0, 1, frameBuffer->Height ) );
    GL( glClear( GL_COLOR_BUFFER_BIT ) );
}

static void ovrRenderer_RenderFrame(ovrRenderer *rendererOVR, const ovrJava *java,
                                    std::shared_ptr<VRORenderer> renderer,
                                    std::shared_ptr<VRODriverOpenGLAndroid> driver,
                                    long long frameIndex,
                                    const ovrTracking2 *tracking, ovrMobile *ovr,
                                    unsigned long long *completionFence,
                                    ovrLayerProjection2 *sceneLayer,
                                    ovrLayerProjection2 *hudLayer)  {

    ovrTracking2 updatedTracking = *tracking;

    // Calculate the view matrix.
    ovrPosef headPose = updatedTracking.HeadPose.Pose;
    VROQuaternion quaternion(headPose.Orientation.x, headPose.Orientation.y, headPose.Orientation.z, headPose.Orientation.w);
    VROMatrix4f headRotation = quaternion.getMatrix();

    // The scene layer renders the Scene, the HUD layer renders headlocked UI
    *sceneLayer = vrapi_DefaultLayerProjection2();
    *hudLayer   = vrapi_DefaultLayerProjection2();

    // Ensure the HUD layer is correctly blended on top of the scene layer
    hudLayer->Header.SrcBlend = VRAPI_FRAME_LAYER_BLEND_SRC_ALPHA;
    hudLayer->Header.DstBlend = VRAPI_FRAME_LAYER_BLEND_ONE_MINUS_SRC_ALPHA;

    sceneLayer->HeadPose = updatedTracking.HeadPose;
    hudLayer->HeadPose   = updatedTracking.HeadPose;

    // Improves the quality of the scene layer
    sceneLayer->Header.Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

    // Ensures "async timewarp" is not applied to the HUD; reduces jitter
    hudLayer->Header.Flags |= VRAPI_FRAME_LAYER_FLAG_FIXED_TO_VIEW;

    VROMatrix4f eyeFromHeadMatrix[VRAPI_FRAME_LAYER_EYE_MAX];
    float interpupillaryDistance = vrapi_GetInterpupillaryDistance(&updatedTracking);

    // Attach each layer to its associated framebuffer's texture swap chain
    for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
        ovrFramebuffer *frameBuffer = &rendererOVR->FrameBuffer[rendererOVR->NumBuffers == 1 ? 0 : eye];
        sceneLayer->Textures[eye].ColorSwapChain = frameBuffer->ColorTextureSwapChain;
        sceneLayer->Textures[eye].SwapChainIndex = frameBuffer->TextureSwapChainIndex;
        sceneLayer->Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection( &updatedTracking.Eye[eye].ProjectionMatrix );

        ovrFramebuffer *hudFrameBuffer = &rendererOVR->HUDFrameBuffer[rendererOVR->NumBuffers == 1 ? 0 : eye];
        hudLayer->Textures[eye].ColorSwapChain = hudFrameBuffer->ColorTextureSwapChain;
        hudLayer->Textures[eye].SwapChainIndex = hudFrameBuffer->TextureSwapChainIndex;

        const float eyeOffset = ( eye ? -0.5f : 0.5f ) * interpupillaryDistance;
        const ovrMatrix4f eyeOffsetMatrix = ovrMatrix4f_CreateTranslation( eyeOffset, 0.0f, 0.0f );
        eyeFromHeadMatrix[eye] = toMatrix4f(eyeOffsetMatrix);
    }

    ovrFramebuffer *leftFB = &rendererOVR->FrameBuffer[0];

    VROViewport leftViewport(0, 0, leftFB->Width, leftFB->Height);
    float fovX = vrapi_GetSystemPropertyFloat( java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_X );
    float fovY = vrapi_GetSystemPropertyFloat( java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_Y );
    VROFieldOfView fov(fovX / 2.0, fovX / 2.0, fovY / 2.0, fovY / 2.0);

    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, renderer->getFarClippingPlane());
    renderer->prepareFrame(frameIndex, leftViewport, fov, headRotation, projection, driver);

    // Render the scene to the textures in the scene layer
    for (int eye = 0; eye < rendererOVR->NumBuffers; eye++) {
        ovrFramebuffer *frameBuffer = &rendererOVR->FrameBuffer[eye];
        ovrFramebuffer_SetCurrent(frameBuffer);
        std::dynamic_pointer_cast<VRODisplayOpenGLOVR>(driver->getDisplay())->setFrameBuffer(frameBuffer);

        GL( glEnable(GL_SCISSOR_TEST) );
        GL( glScissor( 0, 0, frameBuffer->Width, frameBuffer->Height) );
        GL( glViewport(0, 0, frameBuffer->Width, frameBuffer->Height) );

        GL( glEnable(GL_DEPTH_TEST) );
        GL( glEnable(GL_STENCIL_TEST) );
        GL( glClearColor(0.0f, 0.0f, 0.0f, 1.0f) );
        GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) );

        VROEyeType eyeType = (eye == VRAPI_FRAME_LAYER_EYE_LEFT) ? VROEyeType::Left : VROEyeType::Right;
        VROViewport viewport = { 0, 0, frameBuffer->Width, frameBuffer->Height };

        // We use our projection matrix because the one computed by OVR appears to be identical for
        // left an right, but with fixed NCP and FCP. Our projection uses the correct NCP and FCP.
        renderer->renderEye2(eyeType,
                             toMatrix4f(updatedTracking.Eye[eye].ViewMatrix),
                             projection,
                             viewport, driver);

        ovrRenderer_clearBorder(frameBuffer);
        ovrFramebuffer_Resolve(frameBuffer);
        ovrFramebuffer_Advance(frameBuffer);
    }

    // Render the HUD to the textures in the HUD layer
    for (int eye = 0; eye < rendererOVR->NumBuffers; eye++) {
        ovrFramebuffer *frameBuffer = &rendererOVR->HUDFrameBuffer[eye];
        ovrFramebuffer_SetCurrent(frameBuffer);

        GL( glEnable(GL_SCISSOR_TEST) );
        GL( glScissor( 0, 0, frameBuffer->Width, frameBuffer->Height) );
        GL( glViewport(0, 0, frameBuffer->Width, frameBuffer->Height) );

        GL( glEnable(GL_DEPTH_TEST) );
        GL( glClearColor(0.0f, 0.0f, 0.0f, 0.0f) );
        GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

        VROEyeType eyeType = (eye == VRAPI_FRAME_LAYER_EYE_LEFT) ? VROEyeType::Left : VROEyeType::Right;
        renderer->renderHUD(eyeType, eyeFromHeadMatrix[eye], driver);

        ovrRenderer_clearBorder(frameBuffer);
        ovrFramebuffer_Resolve(frameBuffer);
        ovrFramebuffer_Advance(frameBuffer);
    }

    renderer->endFrame(driver);
    ALLOCATION_TRACKER_PRINT();

    ovrFramebuffer_SetNone();

    // Use a single fence to indicate the frame is ready to be displayed.
    ovrFence * fence = &rendererOVR->Fence[rendererOVR->FenceIndex];
    ovrFence_Insert( fence );
    rendererOVR->FenceIndex = ( rendererOVR->FenceIndex + 1 ) % MAX_FENCES;
    *completionFence = (size_t)fence->Sync;
}

/*
================================================================================

ovrApp

================================================================================
*/

typedef enum
{
    BACK_BUTTON_STATE_NONE,
    BACK_BUTTON_STATE_PENDING_DOUBLE_TAP,
    BACK_BUTTON_STATE_PENDING_SHORT_PRESS,
    BACK_BUTTON_STATE_SKIP_UP
} ovrBackButtonState;

typedef struct
{
    ovrJava				Java;
    ovrEgl				Egl;
    ANativeWindow *		NativeWindow;
    bool				Resumed;
    ovrMobile *			Ovr;
    long long			FrameIndex;
    double              DisplayTime;
    int                 SwapInterval;
    ovrBackButtonState	BackButtonState;
    bool				BackButtonDown;
    double				BackButtonDownStartTime;
    ovrRenderer			Renderer;
    bool                UseMultiview;

    // Viro parameters
    std::shared_ptr<VRORenderer> vroRenderer;
    std::shared_ptr<VRODriverOpenGLAndroid> driver;
    bool                suspended;
    double              suspendedNotificationTime;
} ovrApp;

static void ovrApp_Clear( ovrApp * app )
{
    app->Java.Vm = NULL;
    app->Java.Env = NULL;
    app->Java.ActivityObject = NULL;
    app->NativeWindow = NULL;
    app->Resumed = false;
    app->Ovr = NULL;
    app->FrameIndex = 1;
    app->BackButtonState = BACK_BUTTON_STATE_NONE;
    app->BackButtonDown = false;
    app->BackButtonDownStartTime = 0.0;
    app->UseMultiview = true;
    app->suspended = true;
    app->suspendedNotificationTime = VROTimeCurrentSeconds();
    ovrEgl_Clear( &app->Egl );
    ovrRenderer_Clear( &app->Renderer );
}

static void ovrApp_PushBlackFinal( ovrApp * app, const ovrPerformanceParms * perfParms )
{
    ovrFrameParms frameParms = vrapi_DefaultFrameParms( &app->Java, VRAPI_FRAME_INIT_BLACK_FINAL, vrapi_GetTimeInSeconds(), NULL );
    frameParms.FrameIndex = app->FrameIndex;
    frameParms.PerformanceParms = *perfParms;
    vrapi_SubmitFrame( app->Ovr, &frameParms );
}

static void ovrApp_HandleVrModeChanges( ovrApp * app )
{
    if ( app->Resumed != false && app->NativeWindow != NULL )
    {
        if ( app->Ovr == NULL )
        {
            ovrModeParms parms = vrapi_DefaultModeParms( &app->Java );
            // Must reset the FLAG_FULLSCREEN window flag when using a SurfaceView
            parms.Flags |= VRAPI_MODE_FLAG_RESET_WINDOW_FULLSCREEN;

            parms.Flags |= VRAPI_MODE_FLAG_NATIVE_WINDOW;
            parms.Display = (size_t)app->Egl.Display;
            parms.WindowSurface = (size_t)app->NativeWindow;
            parms.ShareContext = (size_t)app->Egl.Context;

            ALOGV( "        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface( EGL_DRAW ) );

            ALOGV( "        vrapi_EnterVrMode()" );

            app->Ovr = vrapi_EnterVrMode( &parms );

            ALOGV( "        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface( EGL_DRAW ) );

            // If entering VR mode failed then the ANativeWindow was not valid.
            if ( app->Ovr == NULL )
            {
                ALOGE( "Invalid ANativeWindow!" );
                app->NativeWindow = NULL;
            }
        }
    }
    else
    {
        if ( app->Ovr != NULL )
        {
            ALOGV( "        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface( EGL_DRAW ) );

            ALOGV( "        vrapi_LeaveVrMode()" );

            vrapi_LeaveVrMode( app->Ovr );
            app->Ovr = NULL;

            ALOGV( "        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface( EGL_DRAW ) );
        }
    }
}

static void ovrApp_BackButtonAction( ovrApp * app, const ovrPerformanceParms * perfParms )
{
    if ( app->BackButtonState == BACK_BUTTON_STATE_PENDING_DOUBLE_TAP )
    {
        ALOGV( "back button double tap" );
        app->BackButtonState = BACK_BUTTON_STATE_SKIP_UP;
    }
    else if ( app->BackButtonState == BACK_BUTTON_STATE_PENDING_SHORT_PRESS && !app->BackButtonDown )
    {
        if ( ( vrapi_GetTimeInSeconds() - app->BackButtonDownStartTime ) >
             vrapi_GetSystemPropertyFloat( &app->Java, VRAPI_SYS_PROP_BACK_BUTTON_DOUBLETAP_TIME ) )
        {
            ALOGV( "back button short press" );
            ALOGV( "        ovrApp_PushBlackFinal()" );
            ovrApp_PushBlackFinal( app, perfParms );
            ALOGV( "        vrapi_ShowSystemUI( confirmQuit )" );
            vrapi_ShowSystemUI( &app->Java, VRAPI_SYS_UI_CONFIRM_QUIT_MENU );
            app->BackButtonState = BACK_BUTTON_STATE_NONE;
        }
    }
}

static int ovrApp_HandleKeyEvent( ovrApp * app, const int keyCode, const int action )
{
    // Handle GearVR back button.
    if ( keyCode == AKEYCODE_BACK )
    {
        if ( action == AKEY_EVENT_ACTION_DOWN )
        {
            if ( !app->BackButtonDown )
            {
                if ( ( vrapi_GetTimeInSeconds() - app->BackButtonDownStartTime ) <
                     vrapi_GetSystemPropertyFloat( &app->Java, VRAPI_SYS_PROP_BACK_BUTTON_DOUBLETAP_TIME ) )
                {
                    app->BackButtonState = BACK_BUTTON_STATE_PENDING_DOUBLE_TAP;
                }
                app->BackButtonDownStartTime = vrapi_GetTimeInSeconds();
            }
            app->BackButtonDown = true;
        }
        else if ( action == AKEY_EVENT_ACTION_UP )
        {
            if ( app->BackButtonState == BACK_BUTTON_STATE_NONE )
            {
                if ( ( vrapi_GetTimeInSeconds() - app->BackButtonDownStartTime ) <
                     vrapi_GetSystemPropertyFloat( &app->Java, VRAPI_SYS_PROP_BACK_BUTTON_SHORTPRESS_TIME ) )
                {
                    app->BackButtonState = BACK_BUTTON_STATE_PENDING_SHORT_PRESS;
                }
            }
            else if ( app->BackButtonState == BACK_BUTTON_STATE_SKIP_UP )
            {
                app->BackButtonState = BACK_BUTTON_STATE_NONE;
            }
            app->BackButtonDown = false;
        }
        std::shared_ptr<VROInputControllerBase> inputControllerBase
                = app->vroRenderer->getInputController();
        std::shared_ptr<VROInputControllerOVR> inputControllerOvr
                = std::dynamic_pointer_cast<VROInputControllerOVR>(inputControllerBase);
        inputControllerOvr->handleOVRKeyEvent(keyCode, action);
        return 1;
    }
    return 0;
}

static int ovrApp_HandleTouchEvent( ovrApp * app, const int action, const float x, const float y )
{
    std::shared_ptr<VROInputControllerBase> inputControllerBase
            = app->vroRenderer->getInputController();
    std::shared_ptr<VROInputControllerOVR> inputControllerOvr
            = std::dynamic_pointer_cast<VROInputControllerOVR>(inputControllerBase);
    inputControllerOvr->handleOVRTouchEvent(action, x, y);

    // Handle GearVR touch pad.
    if ( app->Ovr != NULL && action == AMOTION_EVENT_ACTION_UP )
    {
#if 0
        // Cycle through 60Hz, 30Hz, 20Hz and 15Hz synthesis.
		app->MinimumVsyncs++;
		if ( app->MinimumVsyncs > 4 )
		{
			app->MinimumVsyncs = 1;
		}
		ALOGV( "        MinimumVsyncs = %d", app->MinimumVsyncs );
#endif
    }
    return 1;
}

/*
================================================================================

ovrMessageQueue

================================================================================
*/

typedef enum
{
    MQ_WAIT_NONE,		// don't wait
    MQ_WAIT_RECEIVED,	// wait until the consumer thread has received the message
    MQ_WAIT_PROCESSED	// wait until the consumer thread has processed the message
} ovrMQWait;

#define MAX_MESSAGE_PARMS	8
#define MAX_MESSAGES		1024

typedef struct
{
    int			Id;
    ovrMQWait	Wait;
    long long	Parms[MAX_MESSAGE_PARMS];
} ovrMessage;

static void ovrMessage_Init( ovrMessage * message, const int id, const int wait )
{
    message->Id = id;
    message->Wait = (ovrMQWait)wait;
    memset( message->Parms, 0, sizeof( message->Parms ) );
}

static void		ovrMessage_SetPointerParm( ovrMessage * message, int index, void * ptr ) { *(void **)&message->Parms[index] = ptr; }
static void *	ovrMessage_GetPointerParm( ovrMessage * message, int index ) { return *(void **)&message->Parms[index]; }
static void		ovrMessage_SetIntegerParm( ovrMessage * message, int index, int value ) { message->Parms[index] = value; }
static int		ovrMessage_GetIntegerParm( ovrMessage * message, int index ) { return (int)message->Parms[index]; }
static void		ovrMessage_SetFloatParm( ovrMessage * message, int index, float value ) { *(float *)&message->Parms[index] = value; }
static float	ovrMessage_GetFloatParm( ovrMessage * message, int index ) { return *(float *)&message->Parms[index]; }

// Cyclic queue with messages.
typedef struct
{
    ovrMessage	 		Messages[MAX_MESSAGES];
    volatile int		Head;	// dequeue at the head
    volatile int		Tail;	// enqueue at the tail
    ovrMQWait			Wait;
    volatile bool		EnabledFlag;
    volatile bool		PostedFlag;
    volatile bool		ReceivedFlag;
    volatile bool		ProcessedFlag;
    pthread_mutex_t		Mutex;
    pthread_cond_t		PostedCondition;
    pthread_cond_t		ReceivedCondition;
    pthread_cond_t		ProcessedCondition;
} ovrMessageQueue;

static void ovrMessageQueue_Create( ovrMessageQueue * messageQueue )
{
    messageQueue->Head = 0;
    messageQueue->Tail = 0;
    messageQueue->Wait = MQ_WAIT_NONE;
    messageQueue->EnabledFlag = false;
    messageQueue->PostedFlag = false;
    messageQueue->ReceivedFlag = false;
    messageQueue->ProcessedFlag = false;

    pthread_mutexattr_t	attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
    pthread_mutex_init( &messageQueue->Mutex, &attr );
    pthread_mutexattr_destroy( &attr );
    pthread_cond_init( &messageQueue->PostedCondition, NULL );
    pthread_cond_init( &messageQueue->ReceivedCondition, NULL );
    pthread_cond_init( &messageQueue->ProcessedCondition, NULL );
}

static void ovrMessageQueue_Destroy( ovrMessageQueue * messageQueue )
{
    pthread_mutex_destroy( &messageQueue->Mutex );
    pthread_cond_destroy( &messageQueue->PostedCondition );
    pthread_cond_destroy( &messageQueue->ReceivedCondition );
    pthread_cond_destroy( &messageQueue->ProcessedCondition );
}

static void ovrMessageQueue_Enable( ovrMessageQueue * messageQueue, const bool set )
{
    messageQueue->EnabledFlag = set;
}

static void ovrMessageQueue_PostMessage( ovrMessageQueue * messageQueue, const ovrMessage * message )
{
    if ( !messageQueue->EnabledFlag )
    {
        return;
    }
    while ( messageQueue->Tail - messageQueue->Head >= MAX_MESSAGES )
    {
        usleep( 1000 );
    }
    pthread_mutex_lock( &messageQueue->Mutex );
    messageQueue->Messages[messageQueue->Tail & ( MAX_MESSAGES - 1 )] = *message;
    messageQueue->Tail++;
    messageQueue->PostedFlag = true;
    pthread_cond_broadcast( &messageQueue->PostedCondition );
    if ( message->Wait == MQ_WAIT_RECEIVED )
    {
        while ( !messageQueue->ReceivedFlag )
        {
            pthread_cond_wait( &messageQueue->ReceivedCondition, &messageQueue->Mutex );
        }
        messageQueue->ReceivedFlag = false;
    }
    else if ( message->Wait == MQ_WAIT_PROCESSED )
    {
        while ( !messageQueue->ProcessedFlag )
        {
            pthread_cond_wait( &messageQueue->ProcessedCondition, &messageQueue->Mutex );
        }
        messageQueue->ProcessedFlag = false;
    }
    pthread_mutex_unlock( &messageQueue->Mutex );
}

static void ovrMessageQueue_SleepUntilMessage( ovrMessageQueue * messageQueue )
{
    if ( messageQueue->Wait == MQ_WAIT_PROCESSED )
    {
        messageQueue->ProcessedFlag = true;
        pthread_cond_broadcast( &messageQueue->ProcessedCondition );
        messageQueue->Wait = MQ_WAIT_NONE;
    }
    pthread_mutex_lock( &messageQueue->Mutex );
    if ( messageQueue->Tail > messageQueue->Head )
    {
        pthread_mutex_unlock( &messageQueue->Mutex );
        return;
    }
    while ( !messageQueue->PostedFlag )
    {
        pthread_cond_wait( &messageQueue->PostedCondition, &messageQueue->Mutex );
    }
    messageQueue->PostedFlag = false;
    pthread_mutex_unlock( &messageQueue->Mutex );
}

static bool ovrMessageQueue_GetNextMessage( ovrMessageQueue * messageQueue, ovrMessage * message, bool waitForMessages )
{
    if ( messageQueue->Wait == MQ_WAIT_PROCESSED )
    {
        messageQueue->ProcessedFlag = true;
        pthread_cond_broadcast( &messageQueue->ProcessedCondition );
        messageQueue->Wait = MQ_WAIT_NONE;
    }
    if ( waitForMessages )
    {
        ovrMessageQueue_SleepUntilMessage( messageQueue );
    }
    pthread_mutex_lock( &messageQueue->Mutex );
    if ( messageQueue->Tail <= messageQueue->Head )
    {
        pthread_mutex_unlock( &messageQueue->Mutex );
        return false;
    }
    *message = messageQueue->Messages[messageQueue->Head & ( MAX_MESSAGES - 1 )];
    messageQueue->Head++;
    pthread_mutex_unlock( &messageQueue->Mutex );
    if ( message->Wait == MQ_WAIT_RECEIVED )
    {
        messageQueue->ReceivedFlag = true;
        pthread_cond_broadcast( &messageQueue->ReceivedCondition );
    }
    else if ( message->Wait == MQ_WAIT_PROCESSED )
    {
        messageQueue->Wait = MQ_WAIT_PROCESSED;
    }
    return true;
}

/*
================================================================================

ovrAppThread

================================================================================
*/

enum
{
    MESSAGE_ON_CREATE,
    MESSAGE_ON_START,
    MESSAGE_ON_RESUME,
    MESSAGE_ON_PAUSE,
    MESSAGE_ON_STOP,
    MESSAGE_ON_DESTROY,
    MESSAGE_ON_SURFACE_CREATED,
    MESSAGE_ON_SURFACE_DESTROYED,
    MESSAGE_ON_KEY_EVENT,
    MESSAGE_ON_TOUCH_EVENT,
    MESSAGE_SUSPEND,
    MESSAGE_RECENTER_TRACKING
};

struct ovrAppThread
{
    JavaVM *		JavaVm;
    jobject			ActivityObject;
    pthread_t		Thread;
    ovrMessageQueue	MessageQueue;
    ANativeWindow * NativeWindow;

    std::shared_ptr<VRORenderer> vroRenderer;
    std::shared_ptr<VRODriverOpenGLAndroid> driver;
    jobject view;
};

void * AppThreadFunction( void * parm )
{
    ovrAppThread * appThread = (ovrAppThread *)parm;

    ovrJava java;
    java.Vm = appThread->JavaVm;
    java.Vm->AttachCurrentThread( &java.Env, NULL );
    java.ActivityObject = appThread->ActivityObject;

    // Note that AttachCurrentThread will reset the thread name.
    prctl( PR_SET_NAME, (long)"OVR::Main", 0, 0, 0 );

    const ovrInitParms initParms = vrapi_DefaultInitParms( &java );
    int32_t initResult = vrapi_Initialize( &initParms );
    if ( initResult != VRAPI_INITIALIZE_SUCCESS )
    {
        vrapi_Shutdown();
        exit( 0 );
    }

    ovrApp appState;
    ovrApp_Clear( &appState );
    appState.Java = java;

    ovrEgl_CreateContext( &appState.Egl, NULL );

    EglInitExtensions();

    appState.UseMultiview &= ( glExtensions.multi_view &&
                               vrapi_GetSystemPropertyInt( &appState.Java, VRAPI_SYS_PROP_MULTIVIEW_AVAILABLE ) );

    // TODO VIRO-725: enable multiview rendering (start by removing this line to detect multiview support correctly)
    appState.UseMultiview = false;

    ALOGV( "AppState UseMultiview : %d", appState.UseMultiview );

    ovrPerformanceParms perfParms = vrapi_DefaultPerformanceParms();
    perfParms.CpuLevel = CPU_LEVEL;
    perfParms.GpuLevel = GPU_LEVEL;
    perfParms.MainThreadTid = gettid();

    ovrRenderer_Create( &appState.Renderer, &java, appState.UseMultiview );
    appState.vroRenderer = appThread->vroRenderer;
    appState.driver = appThread->driver;

    jclass viewCls = java.Env->GetObjectClass(appThread->view);
    jmethodID drawFrameMethod = java.Env->GetMethodID(viewCls, "onDrawFrame", "()V");

    for ( bool destroyed = false; destroyed == false; )
    {
        for ( ; ; )
        {
            ovrMessage message;
            const bool waitForMessages = ( appState.Ovr == NULL && destroyed == false );
            if ( !ovrMessageQueue_GetNextMessage( &appThread->MessageQueue, &message, waitForMessages ) )
            {
                break;
            }

            switch ( message.Id )
            {
                case MESSAGE_ON_CREATE:				{ break; }
                case MESSAGE_ON_START:				{ break; }
                case MESSAGE_ON_RESUME:				{ appState.driver->onResume(); appState.Resumed = true; break; }
                case MESSAGE_ON_PAUSE:				{ appState.driver->onPause(); appState.Resumed = false; break; }
                case MESSAGE_ON_STOP:				{ break; }
                case MESSAGE_ON_DESTROY:			{ appState.NativeWindow = NULL; destroyed = true; break; }
                case MESSAGE_ON_SURFACE_CREATED:	{ appState.NativeWindow = (ANativeWindow *)ovrMessage_GetPointerParm( &message, 0 ); break; }
                case MESSAGE_ON_SURFACE_DESTROYED:	{ appState.NativeWindow = NULL; break; }
                case MESSAGE_ON_KEY_EVENT:			{ ovrApp_HandleKeyEvent( &appState,
                                                                               ovrMessage_GetIntegerParm( &message, 0 ),
                                                                               ovrMessage_GetIntegerParm( &message, 1 ) ); break; }
                case MESSAGE_ON_TOUCH_EVENT:		{ ovrApp_HandleTouchEvent( &appState,
                                                                               ovrMessage_GetIntegerParm( &message, 0 ),
                                                                               ovrMessage_GetFloatParm( &message, 1 ),
                                                                               ovrMessage_GetFloatParm( &message, 2 ) ); break; }
                case MESSAGE_SUSPEND:               {appState.suspended = ovrMessage_GetIntegerParm( &message, 0); break; }
                case MESSAGE_RECENTER_TRACKING:     {if (appState.Ovr) vrapi_RecenterPose(appState.Ovr); break;}
            }

            ovrApp_HandleVrModeChanges( &appState );
        }

        ovrApp_BackButtonAction( &appState, &perfParms );


        // Invoke the frame listeners on the Java side
        java.Env->CallVoidMethod(appThread->view, drawFrameMethod);

        if ( appState.Ovr == NULL )
        {
            continue;
        }

        if ( appState.suspended ) {
            double newTime = VROTimeCurrentSeconds();
            // notify the user about bad keys 5 times a second (every 200ms/.2s)
            if (newTime - appState.suspendedNotificationTime > .2) {
                perr("Renderer suspended! Do you have a valid key?");
                appState.suspendedNotificationTime = newTime;
            }
            continue;
        }

        // This is the only place the frame index is incremented, right before
        // calling vrapi_GetPredictedDisplayTime().
        appState.FrameIndex++;

        // Get the HMD pose, predicted for the middle of the time period during which
        // the new eye images will be displayed. The number of frames predicted ahead
        // depends on the pipeline depth of the engine and the synthesis rate.
        // The better the prediction, the less black will be pulled in at the edges.
        const double predictedDisplayTime = vrapi_GetPredictedDisplayTime( appState.Ovr, appState.FrameIndex );
        const ovrTracking2 tracking = vrapi_GetPredictedTracking2( appState.Ovr, predictedDisplayTime );

        appState.DisplayTime = predictedDisplayTime;
        unsigned long long completionFence = 0;

        // Render eye images and setup the primary layer using ovrTracking2.
        ovrLayerProjection2 worldLayer;
        ovrLayerProjection2 hudLayer;
        ovrRenderer_RenderFrame(&appState.Renderer, &appState.Java,
                                appState.vroRenderer, appState.driver,
                                appState.FrameIndex,
                                &tracking,
                                appState.Ovr, &completionFence, &worldLayer, &hudLayer );

        const ovrLayerHeader2 * layers[] = {
            &worldLayer.Header,
            &hudLayer.Header
        };

        ovrSubmitFrameDescription2 frameDesc = {};
        frameDesc.Flags = 0;
        frameDesc.SwapInterval = appState.SwapInterval;
        frameDesc.FrameIndex = appState.FrameIndex;
        frameDesc.CompletionFence = completionFence;
        frameDesc.DisplayTime = appState.DisplayTime;
        frameDesc.LayerCount = 2;
        frameDesc.Layers = layers;

        // Hand over the eye images to the time warp.
        vrapi_SubmitFrame2( appState.Ovr, &frameDesc );
    }

    ovrRenderer_Destroy( &appState.Renderer );
    ovrEgl_DestroyContext( &appState.Egl );
    vrapi_Shutdown();

    java.Vm->DetachCurrentThread( );
    return NULL;
}

static void ovrAppThread_Create( ovrAppThread * appThread, JNIEnv * env, jobject activityObject, jobject viewObject,
                                 std::shared_ptr<VRORenderer> renderer, std::shared_ptr<VRODriverOpenGLAndroid> driver)
{
    env->GetJavaVM( &appThread->JavaVm );
    appThread->ActivityObject = env->NewGlobalRef( activityObject );
    appThread->Thread = 0;
    appThread->NativeWindow = NULL;
    appThread->view = env->NewGlobalRef(viewObject);
    appThread->vroRenderer = renderer;
    appThread->driver = driver;
    ovrMessageQueue_Create( &appThread->MessageQueue );

    const int createErr = pthread_create( &appThread->Thread, NULL, AppThreadFunction, appThread );
    if ( createErr != 0 )
    {
        ALOGE( "pthread_create returned %i", createErr );
    }
}

static void ovrAppThread_Destroy( ovrAppThread * appThread, JNIEnv * env )
{
    pthread_join( appThread->Thread, NULL );
    env->DeleteGlobalRef( appThread->ActivityObject );
    env->DeleteGlobalRef( appThread->view );
    appThread->vroRenderer.reset();
    appThread->driver.reset();
    ovrMessageQueue_Destroy( &appThread->MessageQueue );
}

/*
================================================================================

Activity lifecycle

================================================================================
*/

VROSceneRendererOVR::VROSceneRendererOVR(std::shared_ptr<gvr::AudioApi> gvrAudio,
                                         jobject view, jobject activity, JNIEnv *env) {
    _renderer = std::make_shared<VRORenderer>(std::make_shared<VROInputControllerOVR>());
    _driver = std::make_shared<VRODriverOpenGLAndroidOVR>(gvrAudio);

    ALOGV( "    GLES3JNILib::onCreate()" );

    _appThread = (ovrAppThread *) malloc( sizeof( ovrAppThread ) );
    ovrAppThread_Create( _appThread, env, activity, view, _renderer, _driver );

    ovrMessageQueue_Enable( &_appThread->MessageQueue, true );
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_CREATE, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &_appThread->MessageQueue, &message );
}

VROSceneRendererOVR::~VROSceneRendererOVR() {
}

void VROSceneRendererOVR::onDestroy(){
    ALOGV( "    GLES3JNILib::onDestroy()" );
    JNIEnv *env = VROPlatformGetJNIEnv();
    ovrAppThread * appThread = _appThread;
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_DESTROY, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
    ovrMessageQueue_Enable( &appThread->MessageQueue, false );

    ovrAppThread_Destroy( appThread, env );
    free( appThread );
}

void VROSceneRendererOVR::onStart() {
    ALOGV( "    GLES3JNILib::onStart()" );
    ovrAppThread * appThread = _appThread;
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_START, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

void VROSceneRendererOVR::onResume() {
    ALOGV( "    GLES3JNILib::onResume()" );
    ovrAppThread * appThread = _appThread;
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_RESUME, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

void VROSceneRendererOVR::onPause() {
    ALOGV( "    GLES3JNILib::onPause()" );
    ovrAppThread * appThread = _appThread;
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_PAUSE, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

void VROSceneRendererOVR::onStop() {
    ALOGV( "    GLES3JNILib::onStop()" );
    ovrAppThread * appThread = _appThread;
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_STOP, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

/*
================================================================================

Surface lifecycle

================================================================================
*/

void VROSceneRendererOVR::onSurfaceCreated(jobject surface) {
    ALOGV( "    GLES3JNILib::onSurfaceCreated()" );
    ovrAppThread * appThread = _appThread;

    JNIEnv *env = VROPlatformGetJNIEnv();

    ANativeWindow * newNativeWindow = ANativeWindow_fromSurface( env, surface );
    if ( ANativeWindow_getWidth( newNativeWindow ) < ANativeWindow_getHeight( newNativeWindow ) )
    {
        // An app that is relaunched after pressing the home button gets an initial surface with
        // the wrong orientation even though android:screenOrientation="landscape" is set in the
        // manifest. The choreographer callback will also never be called for this surface because
        // the surface is immediately replaced with a new surface with the correct orientation.
        ALOGE( "        Surface not in landscape mode!" );
    }

    ALOGV( "        NativeWindow = ANativeWindow_fromSurface( env, surface )" );
    appThread->NativeWindow = newNativeWindow;
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED );
    ovrMessage_SetPointerParm( &message, 0, appThread->NativeWindow );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

void VROSceneRendererOVR::onSurfaceChanged(jobject surface, jint width, jint height) {
    ALOGV( "    GLES3JNILib::onSurfaceChanged()" );
    ovrAppThread * appThread = _appThread;

    JNIEnv *env = VROPlatformGetJNIEnv();

    ANativeWindow * newNativeWindow = ANativeWindow_fromSurface( env, surface );
    if ( ANativeWindow_getWidth( newNativeWindow ) < ANativeWindow_getHeight( newNativeWindow ) )
    {
        // An app that is relaunched after pressing the home button gets an initial surface with
        // the wrong orientation even though android:screenOrientation="landscape" is set in the
        // manifest. The choreographer callback will also never be called for this surface because
        // the surface is immediately replaced with a new surface with the correct orientation.
        ALOGE( "        Surface not in landscape mode!" );
    }

    if ( newNativeWindow != appThread->NativeWindow )
    {
        if ( appThread->NativeWindow != NULL )
        {
            ovrMessage message;
            ovrMessage_Init( &message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED );
            ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
            ALOGV( "        ANativeWindow_release( NativeWindow )" );
            ANativeWindow_release( appThread->NativeWindow );
            appThread->NativeWindow = NULL;
        }
        if ( newNativeWindow != NULL )
        {
            ALOGV( "        NativeWindow = ANativeWindow_fromSurface( env, surface )" );
            appThread->NativeWindow = newNativeWindow;
            ovrMessage message;
            ovrMessage_Init( &message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED );
            ovrMessage_SetPointerParm( &message, 0, appThread->NativeWindow );
            ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
        }
    }
    else if ( newNativeWindow != NULL )
    {
        ANativeWindow_release( newNativeWindow );
    }
}

void VROSceneRendererOVR::onSurfaceDestroyed() {
    ALOGV( "    GLES3JNILib::onSurfaceDestroyed()" );
    ovrAppThread * appThread = _appThread;
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
    ALOGV( "        ANativeWindow_release( NativeWindow )" );
    ANativeWindow_release( appThread->NativeWindow );
    appThread->NativeWindow = NULL;
}
/*
================================================================================

Input

================================================================================
*/

void VROSceneRendererOVR::onKeyEvent(int keyCode, int action) {
    if ( action == AKEY_EVENT_ACTION_UP )
    {
        ALOGV( "    GLES3JNILib::onKeyEvent( %d, %d )", keyCode, action );
    }
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_KEY_EVENT, MQ_WAIT_NONE );
    ovrMessage_SetIntegerParm( &message, 0, keyCode );
    ovrMessage_SetIntegerParm( &message, 1, action );
    ovrMessageQueue_PostMessage( &_appThread->MessageQueue, &message );
}

void VROSceneRendererOVR::onTouchEvent(int action, float x, float y) {
    if ( action == AMOTION_EVENT_ACTION_UP )
    {
        ALOGV( "    GLES3JNILib::onTouchEvent( %d, %1.0f, %1.0f )", action, x, y );
    }
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_TOUCH_EVENT, MQ_WAIT_NONE );
    ovrMessage_SetIntegerParm( &message, 0, action );
    ovrMessage_SetFloatParm( &message, 1, x );
    ovrMessage_SetFloatParm( &message, 2, y );
    ovrMessageQueue_PostMessage( &_appThread->MessageQueue, &message );
}

void VROSceneRendererOVR::setSuspended(bool suspendRenderer) {
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_SUSPEND, MQ_WAIT_NONE );
    ovrMessage_SetIntegerParm( &message, 0, suspendRenderer );
    ovrMessageQueue_PostMessage( &_appThread->MessageQueue, &message );
}

void VROSceneRendererOVR::recenterTracking() {
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_RECENTER_TRACKING, MQ_WAIT_NONE);
    ovrMessageQueue_PostMessage( &_appThread->MessageQueue, &message );
};
