/*
 *-----------------------------------------------------------------------------
 * Filename: state3d.h
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
 *  This file is an inter-module header file for manipulating the 3D
 *  State variables.
 *-----------------------------------------------------------------------------
 */
/*
 * This is referenced from Napa code. For Poulsbo, the HW spec may be
 * different. Be ready for the new change.
 */

#ifndef _STATE3D_H
#define _STATE3D_H

#include <plb/appcontext.h>

/* The dispatch table */
typedef struct _state3d_dispatch_plb_t {
	int lsi1_s2_needs_s3;     /* Does LSI1 S2 also require S3? */
	int lsi2_s3_830_845;
} state3d_dispatch_plb_t;

/* The array of buffer infos are used for these purposes */
#define COLOR_BUFFER_INDEX 0
#define DEPTH_BUFFER_INDEX 1
#define AUX0_BUFFER_INDEX  2
#define AUX1_BUFFER_INDEX  3
#define INTRA_BUFFER_INDEX 4

typedef struct _state3d_buffer_info_plb {
	union {
		struct {
			unsigned long pitch :14;
			unsigned long :7;
			unsigned long tile_walk :1;
			unsigned long tiled :1;
			unsigned long fenced :1;
			unsigned long buffer_id :4;
			unsigned long aux_id :1;
			unsigned long :3;
		};
		unsigned long dw1;
	};
	union {
		unsigned long base;
		unsigned long dw2;
	};
	unsigned long mod;
} state3d_buffer_info_plb_t;

typedef struct _state3d_dest_buffer_vars_plb {
	union {
		struct {
			unsigned long vert_offset  :1;
			unsigned long vert_stride  :1;
			unsigned long depth_format :3;
			unsigned long :1;
			unsigned long depth_component :1;
			unsigned long :1;
			unsigned long color_format :4;
			unsigned long write_select :3;
			unsigned long :1;
			unsigned long vert_bias  :4;
			unsigned long horiz_bias :4;
			unsigned long dither_disable :1;
			unsigned long gamma_blend_enable :1;
			unsigned long dither_patten :2;
			unsigned long :4;
		};
		unsigned long dw1;
	};
	unsigned long mod;
} state3d_dest_buffer_vars_plb_t;

typedef struct _state3d_vertex_buffer {
	unsigned long addr;
	unsigned long width;
	unsigned long pitch;
	unsigned long enable;
	unsigned long mod;
} state3d_vertex_buffer_t;

#define VERTEX_BUFFER_ADDR_MODIFIED    0x1
#define VERTEX_BUFFER_WIDTH_MODIFIED   0x2
#define VERTEX_BUFFER_PITCH_MODIFIED   0x4
#define VERTEX_BUFFER_ENABLE_MODIFIED  0x8

typedef struct _state3d_texture_coord_set {
	unsigned long format;
	unsigned long enable;
	unsigned long mod;
} state3d_texture_coord_set_t;

typedef struct _state3d_coord_set {
	unsigned long transform_enable;
	unsigned long normalized_coords;
	unsigned long source;
	unsigned long type;
	unsigned long addr_v_control_mode;
	unsigned long addr_u_control_mode;
	unsigned long mod;
} state3d_coord_set_t;

typedef struct _state3d_texel_stream {
	unsigned long modification_enable;
	unsigned long modifier_unit_index;
	unsigned long coord_select;
	unsigned long map_select;
	unsigned long mod;
} state3d_texel_stream_t;

#define MODIFICATION_ENABLE_MODIFIED 0x1
#define MODIFIER_UNIT_INDEX_MODIFIED 0x2
#define COORD_SELECT_MODIFIED        0x4
#define MAP_SELECT_MODIFIED          0x8

typedef struct _state3d_texel_modifier {
	unsigned long texel_stream_index;
	unsigned long bump_param_table_index;
	unsigned long enable;
	unsigned long mod;
} state3d_texel_modifier_t;

#define TEXEL_STREAM_INDEX_MODIFIED     0x1
#define BUMP_PARAM_TABLE_INDEX_MODIFIED 0x2
#define ENABLE_MODIFIED                 0x4

/*
 * These can be changed with the LOAD_STATE_IMMEDIATE_1 S3 Instruction.
 */
typedef struct _state3d_imm1_state3 {
	unsigned long point_width;
	unsigned long line_width;
	unsigned long alpha_shade_mode;
	unsigned long fog_shade_mode;
	unsigned long specular_shade_mode;
	unsigned long color_shade_mode;
	unsigned long cull_mode;
	unsigned long point_width_present;
	unsigned long specular_color_present; /* And Fog Factor */
	unsigned long diffuse_color_present;
	unsigned long depth_offset_present;
	unsigned long position_mask;
	unsigned long specular_add_enable;
	unsigned long fog_enable;
	unsigned long local_depth_bias_enable;
	unsigned long sprite_point_enable;
	unsigned long antialiasing_enable;
	unsigned long mod;
} state3d_imm1_state3_t;

