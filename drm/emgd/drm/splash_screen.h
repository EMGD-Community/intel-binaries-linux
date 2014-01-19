/*
 *-----------------------------------------------------------------------------
 * Filename: splash_screen.h
 * $Revision: 1.6 $
 *-----------------------------------------------------------------------------
 * Copyright (c) 2002-2010, Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *-----------------------------------------------------------------------------
 * Description:
 *  This is the Intel Embedded Graphics EFI Driver Splash Screen header file.
 *  This file contains data structures pertinent to showing a splash screen
 *  with a customizable icon.
 *-----------------------------------------------------------------------------
 */

#ifndef _SPLASH_SCREEN_H
#define _SPLASH_SCREEN_H

#include <user_config.h>

#define CONV_16_TO_32_BIT(a) (0xFF000000 | ((a & 0xF800)<<8) |\
						((a & 0x7E0)<<5) | (a & 0x1F)<<3)
#define CONV_GS_4_TO_32(a) (0xFF000000 | ((a)<<20) | ((a)<<16) |\
						((a)<<12) | ((a)<<8) | ((a)<<4) | ((a)))

#define CONV_GS_2_TO_32(a) (0xFF000000 | ((a)<<22) | ((a)<<20) | ((a)<<18) |\
						((a)<<16) | ((a)<<14) | ((a)<<12) | ((a)<<10) |\
						((a)<<8) | ((a)<<6) | ((a)<<4) | ((a)<<2) | ((a)))

#define CONV_GS_1_TO_32(a) (0xFF000000 | ((a)<<23) | ((a)<<22) | ((a)<<21) |\
						((a)<<20) | ((a)<<19) | ((a)<<18) | ((a)<<17) |\
						((a)<<16) |	((a)<<15) | ((a)<<14) | ((a)<<13) |\
						((a)<<12) |	((a)<<11) | ((a)<<10) | ((a)<<9) |\
						((a)<<8) | ((a)<<7) | ((a)<<6) | ((a)<<5) | ((a)<<4) |\
						((a)<<3) | ((a)<<2) | ((a)<<1) | ((a)))

/* Colour_type options */
#define COLOR_GREY       0
#define COLOR_TRUE       2
#define COLOR_INDEXED    3
#define COLOR_GREY_ALPHA 4
#define COLOR_TRUE_ALPHA 6

#define CONV_GS_4_TO_32(a) (0xFF000000 | ((a)<<20) | ((a)<<16) | ((a)<<12) |\
						((a)<<8) | ((a)<<4) | ((a)))

#define CONV_GS_2_TO_32(a) (0xFF000000 | ((a)<<22) | ((a)<<20) | ((a)<<18) |\
						((a)<<16) | ((a)<<14) | ((a)<<12) | ((a)<<10) |\
						((a)<<8) | ((a)<<6) | ((a)<<4) | ((a)<<2) | ((a)))

#define CONV_GS_1_TO_32(a) (0xFF000000 | ((a)<<23) | ((a)<<22) | ((a)<<21) |\
						((a)<<20) |	((a)<<19) | ((a)<<18) | ((a)<<17) |\
						((a)<<16) |	((a)<<15) | ((a)<<14) | ((a)<<13) |\
						((a)<<12) |	((a)<<11) | ((a)<<10) | ((a)<<9) |\
						((a)<<8) | ((a)<<7) | ((a)<<6) | ((a)<<5) | ((a)<<4) |\
						((a)<<3) | ((a)<<2) | ((a)<<1) | ((a)))

#define PNG_HEADER_SIZE                   8
#define PNG_CRC_SIZE                      4
#define CLC_MAX_BITS                      7
#define CLC_NUM_CODES                    19
#define LEN_MAX_BITS                     15
#define LEN_NUM_CODES                   288
#define DIST_MAX_BITS                    15
#define DIST_NUM_CODES                   32
#define LEN_NUM_DISTINCT_EXTRA_BITS       7
#define DIST_NUM_DISTINCT_EXTRA_BITS     14
#define LEN_START_REAL_VALUES           257
#define DIST_START_REAL_VALUES            1
#define DISPLAY_START                  8365
#define DISPLAY_MAX                    8372
#define DISPLAY_MAX2                   8372

/* Chunk types */
#define CHUNK_IHDR 0x49484452
#define CHUNK_SRGB 0x73524742
#define CHUNK_PHYS 0x70485973
#define CHUNK_TIME 0x74494D45
#define CHUNK_BKGD 0x624B4744
#define CHUNK_TRNS 0x74524E53
#define CHUNK_CHRM 0x6348524D
#define CHUNK_GAMA 0x67414D41
#define CHUNK_ICCP 0x69434350
#define CHUNK_SBIT 0x73424954
#define CHUNK_TEXT 0x74455874
#define CHUNK_ZTXT 0x7A545874
#define CHUNK_ITXT 0x69545874
#define CHUNK_HIST 0x68495354
#define CHUNK_SPLT 0x73504C54
#define CHUNK_PLTE 0x504C5445
#define CHUNK_IDAT 0x49444154
#define CHUNK_IEND 0x49454E44

