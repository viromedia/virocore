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

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

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

#define MULTI_THREADED			0
#define REDUCED_LATENCY			0

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

#ifdef CHECK_GL_ERRORS

static const char * GlErrorString( GLenum error )
{
	switch ( error )
	{
		case GL_NO_ERROR:						return "GL_NO_ERROR";
		case GL_INVALID_ENUM:					return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:					return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:				return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:	return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:					return "GL_OUT_OF_MEMORY";
		default: return "unknown";
	}
}

static void GLCheckErrors( int line )
{
	for ( int i = 0; i < 10; i++ )
	{
		const GLenum error = glGetError();
		if ( error == GL_NO_ERROR )
		{
			break;
		}
		ALOGE( "GL error on line %d: %s", line, GlErrorString( error ) );
	}
}

#define GL( func )		func; GLCheckErrors( __LINE__ );

#else // CHECK_GL_ERRORS

#define GL( func )		func;

#endif // CHECK_GL_ERRORS

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

ovrGeometry

================================================================================
*/

typedef struct
{
    GLuint			Index;
    GLint			Size;
    GLenum			Type;
    GLboolean		Normalized;
    GLsizei			Stride;
    const GLvoid *	Pointer;
} ovrVertexAttribPointer;

#define MAX_VERTEX_ATTRIB_POINTERS		3

typedef struct
{
    GLuint					VertexBuffer;
    GLuint					IndexBuffer;
    GLuint					VertexArrayObject;
    int						VertexCount;
    int 					IndexCount;
    ovrVertexAttribPointer	VertexAttribs[MAX_VERTEX_ATTRIB_POINTERS];
} ovrGeometry;

enum class OVRVertexAttributeLocation
{
    VERTEX_ATTRIBUTE_LOCATION_POSITION,
    VERTEX_ATTRIBUTE_LOCATION_COLOR,
    VERTEX_ATTRIBUTE_LOCATION_UV,
    VERTEX_ATTRIBUTE_LOCATION_TRANSFORM
}				location;

typedef struct
{
    OVRVertexAttributeLocation location;
    const char *	name;
} ovrVertexAttribute;

static ovrVertexAttribute ProgramVertexAttributes[] =
        {
                { OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_POSITION,	"vertexPosition" },
                { OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_COLOR,		"vertexColor" },
                { OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_UV,			"vertexUv" },
                { OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_TRANSFORM,	"vertexTransform" }
        };

static void ovrGeometry_Clear( ovrGeometry * geometry )
{
    geometry->VertexBuffer = 0;
    geometry->IndexBuffer = 0;
    geometry->VertexArrayObject = 0;
    geometry->VertexCount = 0;
    geometry->IndexCount = 0;
    for ( int i = 0; i < MAX_VERTEX_ATTRIB_POINTERS; i++ )
    {
        memset( &geometry->VertexAttribs[i], 0, sizeof( geometry->VertexAttribs[i] ) );
        geometry->VertexAttribs[i].Index = -1;
    }
}

static void ovrGeometry_CreateCube( ovrGeometry * geometry )
{
    typedef struct
    {
        char positions[8][4];
        unsigned char colors[8][4];
    } ovrCubeVertices;

    static const ovrCubeVertices cubeVertices =
            {
                    // positions
                    {
                            { (char)-127, (char)+127, (char)-127, (char)+127 }, { (char)+127, (char)+127, (char)-127, (char)+127 }, { (char)+127, (char)+127, (char)+127, (char)+127 }, { (char)-127, (char)+127, (char)+127, (char)+127 },	// top
                            { (char)-127, (char)-127, (char)-127, (char)+127 }, { (char)-127, (char)-127, (char)+127, (char)+127 }, { (char)+127, (char)-127, (char)+127, (char)+127 }, { (char)+127, (char)-127, (char)-127, (char)+127 }	// bottom
                    },
                    // colors
                    {
                            { 255,   0, 255, 255 }, {   0, 255,   0, 255 }, {   0,   0, 255, 255 }, { 255,   0,   0, 255 },
                            {   0,   0, 255, 255 }, {   0, 255,   0, 255 }, { 255,   0, 255, 255 }, { 255,   0,   0, 255 }
                    },
            };

    static const unsigned short cubeIndices[36] =
            {
                    0, 2, 1, 2, 0, 3,	// top
                    4, 6, 5, 6, 4, 7,	// bottom
                    2, 6, 7, 7, 1, 2,	// right
                    0, 4, 5, 5, 3, 0,	// left
                    3, 5, 6, 6, 2, 3,	// front
                    0, 1, 7, 7, 4, 0	// back
            };

    geometry->VertexCount = 8;
    geometry->IndexCount = 36;

    geometry->VertexAttribs[0].Index = (int)OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_POSITION;
    geometry->VertexAttribs[0].Size = 4;
    geometry->VertexAttribs[0].Type = GL_BYTE;
    geometry->VertexAttribs[0].Normalized = true;
    geometry->VertexAttribs[0].Stride = sizeof( cubeVertices.positions[0] );
    geometry->VertexAttribs[0].Pointer = (const GLvoid *)offsetof( ovrCubeVertices, positions );

    geometry->VertexAttribs[1].Index = (int)OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_COLOR;
    geometry->VertexAttribs[1].Size = 4;
    geometry->VertexAttribs[1].Type = GL_UNSIGNED_BYTE;
    geometry->VertexAttribs[1].Normalized = true;
    geometry->VertexAttribs[1].Stride = sizeof( cubeVertices.colors[0] );
    geometry->VertexAttribs[1].Pointer = (const GLvoid *)offsetof( ovrCubeVertices, colors );

    GL( glGenBuffers( 1, &geometry->VertexBuffer ) );
    GL( glBindBuffer( GL_ARRAY_BUFFER, geometry->VertexBuffer ) );
    GL( glBufferData( GL_ARRAY_BUFFER, sizeof( cubeVertices ), &cubeVertices, GL_STATIC_DRAW ) );
    GL( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

    GL( glGenBuffers( 1, &geometry->IndexBuffer ) );
    GL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, geometry->IndexBuffer ) );
    GL( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( cubeIndices ), cubeIndices, GL_STATIC_DRAW ) );
    GL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
}

static void ovrGeometry_Destroy( ovrGeometry * geometry )
{
    GL( glDeleteBuffers( 1, &geometry->IndexBuffer ) );
    GL( glDeleteBuffers( 1, &geometry->VertexBuffer ) );

    ovrGeometry_Clear( geometry );
}

