/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_drv.h
 * $Revision: 1.76 $
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
 *
 *-----------------------------------------------------------------------------
 */
#ifndef _EMGD_DRV_H_
#define _EMGD_DRV_H_

#include <linux/io-mapping.h>
#include <emgd_shared.h>
#include <igd_version.h>
#include "user_config.h"

#define DRIVER_AUTHOR     "Intel Corporation."
#define DRIVER_NAME       EMGD_MODULE_NAME
#define DRIVER_DESC       "Intel Embedded Media and Grahics Driver"
#define DRIVER_DATE       PVR_BUILD_DATE
#define DRIVER_MAJOR      IGD_MAJOR_NUM
#define DRIVER_MINOR      IGD_MINOR_NUM
#define DRIVER_PATCHLEVEL IGD_BUILD_NUM

#define INTELFB_CONN_LIMIT 4

/*
 *  * Special "handle" that indicates the framebuffer being referred to is the
 *   * EMGD initial framebuffer (which does not have a PVR meminfo handle that
 *    * can be passed.
 *     */
#define EMGD_INITIAL_FRAMEBUFFER 0


/* Function prototypes */
extern int emgd_driver_load(struct drm_device *dev, unsigned long flags);
extern int emgd_driver_unload(struct drm_device *dev);
extern int emgd_driver_open(struct drm_device *dev,
		struct drm_file *file_priv);
extern void emgd_driver_lastclose(struct drm_device *dev);
extern void emgd_driver_preclose(struct drm_device *dev,
		struct drm_file *file_priv);
extern void emgd_driver_postclose(struct drm_device *dev,
		struct drm_file *file_priv);
extern int emgd_driver_device_is_agp(struct drm_device * dev);
extern long egd(struct file *filp, unsigned int cmd, unsigned long arg);

extern int emgd_startup_hal(struct drm_device *dev, igd_param_t *params);
int disp_splash_screen(emgd_drm_splash_screen_t *ss_data);
int disp_splash_video(emgd_drm_splash_video_t *sv_data);
extern irqreturn_t emgd_driver_irq_handler(DRM_IRQ_ARGS);
extern void emgd_driver_irq_preinstall(struct drm_device * dev);
extern int emgd_driver_irq_postinstall(struct drm_device *dev);
extern void emgd_driver_irq_uninstall(struct drm_device * dev);
extern int emgd_driver_enable_vblank(struct drm_device *dev, int crtc);
extern void emgd_driver_disable_vblank(struct drm_device *dev, int crtc);
extern u32 emgd_driver_get_vblank_counter(struct drm_device *dev, int crtc);

extern int emgd_driver_suspend(struct drm_device *dev, pm_message_t state);
extern int emgd_driver_resume(struct drm_device *dev);
extern int emgd_mmap(struct file *filp, struct vm_area_struct *vma);


/* Module parameters: */
extern int drm_emgd_configid;



typedef struct drm_device drm_device_t;


/*
 * IOCTL handler function prototypes:
 */
int emgd_alter_cursor(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_alter_cursor_pos(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_alter_displays(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_display_info(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_alter_ovl(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_appcontext_alloc(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_appcontext_free(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_driver_save_restore(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_enable_port(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_attrs(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_display(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
extern int emgd_alter_ovl2(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
extern int emgd_get_ovl_init_params(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_drm_config(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_EDID_block(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_EDID_info(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_pixelformats(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_port_info(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_gmm_alloc_region(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_gmm_alloc_surface(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_gmm_get_num_surface(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_gmm_get_surface_list(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_gmm_free(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_gmm_flush_cache(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_pan_display(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_power_display(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_pwr_alter(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_query_dc(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_query_max_size_ovl(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_query_ovl(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_query_mode_list(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_set_attrs(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_set_palette_entry(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_set_surface(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_dihclone_set_surface(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_sync(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_driver_pre_init(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_driver_get_ports(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_start_pvrsrv(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_test_pvrsrv(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_chipset_info(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_video_cmd_buf(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_device_info(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_page_list(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_init_video(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_video_get_info(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_video_flush_tlb(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_preinit_mmu(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_get_golden_htotal(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_control_plane_format(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_set_overlay_display(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_query_2d_caps_hwhint(struct drm_device *dev, void *arg,
	struct drm_file *file_priv);
int emgd_unlock_planes(struct drm_device *dev, void *arg,
    struct drm_file *file_priv);
/* For Buffer Class of Texture Stream */
int emgd_bc_ts_cmd_init(struct drm_device *dev, void *arg, struct drm_file *file_priv);
int emgd_bc_ts_cmd_uninit(struct drm_device *dev, void *arg, struct drm_file *file_priv);
int emgd_bc_ts_cmd_request_buffers(struct drm_device *dev, void *arg, struct drm_file *file_priv);
int emgd_bc_ts_cmd_release_buffers(struct drm_device *dev, void *arg, struct drm_file *file_priv);
int emgd_bc_ts_set_buffer_info(struct drm_device *dev, void *arg, struct drm_file *file_priv);
int emgd_bc_ts_get_buffers_count(struct drm_device *dev, void *arg, struct drm_file *file_priv);
int emgd_bc_ts_get_buffer_index(struct drm_device *dev, void *arg, struct drm_file *file_priv);
#endif
