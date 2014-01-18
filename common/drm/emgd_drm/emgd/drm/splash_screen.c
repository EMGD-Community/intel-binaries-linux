/*
 *-----------------------------------------------------------------------------
 * Filename: splash_screen.c
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
 *  This is the Intel Embedded Graphics EFI Driver Splash Screen implementation
 *  file. This code shows a splash screen with a customizable icon.
 *-----------------------------------------------------------------------------
 */
#define MODULE_NAME hal.oal

#include <drm/drmP.h>
#include <drm/drm.h>
#include <memory.h>
#include "sched.h"
#include "image_data.h"
#include "splash_screen.h"
#include "io.h"
#include "igd_debug.h"


/**
 * Function to display a splash screen to the user. The splash screen must be
 * accessible to the kernel mode driver as it has to be displayed immediately
 * after setting the mode (if requested by the user through config options).
 *
 * @param ss_data (IN) a non null pointer to splash screen information like
 * width, height etc.
 */
void display_splash_screen(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	emgd_drm_splash_screen_t *ss_data)
{
	if (image_data[0] == 0x89) {
		display_png_splash_screen(fb_info, fb, ss_data);
	} else {
		display_bmp_splash_screen(fb_info, fb, ss_data);
	}
}

/*
 * This is the function to display the bmp splash screen.
 *
 * @param ss_data (IN) a non null pointer to splash screen information like
 * width, height etc.
 */
void display_bmp_splash_screen(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	emgd_drm_splash_screen_t *ss_data)
{
	unsigned char *fb_addr, *icon_temp;
	unsigned long *fb_addr_long, icon_long;
	unsigned long bitmap_pitch;
	short x, y;
	unsigned long init_x_shift, init_y_shift;
	unsigned long row, col, fb_index;
	unsigned long bytecount, temp;
	unsigned long limit_width, limit_height;

	EMGD_TRACE_ENTER;

	x = (short) ss_data->x;
	y = (short) ss_data->y;

	if(x < 0) {
		init_x_shift = (fb_info->width + x) * 4;
		init_y_shift = (fb_info->height + y) * fb_info->screen_pitch;
	} else {
		init_x_shift = x * 4;
		init_y_shift = y * fb_info->screen_pitch;
	}

	fb_addr = fb + init_y_shift;
	bytecount = (unsigned long) image_data[1];
	bitmap_pitch = ss_data->width * bytecount;

	limit_width = ss_data->width;
	limit_height = ss_data->height;

	if (x+ss_data->width > fb_info->width) {
		limit_width = fb_info->width - x;
	}
	if (y+ss_data->height > fb_info->height) {
		limit_height = fb_info->height - y;
	}

	for(row = 0; row < limit_height; row++) {
		fb_addr_long =
			(unsigned long *) &fb_addr[fb_info->screen_pitch * row +
			init_x_shift];
		/*
		 * We are adding 3 bytes here, the first byte indicates BMP or PNG,
		 * the second byte is the bytecount
		 * and the third byte is the palette count
		 */
		icon_temp = &image_data[3 + (row * bitmap_pitch)];
		fb_index = 0;

		for(col = 0; col < limit_width; col++) {

			icon_long = *((unsigned long *) &icon_temp[col*bytecount]);
			switch(bytecount) {
				case 1:
					/* 8 bit */
					temp = (icon_long & 0xFF);
					icon_long = ((temp & 0xE0)<<16) | ((temp & 0x1C)<<11) |
						((temp & 0x3)<<6);
					break;
				case 2:
					/* 16 bit */
					temp = (icon_long & 0xFFFF);
					icon_long = CONV_16_TO_32_BIT(temp);
					break;
			}
			/*
			 * For 24 bit we don't really have to do anything as it is
			 * already in RGB 888 format
			 */
			fb_addr_long[fb_index++] = icon_long & 0x00FFFFFF;
		}
	}
	EMGD_TRACE_EXIT;
}


/*
 * This is the function to display the png splash screen.
 *
 * @param ss_data (IN) a non null pointer to splash screen information like
 * width, height etc.
 */
