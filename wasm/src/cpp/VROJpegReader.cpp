//
//  VROJpegReader.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/28/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

// Modifed from SDL_Image (JPG) to work without SDL_RW

/*
 SDL_image:  An example image loading library for use with SDL
 Copyright (C) 1997-2013 Sam Lantinga <slouken@libsdl.org>
 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 */

/* This is a JPEG image file loading framework */

#include "VROJpegReader.h"
#include "VROLog.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "VROByteBuffer.h"
#include "SDL_image.h"
#include "jpeglib.h"

#ifdef JPEG_TRUE  /* MinGW version of jpeg-8.x renamed TRUE to JPEG_TRUE etc. */
typedef JPEG_boolean boolean;
#define TRUE JPEG_TRUE
#define FALSE JPEG_FALSE
#endif

/* Define this for fast loading and not as good image quality */
/*#define FAST_JPEG*/

/* Define this for quicker (but less perfect) JPEG identification */
#define FAST_IS_JPEG

bool VROJpegReader::isJPG(void *data, int length) {
    // Check for the FF D8 FF magic number at the start
    return !strncmp((const char *)data, "\xFF\xD8\xFF", 3);
}

namespace viro {
    
#define INPUT_BUFFER_SIZE   4096
typedef struct {
    struct jpeg_source_mgr pub;
    
    VROByteBuffer *ctx;
    Uint8 buffer[INPUT_BUFFER_SIZE];
} my_source_mgr;

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_source (j_decompress_ptr cinfo)
{
    /* We don't actually need to do anything */
    return;
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 */
static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    my_source_mgr * src = (my_source_mgr *) cinfo->src;
    int nbytes;
    
    VROByteBuffer *buffer = (VROByteBuffer *) src->ctx;
    nbytes = buffer->capacity() - buffer->getPosition();
    if (nbytes > INPUT_BUFFER_SIZE) {
        nbytes = INPUT_BUFFER_SIZE;
    }
    
    if (nbytes <= 0) {
        /* Insert a fake EOI marker */
        src->buffer[0] = (Uint8) 0xFF;
        src->buffer[1] = (Uint8) JPEG_EOI;
        nbytes = 2;
    }
    else {
        buffer->copyBytes((void *) src->buffer, nbytes);
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr * src = (my_source_mgr *) cinfo->src;
    
    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
        while (num_bytes > (long) src->pub.bytes_in_buffer) {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            (void) src->pub.fill_input_buffer(cinfo);
            /* note we assume that fill_input_buffer will never
             * return FALSE, so suspension need not be handled.
             */
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.
 */
static void term_source (j_decompress_ptr cinfo)
{
    /* We don't actually need to do anything */
    return;
}

/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */
static void jpeg_buffer_src (j_decompress_ptr cinfo, VROByteBuffer *ctx)
{
    my_source_mgr *src;
    
    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */
    if (cinfo->src == NULL) { /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
        (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                    sizeof(my_source_mgr));
        src = (my_source_mgr *) cinfo->src;
    }
    
    src = (my_source_mgr *) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term_source;
    src->ctx = ctx;
    src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = NULL; /* until buffer loaded */
}

struct my_error_mgr {
    struct jpeg_error_mgr errmgr;
    jmp_buf escape;
};

static void my_error_exit(j_common_ptr cinfo)
{
    struct my_error_mgr *err = (struct my_error_mgr *)cinfo->err;
    longjmp(err->escape, 1);
}

static void output_no_message(j_common_ptr cinfo)
{
    /* do nothing */
}

/* Load a JPEG type image from an VROByteBuffer */
SDL_Surface *IMG_LoadJPG_RW(VROByteBuffer *src)
{
    Sint64 start;
    struct jpeg_decompress_struct cinfo;
    JSAMPROW rowptr[1];
    SDL_Surface *volatile surface = NULL;
    struct my_error_mgr jerr;
    
    if ( !src ) {
        return NULL;
    }
    start = 0;
    
    if ( !IMG_Init(IMG_INIT_JPG) ) {
        return NULL;
    }
    
    /* Create a decompression structure and load the JPEG header */
    cinfo.err = jpeg_std_error(&jerr.errmgr);
    jerr.errmgr.error_exit = my_error_exit;
    jerr.errmgr.output_message = output_no_message;
    if(setjmp(jerr.escape)) {
        /* If we get here, libjpeg found an error */
        jpeg_destroy_decompress(&cinfo);
        if ( surface != NULL ) {
            SDL_FreeSurface(surface);
        }
        return NULL;
    }
    
    jpeg_create_decompress(&cinfo);
    jpeg_buffer_src(&cinfo, src);
    jpeg_read_header(&cinfo, TRUE);
    
    if(cinfo.num_components == 4) {
        /* Set 32-bit Raw output */
        cinfo.out_color_space = JCS_CMYK;
        cinfo.quantize_colors = FALSE;
        jpeg_calc_output_dimensions(&cinfo);
        
        /* Allocate an output surface to hold the image */
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                       cinfo.output_width, cinfo.output_height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                       0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
#else
        0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF);
#endif
    } else {
        /* Set 24-bit RGB output */
        cinfo.out_color_space = JCS_RGB;
        cinfo.quantize_colors = FALSE;
#ifdef FAST_JPEG
        cinfo.scale_num   = 1;
        cinfo.scale_denom = 1;
        cinfo.dct_method = JDCT_FASTEST;
        cinfo.do_fancy_upsampling = FALSE;
#endif
        jpeg_calc_output_dimensions(&cinfo);
        
        /* Allocate an output surface to hold the image */
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                       cinfo.output_width, cinfo.output_height, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                       0x0000FF, 0x00FF00, 0xFF0000,
#else
                                       0xFF0000, 0x00FF00, 0x0000FF,
#endif
                                       0);
    }
    
    if ( surface == NULL ) {
        jpeg_destroy_decompress(&cinfo);
        return NULL;
    }
    
    /* Decompress the image */
    jpeg_start_decompress(&cinfo);
    while ( cinfo.output_scanline < cinfo.output_height ) {
        rowptr[0] = (JSAMPROW)(Uint8 *)surface->pixels +
        cinfo.output_scanline * surface->pitch;
        jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    
    return(surface);
}
}

SDL_Surface *VROJpegReader::loadJPG(void *data, int length) {
    VROByteBuffer buffer(data, length, false);
    return viro::IMG_LoadJPG_RW(&buffer);
}