static void ovrGeometry_CreateVAO( ovrGeometry * geometry )
{
    GL( glGenVertexArrays( 1, &geometry->VertexArrayObject ) );
    GL( glBindVertexArray( geometry->VertexArrayObject ) );

    GL( glBindBuffer( GL_ARRAY_BUFFER, geometry->VertexBuffer ) );

    for ( int i = 0; i < MAX_VERTEX_ATTRIB_POINTERS; i++ )
    {
        if ( geometry->VertexAttribs[i].Index != -1 )
        {
            GL( glEnableVertexAttribArray( geometry->VertexAttribs[i].Index ) );
            GL( glVertexAttribPointer( geometry->VertexAttribs[i].Index, geometry->VertexAttribs[i].Size,
                                       geometry->VertexAttribs[i].Type, geometry->VertexAttribs[i].Normalized,
                                       geometry->VertexAttribs[i].Stride, geometry->VertexAttribs[i].Pointer ) );
        }
    }

    GL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, geometry->IndexBuffer ) );

    GL( glBindVertexArray( 0 ) );
}

static void ovrGeometry_DestroyVAO( ovrGeometry * geometry )
{
    GL( glDeleteVertexArrays( 1, &geometry->VertexArrayObject ) );
}

/*
================================================================================

ovrProgram

================================================================================
*/

#define MAX_PROGRAM_UNIFORMS	8
#define MAX_PROGRAM_TEXTURES	8

typedef struct
{
    GLuint	Program;
    GLuint	VertexShader;
    GLuint	FragmentShader;
    // These will be -1 if not used by the program.
    GLint	UniformLocation[MAX_PROGRAM_UNIFORMS];	// ProgramUniforms[].name
    GLint	UniformBinding[MAX_PROGRAM_UNIFORMS];	// ProgramUniforms[].name
    GLint	Textures[MAX_PROGRAM_TEXTURES];			// Texture%i
} ovrProgram;

int UNIFORM_MODEL_MATRIX = 0;
int UNIFORM_VIEW_ID = 1;
int UNIFORM_SCENE_MATRICES = 2;

int UNIFORM_TYPE_VECTOR4 = 0;
int UNIFORM_TYPE_MATRIX4X4 = 1;
int UNIFORM_TYPE_INT = 2;
int UNIFORM_TYPE_BUFFER = 3;

typedef struct
{
    int index;
    int type;
    const char *	name;
} ovrUniform;

static ovrUniform ProgramUniforms[] =
        {
                { UNIFORM_MODEL_MATRIX,			UNIFORM_TYPE_MATRIX4X4,	"ModelMatrix"	},
                { UNIFORM_VIEW_ID,				UNIFORM_TYPE_INT,       "ViewID"		},
                { UNIFORM_SCENE_MATRICES,		UNIFORM_TYPE_BUFFER,	"SceneMatrices" },
        };

static const char * FindShaderVersionEnd( const char * src )
{
    if ( src == NULL || strncmp( src, "#version ", 9 ) != 0 )
    {
        return src;
    }
    while ( *src != 0 && *src != '\n' )
    {
        src++;
    }
    if ( *src == '\n' )
    {
        src++;
    }
    return src;
}

static void ovrProgram_Clear( ovrProgram * program )
{
    program->Program = 0;
    program->VertexShader = 0;
    program->FragmentShader = 0;
    memset( program->UniformLocation, 0, sizeof( program->UniformLocation ) );
    memset( program->UniformBinding, 0, sizeof( program->UniformBinding ) );
    memset( program->Textures, 0, sizeof( program->Textures ) );
}

static bool ovrProgram_Create( ovrProgram * program, const char * vertexSource, const char * fragmentSource, const bool useMultiview )
{
    GLint r;

    GL( program->VertexShader = glCreateShader( GL_VERTEX_SHADER ) );

    const char * postVersion = FindShaderVersionEnd( vertexSource );
    const char * sources[3] = { "#version 300 es\n",
                                ( useMultiview ) ? "#define DISABLE_MULTIVIEW 0\n" : "#define DISABLE_MULTIVIEW 1\n",
                                postVersion
    };
    GL( glShaderSource( program->VertexShader, 3, sources, 0 ) );
    GL( glCompileShader( program->VertexShader ) );
    GL( glGetShaderiv( program->VertexShader, GL_COMPILE_STATUS, &r ) );
    if ( r == GL_FALSE )
    {
        GLchar msg[4096];
        GL( glGetShaderInfoLog( program->VertexShader, sizeof( msg ), 0, msg ) );
        ALOGE( "%s\n%s\n", vertexSource, msg );
        return false;
    }

    GL( program->FragmentShader = glCreateShader( GL_FRAGMENT_SHADER ) );
    GL( glShaderSource( program->FragmentShader, 1, &fragmentSource, 0 ) );
    GL( glCompileShader( program->FragmentShader ) );
    GL( glGetShaderiv( program->FragmentShader, GL_COMPILE_STATUS, &r ) );
    if ( r == GL_FALSE )
    {
        GLchar msg[4096];
        GL( glGetShaderInfoLog( program->FragmentShader, sizeof( msg ), 0, msg ) );
        ALOGE( "%s\n%s\n", fragmentSource, msg );
        return false;
    }

    GL( program->Program = glCreateProgram() );
    GL( glAttachShader( program->Program, program->VertexShader ) );
    GL( glAttachShader( program->Program, program->FragmentShader ) );

    // Bind the vertex attribute locations.
    for ( int i = 0; i < sizeof( ProgramVertexAttributes ) / sizeof( ProgramVertexAttributes[0] ); i++ )
    {
        GL( glBindAttribLocation( program->Program, (int)ProgramVertexAttributes[i].location, ProgramVertexAttributes[i].name ) );
    }

    GL( glLinkProgram( program->Program ) );
    GL( glGetProgramiv( program->Program, GL_LINK_STATUS, &r ) );
    if ( r == GL_FALSE )
    {
        GLchar msg[4096];
        GL( glGetProgramInfoLog( program->Program, sizeof( msg ), 0, msg ) );
        ALOGE( "Linking program failed: %s\n", msg );
        return false;
    }

    int numBufferBindings = 0;

    // Get the uniform locations.
    memset( program->UniformLocation, -1, sizeof( program->UniformLocation ) );
    for ( int i = 0; i < sizeof( ProgramUniforms ) / sizeof( ProgramUniforms[0] ); i++ )
    {
        const int uniformIndex = ProgramUniforms[i].index;
        if ( ProgramUniforms[i].type == UNIFORM_TYPE_BUFFER )
        {
            GL( program->UniformLocation[uniformIndex] = glGetUniformBlockIndex( program->Program, ProgramUniforms[i].name ) );
            program->UniformBinding[uniformIndex] = numBufferBindings++;
            GL( glUniformBlockBinding( program->Program, program->UniformLocation[uniformIndex], program->UniformBinding[uniformIndex] ) );
        }
        else
        {
            GL( program->UniformLocation[uniformIndex] = glGetUniformLocation( program->Program, ProgramUniforms[i].name ) );
            program->UniformBinding[uniformIndex] = program->UniformLocation[uniformIndex];
        }
    }

    GL( glUseProgram( program->Program ) );

    // Get the texture locations.
    for ( int i = 0; i < MAX_PROGRAM_TEXTURES; i++ )
    {
        char name[32];
        sprintf( name, "Texture%i", i );
        program->Textures[i] = glGetUniformLocation( program->Program, name );
        if ( program->Textures[i] != -1 )
        {
            GL( glUniform1i( program->Textures[i], i ) );
        }
    }

    GL( glUseProgram( 0 ) );

    return true;
}