void display_png_splash_screen(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	emgd_drm_splash_screen_t *ss_data)
{
	unsigned long image_size;
	unsigned long i;
	unsigned long chunk_size;
	unsigned long chunk_type;
	unsigned long iter = PNG_HEADER_SIZE;
	png_header image_header;
	png_frame *frames = NULL;
	png_frame *default_image = NULL;
	unsigned long gama = 0;
	unsigned long palette_size = 0;
	unsigned char *input_data = NULL;
	unsigned long input_iter = 0;
	unsigned long apng_num_frames = 0;
	unsigned long apng_num_plays = 0;
	unsigned long sequence_number = 0;
	unsigned long cur_seq_num = 0;
	unsigned long orig_x = 0;
	unsigned long orig_y = 0;
	unsigned long apng_file = 0;
	unsigned long cur_frame = 0;
	unsigned char trans_p = 0;
	unsigned long prev_dispose_op = 0;
	unsigned short delay_num, delay_den;

	EMGD_TRACE_ENTER;

	/*
	 * Just incase there is no background and we have alpha values, lets
	 * use the background specified in ss_data.
	 */
	image_header.background = 0xFF000000 | ss_data->bg_color;
	image_header.background_r = (image_header.background >> 16) & 0xFF;
	image_header.background_g = (image_header.background >> 8) & 0xFF;
	image_header.background_b = image_header.background & 0xFF;

	image_size = sizeof(image_data)/sizeof(unsigned char);
	input_data = (unsigned char *)vmalloc(sizeof(image_data));
	if (!input_data) {
		EMGD_ERROR("Out of memory");
		return;
	}
	OS_MEMSET(input_data, 0, sizeof(image_data));

	orig_x = (short) ss_data->x;
	orig_y = (short) ss_data->y;

	/*
	 * Lets get the information for the first chunk, which should be
	 * the header chunk: IHDR.
	 */
	read_int_from_stream(image_data, &iter, &chunk_size);
	read_int_from_stream(image_data, &iter, &chunk_type);

	/*
	 * Initialize image_header
	 */
	image_header.width = 0;
	image_header.height = 0;
	image_header.bit_depth = 0;
	image_header.colour_type = 0;
	image_header.compression_method = 0;
	image_header.filter_method = 0;
	image_header.interlace_method = 0;
	image_header.bpp = 0;
	image_header.bytes_pp = 0;
	image_header.bytes_pl = 0;

	/* Loop through the PNG chunks */
	while (iter <= image_size) {
		switch (chunk_type) {
		case CHUNK_IHDR:
			read_int_from_stream(image_data, &iter, &image_header.width);
			read_int_from_stream(image_data, &iter, &image_header.height);
			image_header.bit_depth = (unsigned char)image_data[iter++];
			image_header.colour_type = (unsigned char)image_data[iter++];
			image_header.compression_method = (unsigned char)image_data[iter++];
			image_header.filter_method = (unsigned char)image_data[iter++];
			image_header.interlace_method = (unsigned char)image_data[iter++];
			image_header.x_offset = orig_x;
			image_header.y_offset = orig_y;

			/* store bits per pixel based on PNG spec */
			switch (image_header.colour_type) {
			case COLOR_GREY:
				image_header.bpp = image_header.bit_depth;
				break;
			case COLOR_TRUE:
				image_header.bpp = 3 * image_header.bit_depth;
				break;
			case COLOR_INDEXED:
				image_header.bpp = image_header.bit_depth;
				break;
			case COLOR_GREY_ALPHA:
				image_header.bpp = 2 * image_header.bit_depth;
				break;
			case COLOR_TRUE_ALPHA:
				image_header.bpp = 4 * image_header.bit_depth;
				break;
			}
			/*
			 * Adding 7 to the bits per pixel before we divide by 8
			 * gives us the ceiling of bytes per pixel instead of the floor.
			 */
			image_header.bytes_pp = (image_header.bpp + 7) / 8;
			image_header.bytes_pl =
				((image_header.width * image_header.bpp) + 7) / 8;
			break;

		case CHUNK_TRNS:
			image_header.using_transparency = 1;
			switch (image_header.colour_type) {
				case COLOR_GREY:
					read_short_from_stream(image_data, &iter,
						&image_header.transparency_r);
					break;
				case COLOR_TRUE:
					read_short_from_stream(image_data, &iter,
						&image_header.transparency_r);
					read_short_from_stream(image_data, &iter,
						&image_header.transparency_g);
					read_short_from_stream(image_data, &iter,
						&image_header.transparency_b);
					break;
				case COLOR_INDEXED:
					if (image_header.image_palette) {
						if (chunk_size > palette_size) {
							EMGD_ERROR("Palette size is smaller than "
								"transparency values for the palette");
						}
						for (i=0; i<chunk_size; i++) {
							read_char_from_stream(image_data, &iter, &trans_p);
							image_header.image_palette[i] &= 0xFFFFFF |
								((trans_p << 28) | (trans_p << 24));
						}
					} else {
						EMGD_ERROR("Palette has not been initialized yet");
					}
					break;
			}
			break;

		case CHUNK_BKGD:
			/* Truecolor */
			if (image_header.colour_type == COLOR_TRUE_ALPHA ||
				image_header.colour_type == COLOR_TRUE) {

				switch (image_header.bit_depth) {
				case 16:
					read_char_from_stream(image_data, &iter,
						&image_header.background_r);
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_g);
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_b);
					iter++;
					break;
				case 8:
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_r);
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_g);
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_b);
					break;
				}
			}

			/* Grayscale */
			if (image_header.colour_type == COLOR_GREY_ALPHA ||
				image_header.colour_type == COLOR_GREY) {

				switch (image_header.bit_depth) {
				case 16:
					read_char_from_stream(image_data, &iter,
						&image_header.background_r);
					iter++;
					break;
				case 8:
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_r);
					break;
				case 4:
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_r);
					image_header.background_r =
						((image_header.background_r >> 4) << 4) |
						(image_header.background_r >> 4);
					break;
				case 2:
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_r);
					image_header.background_r =
						((image_header.background_r >> 2) << 6) |
						((image_header.background_r >> 2) << 4) |
						((image_header.background_r >> 2) << 2) |
						(image_header.background_r >> 2);
					break;
				case 1:
					iter++;
					read_char_from_stream(image_data, &iter,
						&image_header.background_r);
					image_header.background_r =
						(image_header.background_r << 7) |
						(image_header.background_r << 6) |
						(image_header.background_r << 5) |
						(image_header.background_r << 4) |
						(image_header.background_r << 3) |
						(image_header.background_r << 2) |
						(image_header.background_r << 1) |
						image_header.background_r;
					break;
				}
				image_header.background_g = image_header.background_r;
				image_header.background_b = image_header.background_r;
			}

			image_header.background = 0xFF000000 |
				image_header.background_r<<16 |
				image_header.background_g<<8 |
				image_header.background_b;
			break;

		case CHUNK_GAMA:
			read_int_from_stream(image_data, &iter, &gama);
			break;

		case CHUNK_PLTE:
			palette_size = chunk_size/3;
			image_header.image_palette =
				vmalloc(sizeof(unsigned long) * palette_size);
			if (!image_header.image_palette) {
				EMGD_ERROR("Out of memory");
				return;
			}
			OS_MEMSET(image_header.image_palette, 0,
				sizeof(unsigned long) * palette_size);

			for (i=0; i<palette_size; i++) {
				image_header.image_palette[i] = (
					0xFF000000 |
					((unsigned char)image_data[iter] << 16) |
					((unsigned char)image_data[iter+1] << 8) |
					(unsigned char)image_data[iter+2]);
				iter += 3;
			}
			break;

		case CHUNK_IDAT:
			if (!default_image) {
				default_image = vmalloc(sizeof(png_header));
				if (!default_image) {
					EMGD_ERROR("Out of memory");
					return;
				}
				OS_MEMSET(default_image, 0, sizeof(png_header));
				default_image->width = image_header.width;
				default_image->height = image_header.height;
				default_image->x_offset = 0;
				default_image->y_offset = 0;
				default_image->bytes_pp = image_header.bytes_pp;
				default_image->bytes_pl = image_header.bytes_pl;
				default_image->blend_op = APNG_BLEND_OP_SOURCE;
				default_image->dispose_op = APNG_DISPOSE_OP_NONE;
			}
			for (i=0; i<chunk_size; i++) {
				input_data[input_iter++] = image_data[iter++];
			}
			break;

		case CHUNK_ACTL:
			apng_file = 1;
			read_int_from_stream(image_data, &iter, &apng_num_frames);
			read_int_from_stream(image_data, &iter, &apng_num_plays);
			frames = vmalloc(apng_num_frames * sizeof(png_frame));
			if (!frames) {
				EMGD_ERROR("Out of memory.");
				return;
			}
			OS_MEMSET(frames, 0, apng_num_frames * sizeof(png_frame));
			break;

		case CHUNK_FCTL:
			if (cur_seq_num > 0) {
				decode_png_data(&image_header,input_data,&frames[cur_frame-1]);
			} else {
				if (default_image) {
					decode_png_data(&image_header, input_data, default_image);
				}
			}

			/* Should we wipe out the input_data buffer? */
			input_iter = 0;

			read_int_from_stream(image_data, &iter, &sequence_number);
			read_int_from_stream(image_data, &iter, &frames[cur_frame].width);
			read_int_from_stream(image_data, &iter, &frames[cur_frame].height);
			read_int_from_stream(image_data, &iter, &frames[cur_frame].x_offset);
			read_int_from_stream(image_data, &iter, &frames[cur_frame].y_offset);
			read_short_from_stream(image_data, &iter, &delay_num);
			read_short_from_stream(image_data, &iter, &delay_den);
			read_char_from_stream(image_data, &iter, &frames[cur_frame].dispose_op);
			read_char_from_stream(image_data, &iter, &frames[cur_frame].blend_op);

			if (delay_num) {
				if (!delay_den) {
					frames[cur_frame].delay = 10000 * (unsigned long)delay_num;
				} else {
					frames[cur_frame].delay = (1000000 *
						(unsigned long)delay_num) / (unsigned long)delay_den;
				}
			}

			/*
			 * Adding 7 to the bits per pixel before we divide by 8
			 * gives us the ceiling of bytes per pixel instead of the floor.
			 */
			frames[cur_frame].bytes_pp = (image_header.bpp + 7) / 8;
			frames[cur_frame].bytes_pl =
				((frames[cur_frame].width * image_header.bpp) + 7) / 8;

			cur_frame++;

			if (sequence_number != cur_seq_num++) {
				EMGD_ERROR("Sequence numbers do not match!");
				return;
			}
			break;

		case CHUNK_FDAT:
			read_int_from_stream(image_data, &iter, &sequence_number);
			if (sequence_number != cur_seq_num++) {
				EMGD_ERROR("Sequence numbers do not match!");
				return;
			}
			for (i=4; i<chunk_size; i++) {
				input_data[input_iter++] = image_data[iter++];
			}
			break;

		case CHUNK_IEND:
			if (!frames && default_image) {
				decode_png_data(&image_header, input_data,
					default_image);
			} else {
				decode_png_data(&image_header, input_data,
					&frames[cur_frame-1]);
			}
			break;

		default:
			iter += chunk_size;
			break;
		}

		/*
		 * Skip over the CRC for now, do we actually want to spend
		 * time checking this? Per the spec, unless there is an a corrupted
		 * header, the only possible outcome is a corrupted image.  It's
		 * either that, or we don't display any image, maybe in this case we
		 * should display a blue screen. :)
		 */
		iter += 4;

		/* Get the next chunk */
		read_int_from_stream(image_data, &iter, &chunk_size);
		read_int_from_stream(image_data, &iter, &chunk_type);
	}

	if (input_data) {
		vfree(input_data);
		input_data = NULL;
	}

	if (apng_file && !apng_num_plays) {
		apng_num_plays = 20;
	}
	if (apng_num_frames > 0) {
		frames[apng_num_frames-1].dispose_op = APNG_DISPOSE_OP_NONE;
		if (frames[0].dispose_op == APNG_DISPOSE_OP_PREVIOUS) {
			frames[0].dispose_op = APNG_DISPOSE_OP_BACKGROUND;
		}
	}
	for (i=0; i<apng_num_plays; i++) {
		for (cur_frame=0; cur_frame<apng_num_frames; cur_frame++) {
			if (cur_frame > 0) {
				prev_dispose_op = frames[cur_frame-1].dispose_op;
			}
			display_png_frame(fb_info, fb, image_header, &frames[cur_frame],
				prev_dispose_op);
		}
	}
	if (!apng_file) {
		display_png_frame(fb_info, fb, image_header, default_image,
			APNG_DISPOSE_OP_NONE);
	}

	for (cur_frame=0; cur_frame<apng_num_frames; cur_frame++) {
		if (frames[cur_frame].output) {
			vfree(frames[cur_frame].output);
			frames[cur_frame].output = NULL;
		}
	}
	if (frames) {
		vfree(frames);
		frames = NULL;
	}

	if (default_image->output) {
		vfree(default_image->output);
		default_image->output = NULL;
	}

	if (default_image) {
		vfree(default_image);
		default_image = NULL;
	}

	EMGD_TRACE_EXIT;
}

