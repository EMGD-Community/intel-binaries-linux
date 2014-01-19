/*
 *-----------------------------------------------------------------------------
 * Filename: ovl_regs_plb.h
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
 *  This is the internal header file for overlay. It should be not be
 *  by any other module besides the overlay module itself. It contains the
 *  neccessary hardware virtualized register dependant information including
 *  values, structures and addresses specifically for the Napa core
 *-----------------------------------------------------------------------------
 */

#ifndef OVL_REGS_PLB_H_
#define OVL_REGS_PLB_H_

/* Overlay Update Register Image Structure.*/
typedef struct _ovl_reg_image_plb{

	volatile unsigned int			buffer0_yrgb_ptr;
	volatile unsigned int			buffer1_yrgb_ptr;
	volatile unsigned int			buffer0_u_ptr;
	volatile unsigned int			buffer0_v_ptr;
	volatile unsigned int			buffer1_u_ptr;
	volatile unsigned int			buffer1_v_ptr;
	volatile unsigned short			yrgb_stride;
	volatile unsigned short			uv_stride;
	volatile unsigned short			yrgb_vert_phase_field0;
	volatile unsigned short			yrgb_vert_phase_field1;
	volatile unsigned short			uv_vert_phase_field0;
	volatile unsigned short			uv_vert_phase_field1;
	volatile unsigned short			yrgb_hphase;
	volatile unsigned short			uv_hphase;
	volatile unsigned int			init_phase_shift;
	volatile unsigned short			dest_pos_x_left;
	volatile unsigned short			dest_pos_y_top;
	volatile unsigned short			dest_width_x;
	volatile unsigned short			dest_height_y;
	volatile unsigned short			source_yrgb_width;
	volatile unsigned short			source_uv_width;
	volatile unsigned short			source_yrgb_width_swords;
	volatile unsigned short			source_uv_width_swords;
	volatile unsigned short			source_yrgb_height;
	volatile unsigned short			source_uv_height;
	volatile unsigned int			yrgb_scale;
	volatile unsigned int			uv_scale;
	volatile unsigned int			col_ctl_brt_con;
	volatile unsigned int			col_ctl_sat_hue;
	volatile unsigned int			dest_ckey_val;
	volatile unsigned int			dest_ckey_mask;
	volatile unsigned int			source_ckey_high;
	volatile unsigned int			source_ckey_low;
	volatile unsigned int			source_ckey_mask;
	volatile unsigned int			config;
	volatile unsigned int			command;
	volatile unsigned int			reserved1;
	volatile unsigned short			alpha_pos_x_left;
	volatile unsigned short			alpha_pos_y_top;
	volatile unsigned short			alpha_width_x;
	volatile unsigned short			alpha_height_y;
	volatile unsigned int			reserved2;
	volatile unsigned int			reserved3;
	volatile unsigned int			reserved4;
	volatile unsigned int			reserved5;
	volatile unsigned int			reserved6;
	volatile unsigned int			reserved7;
	volatile unsigned int			reserved8;
	volatile unsigned int			reserved9;
	volatile unsigned int			reserved10;
	volatile unsigned int			reserved11;
	volatile unsigned int			reserved11a; /*ovl_fast_horz_downscale;*/
	volatile unsigned int			vert_downscale;
	volatile unsigned int			reserved12[86];
	volatile unsigned short			y_vert_coeff_single[52]; /*offset 0x200 --> 3*17/2 + 1*/
	volatile unsigned int			reserved13[38];
	volatile unsigned short			y_horz_coeff_single[86]; /*offset 0x300 --> 5*17/2 + 1*/
	volatile unsigned int			reserved14[85];
	volatile unsigned short			uv_vert_coeff_single[52]; /*offset 0x500 --> 3*17/2 + 1*/
	volatile unsigned int			reserved15[38];
	volatile unsigned short			uv_horz_coeff_single[52]; /*offset 0x600 --> 3*17/2 + 1*/
	volatile unsigned int			reserved16[38];

} ovl_reg_image_plb_t;

/* Color Correction */
#define OVL_YUV_COLOR_DEF_CONT_BRGHT    0x10c00fb
#define OVL_YUV_COLOR_DEF_SATN_HUE      0x0000091
#define OVL_RGB_COLOR_DEF_CONT_BRGHT    0x1000000
#define OVL_RGB_COLOR_DEF_SATN_HUE      0x0000080