static void ovrProgram_Destroy( ovrProgram * program )
{
    if ( program->Program != 0 )
    {
        GL( glDeleteProgram( program->Program ) );
        program->Program = 0;
    }
    if ( program->VertexShader != 0 )
    {
        GL( glDeleteShader( program->VertexShader ) );
        program->VertexShader = 0;
    }
    if ( program->FragmentShader != 0 )
    {
        GL( glDeleteShader( program->FragmentShader ) );
        program->FragmentShader = 0;
    }
}

/*
================================================================================

ovrFramebuffer

================================================================================
*/

typedef struct
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
} ovrFramebuffer;

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

ovrScene

================================================================================
*/

#define NUM_INSTANCES		1500
#define NUM_ROTATIONS		16

typedef struct
{
    bool				CreatedScene;
    bool				CreatedVAOs;
    unsigned int		Random;
    ovrProgram			Program;
    ovrGeometry			Cube;
    GLuint				SceneMatrices;
    GLuint				InstanceTransformBuffer;
    ovrVector3f			Rotations[NUM_ROTATIONS];
    ovrVector3f			CubePositions[NUM_INSTANCES];
    int					CubeRotations[NUM_INSTANCES];
} ovrScene;

static const char VERTEX_SHADER[] =
        "#version 300 es\n"
                "#ifndef DISABLE_MULTIVIEW\n"
                "	#define DISABLE_MULTIVIEW 0\n"
                "#endif\n"
                "#define NUM_VIEWS 2\n"
                "#if defined( GL_OVR_multiview2 ) && ! DISABLE_MULTIVIEW\n"
                "	#extension GL_OVR_multiview2 : enable\n"
                "	layout(num_views=NUM_VIEWS) in;\n"
                "	#define VIEW_ID gl_ViewID_OVR\n"
                "#else\n"
                "	uniform lowp int ViewID;\n"
                "	#define VIEW_ID ViewID\n"
                "#endif\n"
                "in vec3 vertexPosition;\n"
                "in vec4 vertexColor;\n"
                "in mat4 vertexTransform;\n"
                "uniform SceneMatrices\n"
                "{\n"
                "	uniform mat4 ViewMatrix[NUM_VIEWS];\n"
                "	uniform mat4 ProjectionMatrix;\n"
                "} sm;\n"
                "out vec4 fragmentColor;\n"
                "void main()\n"
                "{\n"
                "	gl_Position = sm.ProjectionMatrix * ( sm.ViewMatrix[VIEW_ID] * ( vertexTransform * vec4( vertexPosition, 1.0 ) ) );\n"
                "	fragmentColor = vertexColor;\n"
                "}\n";

static const char FRAGMENT_SHADER[] =
        "#version 300 es\n"
                "in lowp vec4 fragmentColor;\n"
                "out lowp vec4 outColor;\n"
                "void main()\n"
                "{\n"
                "	outColor = fragmentColor;\n"
                "}\n";

static void ovrScene_Clear( ovrScene * scene )
{
    scene->CreatedScene = false;
    scene->CreatedVAOs = false;
    scene->Random = 2;
    scene->SceneMatrices = 0;
    scene->InstanceTransformBuffer = 0;

    ovrProgram_Clear( &scene->Program );
    ovrGeometry_Clear( &scene->Cube );
}

static bool ovrScene_IsCreated( ovrScene * scene )
{
    return scene->CreatedScene;
}

static void ovrScene_CreateVAOs( ovrScene * scene )
{
    if ( !scene->CreatedVAOs )
    {
        ovrGeometry_CreateVAO( &scene->Cube );

        // Modify the VAO to use the instance transform attributes.
        GL( glBindVertexArray( scene->Cube.VertexArrayObject ) );
        GL( glBindBuffer( GL_ARRAY_BUFFER, scene->InstanceTransformBuffer ) );
        for ( int i = 0; i < 4; i++ )
        {
            GL( glEnableVertexAttribArray( (int)OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_TRANSFORM + i ) );
            GL( glVertexAttribPointer( (int)OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_TRANSFORM + i, 4, GL_FLOAT,
                                       false, 4 * 4 * sizeof( float ), (void *)( i * 4 * sizeof( float ) ) ) );
            GL( glVertexAttribDivisor( (int)OVRVertexAttributeLocation::VERTEX_ATTRIBUTE_LOCATION_TRANSFORM + i, 1 ) );
        }
        GL( glBindVertexArray( 0 ) );

        scene->CreatedVAOs = true;
    }
}

static void ovrScene_DestroyVAOs( ovrScene * scene )
{
    if ( scene->CreatedVAOs )
    {
        ovrGeometry_DestroyVAO( &scene->Cube );

        scene->CreatedVAOs = false;
    }
}

// Returns a random float in the range [0, 1].
static float ovrScene_RandomFloat( ovrScene * scene )
{
    scene->Random = 1664525L * scene->Random + 1013904223L;
    unsigned int rf = 0x3F800000 | ( scene->Random & 0x007FFFFF );
    return (*(float *)&rf) - 1.0f;
}