void display_png_frame(
	igd_framebuffer_info_t *fb_info,
	unsigned char *fb,
	png_header image_header,
	png_frame *frame,
	unsigned long prev_dispose_op)
{
	unsigned char *fb_addr = NULL;
	unsigned long *fb_addr_long = NULL;
	unsigned long init_x_shift, init_y_shift, row, col, j;
	unsigned char image_alpha;
	unsigned char background_alpha;
	unsigned long *previous = NULL;

	if (frame->dispose_op == APNG_DISPOSE_OP_PREVIOUS) {
		previous = vmalloc(frame->width*frame->height*sizeof(unsigned long));
		if (!previous) {
			EMGD_ERROR("Out of memory.");
			return;
		}
	}

	/* Lets position our image at the supplied offsets on the screen */
	/* TODO: Need to account for negative offset */
	init_x_shift = (image_header.x_offset + frame->x_offset) *
		sizeof(unsigned long);
	init_y_shift = (image_header.y_offset + frame->y_offset) *
		fb_info->screen_pitch;
	fb_addr = fb + init_y_shift;
	fb_addr_long = (unsigned long *) &fb_addr[init_x_shift];

	row = 0;
	j = 0;

	switch (frame->blend_op) {

	/* Blending against our background color */
	case APNG_BLEND_OP_SOURCE:
		while (row < frame->height){
			col = 0;
			fb_addr_long = (unsigned long *)
				&fb_addr[fb_info->screen_pitch * row + init_x_shift];

			if (frame->dispose_op == APNG_DISPOSE_OP_PREVIOUS) {
				/* Save the previous since we need to dispose to it */
				OS_MEMCPY((void *)&previous[j], (void *)fb_addr_long,
					frame->width * sizeof(unsigned long));
			}

			/* Put together the pixel and output to framebuffer */
			while (col < frame->width) {
				image_alpha = frame->output[j]>>24;
				if (image_alpha){
					if (image_alpha != 0xFF){
						background_alpha = (0xFF - image_alpha) & 0xFF;

						frame->output[j] = 0xFF000000 |
							((((((frame->output[j]&0xFF0000)>>16) *
								image_alpha)/0xFF) +
							((((image_header.background&0xFF0000)>>16) *
								background_alpha)/0xFF))<<16) |
							((((((frame->output[j]&0x00FF00)>>8) *
								image_alpha)/0xFF) +
							((((image_header.background&0x00FF00)>>8) *
								background_alpha)/0xFF))<<8) |
							((((((frame->output[j]&0x0000FF)) *
								image_alpha)/0xFF) +
							((((image_header.background&0x0000FF)) *
								background_alpha)/0xFF)));
					}
				} else {
					frame->output[j] = image_header.background;
				}
				fb_addr_long[col] = frame->output[j];
				col++;
				j++;
			}
			row++;
		}
		break;

	/* Blending against previous frame */
	case APNG_BLEND_OP_OVER:
		while (row < frame->height){
			col = 0;
			fb_addr_long = (unsigned long *)
				&fb_addr[fb_info->screen_pitch * row + init_x_shift];

			if (frame->dispose_op == APNG_DISPOSE_OP_PREVIOUS) {
				/* Save the previous isince we need to dispose to it */
				OS_MEMCPY((void *)&previous[j], (void *)fb_addr_long,
						frame->width * sizeof(unsigned long));
			}

			/* Blend the pixel with existing framebuffer pixel */
			while (col < frame->width) {
				image_alpha = frame->output[j]>>24;

				if (image_alpha){
					if (image_alpha != 0xFF){
						background_alpha = (0xFF - image_alpha) & 0xFF;

						frame->output[j] = 0xFF000000 |
							((((((frame->output[j]&0xFF0000)>>16) *
								image_alpha)/0xFF) +
							  ((((fb_addr_long[col]&0xFF0000)>>16) *
								background_alpha)/0xFF))<<16) |
							((((((frame->output[j]&0x00FF00)>>8) *
								image_alpha)/0xFF) +
							  ((((fb_addr_long[col]&0x00FF00)>>8) *
								background_alpha)/0xFF))<<8) |
							((((((frame->output[j]&0x0000FF)) *
								image_alpha)/0xFF) +
							  ((((fb_addr_long[col]&0x0000FF)) *
								background_alpha)/0xFF)));
					}
					fb_addr_long[col] = frame->output[j];
				}
				col++;
				j++;
			}
			row++;
		}
		break;
	}

	if (frame->delay) {
		OS_SLEEP(frame->delay);
	}

	fb_addr = fb + init_y_shift;
	fb_addr_long = (unsigned long *) &fb_addr[init_x_shift];
	row = 0;

	/* TODO: It would be better to only do this to the portions of the
	 * frame that will not get overwritten by the next frame.
	 */
	switch (frame->dispose_op) {
	case APNG_DISPOSE_OP_PREVIOUS:
		j = 0;
		while (row < frame->height){
			fb_addr_long = (unsigned long *)
				&fb_addr[fb_info->screen_pitch * row + init_x_shift];

			OS_MEMCPY((void *)fb_addr_long, (void *)&previous[j],
				frame->width * sizeof(unsigned long));

			j+= frame->width;
			row++;
		}
		if (previous) {
			vfree(previous);
			previous = NULL;
		}
		break;
	case APNG_DISPOSE_OP_BACKGROUND:
		while (row < frame->height){
			fb_addr_long = (unsigned long *)
				&fb_addr[fb_info->screen_pitch * row + init_x_shift];

			OS_MEMSET((void *)fb_addr_long, image_header.background,
				frame->width * sizeof(unsigned long));

			row++;
		}
		break;
	}
}


