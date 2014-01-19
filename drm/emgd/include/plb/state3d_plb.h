/*
 *-----------------------------------------------------------------------------
 * Filename: state3d_plb.h
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

#ifndef _STATE3D_PLB_H
#define _STATE3D_PLB_H

#include <plb/appcontext.h>

typedef struct _state3d_anti_aliasing_plb {
	union {
		struct {
			unsigned int :6;
			unsigned int line_aa_region_width :2;
			unsigned int line_aa_region_width_mod_en :1;
			unsigned int :5;
			unsigned int line_end_cap_aa_region_width :2;
			unsigned int line_end_cap_aa_region_width_mod_en :1;
			unsigned int :15;
		};
		unsigned int dw0;
	};
	unsigned int updated;

} state3d_anti_aliasing_plb_t;

#define STATE3D_ANTI_ALIASING_UPDATED_PLB	1
#define STATE3D_ANTI_ALIASING_DWORD_COUNT_PLB 	1

typedef struct _state3d_backface_stencil_ops_plb {
	union {
		struct {
			unsigned int double_sided_en :1;
			unsigned int double_sided_en_mod_en :1;
			unsigned int pass_depth_pass_op :3;
			unsigned int pass_depth_fail_op :3;
			unsigned int fail_op :3;
			unsigned int test_function :3;
			unsigned int multiple_mod_en :1;
			unsigned int ref_val :8;
			unsigned int ref_val_mod_en :1;
			unsigned int :8;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_backface_stencil_ops_plb_t;
#define STATE3D_BACKFACE_STENCIL_OPS_UPDATED_PLB	1
#define STATE3D_BACKFACE_STENCIL_OPS_DWORD_COUNT_PLB	1

typedef struct _state3d_backface_stencil_masks_plb {
	union {
		struct {
			unsigned int write_mask :8;
			unsigned int test_mask :8;
			unsigned int write_mask_mod_en :1;
			unsigned int test_mask_mod_en :1;
			unsigned int :14;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_backface_stencil_masks_plb_t;
#define STATE3D_BACKFACE_STENCIL_MASK_UPDATED_PLB	1
#define STATE3D_BACKFACE_STENCIL_MASK_DWORD_COUNT_PLB	1

/* FIXME: Details are ignored in this struct */
typedef struct _state3d_bin_control_plb {
	unsigned int dw0;
	unsigned int dw1;
	unsigned int dw2;
	unsigned int dw3;
	unsigned int dw4;
	unsigned int dw5;
	unsigned int updated;
} state3d_bin_control_plb_t;
#define STATE3D_BIN_CONTROL_UPDATED_PLB		1
#define STATE3D_BIN_CONTROL_DWORD_COUNT_PLB	6

typedef struct _state3d_buffer_info_plb {
	unsigned int dw0;
	union {
		struct {
			unsigned int :2;
			unsigned int buffer_pitch :12;
			unsigned int :7;
			unsigned int tile_walk :1;
			unsigned int tiled_surface :1;
			unsigned int util_fence_regs :1;
			unsigned int buffer_id :4;
			unsigned int aux_buffer_id :1;
			unsigned int :3;
		};
		unsigned int dw1;
	};
	union {
		struct {
			unsigned int :2;
			unsigned int buffer_base_addr :26;
		};
		unsigned int dw2;
	};
	unsigned int updated;
} state3d_buffer_info_plb_t;
#define STATE3D_BUFFER_INFO_UPDATED_PLB		1
#define STATE3D_BUFFER_INFO_DWORD_COUNT_PLB	3
#define STATE3D_BUFFER_INFO_BUFFERID_COLOR_BACK			0x03
#define STATE3D_BUFFER_INFO_BUFFERID_COLOR_AUX			0x04
#define STATE3D_BUFFER_INFO_BUFFERID_COLOR_MC_INTRA_CORR	0x05
#define STATE3D_BUFFER_INFO_BUFFERID_DEPTH			0x07

typedef struct _state3d_chroma_key_plb {
	union {
		struct {
			unsigned int :30;
			unsigned int table_index :2;
		};
		unsigned int dw1;
	};
	union {
		unsigned int lo_val;
		unsigned int dw2;
	};
	union {
		unsigned int hi_val;
		unsigned int dw3;
	};
	unsigned int updated;
} state3d_chroma_key_plb_t;
#define STATE3D_CHROMA_KEY_UPDATED_PLB		1
#define STATE3D_CHROMA_KEY_DWORD_COUNT_PLB	4

typedef struct _state3d_clear_parameters_plb {
	union {
		struct {
		unsigned int stencil_write_en :1;
		unsigned int depth_buffer_write_en :1;
		unsigned int color_buffer_write_en :1;
			unsigned int :13;
			unsigned int clear_primitive_type :1;
			unsigned int :15;
		};
		unsigned int dw1;
        };
	union {
		unsigned int	clear_color_buffer_val;
		unsigned int dw2;
	};
	union {
	unsigned int	clear_depth_buffer_val;
		unsigned int dw3;
	};
	union {
	unsigned int	clear_color;
		unsigned int dw4;
	};
	union {
	unsigned int	clear_depth;
		unsigned int dw5;
	};
	union {
		struct {
		unsigned int clear_stencil :8;
			unsigned int :24;
		};
		unsigned int dw6;
	};
	unsigned int updated;
} state3d_clear_parameters_plb_t;
#define STATE3D_CLEAR_PARAMETERS_UPDATED_PLB		1
#define STATE3D_CLEAR_PARAMETERS_DWORD_COUNT_PLB	7

typedef struct _state3d_constant_blend_color_plb {
	union {
		unsigned int color;
		unsigned int dw1;
	};
	unsigned int updated;
} state3d_constant_blend_color_plb_t;
#define STATE3D_CONSTANT_BLEND_COLOR_UPDATED_PLB	1
#define STATE3D_CONSTANT_BLEND_COLOR_DWORD_COUNT_PLB	2