#define POINT_WIDTH_MODIFIED             0x1
#define LINE_WIDTH_MODIFIED              0x2
#define ALPHA_SHADE_MODE_MODIFIED        0x4
#define FOG_SHADE_MODE_MODIFIED          0x8
#define SPECULAR_SHADE_MODE_MODIFIED     0x10
#define COLOR_SHADE_MODE_MODIFIED        0x20
#define CULL_MODE_MODIFIED               0x40
#define POINT_WIDTH_PRESENT_MODIFIED     0x80
#define SPECULAR_COLOR_PRESENT_MODIFIED  0x100
#define DIFFUSE_COLOR_PRESENT_MODIFIED   0x200
#define DEPTH_OFFSET_PRESENT_MODIFIED    0x400
#define POSITION_MASK_MODIFIED           0x800
#define SPECULAR_ADD_ENABLE_MODIFIED     0x1000
#define FOG_ENABLE_MODIFIED              0x2000
#define LOCAL_DEPTH_BIAS_ENABLE_MODIFIED 0x4000
#define SPRITE_POINT_ENABLE_MODIFIED     0x8000
#define ANTIALAISING_ENABLE_MODIFIED     0x10000

/*
 * These can be changed with the LOAD_STATE_IMMEDIATE_1 S7 Instruction.
 */
typedef struct _state3d_imm1_state7 {
	unsigned long global_depth_bias;
	unsigned long stencil_reference_value;
	unsigned long stencil_test_function;
	unsigned long stencil_fail_op;
	unsigned long stencil_pass_depth_fail_op;
	unsigned long stencil_pass_depth_pass_op;
	unsigned long stencil_buffer_write_enable;
	unsigned long stencil_test_enable;
	unsigned long color_dither_enable;
	unsigned long logic_op_enable;
	unsigned long mod;
} state3d_imm1_state7_t;

#define GLOBAL_DEPTH_BIAS_MODIFIED           0x1
#define STENCIL_REFERENCE_VALUE_MODIFIED     0x2
#define STENCIL_TEST_FUNCTION_MODIFIED       0x4
#define STENCIL_FAIL_OP_MODIFIED             0x8
#define STENCIL_PASS_DEPTH_FAIL_OP_MODIFIED  0x10
#define STENCIL_PASS_DEPTH_PASS_OP_MODIFIED  0x20
#define STENCIL_BUFFER_WRITE_ENABLE_MODIFIED 0x40
#define STENCIL_TEST_ENABLE_MODIFIED         0x80
#define COLOR_DITHER_ENABLE_MODIFIED         0x100
#define LOGIC_OP_ENABLE_MODIFIED             0x200

/*
 * These can be changed with the LOAD_STATE_IMMEDIATE_1 S8 Instruction.
 */
typedef struct _state3d_imm1_state8 {
	unsigned long alpha_test_enable;
	unsigned long alpha_test_function;
	unsigned long alpha_reference_value;
	unsigned long depth_test_enable;
	unsigned long depth_test_function;
	unsigned long color_buffer_blend_enable;
	unsigned long color_blend_function;
	unsigned long source_blend_factor;
	unsigned long destination_blend_factor;
	unsigned long depth_buffer_write_enable;
	unsigned long color_buffer_write_enable;
	unsigned long triangle_provoking_vertex_select;
} state3d_imm1_state8_t;

typedef struct _state3d_imm2_tms0 {
	unsigned long tm_map_base_addr;
	unsigned long tm_utilize_fence_regs;
	unsigned long reverse_gamma_enable;
} state3d_imm2_tms0_t;

typedef struct _state3d_imm2_tms1 {
	unsigned long tm_height;
	unsigned long tm_width;
	unsigned long tm_palette_select;
	unsigned long tm_surface_format;
	unsigned long tm_texel_format;
	unsigned long tm_color_space_conversion_enable;
	unsigned long tm_tiled_surface;
	unsigned long tm_tile_walk;
} state3d_imm2_tms1_t;

typedef struct _state3d_imm2_tms2 {
	unsigned long tm_dword_pitch;
	unsigned long tm_cube_face_enables;
	unsigned long tm_map_format;
	unsigned long tm_vertical_line_stride;
	unsigned long tm_vertical_line_stride_offset;
	unsigned long tm_output_channel_selection;
	unsigned long tm_base_mip_level;
	unsigned long tm_lod_preclamp_enable;
} state3d_imm2_tms2_t;

typedef struct _state3d_imm2_tms3 {
	unsigned long tm_mip_mode_filter;
	unsigned long tm_mag_mode_filter;
	unsigned long tm_min_mode_filter;
	unsigned long tm_texture_lod_bias;
	unsigned long tm_colorkey_enable;
	unsigned long tm_chromakey_enable;
	unsigned long tm_maximum_mip_level;
	unsigned long tm_minimum_mip_level;
	unsigned long tm_kill_pixel_enable;
	unsigned long tm_keyed_texture_filter_mode;
} state3d_imm2_tms3_t;