void decode_png_data(
	png_header *image_header,
	unsigned char *input_data,
	png_frame *frame)
{
	unsigned char *output;
	unsigned long output_iter = 0;

	unsigned long iter = 0;
	unsigned char bit_iter = 0;
	unsigned long row = 0, col = 0;
	unsigned long end_of_row;
	unsigned long j,k,l;

	unsigned char zlib_cmf = 0;
	unsigned char zlib_flg = 0;
	unsigned char zlib_cm = 0;
	unsigned char zlib_cinfo = 0;
	unsigned char zlib_fcheck = 0;
	unsigned char zlib_fdict = 0;
	unsigned char zlib_flevel = 0;
	unsigned long zlib_dictid = 0;

	huffman_node *length_tree = NULL;
	huffman_node *distance_tree = NULL;

	unsigned char paeth_a, paeth_b, paeth_c;
	unsigned long paeth_p, paeth_pa, paeth_pb, paeth_pc;

	unsigned long filter_type = 0;
	unsigned long bfinal = 0;
	unsigned long btype = 0;
	unsigned char compr_len = 0;
	unsigned char compr_nlen = 0;
	unsigned int small_color;

	/* Allocate space for out output buffer */
	output = (unsigned char *)vmalloc(
		frame->height * frame->bytes_pl + frame->height);
	if (!output) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(output, 0, frame->height * frame->bytes_pl + frame->height);

	frame->size = frame->height * frame->width * sizeof(unsigned long);
	frame->output = vmalloc(frame->size);
	if (!frame->output) {
		frame->size = 0;
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(frame->output, 0, frame->size);

	/* Data, this needs to be decompressed per zlib spec */
	if (!zlib_cmf) {
		zlib_cmf = (unsigned char)input_data[iter++];
		zlib_flg = (unsigned char)input_data[iter++];
		zlib_cm = zlib_cmf & 0xF;
		zlib_cinfo = (zlib_cmf >> 4) & 0xF;
		zlib_fcheck = zlib_flg & 0x1F;
		zlib_fdict = (zlib_flg & 0x20) >> 5;
		zlib_flevel = (zlib_flg >> 6) & 0x3;

		if (zlib_fdict) {
			read_int_from_stream(input_data, &iter, &zlib_dictid);
		}
	}

	/* Here is where we need to process data as a bit stream */
	bfinal = 0;
	while (!bfinal) {
		read_bits_from_stream(input_data, &iter, &bit_iter, 1, &bfinal);
		read_bits_from_stream(input_data, &iter, &bit_iter, 2, &btype);

		if (btype == 0){

			if (bit_iter) {
				iter++;
				bit_iter = 0;
			}

			/* No Compression */
			read_char_from_stream(input_data, &iter, &compr_len);
			read_char_from_stream(input_data, &iter, &compr_nlen);

			for (j = 0;j < compr_len; j++) {
				read_char_from_stream(input_data, &iter, &output[j]);
			}

		} else {

			if (btype == 2){

				/* Compressed with dynamnic Huffman codes */
				build_dynamic_huffman_tree(
						input_data,
						&iter,
						&bit_iter,
						&length_tree,
						&distance_tree);
			} else {

				/* Compressed with static Huffman codes */
				build_static_huffman_tree(&length_tree,	&distance_tree);
			}

			/* Decompress huffman code */
			decompress_huffman(
					input_data,
					&iter,
					&bit_iter,
					&length_tree,
					&distance_tree,
					output,
					&output_iter);

			free_node(length_tree);
			free_node(distance_tree);
		}
	}

	row = 0;
	j = 0;
	l = 0;

	/*
	 * Process the scanline filtering
	 * This filtering works by using a difference from a previous pixel
	 * instead of full pixel data.
	 */
	while (row < frame->height){
		j = row * frame->bytes_pl + row;
		end_of_row = j + frame->bytes_pl;
		filter_type = output[j++];

		switch (filter_type) {
		case 1:
			/* Filter type of 1 uses the previous pixel */
			for (k=j+frame->bytes_pp; k<=end_of_row; k++) {
				output[k] += output[k-frame->bytes_pp];
			}
			break;
		case 2:
			/* Filter type of 2 uses the previous row's pixel */
			if (row) {
				for (k=j; k<=end_of_row; k++) {
					output[k] += output[k-frame->bytes_pl-1];
				}
			}
			break;
		case 3:
			/*
			 * Filter type of 3 uses the average of the
			 * previous pixel and the previous row's pixel
			 */
			if (row) {
				for (k=j; k<j+frame->bytes_pp; k++) {
					output[k] += output[k-frame->bytes_pl-1]/2;
				}
				for (k=j+frame->bytes_pp; k<=end_of_row; k++) {
					output[k] += (output[k-frame->bytes_pp] +
						output[k-frame->bytes_pl-1])/2;
				}
			} else {
				for (k=j+frame->bytes_pp; k<=end_of_row; k++) {
					output[k] = output[k] +
						output[k-frame->bytes_pp]/2;
				}
			}
			break;
		case 4:
			/*
			 * Filter type of 4 uses this algorithm to
			 * determine if it should use the previous pixel,
			 * the previous row's pixel, or the pixel immediately
			 * before the previous row's pixel.
			 */
			for (k=j; k<=end_of_row; k++) {

				if (k >= j + frame->bytes_pp) {
					paeth_a = output[k-frame->bytes_pp];
				} else {
					paeth_a = 0;
				}

				if (row) {
					paeth_b = output[k-frame->bytes_pl-1];
				} else {
					paeth_b = 0;
				}

				if (row && k >= j + frame->bytes_pp) {
					paeth_c = output[k-frame->bytes_pp-frame->bytes_pl-1];
				} else {
					paeth_c = 0;
				}

				paeth_p = paeth_a + paeth_b - paeth_c;
				paeth_pa = abs(paeth_p - paeth_a);
				paeth_pb = abs(paeth_p - paeth_b);
				paeth_pc = abs(paeth_p - paeth_c);

				if (paeth_pa <= paeth_pb && paeth_pa <= paeth_pc) {
					output[k] += paeth_a;
				} else if (paeth_pb <= paeth_pc) {
					output[k] += paeth_b;
				} else {
					output[k] += paeth_c;
				}
			}
			break;
		}

		col = 0;

		/* Put together the pixel and output to framebuffer */
		while (col < frame->width) {

			/* Truecolor with alpha, 16 bits per component */
			if (image_header->colour_type == COLOR_TRUE_ALPHA &&
				image_header->bit_depth == 16) {

				frame->output[l] = (output[j+6]<<24 | output[j]<<16 |
					output[j+2]<<8 | output[j+4]);
			}


			/* Truecolor with alpha, 8 bits per component */
			if (image_header->colour_type == COLOR_TRUE_ALPHA &&
				image_header->bit_depth == 8) {

				frame->output[l] = (output[j+3]<<24 | output[j]<<16 |
					output[j+1]<<8 | output[j+2]);
			}

			/* Grayscale with alpha, 16 bits per component */
			if (image_header->colour_type == COLOR_GREY_ALPHA &&
				image_header->bit_depth == 16) {

				frame->output[l] = (output[j+2]<<24 | output[j]<<16 |
					output[j]<<8 | output[j]);
			}

			/* Grayscale with alpha, 8 bits per component */
			if (image_header->colour_type == COLOR_GREY_ALPHA &&
				image_header->bit_depth == 8) {

				frame->output[l] = (output[j+1]<<24 | output[j]<<16 |
					output[j]<<8 | output[j]);

			}

			/* Truecolor, 16 bits per component */
			if (image_header->colour_type == COLOR_TRUE &&
				image_header->bit_depth == 16) {

				if (!image_header->using_transparency ||
					image_header->transparency_r !=
					(output[j] | output[j+1]) ||
					image_header->transparency_g !=
					(output[j+2] | output[j+3]) ||
					image_header->transparency_b !=
					(output[j+4] | output[j+5])) {

					frame->output[l] = (0xFF000000 | output[j]<<16 |
						output[j+2]<<8 | output[j+4]);
				}
			}

			/* Truecolor, 8 bits per component */
			if (image_header->colour_type == COLOR_TRUE &&
				image_header->bit_depth == 8) {

				if (!image_header->using_transparency ||
					image_header->transparency_r != output[j] ||
					image_header->transparency_g != output[j+1] ||
					image_header->transparency_b != output[j+2]) {

					frame->output[l] = (0xFF000000 | (output[j]<<16) |
						(output[j+1]<<8) | (output[j+2]));
				}
			}

			/* Grayscale, 16 bits per component */
			if (image_header->colour_type == COLOR_GREY &&
				image_header->bit_depth == 16) {

				if (!image_header->using_transparency ||
					image_header->transparency_r !=
					(output[j] | output[j+1])) {

					frame->output[l] = (0xFF000000 |(output[j]<<16) |
						(output[j]<<8) | output[j]);
				}
			}

			/* Grayscale, 8 bits per component */
			if (image_header->colour_type == COLOR_GREY &&
				 image_header->bit_depth == 8) {

				if (!image_header->using_transparency ||
					image_header->transparency_r != output[j]) {
					frame->output[l] = (0xFF000000 | (output[j]<<16) |
						(output[j]<<8) | output[j]);
				}
			}

			/* Grayscale, 4 bits per component */
			if (image_header->colour_type == COLOR_GREY &&
				image_header->bit_depth == 4) {

				if (!image_header->using_transparency ||
					image_header->transparency_r != ((output[j] & 0xF0)>>4)) {

					frame->output[l] =
						CONV_GS_4_TO_32((output[j] & 0xF0)>>4);
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != (output[j] & 0x0F)) {

						frame->output[l] =
							CONV_GS_4_TO_32(output[j] & 0x0F);
					}
				}
			}

			/* Grayscale, 2 bits per component */
			if (image_header->colour_type == COLOR_GREY &&
				image_header->bit_depth == 2) {

				if (!image_header->using_transparency ||
					image_header->transparency_r != ((output[j] & 0xC0)>>6)) {

					frame->output[l] =
						CONV_GS_2_TO_32((output[j] & 0xC0) >> 6);
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x30)>>4)) {

						frame->output[l] =
							CONV_GS_2_TO_32((output[j] & 0x30) >> 4);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x0C)>>2)) {

						frame->output[l] =
							CONV_GS_2_TO_32((output[j] & 0x0C) >> 2);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != (output[j] & 0x03)) {

						frame->output[l] =
							CONV_GS_2_TO_32(output[j] & 0x03);
					}
				}
			}

			/* Grayscale, 1 bit per component */
			if (image_header->colour_type == COLOR_GREY &&
				image_header->bit_depth == 1) {

				if (!image_header->using_transparency ||
					image_header->transparency_r != ((output[j] & 0x80)>>7)) {

					frame->output[l] =
						CONV_GS_1_TO_32((output[j] & 0x80) >> 7);
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x40)>>6)) {

						frame->output[l] =
							CONV_GS_1_TO_32((output[j] & 0x40) >> 6);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x20)>>5)) {

						frame->output[l] =
							CONV_GS_1_TO_32((output[j] & 0x20) >> 5);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x10)>>4)) {

						frame->output[l] =
							CONV_GS_1_TO_32((output[j] & 0x10) >> 4);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x08)>>3)) {

						frame->output[l] =
							CONV_GS_1_TO_32((output[j] & 0x08) >> 3);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x04)>>2)) {

						frame->output[l] =
							CONV_GS_1_TO_32((output[j] & 0x04) >> 2);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != ((output[j] & 0x02)>>1)) {

						frame->output[l] =
							CONV_GS_1_TO_32((output[j] & 0x02) >> 1);
					}
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					if (!image_header->using_transparency ||
						image_header->transparency_r != (output[j] & 0x01)) {

						frame->output[l] =
							CONV_GS_1_TO_32(output[j] & 0x01);
					}
				}
			}

			/* Palette, 8 bit per component */
			if (image_header->colour_type == COLOR_INDEXED &&
				image_header->bit_depth == 8) {

				small_color = output[j];
				frame->output[l] = 0xFF000000 |
					image_header->image_palette[small_color];
			}

			/* Palette, 4 bit per component */
			if (image_header->colour_type == COLOR_INDEXED &&
				image_header->bit_depth == 4) {

				small_color = (output[j] & 0xF0) >> 4;
				frame->output[l] = 0xFF000000 |
					image_header->image_palette[small_color];
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = output[j] & 0x0F;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
			}

			/* Palette, 2 bit per component */
			if (image_header->colour_type == COLOR_INDEXED &&
				image_header->bit_depth == 2) {

				small_color = (output[j] & 0xC0) >> 6;
				frame->output[l] = 0xFF000000 |
					image_header->image_palette[small_color];
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = output[j] & 0x30 >> 4;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = output[j] & 0x0C >> 2;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = output[j] & 0x03;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
			}

			/* Palette, 1 bit per component */
			if (image_header->colour_type == COLOR_INDEXED &&
				image_header->bit_depth == 1) {

				small_color = (output[j] & 0x80) >> 7;
				frame->output[l] = 0xFF000000 |
					image_header->image_palette[small_color];

				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = (output[j] & 0x40) >> 6;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = (output[j] & 0x20) >> 5;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = (output[j] & 0x10) >> 4;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = (output[j] & 0x08) >> 3;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = (output[j] & 0x04) >> 2;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = (output[j] & 0x02) >> 1;
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
				if (col + 1 < frame->width) {
					l++;
					col++;
					small_color = (output[j] & 0x01);
					frame->output[l] = 0xFF000000 |
						image_header->image_palette[small_color];
				}
			}

			j += image_header->bytes_pp;
			l++;
			col++;
			if (l > frame->height * frame->width) {
				EMGD_ERROR("l is larger than frame output size!");
				return;
			}
		}
		row++;
	}

	vfree(output);
}