static void ovrScene_Create( ovrScene * scene, bool useMultiview )
{
    ovrProgram_Create( &scene->Program, VERTEX_SHADER, FRAGMENT_SHADER, useMultiview );
    ovrGeometry_CreateCube( &scene->Cube );

    // Create the instance transform attribute buffer.
    GL( glGenBuffers( 1, &scene->InstanceTransformBuffer ) );
    GL( glBindBuffer( GL_ARRAY_BUFFER, scene->InstanceTransformBuffer ) );
    GL( glBufferData( GL_ARRAY_BUFFER, NUM_INSTANCES * 4 * 4 * sizeof( float ), NULL, GL_DYNAMIC_DRAW ) );
    GL( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

    // Setup the scene matrices.
    GL( glGenBuffers( 1, &scene->SceneMatrices ) );
    GL( glBindBuffer( GL_UNIFORM_BUFFER, scene->SceneMatrices ) );
    GL( glBufferData( GL_UNIFORM_BUFFER, 2 * sizeof( ovrMatrix4f ) /* 2 view matrices */ + sizeof( ovrMatrix4f ) /* 1 projection matrix*/,
                      NULL, GL_STATIC_DRAW ) );
    GL( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );

    // Setup random rotations.
    for ( int i = 0; i < NUM_ROTATIONS; i++ )
    {
        scene->Rotations[i].x = ovrScene_RandomFloat( scene );
        scene->Rotations[i].y = ovrScene_RandomFloat( scene );
        scene->Rotations[i].z = ovrScene_RandomFloat( scene );
    }

    // Setup random cube positions and rotations.
    for ( int i = 0; i < NUM_INSTANCES; i++ )
    {
        // Using volatile keeps the compiler from optimizing away multiple calls to ovrScene_RandomFloat().
        volatile float rx, ry, rz;
        for ( ; ; )
        {
            rx = ( ovrScene_RandomFloat( scene ) - 0.5f ) * ( 50.0f + sqrt( NUM_INSTANCES ) );
            ry = ( ovrScene_RandomFloat( scene ) - 0.5f ) * ( 50.0f + sqrt( NUM_INSTANCES ) );
            rz = ( ovrScene_RandomFloat( scene ) - 0.5f ) * ( 50.0f + sqrt( NUM_INSTANCES ) );
            // If too close to 0,0,0
            if ( fabsf( rx ) < 4.0f && fabsf( ry ) < 4.0f && fabsf( rz ) < 4.0f )
            {
                continue;
            }
            // Test for overlap with any of the existing cubes.
            bool overlap = false;
            for ( int j = 0; j < i; j++ )
            {
                if (	fabsf( rx - scene->CubePositions[j].x ) < 4.0f &&
                        fabsf( ry - scene->CubePositions[j].y ) < 4.0f &&
                        fabsf( rz - scene->CubePositions[j].z ) < 4.0f )
                {
                    overlap = true;
                    break;
                }
            }
            if ( !overlap )
            {
                break;
            }
        }

        // Insert into list sorted based on distance.
        int insert = 0;
        const float distSqr = rx * rx + ry * ry + rz * rz;
        for ( int j = i; j > 0; j-- )
        {
            const ovrVector3f * otherPos = &scene->CubePositions[j - 1];
            const float otherDistSqr = otherPos->x * otherPos->x + otherPos->y * otherPos->y + otherPos->z * otherPos->z;
            if ( distSqr > otherDistSqr )
            {
                insert = j;
                break;
            }
            scene->CubePositions[j] = scene->CubePositions[j - 1];
            scene->CubeRotations[j] = scene->CubeRotations[j - 1];
        }

        scene->CubePositions[insert].x = rx;
        scene->CubePositions[insert].y = ry;
        scene->CubePositions[insert].z = rz;

        scene->CubeRotations[insert] = (int)( ovrScene_RandomFloat( scene ) * ( NUM_ROTATIONS - 0.1f ) );
    }

    scene->CreatedScene = true;

#if !MULTI_THREADED
    ovrScene_CreateVAOs( scene );
#endif
}

static void ovrScene_Destroy( ovrScene * scene )
{
#if !MULTI_THREADED
    ovrScene_DestroyVAOs( scene );
#endif

    ovrProgram_Destroy( &scene->Program );
    ovrGeometry_Destroy( &scene->Cube );
    GL( glDeleteBuffers( 1, &scene->InstanceTransformBuffer ) );
    GL( glDeleteBuffers( 1, &scene->SceneMatrices ) );
    scene->CreatedScene = false;
}

/*
================================================================================

ovrSimulation

================================================================================
*/

typedef struct
{
    ovrVector3f			CurrentRotation;
} ovrSimulation;

static void ovrSimulation_Clear( ovrSimulation * simulation )
{
    simulation->CurrentRotation.x = 0.0f;
    simulation->CurrentRotation.y = 0.0f;
    simulation->CurrentRotation.z = 0.0f;
}

static void ovrSimulation_Advance( ovrSimulation * simulation, double predictedDisplayTime )
{
    // Update rotation.
    simulation->CurrentRotation.x = (float)( predictedDisplayTime );
    simulation->CurrentRotation.y = (float)( predictedDisplayTime );
    simulation->CurrentRotation.z = (float)( predictedDisplayTime );
}

/*
================================================================================

ovrRenderer

================================================================================
*/

typedef struct
{
    ovrFramebuffer	FrameBuffer[VRAPI_FRAME_LAYER_EYE_MAX];
    ovrFence *		Fence[VRAPI_FRAME_LAYER_EYE_MAX];
    ovrMatrix4f		ProjectionMatrix;
    ovrMatrix4f		TexCoordsTanAnglesMatrix;
    int				NumBuffers;
} ovrRenderer;

static void ovrRenderer_Clear( ovrRenderer * renderer )
{
    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        ovrFramebuffer_Clear( &renderer->FrameBuffer[eye] );
    }
    renderer->ProjectionMatrix = ovrMatrix4f_CreateIdentity();
    renderer->TexCoordsTanAnglesMatrix = ovrMatrix4f_CreateIdentity();
    renderer->NumBuffers = VRAPI_FRAME_LAYER_EYE_MAX;
}

static void ovrRenderer_Create( ovrRenderer * renderer, const ovrJava * java, const bool useMultiview )
{
    renderer->NumBuffers = useMultiview ? 1 : VRAPI_FRAME_LAYER_EYE_MAX;

    // Create the frame buffers.
    for ( int eye = 0; eye < renderer->NumBuffers; eye++ )
    {
        ovrFramebuffer_Create( &renderer->FrameBuffer[eye], useMultiview,
                               VRAPI_TEXTURE_FORMAT_8888,
                               vrapi_GetSystemPropertyInt( java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH ),
                               vrapi_GetSystemPropertyInt( java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT ),
                               NUM_MULTI_SAMPLES );

        renderer->Fence[eye] = (ovrFence *) malloc( renderer->FrameBuffer[eye].TextureSwapChainLength * sizeof( ovrFence ) );
        for ( int i = 0; i < renderer->FrameBuffer[eye].TextureSwapChainLength; i++ )
        {
            ovrFence_Create( &renderer->Fence[eye][i] );
        }
    }

    // Setup the projection matrix.
    renderer->ProjectionMatrix = ovrMatrix4f_CreateProjectionFov(
            vrapi_GetSystemPropertyFloat( java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_X ),
            vrapi_GetSystemPropertyFloat( java, VRAPI_SYS_PROP_SUGGESTED_EYE_FOV_DEGREES_Y ),
            0.0f, 0.0f, 1.0f, 0.0f );
    renderer->TexCoordsTanAnglesMatrix = ovrMatrix4f_TanAngleMatrixFromProjection( &renderer->ProjectionMatrix );
}

static void ovrRenderer_Destroy( ovrRenderer * renderer )
{
    for ( int eye = 0; eye < renderer->NumBuffers; eye++ )
    {
        for ( int i = 0; i < renderer->FrameBuffer[eye].TextureSwapChainLength; i++ )
        {
            ovrFence_Destroy( &renderer->Fence[eye][i] );
        }
        free( renderer->Fence[eye] );

        ovrFramebuffer_Destroy( &renderer->FrameBuffer[eye] );
    }
    renderer->ProjectionMatrix = ovrMatrix4f_CreateIdentity();
    renderer->TexCoordsTanAnglesMatrix = ovrMatrix4f_CreateIdentity();
}