typedef struct _state3d_coord_set_bindings_plb {
	union {
		struct {
			unsigned int internal_tc_set_0_src :3;
			unsigned int internal_tc_set_1_src :3;
			unsigned int internal_tc_set_2_src :3;
			unsigned int internal_tc_set_3_src :3;
			unsigned int internal_tc_set_4_src :3;
			unsigned int internal_tc_set_5_src :3;
			unsigned int internal_tc_set_6_src :3;
			unsigned int internal_tc_set_7_src :3;
			unsigned int :8;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_coord_set_bindings_plb_t;
#define STATE3D_COORD_SET_BINDINGS_UPDATED_PLB		1
#define STATE3D_COORD_SET_BINDINGS_DWORD_COUNT_PLB	1

typedef struct _state3d_default_diffuse_plb {
	union {
		unsigned int color;
		unsigned int dw1;
	};
	unsigned int updated;
} state3d_default_diffuse_plb_t;
#define STATE3D_DEFAULT_DIFFUSE_UPDATED_PLB		1
#define STATE3D_DEFAULT_DIFFUSE_DWORD_COUNT_PLB		2

typedef struct _state3d_default_specular_plb {
	union {
		unsigned int color;
		unsigned int dw1;
	};
	unsigned int updated;
} state3d_default_specular_plb_t;
#define STATE3D_DEFAULT_SPECULAR_UPDATED_PLB		1
#define STATE3D_DEFAULT_SPECULAR_DWORD_COUNT_PLB	2

typedef struct _state3d_default_z_plb {
	union {
		unsigned int depth;
		unsigned int dw1;
	};
	unsigned int updated;
} state3d_default_z_plb_t;
#define STATE3D_DEFAULT_Z_UPDATED_PLB			1
#define STATE3D_DEFAULT_Z_DWORD_COUNT_PLB		2

typedef struct _state3d_depth_offset_scale_plb {
	union {
		unsigned int global_val;
		unsigned int dw1;
	};
	unsigned int updated;
} state3d_depth_offset_scale_plb_t;
#define STATE3D_DEPTH_OFFSET_SCALE_UPDATED_PLB		1
#define STATE3D_DEPTH_OFFSET_SCALE_DWORD_COUNT_PLB	2

typedef struct _state3d_depth_subrectangle_enable_plb {
	union {
		struct {
			unsigned int enable :1;	/* B-spec says this is difeatured, MUST BE DISABLE */
			unsigned int mod_en :1;
			unsigned int :30;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_depth_subrectangle_enable_plb_t;
#define STATE3D_DEPTH_SUBRECTANGLE_ENABLE_UPDATED_PLB		1
#define STATE3D_DEPTH_SUBRECTANGLE_ENABLE_DWORD_COUNT_PLB	1

typedef struct _state3d_dest_buffer_variables_plb {
	union {
		struct {
			unsigned int vert_line_stride_offset :1;
			unsigned int vert_line_stride :1;
			unsigned int depth_buffer_format :2;
			unsigned int :4;
			unsigned int color_buffer_format :4;
			unsigned int write_select_422_channel :3;
			unsigned int :1;
			unsigned int dest_origin_vert_bias :4;
			unsigned int dest_origin_horiz_bias :4;
			unsigned int dither_enhance_dis :1;
			unsigned int linear_gamma_blend_en :1;
			unsigned int dither_pattern_select :2;
			unsigned int lod_preclamp_en :1;
			unsigned int early_depth_test_en :1;
			unsigned int texture_default_color_mode :1;
			unsigned int :1;
		};
		unsigned int dw1;
	};
	unsigned int updated;
} state3d_dest_buffer_variables_plb_t;
#define STATE3D_DEST_BUFFER_VARIABLES_UPDATED_PLB	      1
#define STATE3D_DEST_BUFFER_VARIABLES_DWORD_COUNT_PLB	  2

#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_8BIT          0
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_X1R5G6B5      1
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_R5G6B5        2
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_A8R8G8B8      3
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_YCRCB_SWAPY   4
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_YCRCB_NORMAL  5
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_YCRCB_SWAPUV  6
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_YCRCB_SWAPUVY 7
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_A4R4G4B4      8
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_A1R5G5B5      9
#define STATE3D_DEST_BUFF_COLOR_BUFF_FORMAT_A2R10G10B10   0xA

typedef struct _state3d_drawing_rectangle_plb {
	union {
		struct {
			unsigned int :24;
		unsigned int y_dither_offset :2;
		unsigned int x_dither_offset :2;
			unsigned int :2;
		unsigned int depth_buffer_coord_offset_dis :1;
		unsigned int fast_scissor_clip_dis :1;
		};
		unsigned int dw1;
	};
	union {
		struct {
		unsigned int clip_draw_rect_x_min :16;
		unsigned int clip_draw_rect_y_min :16;
		};
		unsigned int dw2;
	};
	union {
		struct {
		unsigned int clip_draw_rect_x_max :16;
		unsigned int clip_draw_rect_y_max :16;
		};
		unsigned int dw3;
	};
	union {
		struct {
		unsigned int draw_rect_origin_x :16;
		unsigned int draw_rect_origin_y :16;
		};
		unsigned int dw4;
	};
	unsigned int updated;
} state3d_drawing_rectangle_plb_t;
#define STATE3D_DRAWING_RECTANGLE_UPDATED_PLB		1
#define STATE3D_DRAWING_RECTANGLE_DWORD_COUNT_PLB	5

typedef struct _state3d_filter_coefficients_4x4_plb {
	/* FIXME: This might be an over simplified abstraction
	 * and may subject to change to meet its semantic later */
	unsigned int filter1[32];
	unsigned int filter2[32];
	unsigned int updated;
} state3d_filter_coefficients_4x4_plb_t;
#define STATE3D_FILTER_COEFFICIENT_4X4_UPDATED_PLB	1
#define STATE3D_FILTER_COEFFICIENT_4X4_DWORD_COUNT_PLB	65

typedef struct _state3d_filter_coefficients_6x5_plb {
	union {
		struct {
			unsigned int k1 :16;
			unsigned int k2 :16;
		};
		unsigned int dw1;
	};
	union {
		struct {
			unsigned int k3 :16;
			unsigned int k4 :16;
		};
		unsigned int dw2;
	};
	union {
		struct {
			unsigned int k5 :16;
			unsigned int k6 :16;
		};
		unsigned int dw3;
	};
	unsigned int updated;
} state3d_filter_coefficients_6x5_plb_t;
#define STATE3D_FILTER_COEFFICIENT_6X5_UPDATED_PLB	1
#define STATE3D_FILTER_COEFFICIENT_6X5_DWORD_COUNT_PLB	4

typedef struct _state3d_fog_color_plb {
	union {
		struct {
			unsigned int blue :8;
			unsigned int green :8;
			unsigned int red :8;
			unsigned int :8;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_fog_color_plb_t;
#define STATE3D_FOG_COLOR_UPDATED_PLB		1
#define STATE3D_FOG_COLOR_DWORD_COUNT_PLB	1

typedef struct _state3d_fog_mode_plb {
	union {
		struct {
			unsigned int :4;
			unsigned int linear_fog_c1_const:16;
			unsigned int :3;
			unsigned int fog_density_mod_en: 1;
			unsigned int linear_fog_c1c2_const_mod_en :1;
			unsigned int fog_source :1;
			unsigned int :1;
			unsigned int fog_source_mod_en :1;
			unsigned int fog_func :2;
			unsigned int :1;
			unsigned int fog_func_mod_en :1;
		};
		unsigned int dw1;
	};
	union {
		unsigned int linear_fog_c2_const;
		unsigned int dw2;
	};
	union {
		unsigned int fog_density;
		unsigned int dw3;
	};
	unsigned int updated;
} state3d_fog_mode_plb_t;

#define STATE3D_FOG_MODE_UPDATED_PLB		1
#define STATE3D_FOG_MODE_DWORD_COUNT_PLB	4

typedef struct _state3d_independent_alpha_blend_plb {
	union {
		struct {
			unsigned int dest_factor :4;
			unsigned int :1;
			unsigned int dest_factor_mod_en :1;
			unsigned int src_factor :4;
			unsigned int :1;
			unsigned int src_factor_mod_en :1;
			unsigned int :4;
			unsigned int function :3;
			unsigned int :2;
			unsigned int function_mod_en :1;
			unsigned int enable :1;
			unsigned int enable_mod_en :1;
			unsigned int :8;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_independent_alpha_blend_plb_t;
#define STATE3D_INDEPENDENT_ALPHA_BLEND_UPDATED_PLB	1
#define STATE3D_INDEPENDENT_ALPHA_BLEND_DWORD_COUNT_PLB	1

typedef struct _state3d_load_indirect_plb {
	union {
		struct {
			unsigned int block_num :8;
			unsigned int block_mask :6;
			unsigned int mem_space_select :1;
			unsigned int :17;
		};
		unsigned int dw0;
	};
	union {
		struct {
			unsigned int static_buffer_valid :1;
			unsigned int force_static_load :1;
			unsigned int static_buffer_addr :30;
		};
		unsigned int dw_sis0;
	};
	union {
		struct {
			unsigned int static_buffer_length :9;
			unsigned int :23;
		};
		unsigned int dw_sis1;
	};
	union {
		struct {
			unsigned int dynamic_buffer_valid :1;
			unsigned int dynamic_buffer_reset :1;
			unsigned int dynamic_buffer_addr :30;
		};
		unsigned int dw_dis0;
	};
	union {
		struct {
			unsigned int sampler_buffer_valid :1;
			unsigned int force_sampler_load :1;
			unsigned int sampler_buffer_addr :30;
		};
		unsigned int dw_ssb0;
	};
	union {
		struct {
			unsigned int sampler_buffer_length :9;
			unsigned int :23;
		};
		unsigned int dw_ssb1;
	};
	union {
		struct {
			unsigned int map_buffer_valid :1;
			unsigned int force_map_load :1;
			unsigned int map_buffer_addr :30;
		};
		unsigned int dw_msb0;
	};
	union {
		struct {
			unsigned int map_buffer_length :9;
			unsigned int :23;
		};
		unsigned int dw_msb1;
	};
	union {
		struct {
			unsigned int psp_buffer_valid :1;
			unsigned int force_psp_load :1;
			unsigned int psp_buffer_addr :30;
		};
		unsigned int dw_psp0;
	};
	union {
		struct {
			unsigned int psp_buffer_length :9;
			unsigned int :23;
		};
		unsigned int dw_psp1;
	};
	union {
		struct {
			unsigned int psc_buffer_valid :1;
			unsigned int force_psc_load :1;
			unsigned int psc_buffer_addr :30;
		};
		unsigned int dw_psc0;
	};
	union {
		struct {
			unsigned int psc_buffer_length :9;
			unsigned int :23;
		};
		unsigned int dw_psc1;
	};
	unsigned int updated;
} state3d_load_indirect_plb_t;
#define STATE3D_LOAD_INDIRECT_UPDATED_PLB		1
#define STATE3D_LOAD_INDIRECT_DWORD_COUNT_PLB		12
#define STATE3D_LOAD_INDIRECT_HEADER_DWORD_COUNT_PLB	1
#define STATE3D_LOAD_INDIRECT_PSP_DWORD_COUNT_PLB	2

typedef struct _state3d_load_state_immediate_1_plb {
	union {
		struct {
			unsigned int state_num :4;
			unsigned int load_dw_s0 :1;
			unsigned int load_dw_s1 :1;
			unsigned int load_dw_s2 :1;
			unsigned int load_dw_s3 :1;
			unsigned int load_dw_s4 :1;
			unsigned int load_dw_s5 :1;
			unsigned int load_dw_s6 :1;
			unsigned int load_dw_s7 :1;
			unsigned int :20;
		};
		unsigned int dw0;
	};
	union {
		struct {
			unsigned int vtx_cache_invalid_dis :1;
			unsigned int :1;
			unsigned int vtx_buf_addr :26;
			unsigned int :4;
		};
		unsigned int dw_s0;
	};
	union {
		struct {
			unsigned int :16;
			unsigned int vtx_buf_pitch :6;
			unsigned int :2;
			unsigned int vtx_buf_width :6;
			unsigned int :2;
		};
		unsigned int dw_s1;
	};
	union {
		/*
		struct {
			unsigned int :4;
		} tex_coord_set_fmt[8];
		*/
		struct {
			unsigned int tex_coord_set_0_fmt :4;
			unsigned int tex_coord_set_1_fmt :4;
			unsigned int tex_coord_set_2_fmt :4;
			unsigned int tex_coord_set_3_fmt :4;
			unsigned int tex_coord_set_4_fmt :4;
			unsigned int tex_coord_set_5_fmt :4;
			unsigned int tex_coord_set_6_fmt :4;
			unsigned int tex_coord_set_7_fmt :4;
		};
		unsigned int dw_s2;
	};
	union {
		/*
		struct {
			unsigned int pspec_crtn_dis :1;
			unsigned int wrap_short_tcz :1;
			unsigned int wrap_short_tcy :1;
			unsigned int wrap_short_tcx :1;
		} tex_coor_set[8];
		*/
		struct {
			unsigned int tex_coord_set_0_pspec_crtn_dis :1;
			unsigned int tex_coord_set_0_wrap_short_tcz :1;
			unsigned int tex_coord_set_0_wrap_short_tcy :1;
			unsigned int tex_coord_set_0_wrap_short_tcx :1;

			unsigned int tex_coord_set_1_pspec_crtn_dis :1;
			unsigned int tex_coord_set_1_wrap_short_tcz :1;
			unsigned int tex_coord_set_1_wrap_short_tcy :1;
			unsigned int tex_coord_set_1_wrap_short_tcx :1;

			unsigned int tex_coord_set_2_pspec_crtn_dis :1;
			unsigned int tex_coord_set_2_wrap_short_tcz :1;
			unsigned int tex_coord_set_2_wrap_short_tcy :1;
			unsigned int tex_coord_set_2_wrap_short_tcx :1;

			unsigned int tex_coord_set_3_pspec_crtn_dis :1;
			unsigned int tex_coord_set_3_wrap_short_tcz :1;
			unsigned int tex_coord_set_3_wrap_short_tcy :1;
			unsigned int tex_coord_set_3_wrap_short_tcx :1;

			unsigned int tex_coord_set_4_pspec_crtn_dis :1;
			unsigned int tex_coord_set_4_wrap_short_tcz :1;
			unsigned int tex_coord_set_4_wrap_short_tcy :1;
			unsigned int tex_coord_set_4_wrap_short_tcx :1;

			unsigned int tex_coord_set_5_pspec_crtn_dis :1;
			unsigned int tex_coord_set_5_wrap_short_tcz :1;
			unsigned int tex_coord_set_5_wrap_short_tcy :1;
			unsigned int tex_coord_set_5_wrap_short_tcx :1;

			unsigned int tex_coord_set_6_pspec_crtn_dis :1;
			unsigned int tex_coord_set_6_wrap_short_tcz :1;
			unsigned int tex_coord_set_6_wrap_short_tcy :1;
			unsigned int tex_coord_set_6_wrap_short_tcx :1;

			unsigned int tex_coord_set_7_pspec_crtn_dis :1;
			unsigned int tex_coord_set_7_wrap_short_tcz :1;
			unsigned int tex_coord_set_7_wrap_short_tcy :1;
			unsigned int tex_coord_set_7_wrap_short_tcx :1;

		};
		unsigned int dw_s3;
	};
	union {
		struct {
			unsigned int anti_alias_en :1;
			unsigned int sprite_point_en :1;
			unsigned int fog_param_present :1;
			unsigned int local_depth_offset_en :1;
			unsigned int force_specular_diffuse_color :1;
			unsigned int force_default_diffuse_color :1;
			unsigned int position_mask :3;
			unsigned int local_depth_offset_present: 1;
			unsigned int diffuse_color_present :1;
			unsigned int specular_color_fog_factor_present :1;
			unsigned int point_width_present :1;
			unsigned int cull_mode :2;
			unsigned int color_shade_mode :1;
			unsigned int specular_shade_mode :1;
			unsigned int fog_shade_mode :1;
			unsigned int alpha_shade_mode :1;
			unsigned int line_width :4;
			unsigned int point_width :9;
		};
		unsigned int dw_s4;
	};
	union {
		struct {
			unsigned int logic_op_en :1;
			unsigned int color_dither_en :1;
			unsigned int stencil_test_en :1;
			unsigned int stencil_buffer_write_en :1;
			unsigned int stencil_pass_depth_pass_op :3;
			unsigned int stencil_pass_depth_fail_op :3;
			unsigned int stencil_fail_op :3;
			unsigned int stencil_test_func :3;
			unsigned int stencil_ref_val :8;
			unsigned int fog_enable :1;
			unsigned int global_depth_offset_en :1;
			unsigned int last_pixel_en :1;
			unsigned int force_default_point_width :1;
			unsigned int color_buffer_component_write_dis :4;
		};
		unsigned int dw_s5;
	};
	union {
		struct {
			unsigned int triang_list_provoke_vtx_sel :2;
			unsigned int color_buffer_write_en :1;
			unsigned int depth_buffer_write_en :1;
			unsigned int dest_blend_factor :4;
			unsigned int src_blend_factor :4;
			unsigned int color_blend_func :3;
			unsigned int color_buffer_blend_en :1;
			unsigned int depth_test_func :3;
			unsigned int depth_test_en :1;
			unsigned int alpha_ref_val :8;
			unsigned int alpha_test_func :3;
			unsigned int alpha_test_en :1;
		};
		unsigned int dw_s6;
	};
	union {
		unsigned int global_depth_offset_const;
		unsigned int dw_s7;
	};
	unsigned int updated;
} state3d_load_state_immediate_1_plb_t;
/* FIXME: Need to refine to smaller enable flags */
#define STATE3D_LOAD_STATE_IMMEDIATE_1_UPDATED_PLB			1
#define STATE3D_LOAD_STATE_IMMEDIATE_1_HEADER_DWORD_COUNT_PLB		1
#define STATE3D_LOAD_STATE_IMMEDIATE_1_STATES_ALL_DWORD_COUNT_PLB	8
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_ZERO			1
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_ONE			2
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_SRC_ALPHA		5
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_INV_SRC_ALPHA	6
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_DST_ALPHA		7
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_INV_DST_ALPHA	8
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_CONST_ALPHA		0xe
#define STATE3D_LOAD_STATE_IMMEDIATE_1_BLENDFACTOR_INV_CONST_ALPHA	0xf

typedef struct _state3d_map_deinterlacer_parameters_plb {
	union {
		struct {
			unsigned int da_en_y :1;
			unsigned int da_en_uv :1;
			unsigned int :30;
		};
		unsigned int dw1;
	};
	union {
		struct {
			unsigned int edge_threshold_low_y :8;
			unsigned int edge_threshold_high_y :8;
			unsigned int edge_threshold_low_uv :8;
			unsigned int edge_threshold_high_uv :8;
		};
		unsigned int dw2;
	};
	unsigned int updated;
} state3d_map_deinterlacer_parameters_plb_t;
#define STATE3D_MAP_DEINTERLACER_PARAMETERS_UPDATED_PLB		1
#define STATE3D_MAP_DEINTERLACER_PARAMETERS_DWORD_COUNT_PLB	3

typedef struct _state3d_map_palette_load_32_plb {
	union {
		struct {
			unsigned int color_num :4;
			unsigned int :12;
			unsigned int :16;
		};
		unsigned int dw0;
	};
	unsigned int color[16];
	unsigned int updated;
} state3d_map_palette_load_32_plb_t;
#define STATE3D_MAP_PALETTE_LOAD_32_UPDATED_PLB		1
#define STATE3D_MAP_PALETTE_LOAD_32_DWORD_COUNT_PLB	17

typedef	struct _state3d_map_state_texmap_plb {
		union {
			struct {
				unsigned int vert_stride_offset :1;
				unsigned int vert_stride :1;
				unsigned int base_addr :26;
				unsigned int :3;
			};
			unsigned int tm_dw0;
		};
		union {
			struct {
				unsigned int tile_walk :1;
				unsigned int tiled_surface :1;
				unsigned int util_fence_regs :1;
				unsigned int texel_format :4;
				unsigned int surface_format :3;
				unsigned int width :11;
				unsigned int height :11;
			};
			unsigned int tm_dw1;
		};
		union {
			struct {
				unsigned int depth :8;
				unsigned int :1;
				unsigned int max_lod :6;
				unsigned int cube_face_en :6;
				unsigned int dword_pitch :11;
			};
			unsigned int tm_dw2;
		};
} state3d_map_state_texmap_plb_t;
#define STATE3D_MAP_STATE_MAPSURF_8BIT_PLB		1
  #define STATE3D_MAP_STATE_MAPSURF_8BIT_MAPTEXEL_I8_PLB		0
  #define STATE3D_MAP_STATE_MAPSURF_8BIT_MAPTEXEL_L8_PLB		1
  #define STATE3D_MAP_STATE_MAPSURF_8BIT_MAPTEXEL_A4P4_PLB		2
  #define STATE3D_MAP_STATE_MAPSURF_8BIT_MAPTEXEL_P4A4_PLB		3
  #define STATE3D_MAP_STATE_MAPSURF_8BIT_MAPTEXEL_A8_PLB		4
  #define STATE3D_MAP_STATE_MAPSURF_8BIT_MAPTEXEL_MONO8_PLB		5
#define STATE3D_MAP_STATE_MAPSURF_16BIT_PLB		2
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_R5G6B5_PLB		0
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_A1R5G5B5_PLB		1
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_A4R4G4B4_PLB		2
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_A8L8_PLB		3
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_V8U8_PLB		5
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_L6V5U5_PLB		6
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_I16_PLB		7
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_L16_PLB		8
  #define STATE3D_MAP_STATE_MAPSURF_16BIT_MAPTEXEL_A16_PLB		9
#define STATE3D_MAP_STATE_MAPSURF_32BIT_PLB		3
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_A8R8G8B8_PLB		0
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_A8B8G8R8_PLB		1
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_X8R8G8B8_PLB		2
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_X8B8G8R8_PLB		3
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_Q8W8V8U8_PLB		4
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_A8X8V8U8_PLB		5
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_L8X8V8U8_PLB		6
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_X8L8V8U8_PLB		7
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_A2R10G10B10_PLB	8
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_A2B10G10R10_PLB	9
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_A2W10V10U10_PLB	0xA
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_G16R16_PLB		0xB
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_V16U16_PLB		0xC
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_X8I24_PLB		0xD
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_X8L24_PLB		0xE
  #define STATE3D_MAP_STATE_MAPSURF_32BIT_MAPTEXEL_X8A24_PLB		0xF
  #define STATE3D_MAP_STATE_MAPSURF_422_PLB		5
  #define STATE3D_MAP_STATE_MAPSURF_422_MAPTEXEL_YCRCB_SWAPY_PLB	0
  #define STATE3D_MAP_STATE_MAPSURF_422_MAPTEXEL_YCRCB_NORMAL_PLB	1
  #define STATE3D_MAP_STATE_MAPSURF_422_MAPTEXEL_YCRCB_SWAPUV_PLB	2
  #define STATE3D_MAP_STATE_MAPSURF_422_MAPTEXEL_YCRCB_SWAPUVY_PLB	3
  #define STATE3D_MAP_STATE_MAPSURF_COMPRESSED_PLB	6
  #define STATE3D_MAP_STATE_MAPSURF_COMPRESSED_MAPTEXEL_DX1_PLB		0
  #define STATE3D_MAP_STATE_MAPSURF_COMPRESSED_MAPTEXEL_DX2_3_PLB	1
  #define STATE3D_MAP_STATE_MAPSURF_COMPRESSED_MAPTEXEL_DX4_5_PLB	2
  #define STATE3D_MAP_STATE_MAPSURF_COMPRESSED_MAPTEXEL_FX1_PLB		3
  #define STATE3D_MAP_STATE_MAPSURF_COMPRESSED_MAPTEXEL_DX1_RGB_PLB	4
#define STATE3D_MAP_STATE_MAPSURF_4BIT_INDEXED_PLB	7
  #define STATE3D_MAP_STATE_MAPSURF_4BIT_INDEXED_MAPTEXEL_A8R8G8B8_PLB	7
typedef struct _state3d_map_state_plb {
	union {
		struct {
			unsigned int map_num :6;
			unsigned int :9;
			unsigned int retain :1;
			unsigned int :16;
		};
		unsigned int dw0;
	};
	union {
		struct {
			unsigned int map_mask :16;
			unsigned int :16;
		};
		unsigned int dw1;
	};
	state3d_map_state_texmap_plb_t texmap[16]; /* Napa support 16 texture maps, B-spec-1a pg198 */
	unsigned int updated;
} state3d_map_state_plb_t;
/* FIXME: Need to refine to smaller enable flags */
#define STATE3D_MAP_STATE_UPDATED_PLB			1
#define STATE3D_MAP_STATE_MAX_DWORD_COUNT_PLB		50
#define STATE3D_MAP_STATE_HEADER_DWORD_COUNT_PLB	2
#define STATE3D_MAP_STATE_SINGLE_TM_DWORD_COUNT_PLB	3

typedef struct _state3d_modes_4_plb {
	union {
		struct {
			unsigned int stencil_write_mask :8;
			unsigned int stencil_test_mask :8;
			unsigned int stencil_write_mask_mod_en :1;
			unsigned int stencil_test_mask_mod_en :1;
			unsigned int logic_op_func :4;
			unsigned int :1;
			unsigned int logic_op_func_mod_en :1;
			unsigned int :8;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_modes_4_plb_t;
#define STATE3D_MODES_4_UPDATED_PLB		1
#define STATE3D_MODES_4_DWORD_COUNT_PLB		1

typedef struct _state3d_modes_5_plb {
	union {
		struct {
			unsigned int :16;
			unsigned int pipelined_tex_cache_op :1;
			unsigned int tex_cache_dis :1;
			unsigned int pipelined_render_cache_op :1;
			unsigned int :13;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_modes_5_plb_t;

#define STATE3D_MODES_5_UPDATED_PLB		1
#define STATE3D_MODES_5_DWORD_COUNT_PLB		1

typedef struct _state3d_pixel_shader_const_elem_plb {
	unsigned int const_x;
	unsigned int const_y;
	unsigned int const_z;
	unsigned int const_w;
} state3d_pixel_shader_const_elem_plb_t;

typedef struct _state3d_pixel_shader_constants_plb {
	union {
		struct {
			unsigned int const_num :8;
			unsigned int :24;
		};
		unsigned int dw0;
	};
	union {
		unsigned int reg_mask;
		unsigned int dw1;
	};
	state3d_pixel_shader_const_elem_plb_t elem[32];
	unsigned int updated;
} state3d_pixel_shader_constants_plb_t;
#define STATE3D_PIXEL_SHADER_CONSTANTS_UPDATED_PLB		1
#define STATE3D_PIXEL_SHADER_CONSTANTS_HEADER_DWORD_COUNT_PLB	2
#define STATE3D_PIXEL_SHADER_CONSTANTS_SINGLE_ELEM_DWORD_COUNT_PLB	4

typedef struct _state3d_pixel_shader_program_ari_instr_plb {
	union {
		struct {
			unsigned int :2;
			unsigned int src0_reg_num :5;
			unsigned int src0_reg_type :3;
			unsigned int dest_channel_mask :4;
			unsigned int dest_reg_num :4;
			unsigned int :1;
			unsigned int dest_reg_type :3;
			unsigned int dest_saturate :1;
			unsigned int :1;
			unsigned int opcode :5;
			unsigned int :3;
		};
		unsigned int dw0;
	};
	union {
		struct {
			unsigned int src1_y_channel_select :3;
			unsigned int src1_y_channel_negate :1;
			unsigned int src1_x_channel_select :3;
			unsigned int src1_x_channel_negate :1;
			unsigned int src1_reg_num :5;
			unsigned int src1_reg_type :3;
			unsigned int src0_w_channel_select :3;
			unsigned int src0_w_channel_negate :1;
			unsigned int src0_z_channel_select :3;
			unsigned int src0_z_channel_negate :1;
			unsigned int src0_y_channel_select :3;
			unsigned int src0_y_channel_negate :1;
			unsigned int src0_x_channel_select :3;
			unsigned int src0_x_channel_negate :1;
		};
		unsigned int dw1;
	};
	union {
		struct {
			unsigned int src2_w_channel_select :3;
			unsigned int src2_w_channel_negate :1;
			unsigned int src2_z_channel_select :3;
			unsigned int src2_z_channel_negate :1;
			unsigned int src2_y_channel_select :3;
			unsigned int src2_y_channel_negate :1;
			unsigned int src2_x_channel_select :3;
			unsigned int src2_x_channel_negate :1;
			unsigned int src2_reg_num :5;
			unsigned int src2_reg_type :3;
			unsigned int src1_w_channel_select :3;
			unsigned int src1_w_channel_negate :1;
			unsigned int src1_z_channel_select :3;
			unsigned int src1_z_channel_negate :1;
		};
		unsigned int dw2;
	};

	void * next_instr;
	unsigned char ps_id;
} state3d_pixel_shader_program_ari_instr_plb_t;

typedef struct _state3d_pixel_shader_program_tex_instr_plb {
	union {
		struct {
			unsigned int samp_reg_num :4;
			unsigned int :10;
			unsigned int dest_reg_num :4;
			unsigned int :1;
			unsigned int dest_reg_type :3;
			unsigned int :2;
			unsigned int opcode :5;
			unsigned int :3;
		};
		unsigned int dw0;
	};
	union {
		struct {
			unsigned int :17;
			unsigned int addr_reg_num :4;
			unsigned int :3;
			unsigned int addr_reg_type :3;
			unsigned int :5;
		};
		unsigned int dw1;
	};
	unsigned int dw2;

	void * next_instr;
	unsigned char ps_id;
} state3d_pixel_shader_program_tex_instr_plb_t;

typedef struct _state3d_pixel_shader_program_dcl_instr_plb {
	union {
		struct {
			unsigned int :10;
			unsigned int dcl_channel_mask :4;
			unsigned int dcl_reg_num :4;
			unsigned int :1;
			unsigned int dcl_reg_type :2;
			unsigned int :1;
			unsigned int samp_type :2;
			unsigned int opcode :5;
			unsigned int :3;
		};
		unsigned int dw0;
	};
	unsigned int dw1;
	unsigned int dw2;

	void * next_instr;
	unsigned char ps_id;
} state3d_pixel_shader_program_dcl_instr_plb_t;

typedef struct _state3d_pixel_shader_program_instr_plb {
	unsigned int idw0;	//instruction's first dword
	unsigned int idw1;	//instruction's second dword
	unsigned int idw2;	//instruction's third dword
} state3d_pixel_shader_program_instr_plb_t;

typedef struct _state3d_pixel_shader_program_plb {
	union {
		struct {
			unsigned int instr_num :9;
			unsigned int :6;
			unsigned int retain :1;
			unsigned int :16;
		};
		unsigned int dw0;
	};
	state3d_pixel_shader_program_instr_plb_t instr[123];
	unsigned int updated;
} state3d_pixel_shader_program_plb_t;

/*
#define STATE3D_PIXEL_SHADER_PROGRAM_ARI_INSTR_UPDATED_PLB	1
#define STATE3D_PIXEL_SHADER_PROGRAM_TEX_INSTR_UPDATED_PLB	1
#define STATE3D_PIXEL_SHADER_PROGRAM_DCL_INSTR_UPDATED_PLB	1
*/
#define STATE3D_PIXEL_SHADER_PROGRAM_HEADER_DWORD_COUNT_PLB	1

typedef struct _state3d_rasterization_rules_plb {
	union {
		struct {
			unsigned int :3;
			unsigned int tri_fan_provok_vtx_sel :2;
			unsigned int tri_fan_provok_vtx_sel_mod_en :1;
			unsigned int line_list_provok_vtx_sel :2;
			unsigned int line_list_provok_vtx_sel_mod_en :1;
			unsigned int texkill_3d4d :1;
			unsigned int texkill_3d4d_mod_en :1;
			unsigned int :1;
			unsigned int snake_walk_dis :1;
			unsigned int point_raster_rule :2;
			unsigned int point_raster_rule_mod_en :1;
			unsigned int zero_pixel_tri_filter_dis :1;
			unsigned int zero_pixel_tri_filter_dis_mod_en :1;
			unsigned int tri_2x2_filter_dis :1;
			unsigned int tri_2x2_filter_dis_mod_en :1;
			unsigned int :12;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_rasterization_rules_plb_t;
#define STATE3D_RASTERIZATION_RULES_UPDATED_PLB		1
#define STATE3D_RASTERIZATION_RULES_DWORD_COUNT_PLB	1

typedef struct _state3d_sampler_state_samp_plb {
	union {
		struct {
			unsigned int shadow_func :3;
			unsigned int max_anisotropy :1;
			unsigned int shadow_en :1;
			unsigned int tex_lod_bias :9;
			unsigned int min_mode_filter :3;
			unsigned int mag_mode_filter :3;
			unsigned int mip_mode_filter :2;
			unsigned int base_mip_level :5;
			unsigned int chroma_index :2;
			unsigned int color_space_conv_en :1;
			unsigned int planar_to_packed_en :1;
			unsigned int reverse_gamma_en :1;
		};
		unsigned int ts_dw0;
	};
	union {
		struct {
			unsigned int east_deinterlace_en:1;
			unsigned int map_index:4;
			unsigned int normalized_coord:1;
			unsigned int tcz_addr_ctrl_mode:3;
			unsigned int tcy_addr_ctrl_mode:3;
			unsigned int tcx_addr_ctrl_mode:3;
			unsigned int chroma_en:1;
			unsigned int keyed_tex_filter_mode:1;
			unsigned int kill_pixel_en:1;
			unsigned int :6;
			unsigned int min_lod:8;
		};
		unsigned int ts_dw1;
	};
	union {
		unsigned int default_color;
		unsigned int ts_dw2;
	};
} state3d_sampler_state_samp_plb_t;

typedef struct _state3d_sampler_state_plb {
	union {
		struct {
			unsigned int samp_num :6;
			unsigned int :26;
		};
		unsigned int dw0;
	};
	union {
		struct {
			unsigned int samp_mask :16;
			unsigned int :16;
		};
		unsigned int dw1;
	};
	state3d_sampler_state_samp_plb_t samp[16]; /* Napa support up to 16 sampler */
	unsigned int updated;
} state3d_sampler_state_plb_t;
/* FIXME: Need to refine smaller enable flags */
#define STATE3D_SAMPLER_STATE_UPDATED_PLB			1
#define STATE3D_SAMPLER_STATE_MAX_DWORD_COUNT_PLB		50
#define STATE3D_SAMPLER_STATE_HEADER_DWORD_COUNT_PLB		2
#define STATE3D_SAMPLER_STATE_SINGLE_TS_DWORD_COUNT_PLB		3

typedef struct _state3d_scissor_enable_plb {
	union {
		struct {
			unsigned int enable :1;
			unsigned int enable_mod_en :1;
			unsigned int :30;
		};
		unsigned int dw0;
	};
	unsigned int updated;
} state3d_scissor_enable_plb_t;
#define STATE3D_SCISSOR_ENABLE_UPDATED_PLB	1
#define STATE3D_SCISSOR_ENABLE_DWORD_COUNT_PLB	1

typedef struct _state3d_scissor_rectangle_plb {
	union {
		struct {
			unsigned int x_min :16;
			unsigned int y_min :16;
		};
		unsigned int dw1;
	};
	union {
		struct {
			unsigned int x_max :16;
			unsigned int y_max :16;
		};
		unsigned int dw2;
	};
	unsigned int updated;

} state3d_scissor_rectangle_plb_t;
#define STATE3D_SCISSOR_RECTANGLE_UPDATED_PLB		1
#define STATE3D_SCISSOR_RECTANGLE_DWORD_COUNT_PLB	3

typedef struct _state3d_span_stipple_plb {
	union {
		struct {
			unsigned int pattern :16;
			unsigned int enable :1;
			unsigned int :15;
		};
		unsigned int dw1;
	};
	unsigned int updated;
} state3d_span_stipple_plb_t;
#define STATE3D_SPAN_STIPPLE_UPDATED_PLB	1
#define STATE3D_SPAN_STIPPLE_DWORD_COUNT_PLB	2

#define COLOR_BUFFER_INDEX	0
#define DEPTH_BUFFER_INDEX	1
#define AUX0_BUFFER_INDEX	2
#define AUX1_BUFFER_INDEX	3
#define INTRA_BUFFER_INDEX	4

#define ZONE_INIT_CLEARPARAM_INDEX  0
#define CLEAR_RECT_CLEARPARAM_INDEX 1

typedef struct _state3d_plb {
	igd_surface_t					color_buffer;
	igd_surface_t					depth_buffer;
	state3d_anti_aliasing_plb_t 			anti_alias;
	state3d_backface_stencil_ops_plb_t 		bface_stencil_ops;
	state3d_backface_stencil_masks_plb_t 		bface_stencil_masks;
	state3d_bin_control_plb_t 			bin_control;
/*
	state3d_buffer_info_plb_t 			dest_buffer_info;
	state3d_buffer_info_plb_t 			depth_buffer_info;
*/
	state3d_buffer_info_plb_t			buffer_info[5];
	state3d_chroma_key_plb_t 			chroma_key[4];
	state3d_clear_parameters_plb_t 			clear_param[2];
	state3d_constant_blend_color_plb_t 		const_blend_color;
	state3d_coord_set_bindings_plb_t 		coord_set_bind;
	state3d_default_diffuse_plb_t 			default_diffuse;
	state3d_default_specular_plb_t 			default_specular;
	state3d_default_z_plb_t 			default_z;
	state3d_depth_offset_scale_plb_t 		depth_offset_scale;
	state3d_depth_subrectangle_enable_plb_t 	depth_subrect_en;
	state3d_dest_buffer_variables_plb_t		dest_buf_vars;
	state3d_drawing_rectangle_plb_t			draw_rect;
	state3d_filter_coefficients_4x4_plb_t		filter_coef_4x4;
	state3d_filter_coefficients_6x5_plb_t		filter_coef_6x5;
	state3d_fog_color_plb_t				fog_color;
	state3d_fog_mode_plb_t				fog_mode;
	state3d_independent_alpha_blend_plb_t		indep_alpha_blend;
	state3d_load_indirect_plb_t			load_indirect;
	state3d_load_state_immediate_1_plb_t		lsi1;
	state3d_map_deinterlacer_parameters_plb_t	map_deinterlace_param;
	state3d_map_palette_load_32_plb_t		map_pal_load_32;
	state3d_map_state_plb_t				map_state;
	state3d_modes_4_plb_t				modes_4;
	state3d_modes_5_plb_t				modes_5;
	state3d_pixel_shader_constants_plb_t		pixel_shader_const;
	state3d_pixel_shader_program_plb_t		pixel_shader_program;
/*	state3d_pixel_shader_program_dcl_instr_plb_t	pixel_shader_dcl;
	state3d_pixel_shader_program_ari_instr_plb_t	pixel_shader_ari;
	state3d_pixel_shader_program_tex_instr_plb_t	pixel_shader_tex;*/
	state3d_rasterization_rules_plb_t		raster_rules;
	state3d_sampler_state_plb_t			sampler_state;
	state3d_scissor_enable_plb_t			scissor_en;
	state3d_scissor_rectangle_plb_t			scissor_rect;
	state3d_span_stipple_plb_t			span_stipple;
} state3d_plb_t;

int state3d_update_plb(igd_command_t ** in, state3d_plb_t *state, int load_all);
int state3d_update_size_plb(state3d_plb_t *state);

#define STATE3D_PLB(ac) ((state3d_plb_t*)((appcontext_plb_t*)ac)->state3d)

#endif