/*
 * This is the function to decompress a huffman tree.
 *
 * @param stream (IN) This is the input data stream from which we are reading.
 * @param iter (IN/OUT) This is the input data stream's char iterator
 * @param bit_iter (IN/OUT) This is the bit iterator for the particular char we
 *                          are reading.
 * @param length_tree (IN) This is the huffman code's length tree.
 * @param distance_tree (IN) This is the huffman code's distance tree.
 * @param output (IN/OUT) This is an output stream to which we write out the
 *                        decompressed huffman data.
 * @param output_iter (IN/OUT) This is the output iterator.
 */
void decompress_huffman(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *bit_iter,
	huffman_node **length_tree,
	huffman_node **distance_tree,
	unsigned char *output,
	unsigned long *output_iter)
{

	unsigned long j,k;
	huffman_node *final_node;
	unsigned long extra_value = 0;
	unsigned long length_value = 0;
	unsigned long distance_value = 0;

	/* Start going along the bitstream and traversing the tree
	 * until you get to a leaf
	 */
	get_huffman_code(stream, iter, bit_iter, length_tree, &final_node);

	while (final_node->value != 256) {

		if (final_node->value < 256){
			/* literal value */
			output[*output_iter] = final_node->value;
			(*output_iter)++;
		}
		if (final_node->value > 256){
			/* We have the initial length value,
			 * now get the extra length bits, if any
			 */
			extra_value = 0;
			length_value = 0;
			for (j=0; j<final_node->extra_bits; j++){
				extra_value = read_bit_from_stream(stream, iter, bit_iter);
				length_value += extra_value << j;
			}
			length_value += final_node->real;

			/* Now its time to get the distance value */
			get_huffman_code(stream, iter, bit_iter, distance_tree, &final_node);

			/* Get any extra bits for the distance value */
			extra_value = 0;
			distance_value = 0;
			for (j=0; j<final_node->extra_bits; j++){
				extra_value = read_bit_from_stream(stream, iter, bit_iter);
				distance_value += extra_value << j;
			}
			distance_value += final_node->real;

			/*
			 * Now we need to use the distance and length values
			 * to copy previously existing values
			 */
			distance_value = (*output_iter) - distance_value;
			for (k=0; k<length_value; k++){
				output[*output_iter] = output[distance_value];
				(*output_iter)++;
				distance_value++;
			}
		}

		/* Get the next code */
		get_huffman_code(stream, iter, bit_iter, length_tree, &final_node);
	}
}


/*
 * This is the function to build a static huffman tree.
 *
 * @param length_tree (OUT) This is the huffman code's length tree.
 * @param distance_tree (OUT) This is the huffman code's distance tree.
 */