static ovrFrameParms ovrRenderer_RenderFrame( ovrRenderer * renderer, const ovrJava * java,
                                              long long frameIndex, int minimumVsyncs, const ovrPerformanceParms * perfParms,
                                              const ovrScene * scene, const ovrSimulation * simulation,
                                              const ovrTracking * tracking, ovrMobile * ovr )
{
    ovrFrameParms parms = vrapi_DefaultFrameParms( java, VRAPI_FRAME_INIT_DEFAULT, vrapi_GetTimeInSeconds(), NULL );
    parms.FrameIndex = frameIndex;
    parms.MinimumVsyncs = minimumVsyncs;
    parms.PerformanceParms = *perfParms;

    ovrMatrix4f rotationMatrices[NUM_ROTATIONS];
    for ( int i = 0; i < NUM_ROTATIONS; i++ )
    {
        rotationMatrices[i] = ovrMatrix4f_CreateRotation(
                scene->Rotations[i].x * simulation->CurrentRotation.x,
                scene->Rotations[i].y * simulation->CurrentRotation.y,
                scene->Rotations[i].z * simulation->CurrentRotation.z );
    }

    // Update the instance transform attributes.
    GL( glBindBuffer( GL_ARRAY_BUFFER, scene->InstanceTransformBuffer ) );
    GL( ovrMatrix4f * cubeTransforms = (ovrMatrix4f *) glMapBufferRange( GL_ARRAY_BUFFER, 0,
                NUM_INSTANCES * sizeof( ovrMatrix4f ), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT ) );
    for ( int i = 0; i < NUM_INSTANCES; i++ )
    {
        const int index = scene->CubeRotations[i];

        // Write in order in case the mapped buffer lives on write-combined memory.
        cubeTransforms[i].M[0][0] = rotationMatrices[index].M[0][0];
        cubeTransforms[i].M[0][1] = rotationMatrices[index].M[0][1];
        cubeTransforms[i].M[0][2] = rotationMatrices[index].M[0][2];
        cubeTransforms[i].M[0][3] = rotationMatrices[index].M[0][3];

        cubeTransforms[i].M[1][0] = rotationMatrices[index].M[1][0];
        cubeTransforms[i].M[1][1] = rotationMatrices[index].M[1][1];
        cubeTransforms[i].M[1][2] = rotationMatrices[index].M[1][2];
        cubeTransforms[i].M[1][3] = rotationMatrices[index].M[1][3];

        cubeTransforms[i].M[2][0] = rotationMatrices[index].M[2][0];
        cubeTransforms[i].M[2][1] = rotationMatrices[index].M[2][1];
        cubeTransforms[i].M[2][2] = rotationMatrices[index].M[2][2];
        cubeTransforms[i].M[2][3] = rotationMatrices[index].M[2][3];

        cubeTransforms[i].M[3][0] = scene->CubePositions[i].x;
        cubeTransforms[i].M[3][1] = scene->CubePositions[i].y;
        cubeTransforms[i].M[3][2] = scene->CubePositions[i].z;
        cubeTransforms[i].M[3][3] = 1.0f;
    }
    GL( glUnmapBuffer( GL_ARRAY_BUFFER ) );
    GL( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

    const ovrHeadModelParms headModelParms = vrapi_DefaultHeadModelParms();

#if REDUCED_LATENCY
    // Update orientation, not position.
	ovrTracking updatedTracking = vrapi_GetPredictedTracking( ovr, tracking->HeadPose.TimeInSeconds );
	updatedTracking.HeadPose.Pose.Position = tracking->HeadPose.Pose.Position;
#else
    ovrTracking updatedTracking = *tracking;
#endif

    // Calculate the view matrix.
    const ovrMatrix4f centerEyeViewMatrix = vrapi_GetCenterEyeViewMatrix( &headModelParms, &updatedTracking, NULL );

    ovrMatrix4f eyeViewMatrix[2];
    eyeViewMatrix[0] = vrapi_GetEyeViewMatrix( &headModelParms, &centerEyeViewMatrix, 0 );
    eyeViewMatrix[1] = vrapi_GetEyeViewMatrix( &headModelParms, &centerEyeViewMatrix, 1 );

    ovrMatrix4f eyeViewMatrixTransposed[2];
    eyeViewMatrixTransposed[0] = ovrMatrix4f_Transpose( &eyeViewMatrix[0] );
    eyeViewMatrixTransposed[1] = ovrMatrix4f_Transpose( &eyeViewMatrix[1] );

    ovrMatrix4f projectionMatrixTransposed;
    projectionMatrixTransposed = ovrMatrix4f_Transpose( &renderer->ProjectionMatrix );

    // Update the scene matrices.
    GL( glBindBuffer( GL_UNIFORM_BUFFER, scene->SceneMatrices ) );
    GL( ovrMatrix4f * sceneMatrices = (ovrMatrix4f *) glMapBufferRange( GL_UNIFORM_BUFFER, 0,
                                                                        2 * sizeof( ovrMatrix4f ) /* 2 view matrices */ + sizeof( ovrMatrix4f ) /* 1 projection matrix */,
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT ) );

    if ( sceneMatrices != NULL )
    {
        memcpy( (char *)sceneMatrices, &eyeViewMatrixTransposed, 2 * sizeof( ovrMatrix4f ) );
        memcpy( (char *)sceneMatrices + 2 * sizeof( ovrMatrix4f ), &projectionMatrixTransposed, 1 * sizeof( ovrMatrix4f ) );
    }

    GL( glUnmapBuffer( GL_UNIFORM_BUFFER ) );
    GL( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );

    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        ovrFramebuffer * frameBuffer = &renderer->FrameBuffer[renderer->NumBuffers == 1 ? 0 : eye];
        parms.Layers[0].Textures[eye].ColorTextureSwapChain = frameBuffer->ColorTextureSwapChain;
        parms.Layers[0].Textures[eye].TextureSwapChainIndex = frameBuffer->TextureSwapChainIndex;
        parms.Layers[0].Textures[eye].TexCoordsFromTanAngles = renderer->TexCoordsTanAnglesMatrix;
        parms.Layers[0].Textures[eye].HeadPose = updatedTracking.HeadPose;
    }

    parms.Layers[0].Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

    unsigned long long completionFence[VRAPI_FRAME_LAYER_EYE_MAX] = { 0 };

    // Render the eye images.
    for ( int eye = 0; eye < renderer->NumBuffers; eye++ )
    {
        // NOTE: In the non-mv case, latency can be further reduced by updating the sensor prediction
        // for each eye (updates orientation, not position)
        ovrFramebuffer * frameBuffer = &renderer->FrameBuffer[eye];
        ovrFramebuffer_SetCurrent( frameBuffer );

        GL( glUseProgram( scene->Program.Program ) );
        GL( glBindBufferBase( GL_UNIFORM_BUFFER, scene->Program.UniformBinding[UNIFORM_SCENE_MATRICES], scene->SceneMatrices ) );
        if ( scene->Program.UniformLocation[UNIFORM_VIEW_ID] >= 0 )  // NOTE: will not be present when multiview path enabled.
        {
            GL( glUniform1i( scene->Program.UniformLocation[UNIFORM_VIEW_ID], eye ) );
        }
        GL( glEnable( GL_SCISSOR_TEST ) );
        GL( glDepthMask( GL_TRUE ) );
        GL( glEnable( GL_DEPTH_TEST ) );
        GL( glDepthFunc( GL_LEQUAL ) );
        GL( glEnable( GL_CULL_FACE ) );
        GL( glCullFace( GL_BACK ) );
        GL( glViewport( 0, 0, frameBuffer->Width, frameBuffer->Height ) );
        GL( glScissor( 0, 0, frameBuffer->Width, frameBuffer->Height ) );
        GL( glClearColor( 0.125f, 0.0f, 0.125f, 1.0f ) );
        GL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );
        GL( glBindVertexArray( scene->Cube.VertexArrayObject ) );
        GL( glDrawElementsInstanced( GL_TRIANGLES, scene->Cube.IndexCount, GL_UNSIGNED_SHORT, NULL, NUM_INSTANCES ) );
        GL( glBindVertexArray( 0 ) );
        GL( glUseProgram( 0 ) );

        // Explicitly clear the border texels to black because OpenGL-ES does not support GL_CLAMP_TO_BORDER.
        {
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

        ovrFramebuffer_Resolve( frameBuffer );

        ovrFence * fence = &renderer->Fence[eye][frameBuffer->TextureSwapChainIndex];
        ovrFence_Insert( fence );
        completionFence[eye] = (size_t)fence->Sync;

        ovrFramebuffer_Advance( frameBuffer );
    }

    for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
    {
        parms.Layers[0].Textures[eye].CompletionFence =
                completionFence[renderer->NumBuffers == 1 ? 0 : eye];
    }

    ovrFramebuffer_SetNone();

    return parms;
}