/* Overlay Config */
#define OVL_CONFIG_NO_LINE_BUFF     0xffffffff
#define OVL_CONFIG_TWO_LINE_BUFF    0x00000000
#define OVL_CONFIG_THREE_LINE_BUFF  0x00000001
#define OVL_CONFIG_LINE_BUFF_MASK   0x00000001

/* Overlay Command Definitions */
/* 422 Swap */
#define OVL_CMD_UV_SWAP				0x00004000
#define OVL_CMD_Y_SWAP				0x00008000
#define OVL_CMD_YUV_SWAP			0x0000C000

/* Source Format */
#define OVL_CMD_RGB_8888			0x00000400
#define OVL_CMD_RGB_565				0x00000C00
#define OVL_CMD_RGB_555				0x00000800
#define OVL_CMD_YUV_NV12Alt			0x00001C00 /*planar NV12, Alternate?*/
#define OVL_CMD_YUV_NV12			0x00002C00 /*planar NV12*/
#define OVL_CMD_YUV_422				0x00002000 /*packed YUV422*/
#define OVL_CMD_YUV_411				0x00002400 /*packed YUV411*/
#define OVL_CMD_YUV_420P			0x00003000 /*planar YUV420*/
#define OVL_CMD_YUV_422P			0x00003400 /*planar YUV422*/
#define OVL_CMD_YUV_410P			0x00003800 /*planar YUV410*/
#define OVL_CMD_YUV_444P			0x00003C00 /*planar YUV444*/
#define OVL_CMD_SRC_FMT_MASK		0x00003C00 /*mask for above*/

/* Flip Type */
#define OVL_CMD_FRAME_MODE          0x00000000
#define OVL_CMD_FIELD_MODE			0x00000020

/* Field Sync Flip Enable */
#define OVL_CMD_FIELD_SYNC_FLIP			0x00000080

/* Buffer and Field */
#define OVL_CMD_ACT_BUF0			0x00000000
#define OVL_CMD_ACT_BUF1			0x00000004
#define OVL_CMD_ACT_FLD0			0x00000000
#define OVL_CMD_ACT_FLD1			0x00000002

/* Initial phase register */
#define Y_VPP_FLD0_PLUS1			0x100000
#define Y_VPP_FLD0_PLUS2			0x200000
#define Y_VPP_FLD0_MINUS1			0xF00000
#define Y_VPP_FLD1_PLUS1			0x010000
#define Y_VPP_FLD1_PLUS2			0x020000
#define Y_VPP_FLD1_MINUS1			0x0F0000
#define Y_HPP_PLUS1					0x001000
#define Y_HPP_PLUS2					0x002000
#define Y_HPP_MINUS1				0x00F000
#define UV_VPP_FLD0_PLUS1			0x000100
#define UV_VPP_FLD0_PLUS2			0x000200
#define UV_VPP_FLD0_MINUS1			0x000F00
#define UV_VPP_FLD1_PLUS1			0x000010
#define UV_VPP_FLD1_PLUS2			0x000020
#define UV_VPP_FLD1_MINUS1			0x0000F0
#define UV_HPP_PLUS1				0x000001
#define UV_HPP_PLUS2				0x000002
#define UV_HPP_MINUS1				0x00000F

#define OVL_REG_ADDR_GAMMA5						0x30010
#define OVL_REG_ADDR_GAMMA4						0x30014
#define OVL_REG_ADDR_GAMMA3						0x30018
#define OVL_REG_ADDR_GAMMA2						0x3001C
#define OVL_REG_ADDR_GAMMA1						0x30020
#define OVL_REG_ADDR_GAMMA0						0x30024
#define OVL_TOTAL_GAMMA_REG						6

/* following value are needed because hardware seems to display yuv slightly dimmer
   than RGB when color data is calculated out to be equal */
#define MID_CONTRAST_YUV					0x43//4a
#define MID_SATURATION_YUV					0x91//92
#define MID_BRIGHTNESS_YUV					-5

#endif /* OVL_REGISTER_IMAGE_H_ */