void build_static_huffman_tree(
	huffman_node **length_tree,
	huffman_node **distance_tree) {

	huffman_node *new_node = NULL;
	huffman_node *cur_node = NULL;
	unsigned long j,k;
	unsigned long running_literal_value = 0;
	unsigned long running_real_value = 0;

	unsigned long ltree_literal_value[10] =
		{256,265,269,273,277,  0,280,281,285,144};
	unsigned long ltree_real_value[10] =
		{  2, 11, 19, 35, 67,  0,115,131,258,144};
	unsigned long ltree_literal_length[10] =
		{  9,  4,  4,  4,  3,144,  1,  4,  3,112};
	unsigned long ltree_code_start[10] =
		{  0,  9, 13, 17, 21, 48,192,193,197,400};
	unsigned long ltree_code_length[10] =
		{  7,  7,  7,  7,  7,  8,  8,  8,  8,  9};
	unsigned long ltree_extra_bits[10] =
		{  0,  1,  2,  3,  4,  0,  4,  5,  0,  0};

	unsigned long dtree_literal_value[15] =
		{0,4,6, 8,10,12, 14, 16, 18,  20,  22,  24,  26,   28,30};
	unsigned long dtree_real_value[15] =
		{1,5,9,17,33,65,129,257,513,1025,2049,4097,8193,16385, 0};
	unsigned long dtree_literal_length[15] =
		{4,2,2, 2, 2, 2,  2,  2,  2,   2,   2,   2,   2,    2, 2};
	unsigned long dtree_code_start[15] =
		{0,4,6, 8,10,12, 14, 16, 18,  20,  22,  24,  26,   28,30};
	unsigned long dtree_code_length[15] =
		{5,5,5, 5, 5, 5,  5,  5,  5,   5,   5,   5,   5,    5, 5};
	unsigned long dtree_extra_bits[15] =
		{0,1,2, 3, 4, 5,  6,  7,  8,   9,   10,  11, 12,   13, 0};

	/* Build our Huffman length tree using the fixed codes */
	new_node = (huffman_node *)kzalloc(sizeof(huffman_node), GFP_KERNEL);
	if (!new_node) {
		EMGD_ERROR("Out of memory.");
		return;
	}

	*length_tree = new_node;

	for (k=0; k<10; k++){
		running_literal_value = ltree_literal_value[k];
		running_real_value = ltree_real_value[k];
		for (j=0; j<ltree_literal_length[k]; j++) {
			new_node = (huffman_node *)kzalloc(sizeof(huffman_node),GFP_KERNEL);
			if (!new_node) {
				EMGD_ERROR("Out of memory.");
				return;
			}

			new_node->extra_bits = (unsigned char)ltree_extra_bits[k];
			new_node->value = running_literal_value;
			new_node->real = running_real_value;
			cur_node = *length_tree;
			add_node(&cur_node,
					new_node,
					ltree_code_start[k] + j,
					ltree_code_length[k]);
			running_literal_value++;
			if (ltree_extra_bits[k]){
				running_real_value += (1<<ltree_extra_bits[k]);
			}else{
				running_real_value++;
			}
		}
	}

	/* Build our Huffman distance tree using the fixed codes */
	new_node = (huffman_node *)kzalloc(sizeof(huffman_node), GFP_KERNEL);
	if (!new_node) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	*distance_tree = new_node;

	for (k=0; k<15; k++){
		running_literal_value = dtree_literal_value[k];
		running_real_value = dtree_real_value[k];
		for (j=0; j<dtree_literal_length[k]; j++) {
			new_node = (huffman_node *)kzalloc(sizeof(huffman_node),GFP_KERNEL);
			if (!new_node) {
				EMGD_ERROR("Out of memory.");
				return;
			}
			new_node->extra_bits = (unsigned char)dtree_extra_bits[k];
			new_node->value = running_literal_value;
			new_node->real = running_real_value;
			cur_node = *distance_tree;
			add_node(&cur_node,
				new_node,
				dtree_code_start[k] + j,
				dtree_code_length[k]);
			running_literal_value++;
			if (dtree_extra_bits[k]){
				running_real_value += (1<<dtree_extra_bits[k]);
			}else{
				running_real_value++;
			}
		}
	}
}


/*
 * This is the function to build a dynamic huffman tree.
 *
 * @param stream (IN) This is the input data stream from which we are reading.
 * @param iter (IN/OUT) This is the input data stream's char iterator
 * @param bit_iter (IN/OUT) This is the bit iterator for the particular char we
 *                          are reading.
 * @param length_tree (OUT) This is the huffman code's length tree.
 * @param distance_tree (OUT) This is the huffman code's distance tree.
 */