/*
================================================================================

ovrRenderThread

================================================================================
*/

#if MULTI_THREADED

typedef enum
{
	RENDER_FRAME,
	RENDER_LOADING_ICON,
	RENDER_BLACK_FLUSH,
	RENDER_BLACK_FINAL
} ovrRenderType;

typedef struct
{
	JavaVM *			JavaVm;
	jobject				ActivityObject;
	const ovrEgl *		ShareEgl;
	pthread_t			Thread;
	int					Tid;
	// Synchronization
	bool				Exit;
	bool				WorkAvailableFlag;
	bool				WorkDoneFlag;
	pthread_cond_t		WorkAvailableCondition;
	pthread_cond_t		WorkDoneCondition;
	pthread_mutex_t		Mutex;
	// Latched data for rendering.
	ovrMobile *			Ovr;
	ovrRenderType		RenderType;
	long long			FrameIndex;
	int					MinimumVsyncs;
	ovrPerformanceParms	PerformanceParms;
	ovrScene *			Scene;
	ovrSimulation		Simulation;
	ovrTracking			Tracking;
} ovrRenderThread;

void * RenderThreadFunction( void * parm )
{
	ovrRenderThread * renderThread = (ovrRenderThread *)parm;
	renderThread->Tid = gettid();

	ovrJava java;
	java.Vm = renderThread->JavaVm;
	(*java.Vm)->AttachCurrentThread( java.Vm, &java.Env, NULL );
	java.ActivityObject = renderThread->ActivityObject;

	// Note that AttachCurrentThread will reset the thread name.
	prctl( PR_SET_NAME, (long)"OVR::Renderer", 0, 0, 0 );

	ovrEgl egl;
	ovrEgl_CreateContext( &egl, renderThread->ShareEgl );

	ovrRenderer renderer;
	ovrRenderer_Create( &renderer, &java );

	ovrScene * lastScene = NULL;

	for( ; ; )
	{
		// Signal work completed.
		pthread_mutex_lock( &renderThread->Mutex );
		renderThread->WorkDoneFlag = true;
		pthread_cond_signal( &renderThread->WorkDoneCondition );
		pthread_mutex_unlock( &renderThread->Mutex );

		// Wait for work.
		pthread_mutex_lock( &renderThread->Mutex );
		while ( !renderThread->WorkAvailableFlag )
		{
			pthread_cond_wait( &renderThread->WorkAvailableCondition, &renderThread->Mutex );
		}
		renderThread->WorkAvailableFlag = false;
		pthread_mutex_unlock( &renderThread->Mutex );

		// Check for exit.
		if ( renderThread->Exit )
		{
			break;
		}

		// Make sure the scene has VAOs created for this context.
		if ( renderThread->Scene != NULL && renderThread->Scene != lastScene )
		{
			if ( lastScene != NULL )
			{
				ovrScene_DestroyVAOs( lastScene );
			}
			ovrScene_CreateVAOs( renderThread->Scene );
			lastScene = renderThread->Scene;
		}

		// Render.
		ovrFrameParms frameParms;
		if ( renderThread->RenderType == RENDER_FRAME )
		{
			frameParms = ovrRenderer_RenderFrame( &renderer, &java,
					renderThread->FrameIndex, renderThread->MinimumVsyncs, &renderThread->PerformanceParms,
					renderThread->Scene, &renderThread->Simulation,
					&renderThread->Tracking, renderThread->Ovr );
		}
		else if ( renderThread->RenderType == RENDER_LOADING_ICON )
		{
			frameParms = vrapi_DefaultFrameParms( &java, VRAPI_FRAME_INIT_LOADING_ICON_FLUSH, vrapi_GetTimeInSeconds(), NULL );
			frameParms.FrameIndex = renderThread->FrameIndex;
			frameParms.MinimumVsyncs = renderThread->MinimumVsyncs;
			frameParms.PerformanceParms = renderThread->PerformanceParms;
		}
		else if ( renderThread->RenderType == RENDER_BLACK_FLUSH )
		{
			frameParms = vrapi_DefaultFrameParms( &java, VRAPI_FRAME_INIT_BLACK_FLUSH, vrapi_GetTimeInSeconds(), NULL );
			frameParms.FrameIndex = renderThread->FrameIndex;
			frameParms.MinimumVsyncs = renderThread->MinimumVsyncs;
			frameParms.PerformanceParms = renderThread->PerformanceParms;
		}
		else if ( renderThread->RenderType == RENDER_BLACK_FINAL )
		{
			frameParms = vrapi_DefaultFrameParms( &java, VRAPI_FRAME_INIT_BLACK_FINAL, vrapi_GetTimeInSeconds(), NULL );
			frameParms.FrameIndex = renderThread->FrameIndex;
			frameParms.MinimumVsyncs = renderThread->MinimumVsyncs;
			frameParms.PerformanceParms = renderThread->PerformanceParms;
		}

		// Hand over the eye images to the time warp.
		vrapi_SubmitFrame( renderThread->Ovr, &frameParms );
	}

	if ( lastScene != NULL )
	{
		ovrScene_DestroyVAOs( lastScene );
	}

	ovrRenderer_Destroy( &renderer );
	ovrEgl_DestroyContext( &egl );

	(*java.Vm)->DetachCurrentThread( java.Vm );
}