typedef struct _state3d_imm2_tms4 {
	unsigned long tm_default_color;
} state3d_imm2_tms4_t;

/*
 * All pointers may be NULL. If so defaults should be assumed.
 */
typedef struct _state3d {
	igd_surface_t color_buffer;
	igd_surface_t depth_buffer;
	state3d_vertex_buffer_t vertex_buffer[2];
	state3d_texture_coord_set_t texture_coord_set[8];
	unsigned long texture_coord_count;
	state3d_imm1_state3_t imm1_s3;
	state3d_coord_set_t coord_set[4];
	state3d_texel_modifier_t texel_modifier[2];
	state3d_texel_stream_t texel_stream[4];
	state3d_imm1_state7_t imm1_s7;
	state3d_imm1_state8_t imm1_s8;
	state3d_imm2_tms0_t imm2_tms0[4];
	state3d_imm2_tms1_t imm2_tms1[4];
	state3d_imm2_tms2_t imm2_tms2[4];
	state3d_imm2_tms3_t imm2_tms3[4];
	state3d_imm2_tms4_t imm2_tms4[4];
	state3d_buffer_info_plb_t buffer_info[5];
	state3d_dest_buffer_vars_plb_t buffer_vars;
	unsigned long mod;
	state3d_dispatch_plb_t *dispatch;
} state3d_t, state3d_plb_t;

#define IMM1_MODIFIED    0x1ff
#define IMM1_S0_MODIFIED 0x1
#define IMM1_S1_MODIFIED 0x2
#define IMM1_S2_MODIFIED 0x4
#define IMM1_S3_MODIFIED 0x8
#define IMM1_S4_MODIFIED 0x10
#define IMM1_S5_MODIFIED 0x20
#define IMM1_S6_MODIFIED 0x40
#define IMM1_S7_MODIFIED 0x80
#define IMM1_S8_MODIFIED 0x100
#define IMM2_MODIFIED     0xf0000
#define IMM2_TM0_MODIFIED 0x10000
#define IMM2_TM1_MODIFIED 0x20000
#define IMM2_TM2_MODIFIED 0x40000
#define IMM2_TM3_MODIFIED 0x80000

#define LOAD_S0 0x1
#define LOAD_S1 0x2
#define LOAD_S2 0x4
#define LOAD_S3 0x8
#define LOAD_S4 0x10
#define LOAD_S5 0x20
#define LOAD_S6 0x40
#define LOAD_S7 0x80
#define LOAD_S8 0x100
#define LOAD_TM  0xf0000
#define LOAD_TM0 0x10000
#define LOAD_TM1 0x20000
#define LOAD_TM2 0x40000
#define LOAD_TM3 0x80000

#define STATE3D_SET_ALPHA_TEST_ENABLE(s, v) \
    s->imm1_s8.alpha_test_enable = (v & 1); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_ALPHA_TEST_FUNCTION(s, v) \
    s->imm1_s8.alpha_test_function = (v & 7); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_ALPHA_REFERENCE_VALUE(s, v) \
    s->imm1_s8.alpha_reference_value = (v & 0xf); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_DEPTH_TEST_ENABLE(s, v) \
    s->imm1_s8.depth_test_enable = (v & 1); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_DEPTH_TEST_FUNCTION(s, v) \
    s->imm1_s8.depth_test_function = (v & 7); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_COLOR_BUFFER_BLEND_ENABLE(s, v) \
    s->imm1_s8.color_buffer_blend_enable = (v & 1); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_COLOR_BLEND_FUNCTION(s, v) \
    s->imm1_s8.color_blend_function = (v & 7); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_SOURCE_BLEND_FACTOR(s, v) \
    s->imm1_s8.source_blend_factor = (v & 0xf); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_DESTINATION_BLEND_FACTOR(s, v) \
    s->imm1_s8.destination_blend_factor = (v & 0xf); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_DEPTH_BUFFER_WRITE_ENABLE(s, v) \
    s->imm1_s8.depth_buffer_write_enable = (v & 1); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_COLOR_BUFFER_WRITE_ENABLE(s, v) \
    s->imm1_s8.color_buffer_write_enable = (v & 1); \
    s->mod |= IMM1_S8_MODIFIED;

#define STATE3D_SET_TRIANGLE_PROVOKING_VERTEX_SELECT(s, v) \
    s->imm1_s8.triangle_provoking_vertex_select = (v & 3); \
    s->mod |= IMM1_S8_MODIFIED;


#define STATE3D(ac) ((state3d_t *)((appcontext_plb_t *)ac)->state3d)

int state3d_update_plb(igd_command_t **in, appcontext_t *appcontext);
int state3d_update_size(appcontext_t *appcontext);

#endif