void build_dynamic_huffman_tree(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *bit_iter,
	huffman_node **length_tree,
	huffman_node **distance_tree) {

	unsigned long j,k;
	unsigned long clc_order[19] =
	{16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
	unsigned long clc_lengths[19] =
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned long clc_extra_bits[19] =
	{2,3,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned long clc_values[19] =
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
	huffman_node *code_length_tree = NULL;
	unsigned long dynamic_hlit = 0;
	unsigned long dynamic_hdist = 0;
	unsigned long dynamic_hclen = 0;

	unsigned long lit_extra_bits_num[LEN_NUM_DISTINCT_EXTRA_BITS] =
	{265,4,4,4,4,4,1};
	unsigned long lit_extra_bits[LEN_NUM_DISTINCT_EXTRA_BITS] =
	{0,1,2,3,4,5,0};
	unsigned long *dynamic_lit_code = NULL;
	unsigned long *dynamic_lit_length = NULL;
	unsigned long *dynamic_lit_extra_bits = NULL;
	unsigned long *dynamic_lit_values = NULL;
	unsigned long *dynamic_lit_real_values = NULL;

	unsigned long dist_extra_bits_num[DIST_NUM_DISTINCT_EXTRA_BITS] =
	{4,2,2,2,2,2,2,2,2,2,2,2,2,2};
	unsigned long dist_extra_bits[DIST_NUM_DISTINCT_EXTRA_BITS] =
	{0,1,2,3,4,5,6,7,8,9,10,11,12,13};
	unsigned long *dynamic_dist_code = NULL;
	unsigned long *dynamic_dist_length = NULL;
	unsigned long *dynamic_dist_extra_bits = NULL;
	unsigned long *dynamic_dist_values = NULL;
	unsigned long *dynamic_dist_real_values = NULL;

	unsigned long prev_real = 0;
	unsigned long code_index;
	huffman_node *new_node = NULL;

	/* Read some initial information about our dynamic huffman tree */
	read_bits_from_stream(stream, iter, bit_iter, 5, &dynamic_hlit);
	read_bits_from_stream(stream, iter, bit_iter, 5, &dynamic_hdist);
	read_bits_from_stream(stream, iter, bit_iter, 4, &dynamic_hclen);

	dynamic_hlit += 257;
	dynamic_hdist++;
	dynamic_hclen += 4;

	/* Build our Huffman length tree using the fixed codes */
	new_node = (huffman_node *)kzalloc(sizeof(huffman_node), GFP_KERNEL);
	if (!new_node) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	code_length_tree = new_node;

	/* Get the code lengths */
	for (k=0; k<19 && k<dynamic_hclen; k++){
		read_bits_from_stream(stream,
				iter, bit_iter,	3, &clc_lengths[clc_order[k]]);
	}

	/* build the code_length tree */
	if (create_tree(CLC_MAX_BITS, CLC_NUM_CODES,
				&clc_lengths[0],
				&clc_extra_bits[0],
				&clc_values[0],
				&clc_values[0],
				&code_length_tree) == 1) {
		EMGD_ERROR("ERROR: create tree failed\n");
		return;
	}

	/* Build the literal/length alphabet */
	dynamic_lit_code = (unsigned long *)vmalloc(
			sizeof(unsigned long) * LEN_NUM_CODES);
	if (!dynamic_lit_code) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_lit_code, 0, sizeof(unsigned long) * LEN_NUM_CODES);

	dynamic_lit_length = (unsigned long *)vmalloc(
			sizeof(unsigned long) * LEN_NUM_CODES);
	if (!dynamic_lit_length) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_lit_length, 0, sizeof(unsigned long) * LEN_NUM_CODES);

	dynamic_lit_extra_bits = (unsigned long *)vmalloc(
			sizeof(unsigned long) * LEN_NUM_CODES);
	if (!dynamic_lit_extra_bits) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_lit_extra_bits, 0, sizeof(unsigned long)*LEN_NUM_CODES);

	dynamic_lit_values = (unsigned long *)vmalloc(
			sizeof(unsigned long) * LEN_NUM_CODES);
	if (!dynamic_lit_values) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_lit_values, 0, sizeof(unsigned long)*LEN_NUM_CODES);

	dynamic_lit_real_values = (unsigned long *)vmalloc(
			sizeof(unsigned long) * LEN_NUM_CODES);
	if (!dynamic_lit_real_values) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_lit_real_values, 0, sizeof(unsigned long)*LEN_NUM_CODES);

	/* build extra information, such as extra bits, values and real_values */
	prev_real = 2;
	code_index = 0;
	for (k=0; k<LEN_NUM_DISTINCT_EXTRA_BITS; k++) {
		for (j=0; j<lit_extra_bits_num[k]; j++) {
			dynamic_lit_extra_bits[code_index] = lit_extra_bits[k];
			dynamic_lit_values[code_index] = code_index;

			if (code_index >= LEN_START_REAL_VALUES){
				dynamic_lit_real_values[code_index] =
					prev_real += (1<<dynamic_lit_extra_bits[code_index-1]);
			} else {
				dynamic_lit_real_values[code_index] = code_index;
			}
			code_index++;
		}
	}

	/* Doesn't seem to follow the pattern? */
	dynamic_lit_real_values[285] = 258;

	/* get code lengths for the literal/length alphabet */
	get_code_lengths(stream, iter, bit_iter, &code_length_tree,
			dynamic_hlit, dynamic_lit_length);

	/* allocate tree for literal/length codes */
	new_node = (huffman_node *)kzalloc(sizeof(huffman_node), GFP_KERNEL);
	if (!new_node) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	*length_tree = new_node;

	/* build the literal/length tree */
	if (create_tree(LEN_MAX_BITS, LEN_NUM_CODES,
				dynamic_lit_length,
				dynamic_lit_extra_bits,
				dynamic_lit_values,
				dynamic_lit_real_values,
				length_tree) == 1) {
		EMGD_ERROR("ERROR: create tree failed\n");
		return;
	}

	/* free all the literal/length data we are no longer using */
	vfree(dynamic_lit_code);
	vfree(dynamic_lit_length);
	vfree(dynamic_lit_extra_bits);
	vfree(dynamic_lit_values);
	vfree(dynamic_lit_real_values);


	/* Build the distance alphabet */
	dynamic_dist_code = (unsigned long *)vmalloc(
		sizeof(unsigned long) * DIST_NUM_CODES);
	if (!dynamic_dist_code) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_dist_code, 0, sizeof(unsigned long) * DIST_NUM_CODES);

	dynamic_dist_length = (unsigned long *)vmalloc(
		sizeof(unsigned long) * DIST_NUM_CODES);
	if (!dynamic_dist_length) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_dist_length, 0, sizeof(unsigned long) * DIST_NUM_CODES);

	dynamic_dist_extra_bits = (unsigned long *)vmalloc(
		sizeof(unsigned long) * DIST_NUM_CODES);
	if (!dynamic_dist_extra_bits) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_dist_extra_bits, 0,
		sizeof(unsigned long) * DIST_NUM_CODES);

	dynamic_dist_values = (unsigned long *)vmalloc(
		sizeof(unsigned long) * DIST_NUM_CODES);
	if (!dynamic_dist_values) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_dist_values, 0, sizeof(unsigned long) * DIST_NUM_CODES);

	dynamic_dist_real_values = (unsigned long *)vmalloc(
		sizeof(unsigned long) * DIST_NUM_CODES);
	if (!dynamic_dist_real_values) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	OS_MEMSET(dynamic_dist_real_values, 0,
		sizeof(unsigned long) * DIST_NUM_CODES);

	/* build extra information, such as extra bits, values and real_values */
	prev_real = 1;
	code_index = 0;
	for (k=0; k<DIST_NUM_DISTINCT_EXTRA_BITS; k++) {
		for (j=0; j<dist_extra_bits_num[k]; j++) {
			dynamic_dist_extra_bits[code_index] = dist_extra_bits[k];
			dynamic_dist_values[code_index] = code_index;

			if (code_index >= DIST_START_REAL_VALUES){
				dynamic_dist_real_values[code_index] =
					prev_real += (1<<dynamic_dist_extra_bits[code_index-1]);
			} else {
				dynamic_dist_real_values[code_index] = code_index+1;
			}
			code_index++;
		}
	}

	/* get code lengths for the distance alphabet */
	get_code_lengths(stream, iter, bit_iter, &code_length_tree,
			dynamic_hdist, dynamic_dist_length);

	/* allocate tree for distance codes */
	new_node = (huffman_node *)kzalloc(sizeof(huffman_node), GFP_KERNEL);
	if (!new_node) {
		EMGD_ERROR("Out of memory.");
		return;
	}
	*distance_tree = new_node;

	/* build the distance tree */
	if (create_tree(DIST_MAX_BITS, DIST_NUM_CODES,
				&dynamic_dist_length[0],
				&dynamic_dist_extra_bits[0],
				&dynamic_dist_values[0],
				&dynamic_dist_real_values[0],
				distance_tree) == 1) {
		EMGD_ERROR("ERROR: create tree failed.\n");
		return;
	}

	/* free all the distance data we are no longer using */
	vfree(dynamic_dist_code);
	vfree(dynamic_dist_length);
	vfree(dynamic_dist_extra_bits);
	vfree(dynamic_dist_values);
	vfree(dynamic_dist_real_values);

	/* All done with the code length tree, lets free this memory */
	free_node(code_length_tree);
}


/*
 * This is the function to get the dynamic code lengths for a specified
 * number of code lenths. There is some overuse of the word code and code
 * lengths, but thats sort of the way the PNG spec is.  This is because
 * we use codes to decode compressed codes.
 *
 * @param stream (IN) This is the input data stream from which we are reading.
 * @param iter (IN/OUT) This is the input data stream's char iterator
 * @param bit_iter (IN/OUT) This is the bit iterator for the particular char we
 *                          are reading.
 * @param code_length_tree (IN) This is the huffman code length tree, which is
 *                              used to get the code lengths.
 * @param num_lengths (IN) The number of code lengths.
 * @param dynamic_lengths (OUT) Gets the dynamic length for the different
 *                              code lengths
 */
void get_code_lengths(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *bit_iter,
	huffman_node **code_length_tree,
	unsigned long num_lengths,
	unsigned long *dynamic_lengths) {

	unsigned long j,k;
	huffman_node *final_node;
	unsigned long dynamic_repeat_length = 0;

	/* get code lengths for the literal/length alphabet */
	for (k=0; k<num_lengths; k++) {
		get_huffman_code(stream, iter, bit_iter,
			code_length_tree, &final_node);

		if (final_node->value < 16){
			dynamic_lengths[k] = final_node->value;
		} else {
			switch (final_node->value) {
			case 16:
				/* get repeat length */
				read_bits_from_stream(stream,
					iter, bit_iter, 2, &dynamic_repeat_length);
				dynamic_repeat_length += 3;
				for (j=0; j<dynamic_repeat_length; j++){
					dynamic_lengths[k+j] = dynamic_lengths[k-1];
				}
				k += j-1;
				break;
			case 17:
				/* get repeat length */
				read_bits_from_stream(stream,
					iter, bit_iter, 3, &dynamic_repeat_length);
				dynamic_repeat_length += 3;
				for (j=0; j<dynamic_repeat_length; j++){
					dynamic_lengths[k+j] = 0;
				}
				k += j-1;
				break;
			case 18:
				/* get repeat length */
				read_bits_from_stream(stream,
					iter, bit_iter, 7, &dynamic_repeat_length);
				dynamic_repeat_length += 11;
				for (j=0; j<dynamic_repeat_length; j++){
					dynamic_lengths[k+j] = 0;
				}
				k += j-1;
				break;
			}
		}
	}
}


/*
 * This function creates a tree given the necessary tree information.
 *
 * @param max_bits (IN) The maximum number of bits for any code
 * @param num_codes (IN) The number of codes
 * @param code_lengths (IN) The code lengths
 * @param extra_bits (IN) The number of extra bits for each huffman code
 * @param values (IN) The values for the huffman code
 * @param real_values (IN) The real values for the huffman code
 * @param tree (OUT) The resulting huffman tree.
 *
 * @return 0 on Success
 * @return >0 on Error
 */