static void ovrRenderThread_Clear( ovrRenderThread * renderThread )
{
	renderThread->JavaVm = NULL;
	renderThread->ActivityObject = NULL;
	renderThread->ShareEgl = NULL;
	renderThread->Thread = 0;
	renderThread->Tid = 0;
	renderThread->Exit = false;
	renderThread->WorkAvailableFlag = false;
	renderThread->WorkDoneFlag = false;
	renderThread->Ovr = NULL;
	renderThread->RenderType = RENDER_FRAME;
	renderThread->FrameIndex = 1;
	renderThread->MinimumVsyncs = 1;
	renderThread->PerformanceParms = vrapi_DefaultPerformanceParms();
	renderThread->Scene = NULL;
	ovrSimulation_Clear( &renderThread->Simulation );
}

static void ovrRenderThread_Create( ovrRenderThread * renderThread, const ovrJava * java, const ovrEgl * shareEgl )
{
	renderThread->JavaVm = java->Vm;
	renderThread->ActivityObject = java->ActivityObject;
	renderThread->ShareEgl = shareEgl;
	renderThread->Thread = 0;
	renderThread->Tid = 0;
	renderThread->Exit = false;
	renderThread->WorkAvailableFlag = false;
	renderThread->WorkDoneFlag = false;
	pthread_cond_init( &renderThread->WorkAvailableCondition, NULL );
	pthread_cond_init( &renderThread->WorkDoneCondition, NULL );
	pthread_mutex_init( &renderThread->Mutex, NULL );

	const int createErr = pthread_create( &renderThread->Thread, NULL, RenderThreadFunction, renderThread );
	if ( createErr != 0 )
	{
		ALOGE( "pthread_create returned %i", createErr );
	}
}

static void ovrRenderThread_Destroy( ovrRenderThread * renderThread )
{
	pthread_mutex_lock( &renderThread->Mutex );
	renderThread->Exit = true;
	renderThread->WorkAvailableFlag = true;
	pthread_cond_signal( &renderThread->WorkAvailableCondition );
	pthread_mutex_unlock( &renderThread->Mutex );

	pthread_join( renderThread->Thread, NULL );
	pthread_cond_destroy( &renderThread->WorkAvailableCondition );
	pthread_cond_destroy( &renderThread->WorkDoneCondition );
	pthread_mutex_destroy( &renderThread->Mutex );
}

static void ovrRenderThread_Submit( ovrRenderThread * renderThread, ovrMobile * ovr,
		ovrRenderType type, long long frameIndex, int minimumVsyncs, const ovrPerformanceParms * perfParms,
		ovrScene * scene, const ovrSimulation * simulation, const ovrTracking * tracking )
{
	// Wait for the renderer thread to finish the last frame.
	pthread_mutex_lock( &renderThread->Mutex );
	while ( !renderThread->WorkDoneFlag )
	{
		pthread_cond_wait( &renderThread->WorkDoneCondition, &renderThread->Mutex );
	}
	renderThread->WorkDoneFlag = false;
	// Latch the render data.
	renderThread->Ovr = ovr;
	renderThread->RenderType = type;
	renderThread->FrameIndex = frameIndex;
	renderThread->MinimumVsyncs = minimumVsyncs;
	renderThread->PerformanceParms = *perfParms;
	renderThread->Scene = scene;
	if ( simulation != NULL )
	{
		renderThread->Simulation = *simulation;
	}
	if ( tracking != NULL )
	{
		renderThread->Tracking = *tracking;
	}
	// Signal work is available.
	renderThread->WorkAvailableFlag = true;
	pthread_cond_signal( &renderThread->WorkAvailableCondition );
	pthread_mutex_unlock( &renderThread->Mutex );
}

static void ovrRenderThread_Wait( ovrRenderThread * renderThread )
{
	// Wait for the renderer thread to finish the last frame.
	pthread_mutex_lock( &renderThread->Mutex );
	while ( !renderThread->WorkDoneFlag )
	{
		pthread_cond_wait( &renderThread->WorkDoneCondition, &renderThread->Mutex );
	}
	pthread_mutex_unlock( &renderThread->Mutex );
}

static int ovrRenderThread_GetTid( ovrRenderThread * renderThread )
{
	ovrRenderThread_Wait( renderThread );
	return renderThread->Tid;
}

#endif // MULTI_THREADED

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
    ovrScene			Scene;
    ovrSimulation		Simulation;
    long long			FrameIndex;
    int					MinimumVsyncs;
    ovrBackButtonState	BackButtonState;
    bool				BackButtonDown;
    double				BackButtonDownStartTime;
#if MULTI_THREADED
    ovrRenderThread		RenderThread;
#else
    ovrRenderer			Renderer;
#endif
    bool				UseMultiview;
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
    app->MinimumVsyncs = 1;
    app->BackButtonState = BACK_BUTTON_STATE_NONE;
    app->BackButtonDown = false;
    app->BackButtonDownStartTime = 0.0;
    app->UseMultiview = true;

    ovrEgl_Clear( &app->Egl );
    ovrScene_Clear( &app->Scene );
    ovrSimulation_Clear( &app->Simulation );
#if MULTI_THREADED
    ovrRenderThread_Clear( &app->RenderThread );
#else
    ovrRenderer_Clear( &app->Renderer );
#endif
}

static void ovrApp_PushBlackFinal( ovrApp * app, const ovrPerformanceParms * perfParms )
{
#if MULTI_THREADED
    ovrRenderThread_Submit( &app->RenderThread, app->Ovr,
			RENDER_BLACK_FINAL, app->FrameIndex, app->MinimumVsyncs, perfParms,
			NULL, NULL, NULL );
#else
    ovrFrameParms frameParms = vrapi_DefaultFrameParms( &app->Java, VRAPI_FRAME_INIT_BLACK_FINAL, vrapi_GetTimeInSeconds(), NULL );
    frameParms.FrameIndex = app->FrameIndex;
    frameParms.PerformanceParms = *perfParms;
    vrapi_SubmitFrame( app->Ovr, &frameParms );
#endif
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
#if MULTI_THREADED
            // Make sure the renderer thread is no longer using the ovrMobile.
			ovrRenderThread_Wait( &app->RenderThread );
#endif
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
        return 1;
    }
    return 0;
}

static int ovrApp_HandleTouchEvent( ovrApp * app, const int action, const float x, const float y )
{
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
    MESSAGE_ON_TOUCH_EVENT
};

struct ovrAppThread
{
    JavaVM *		JavaVm;
    jobject			ActivityObject;
    pthread_t		Thread;
    ovrMessageQueue	MessageQueue;
    ANativeWindow * NativeWindow;
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

    ALOGV( "AppState UseMultiview : %d", appState.UseMultiview );