/* APNG Chunks */
#define CHUNK_ACTL 0x6163544C
#define CHUNK_FCTL 0x6663544C
#define CHUNK_FDAT 0x66644154

/* Colour_type options */
#define COLOR_GREY       0
#define COLOR_TRUE       2
#define COLOR_INDEXED    3
#define COLOR_GREY_ALPHA 4
#define COLOR_TRUE_ALPHA 6

/* APNG dispose_op codes */
#define APNG_DISPOSE_OP_NONE       0
#define APNG_DISPOSE_OP_BACKGROUND 1
#define APNG_DISPOSE_OP_PREVIOUS   2

/* APNG blend_op codes */
#define APNG_BLEND_OP_SOURCE 0
#define APNG_BLEND_OP_OVER   1

typedef struct _bitmap_header {
	/* What is the widht and height of the bitmap */
	unsigned short width;
	unsigned short height;
	/* If Negative, from bottom right, how much to go left by */
	/* If Positive, from top left, how much to go right by */
	short x_coord;
	/* If Negative, from bottom right, how much to go up by */
	/* If Positive, from top left, how much to go down by */
	short y_coord;
} bitmap_header;

typedef struct _png_header {
	unsigned long width;
	unsigned long height;
	unsigned long x_offset;
	unsigned long y_offset;
	unsigned char bit_depth;
	unsigned char colour_type;
	unsigned char compression_method;
	unsigned char filter_method;
	unsigned char interlace_method;
	unsigned long bpp;
	unsigned long bytes_pp;
	unsigned long bytes_pl;
	unsigned long background;
	unsigned char background_r;
	unsigned char background_g;
	unsigned char background_b;
	unsigned long *image_palette;
	unsigned long using_transparency;
	unsigned short transparency_r;
	unsigned short transparency_g;
	unsigned short transparency_b;
} png_header;

typedef struct _png_frame {
	unsigned long *output;
	unsigned long size;
	unsigned long width;
	unsigned long height;
	unsigned long x_offset;
	unsigned long y_offset;
	unsigned long bytes_pp;
	unsigned long bytes_pl;
	unsigned long delay;
	unsigned char dispose_op;
	unsigned char blend_op;
} png_frame;

typedef struct _huffman_node {
	unsigned long value;
	unsigned long real;
	unsigned char extra_bits;
	struct huffman_node *leaf[2];
} huffman_node;

void display_png_frame(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	png_header image_header,
	png_frame *frame,
	unsigned long prev_dispose_op);
void decode_png_data(
	png_header *image_header,
	unsigned char *input_data,
	png_frame *frame);
int create_tree(
    unsigned long max_bits,
    unsigned long num_codes,
    unsigned long *code_lengths,
    unsigned long *extra_bits,
    unsigned long *values,
    unsigned long *real_values,
    huffman_node **tree);
void free_node(huffman_node *node);
void display_splash_screen(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	emgd_drm_splash_screen_t *ss_data);
void display_bmp_splash_screen(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	emgd_drm_splash_screen_t *ss_data);
void display_png_splash_screen(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	emgd_drm_splash_screen_t *ss_data);
void decompress_huffman(
    unsigned char *stream,
    unsigned long *iter,
    unsigned char *bit_iter,
    huffman_node **length_tree,
    huffman_node **distance_tree,
	unsigned char *output,
	unsigned long *output_iter);
void build_static_huffman_tree(
    huffman_node **length_tree,
    huffman_node **distance_tree);
void build_dynamic_huffman_tree(
    unsigned char *stream,
    unsigned long *iter,
    unsigned char *bit_iter,
    huffman_node **length_tree,
    huffman_node **distance_tree);
void get_code_lengths(
    unsigned char *stream,
    unsigned long *iter,
    unsigned char *bit_iter,
    huffman_node **code_length_tree,
    unsigned long num_lengths,
    unsigned long *dynamic_lengths);
void get_huffman_code(
    unsigned char *stream,
    unsigned long *iter,
    unsigned char *bit_iter,
    huffman_node **tree,
    huffman_node **final_node);
int add_node(
    huffman_node **tree,
    huffman_node *node,
    unsigned long code,
    unsigned long code_length);
int read_int_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned long *value);
int read_short_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned short *value);
int read_char_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *value);
int read_bits_from_stream(
    unsigned char *stream,
    unsigned long *iter,
    unsigned char *bit_iter,
    unsigned long num_bits,
    unsigned long *value);
unsigned int read_bit_from_stream(
    unsigned char *stream,
    unsigned long *iter,
    unsigned char *bit_iter);

#endif