int create_tree(
	unsigned long max_bits,
	unsigned long num_codes,
	unsigned long *code_lengths,
	unsigned long *extra_bits,
	unsigned long *values,
	unsigned long *real_values,
	huffman_node **tree) {

	unsigned long *clc_count;
	unsigned long *clc_next_code;
	unsigned long *codes;
	unsigned long clc_code;
	unsigned long k;
	huffman_node *cur_node;
	huffman_node *new_node;

	if (!tree) {
		EMGD_ERROR("Bad tree pointer.");
		return 1;
	}

	/* Step 1: Count the number of codes for each code length */
	clc_count = (unsigned long *)vmalloc(
		sizeof(unsigned long) * (max_bits+1));
    if (!clc_count) {
		EMGD_ERROR("Out of memory.");
		return 1;
    }
    OS_MEMSET(clc_count, 0, sizeof(unsigned long) * (max_bits+1));

	for (k=0; k<num_codes; k++){
		clc_count[code_lengths[k]]++;
	}

	/* Step 2: Get numerical value of smallest code for each code length */
	clc_next_code = (unsigned long *)vmalloc(
		sizeof(unsigned long) * (max_bits+1));
    if (!clc_next_code) {
		EMGD_ERROR("Out of memory.");
		return 1;
    }
    OS_MEMSET(clc_next_code, 0, sizeof(unsigned long) * (max_bits+1));

	clc_code = 0;
	clc_next_code[0] = 2;
	for (k=1; k<=max_bits; k++){
	    clc_code = (clc_code + clc_count[k-1]) << 1;
	    clc_next_code[k] = clc_code;
	}

	/* Step 3: Assign numerical values to all codes */
	codes = (unsigned long *)vmalloc(sizeof(unsigned long) * num_codes);
    if (!codes) {
		EMGD_ERROR("Out of memory.");
		return 1;
    }
    OS_MEMSET(codes, 0, sizeof(unsigned long) * num_codes);

	for (k=0; k<num_codes; k++){
	    if (code_lengths[k] > 0){
			codes[k] = clc_next_code[code_lengths[k]]++;

			/* Add this node to the code length tree */
			new_node = (huffman_node *)kzalloc(sizeof(huffman_node),GFP_KERNEL);
			if (!new_node) {
				EMGD_ERROR("Out of memory.");
				return 1;
			}

			new_node->extra_bits = (unsigned char)extra_bits[k];
			new_node->value = values[k];
			new_node->real = real_values[k];
			cur_node = *tree;
			add_node(&cur_node, new_node, codes[k], code_lengths[k]);
	    }
	}

	vfree(clc_count);
	vfree(clc_next_code);
	vfree(codes);
	return 0;
}


/*
 * This function recursively frees a huffman node and all its sub nodes.
 * First we free any sub nodes, then we free itself.
 *
 * @param node (IN) The huffman node to free.
 */
void free_node(huffman_node *node) {
	if (node->leaf[0]) {
		free_node((huffman_node *)(node->leaf[0]));
	}
	if (node->leaf[1]) {
		free_node((huffman_node *)(node->leaf[1]));
	}
	kfree(node);
}


/*
 * This function gets a huffman code by traversing through a bit
 * stream as if those are directions for traversling through
 * a binary tree.   When we hit a leaf node, we have our value.
 *
 * @param stream (IN) This is the input data stream from which we are reading.
 * @param iter (IN/OUT) This is the input data stream's char iterator
 * @param bit_iter (IN/OUT) This is the bit iterator for the particular char we
 *                          are reading.
 * @param tree (IN) This is the huffman tree.
 * @param final_node (OUT) The final leaf node we have reached.
 */
void get_huffman_code(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *bit_iter,
	huffman_node **tree,
	huffman_node **final_node){

	*final_node = *tree;
	while ((*final_node)->leaf[0] || (*final_node)->leaf[1]) {
		(*final_node) = (huffman_node *)(*final_node)->
			leaf[((stream[*iter] >> *bit_iter) & 1)];

		if (++(*bit_iter) == 8) {
			(*iter)++;
			(*bit_iter) = 0;
		}
	}
}


/*
 * This function adds a node into a tree.
 *
 * @param tree (IN/OUT) This is the tree's root to which we'll be adding a
 *                      node.
 * @param node (IN) This is the node we'll be adding.
 * @param code (IN) This is the code which will be used as a map to determine
 *                  where the new node goes on the tree.
 * @param code_length (IN) This is the code length for the code passed in.
 *
 * @return 0 on Success
 * @return >0 on Error
 */
int add_node(
	huffman_node **tree,
	huffman_node *node,
	unsigned long code,
	unsigned long code_length){

	huffman_node *new_node;

	if (!(*tree)) {
		EMGD_ERROR("Invalid tree pointer.");
		return 1;
	}

	if (code_length > 1){

		/* Build a leaf node if it doesn't exist */
		if (!(*tree)->leaf[(code >> (code_length-1)) & 1]){
			new_node = (huffman_node *)kzalloc(sizeof(huffman_node),GFP_KERNEL);
			if (!new_node) {
				EMGD_ERROR("Out of memory");
				return 1;
			}

			(*tree)->leaf[(code >> (code_length-1)) & 1] =
				(struct huffman_node *)new_node;
			(*tree) = new_node;
		} else {
			(*tree) =
				(huffman_node *)(*tree)->leaf[(code >> (code_length-1)) & 1];
		}

		/* Recursively add the tree node */
		add_node(&(*tree), node, code, --code_length);

	} else {
		/* This is where our leaf node belongs */
		(*tree)->leaf[code & 1] = (struct huffman_node *)node;
	}
	return 0;
}


/*
 * This function reads a 4 byte value from a given stream.
 * This assumes the passed in stream is byte aligned.
 *
 * @param stream (IN) The stream from which we are reading.
 * @param iter (IN/OUT) The stream iterator.
 * @param value (OUT) The value read from the stream.
 *
 * @return 0 on Success
 * @return >0 on Error
 */
int read_int_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned long *value){

	*value = stream[*iter] << 24 |
		stream[(*iter)+1] << 16 |
		stream[(*iter)+2] << 8 |
		stream[(*iter)+3];
	*iter += 4;
	return 0;
}


/*
 * This function reads a 2 byte value from a given stream.
 * This assumes the passed in stream is byte aligned.
 *
 * @param stream (IN) The stream from which we are reading.
 * @param iter (IN/OUT) The stream iterator.
 * @param value (OUT) The value read from the stream.
 *
 * @return 0 on Success
 * @return >0 on Error
 */
int read_short_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned short *value){

	*value = stream[(*iter)] << 8 |
		stream[(*iter)+1];
	*iter += 2;
	return 0;
}


/*
 * This function reads a 1 byte value from a given stream.
 * This assumes the passed in stream is byte aligned.
 *
 * @param stream (IN) The stream from which we are reading.
 * @param iter (IN/OUT) The stream iterator.
 * @param value (OUT) The value read from the stream.
 *
 * @return 0 on Success
 * @return >0 on Error
 */
int read_char_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *value){

	*value = stream[*iter];
	(*iter)++;

	return 0;
}


/*
 * This function reads a given number of bits from a given stream.
 * This does not assume the passed in stream is byte aligned.
 *
 * @param stream (IN) The stream from which we are reading.
 * @param iter (IN/OUT) The stream iterator.
 * @param bit_iter (IN/OUT) The stream's bit iterator.
 * @param num_bits (IN) The number of bits to read.
 * @param value (OUT) The value read from the stream.
 *
 * @return 0 on Success
 * @return >0 on Error
 */
int read_bits_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *bit_iter,
	unsigned long num_bits,
	unsigned long *value){

	unsigned long i;
	*value = 0;

	for (i=0; i<num_bits; i++){
		*value += read_bit_from_stream(stream, iter, bit_iter) << i;
	}

	return 0;
}


/*
 * This function reads a single bit from a given stream.
 * This does not assume the passed in stream is byte aligned.
 *
 * @param stream (IN) The stream from which we are reading.
 * @param iter (IN/OUT) The stream iterator.
 * @param bit_iter (IN/OUT) The stream's bit iterator.
 *
 * @return The bit value read.
 */
unsigned int read_bit_from_stream(
	unsigned char *stream,
	unsigned long *iter,
	unsigned char *bit_iter){

	unsigned int result = 0;

	/* get our bit */
	result = (stream[*iter] >> *bit_iter) & 1;

	/* This is faster than above */
	if (++(*bit_iter) == 8) {
		(*iter)++;
		(*bit_iter) = 0;
	}

	return result;
}