    ovrPerformanceParms perfParms = vrapi_DefaultPerformanceParms();
    perfParms.CpuLevel = CPU_LEVEL;
    perfParms.GpuLevel = GPU_LEVEL;
    perfParms.MainThreadTid = gettid();

#if MULTI_THREADED
    ovrRenderThread_Create( &appState.RenderThread, &appState.Java, &appState.Egl );
	// Also set the renderer thread to SCHED_FIFO.
	perfParms.RenderThreadTid = ovrRenderThread_GetTid( &appState.RenderThread );
#else
    ovrRenderer_Create( &appState.Renderer, &java, appState.UseMultiview );
#endif

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
                case MESSAGE_ON_RESUME:				{ appState.Resumed = true; break; }
                case MESSAGE_ON_PAUSE:				{ appState.Resumed = false; break; }
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
            }

            ovrApp_HandleVrModeChanges( &appState );
        }

        ovrApp_BackButtonAction( &appState, &perfParms );

        if ( appState.Ovr == NULL )
        {
            continue;
        }

        // Create the scene if not yet created.
        // The scene is created here to be able to show a loading icon.
        if ( !ovrScene_IsCreated( &appState.Scene ) )
        {
#if MULTI_THREADED
            // Show a loading icon.
			ovrRenderThread_Submit( &appState.RenderThread, appState.Ovr,
					RENDER_LOADING_ICON, appState.FrameIndex, appState.MinimumVsyncs, &perfParms,
					NULL, NULL, NULL );
#else
            // Show a loading icon.
            ovrFrameParms frameParms = vrapi_DefaultFrameParms( &appState.Java, VRAPI_FRAME_INIT_LOADING_ICON_FLUSH, vrapi_GetTimeInSeconds(), NULL );
            frameParms.FrameIndex = appState.FrameIndex;
            frameParms.PerformanceParms = perfParms;
            vrapi_SubmitFrame( appState.Ovr, &frameParms );
#endif

            // Create the scene.
            ovrScene_Create( &appState.Scene, appState.UseMultiview );
        }

        // This is the only place the frame index is incremented, right before
        // calling vrapi_GetPredictedDisplayTime().
        appState.FrameIndex++;

        // Get the HMD pose, predicted for the middle of the time period during which
        // the new eye images will be displayed. The number of frames predicted ahead
        // depends on the pipeline depth of the engine and the synthesis rate.
        // The better the prediction, the less black will be pulled in at the edges.
        const double predictedDisplayTime = vrapi_GetPredictedDisplayTime( appState.Ovr, appState.FrameIndex );
        const ovrTracking baseTracking = vrapi_GetPredictedTracking( appState.Ovr, predictedDisplayTime );

        // Apply the head-on-a-stick model if there is no positional tracking.
        const ovrHeadModelParms headModelParms = vrapi_DefaultHeadModelParms();
        const ovrTracking tracking = vrapi_ApplyHeadModel( &headModelParms, &baseTracking );

        // Advance the simulation based on the predicted display time.
        ovrSimulation_Advance( &appState.Simulation, predictedDisplayTime );

#if MULTI_THREADED
        // Render the eye images on a separate thread.
		ovrRenderThread_Submit( &appState.RenderThread, appState.Ovr,
				RENDER_FRAME, appState.FrameIndex, appState.MinimumVsyncs, &perfParms,
				&appState.Scene, &appState.Simulation, &tracking );
#else
        // Render eye images and setup ovrFrameParms using ovrTracking.
        const ovrFrameParms frameParms = ovrRenderer_RenderFrame( &appState.Renderer, &appState.Java,
                                                                  appState.FrameIndex, appState.MinimumVsyncs, &perfParms,
                                                                  &appState.Scene, &appState.Simulation, &tracking,
                                                                  appState.Ovr );

        // Hand over the eye images to the time warp.
        vrapi_SubmitFrame( appState.Ovr, &frameParms );
#endif
    }

#if MULTI_THREADED
    ovrRenderThread_Destroy( &appState.RenderThread );
#else
    ovrRenderer_Destroy( &appState.Renderer );
#endif

    ovrScene_Destroy( &appState.Scene );
    ovrEgl_DestroyContext( &appState.Egl );

    vrapi_Shutdown();

    java.Vm->DetachCurrentThread( );

    return NULL;
}

static void ovrAppThread_Create( ovrAppThread * appThread, JNIEnv * env, jobject activityObject )
{
    env->GetJavaVM( &appThread->JavaVm );
    appThread->ActivityObject = env->NewGlobalRef( activityObject );
    appThread->Thread = 0;
    appThread->NativeWindow = NULL;
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
    ovrMessageQueue_Destroy( &appThread->MessageQueue );
}

/*
================================================================================

Activity lifecycle

================================================================================
*/

VROSceneRendererOVR::VROSceneRendererOVR(std::shared_ptr<gvr::AudioApi> gvrAudio,
                                         jobject activity, JNIEnv *env) {
    _driver = std::make_shared<VRODriverOpenGLAndroid>(gvrAudio);

    ALOGV( "    GLES3JNILib::onCreate()" );

    _appThread = (ovrAppThread *) malloc( sizeof( ovrAppThread ) );
    ovrAppThread_Create( _appThread, env, activity );

    ovrMessageQueue_Enable( &_appThread->MessageQueue, true );
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_CREATE, MQ_WAIT_PROCESSED );
    ovrMessageQueue_PostMessage( &_appThread->MessageQueue, &message );
}

VROSceneRendererOVR::~VROSceneRendererOVR() {
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

void VROSceneRendererOVR::onSurfaceChanged(jobject surface) {
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

JNIEXPORT void JNICALL Java_com_oculus_gles3jni_GLES3JNILib_onKeyEvent( JNIEnv * env, jobject obj, jlong handle, int keyCode, int action )
{
    if ( action == AKEY_EVENT_ACTION_UP )
    {
        ALOGV( "    GLES3JNILib::onKeyEvent( %d, %d )", keyCode, action );
    }
    ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_KEY_EVENT, MQ_WAIT_NONE );
    ovrMessage_SetIntegerParm( &message, 0, keyCode );
    ovrMessage_SetIntegerParm( &message, 1, action );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

JNIEXPORT void JNICALL Java_com_oculus_gles3jni_GLES3JNILib_onTouchEvent( JNIEnv * env, jobject obj, jlong handle, int action, float x, float y )
{
    if ( action == AMOTION_EVENT_ACTION_UP )
    {
        ALOGV( "    GLES3JNILib::onTouchEvent( %d, %1.0f, %1.0f )", action, x, y );
    }
    ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
    ovrMessage message;
    ovrMessage_Init( &message, MESSAGE_ON_TOUCH_EVENT, MQ_WAIT_NONE );
    ovrMessage_SetIntegerParm( &message, 0, action );
    ovrMessage_SetFloatParm( &message, 1, x );
    ovrMessage_SetFloatParm( &message, 2, y );
    ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

#pragma mark - Rendering

void VROSceneRendererOVR::initGL() {
}


void VROSceneRendererOVR::onDrawFrame() {

}

void VROSceneRendererOVR::onTriggerEvent() {

}
