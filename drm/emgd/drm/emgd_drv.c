/*
 *-----------------------------------------------------------------------------
 * Filename: emgd_drv.c
 * $Revision: 1.147 $
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
 *  The main part of the kernel module.  This part gets everything going and
 *  connected, and then the rest can function.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.oal

#include <drm/drmP.h>
#include <drm/drm.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <linux/version.h>
#include <linux/device.h>
#include <drm/drm_pciids.h>
#include <intelpci.h>
#include "drm_emgd_private.h"
#include "user_config.h"
#include "emgd_drv.h"
#include "emgd_drm.h"
#include "memory.h"
#include "io.h"
#include "mode_dispatch.h"
#include "igd_debug.h"
#include "splash_screen.h"
#include "msvdx.h"
/*
 * Imagination includes.
 */
#include <img_types.h>
#include <pvr_drm.h>
#include <pvr_drm_shared.h>
#include <pvr_bridge.h>
#include <linkage.h>
#include <sysconfig.h>

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
#include <linux/module.h>
#include <linux/export.h>
#endif
/* For Buffer Class of Texture Stream*/
/* pvr/services4/3rdparty/emgd_bufferclass/emgd_bc_linux.c */
extern int emgd_bc_ts_init(void);
extern int emgd_bc_ts_uninit(void);

/*------------------------------------------------------------------------------
 * Formal Declaration
 *------------------------------------------------------------------------------
 */
extern void emgd_set_real_handle(igd_driver_h drm_handle);
extern void emgd_set_real_dispatch(igd_dispatch_t *drm_dispatch);
extern void emgd_modeset_init(struct drm_device *dev);
extern void emgd_modeset_destroy(struct drm_device *dev);
extern int  msvdx_pre_init_plb(struct drm_device *dev);
extern int msvdx_shutdown_plb(igd_context_t *context);
extern int topaz_shutdown_tnc(igd_context_t *context);

extern emgd_drm_config_t config_drm;
extern int context_count;
#ifdef SUPPORT_V2G_CAMERA
/* V2G Camera Module Exported API */
extern int v2g_start_camera();
#endif

/* This must be defined whether debug or release build */
igd_debug_t emgd_debug_flag = {
	{
		CONFIG_DEBUG_FLAGS
	}
};
igd_debug_t *emgd_debug = &emgd_debug_flag;

#ifdef DEBUG_BUILD_TYPE

MODULE_PARM_DESC(debug_cmd, "Debug: cmd");
module_param_named(debug_cmd, emgd_debug_flag.hal.cmd, short, 0600);

MODULE_PARM_DESC(debug_dsp, "Debug: dsp");
module_param_named(debug_dsp, emgd_debug_flag.hal.dsp, short, 0600);

MODULE_PARM_DESC(debug_mode, "Debug: mode");
module_param_named(debug_mode, emgd_debug_flag.hal.mode, short, 0600);

MODULE_PARM_DESC(debug_init, "Debug: init");
module_param_named(debug_init, emgd_debug_flag.hal.init, short, 0600);

MODULE_PARM_DESC(debug_overlay, "Debug: overlay");
module_param_named(debug_overlay, emgd_debug_flag.hal.overlay, short, 0600);

MODULE_PARM_DESC(debug_power, "Debug: power");
module_param_named(debug_power, emgd_debug_flag.hal.power, short, 0600);

MODULE_PARM_DESC(debug_2D, "Debug: 2D");
module_param_named(debug_2D, emgd_debug_flag.hal._2d, short, 0600);

MODULE_PARM_DESC(debug_blend, "Debug: blend");
module_param_named(debug_blend, emgd_debug_flag.hal.blend, short, 0600);

MODULE_PARM_DESC(debug_state, "Debug: state");
module_param_named(debug_state, emgd_debug_flag.hal.state, short, 0600);

MODULE_PARM_DESC(debug_gmm, "Debug: GMM");
module_param_named(debug_gmm, emgd_debug_flag.hal.gmm, short, 0600);

MODULE_PARM_DESC(debug_gart, "Debug: GART");
module_param_named(debug_gart, emgd_debug_flag.hal.gart, short, 0600);

MODULE_PARM_DESC(debug_oal, "Debug: OAL");
module_param_named(debug_oal, emgd_debug_flag.hal.oal, short, 0600);

MODULE_PARM_DESC(debug_intr, "Debug: intr");
module_param_named(debug_intr, emgd_debug_flag.hal.intr, short, 0600);

MODULE_PARM_DESC(debug_dpd, "Debug: dpd");
module_param_named(debug_dpd, emgd_debug_flag.hal.dpd, short, 0600);

MODULE_PARM_DESC(debug_video, "Debug: video");
module_param_named(debug_video, emgd_debug_flag.hal.video, short, 0600);

MODULE_PARM_DESC(debug_pvr3dd, "Debug: PVR3DD");
module_param_named(debug_pvr3dd, emgd_debug_flag.hal.pvr3dd, short, 0600);

MODULE_PARM_DESC(debug_trace, "Global Debug: trace");
module_param_named(debug_trace, emgd_debug_flag.hal.trace, short, 0600);

MODULE_PARM_DESC(debug_instr, "Global Debug: instr");
module_param_named(debug_instr, emgd_debug_flag.hal.instr, short, 0600);

MODULE_PARM_DESC(debug_debug, "Global Debug: Debug with no associated module ");
module_param_named(debug_debug, emgd_debug_flag.hal.debug, short, 0600);

MODULE_PARM_DESC(debug_blend_stats, "Verbose Debug: Blend stats");
module_param_named(debug_blend_stats, emgd_debug_flag.hal.blend_stats, short, 0600);

MODULE_PARM_DESC(debug_dump_overlay_regs, "Verbose Debug: Dump overlay regs");
module_param_named(debug_dump_overlay_regs, emgd_debug_flag.hal.dump_overlay_regs, short, 0600);

MODULE_PARM_DESC(debug_dump_command_queue, "Verbose Debug: dump command queue");
module_param_named(debug_dump_command_queue, emgd_debug_flag.hal.dump_command_queue, short, 0600);

MODULE_PARM_DESC(debug_dump_gmm_on_fail, "Verbose Debug: dump gmm on fail");
module_param_named(debug_dump_gmm_on_fail, emgd_debug_flag.hal.dump_gmm_on_fail, short, 0600);

MODULE_PARM_DESC(debug_dump_shaders, "Verbose Debug: dump shaders");
module_param_named(debug_dump_shaders, emgd_debug_flag.hal.dump_shaders, short, 0600);

MODULE_PARM_DESC(debug_bc_ts, "Debug: Texture Stream");
module_param_named(debug_bc_ts, emgd_debug_flag.hal.buf_class, short, 0600);
#endif


static struct drm_driver driver;  /* TODO: what? */

/* Note: The following module paramter values are advertised to the root user,
 * via the files in the /sys/module/emgd/parameters directory (e.g. the "init"
 * file contains the value of the "init" module parameter), and so we keep
 * these values up to date.
 *
 * Note: The initial values are all set to -1, so that we can tell if the user
 * set them.
 */
int drm_emgd_portorder[IGD_MAX_PORTS] = {-1, -1, -1, -1, -1};
int drm_emgd_numports;
int drm_emgd_configid = -1;
int drm_emgd_init = -1;
int drm_emgd_dc = -1;
int drm_emgd_width = -1;
int drm_emgd_height = -1;
int drm_emgd_refresh = -1;
MODULE_PARM_DESC(portorder, "Display port order (e.g. \"4,2,0,0,0\")");
MODULE_PARM_DESC(configid, "Which defined configuration number to use (e.g. "
	"\"1\")");
MODULE_PARM_DESC(init, "Whether to initialize the display at startup (1=yes, "
	"0=no)");
MODULE_PARM_DESC(dc, "Display configuration (i.e. 1=single, 2=clone)");
MODULE_PARM_DESC(width, "Display resolution's width (e.g. \"1024\")");
MODULE_PARM_DESC(height, "Display resolution's height (e.g. \"768\")");
MODULE_PARM_DESC(refresh, "Monitor refresh rate (e.g. 60, as in 60Hz)");
module_param_array_named(portorder, drm_emgd_portorder, int, &drm_emgd_numports,
	0600);
module_param_named(configid, drm_emgd_configid, int, 0600);
module_param_named(init, drm_emgd_init, int, 0600);
module_param_named(dc, drm_emgd_dc, int, 0600);
module_param_named(width, drm_emgd_width, int, 0600);
module_param_named(height, drm_emgd_height, int, 0600);
module_param_named(refresh, drm_emgd_refresh, int, 0600);


/** The DC to use when the DRM module [re-]initializes the display. */
static unsigned long *desired_dc = NULL;
/** The DC to use when the DRM module [re-]initializes the display. */
static unsigned short port_number = 0;
/** The mode to use when the DRM module [re-]initializes the display. */
static igd_display_info_t *desired_mode = NULL;
/** Set to true when we know X is initialized. This flag should be set
 ** earlier, but right now we're setting it in emgd_driver_preclose() */
unsigned x_started = false;

/**
 * The primary fb_info to use when the DRM module [re-]initializes the display.
 */
igd_framebuffer_info_t *primary_fb_info;
/**
 * The secondary fb_info to use when the DRM module [re-]initializes the
 * display.
 */
igd_framebuffer_info_t *secondary_fb_info;
/**
 * The primary display structure (filled in by alter_displays()) to use when
 * the DRM module [re-]initializes the display.
 */
igd_display_h primary;
/**
 * The secondary display structure (filled in by alter_displays()) to use when
 * the DRM module [re-]initializes the display.
 */
igd_display_h secondary;
/**
 * The display information to use when the DRM module [re-]initializes the
 * display.
 */
igd_display_info_t pt_info;
extern mode_context_t mode_context[1];

/* Note: This macro is #define'd in "oal/os/memory.h" */
#ifdef INSTRUMENT_KERNEL_ALLOCS
os_allocd_mem *list_head = NULL;
os_allocd_mem *list_tail = NULL;
#endif


#define emgd_PCI_IDS \
	{0x8086, 0x8108, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_PSB_8108}, \
    {0x8086, 0x8109, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_PSB_8109}, \
    {0x8086, 0x4108, PCI_ANY_ID, PCI_ANY_ID, 0, 0, CHIP_TC_4108}, \
    {0, 0, 0}

static struct pci_device_id pciidlist[] = {
	    emgd_PCI_IDS
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)

/*
 * To use DRM_IOCTL_DEF, the first arg should be the local (zero based)
 * IOCTL number, not the global number.
 */
#define EMGD_IOCTL_DEF(ioctl, _func, _flags) \
 [DRM_IOCTL_NR(ioctl) - DRM_COMMAND_BASE] = \
 {.cmd = ioctl, .func = _func, .flags = _flags}

#else

/*
 * To use DRM_IOCTL_DEF_DRV, the first arg should be the local (zero
 * based) IOCTL number, not the global number.
 */
#define EMGD_IOCTL_DEF_DRV(ioctl, _func, _flags) \
	[DRM_IOCTL_NR(DRM_IOCTL_##ioctl) - DRM_COMMAND_BASE] = \
	{.cmd = DRM_##ioctl, .func = _func, .flags = _flags, .cmd_drv = DRM_IOCTL_##ioctl, .name = #ioctl}

#endif

static struct drm_ioctl_desc emgd_ioctl[] = {
	/*
	 * NOTE: The flag "DRM_MASTER" for the final parameter indicates an ioctl
	 * can only be used by the DRM master process.  In an X environment, the
	 * X server will be the master, in a Wayland environment, the Wayland
	 * compositor will be master, and if just running standalone GBM apps,
	 * they'll gain master.  For ioctl's that we want to run from an X
	 * client app or a Wayland client app, we instead use DRM_AUTH; these
	 * clients will get the DRM master to authenticate them, after which
	 * they'll be able to call the ioctl's.  Random programs that haven't
	 * authenticated with the DRM master won't be able to call them.
	 *
	 * For all private EMGD ioctl's added declaration DRM_UNLOCKED,
	 * now ioctl's can run in parallel. Before it, without declaration
	 * DRM_UNLOCKED private EMGD ioctl's can run/work in serial mode only,
	 * one by one.
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_ALTER_CURSOR, emgd_alter_cursor,
		DRM_MASTER|DRM_UNLOCKED),
    EMGD_IOCTL_DEF(DRM_IOCTL_IGD_ALTER_CURSOR_POS, emgd_alter_cursor_pos,
		DRM_MASTER|DRM_UNLOCKED),
    EMGD_IOCTL_DEF(DRM_IOCTL_IGD_ALTER_DISPLAYS, emgd_alter_displays,
		DRM_MASTER|DRM_UNLOCKED),
    EMGD_IOCTL_DEF(DRM_IOCTL_IGD_ALTER_OVL, emgd_alter_ovl, DRM_MASTER|DRM_UNLOCKED),
#else
	EMGD_IOCTL_DEF_DRV(IGD_ALTER_CURSOR, emgd_alter_cursor,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_ALTER_CURSOR_POS, emgd_alter_cursor_pos,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_ALTER_DISPLAYS, emgd_alter_displays,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_ALTER_OVL, emgd_alter_ovl, DRM_MASTER|DRM_UNLOCKED),
#endif
	/* Making DRM_IOCTL_IGD_ALTER_OVL2 DRM_AUTH so that libva wayland can 
	 * call alter_ovl without going through X server.
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_ALTER_OVL2, emgd_alter_ovl2, DRM_AUTH|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_APPCTX_ALLOC, emgd_appcontext_alloc,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_APPCTX_FREE, emgd_appcontext_free,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_DRIVER_SAVE_RESTORE, emgd_driver_save_restore,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_ENABLE_PORT, emgd_enable_port, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_ATTRS, emgd_get_attrs, DRM_MASTER|DRM_UNLOCKED),
#else
	EMGD_IOCTL_DEF_DRV(IGD_ALTER_OVL2, emgd_alter_ovl2, DRM_AUTH|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_APPCTX_ALLOC, emgd_appcontext_alloc,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_APPCTX_FREE, emgd_appcontext_free,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_DRIVER_SAVE_RESTORE, emgd_driver_save_restore,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_ENABLE_PORT, emgd_enable_port, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_ATTRS, emgd_get_attrs, DRM_MASTER|DRM_UNLOCKED),
#endif
	/* Making DRM_IOCTL_IGD_GET_DISPLAY DRM_AUTH so that libva wayland can
	 * obtain the display handle without going through x server.
	 */ 
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_DISPLAY, emgd_get_display, DRM_AUTH|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_DRM_CONFIG, emgd_get_drm_config,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_EDID_BLOCK, emgd_get_EDID_block,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_EDID_INFO, emgd_get_EDID_info,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_PIXELFORMATS, emgd_get_pixelformats,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_PORT_INFO, emgd_get_port_info,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GMM_ALLOC_REGION, emgd_gmm_alloc_region,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GMM_ALLOC_SURFACE, emgd_gmm_alloc_surface,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GMM_GET_NUM_SURFACE, emgd_gmm_get_num_surface,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GMM_GET_SURFACE_LIST,emgd_gmm_get_surface_list,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GMM_FREE, emgd_gmm_free,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GMM_FLUSH_CACHE, emgd_gmm_flush_cache,
		DRM_MASTER|DRM_UNLOCKED),
#else
    EMGD_IOCTL_DEF_DRV(IGD_GET_DISPLAY, emgd_get_display, DRM_AUTH|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_DRM_CONFIG, emgd_get_drm_config,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_EDID_BLOCK, emgd_get_EDID_block,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_EDID_INFO, emgd_get_EDID_info,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_PIXELFORMATS, emgd_get_pixelformats,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_PORT_INFO, emgd_get_port_info,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GMM_ALLOC_REGION, emgd_gmm_alloc_region,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GMM_ALLOC_SURFACE, emgd_gmm_alloc_surface,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GMM_GET_NUM_SURFACE, emgd_gmm_get_num_surface,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GMM_GET_SURFACE_LIST,emgd_gmm_get_surface_list,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GMM_FREE, emgd_gmm_free,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GMM_FLUSH_CACHE, emgd_gmm_flush_cache,
		DRM_MASTER|DRM_UNLOCKED),
#endif
	/*
	 * Externally handled IOCTL's. These are routed to the Imagination Tech
	 * kernel services.
	 *   function prototypes in services4/srvkm/env/linux/pvr_drm.h
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_RESERVED_1, PVRSRV_BridgeDispatchKM, DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_RESERVED_2, PVRDRM_Dummy_ioctl, DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_RESERVED_3, PVRDRM_Dummy_ioctl, DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_RESERVED_4, PVRDRMIsMaster, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_RESERVED_5, PVRDRMUnprivCmd, DRM_UNLOCKED),
	
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_PAN_DISPLAY, emgd_pan_display,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_POWER_DISPLAY, emgd_power_display,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_PWR_ALTER, emgd_pwr_alter, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_QUERY_DC, emgd_query_dc,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_QUERY_MAX_SIZE_OVL, emgd_query_max_size_ovl,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_QUERY_OVL, emgd_query_ovl, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_QUERY_MODE_LIST, emgd_query_mode_list,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_GOLDEN_HTOTAL, emgd_get_golden_htotal,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_CONTROL_PLANE_FORMAT, emgd_control_plane_format,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_SET_OVERLAY_DISPLAY, emgd_set_overlay_display,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF(DRM_IOCTL_IGD_QUERY_2D_CAPS_HWHINT, emgd_query_2d_caps_hwhint,
		DRM_MASTER|DRM_UNLOCKED),
#else
	EMGD_IOCTL_DEF_DRV(IGD_RESERVED_1, PVRSRV_BridgeDispatchKM, DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_RESERVED_2, PVRDRM_Dummy_ioctl, DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_RESERVED_3, PVRDRM_Dummy_ioctl, DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_RESERVED_4, PVRDRMIsMaster, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_RESERVED_5, PVRDRMUnprivCmd, DRM_UNLOCKED),

	EMGD_IOCTL_DEF_DRV(IGD_PAN_DISPLAY, emgd_pan_display,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_POWER_DISPLAY, emgd_power_display,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_PWR_ALTER, emgd_pwr_alter, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_QUERY_DC, emgd_query_dc,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_QUERY_MAX_SIZE_OVL, emgd_query_max_size_ovl,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_QUERY_OVL, emgd_query_ovl, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_QUERY_MODE_LIST, emgd_query_mode_list,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_GOLDEN_HTOTAL, emgd_get_golden_htotal,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_CONTROL_PLANE_FORMAT, emgd_control_plane_format,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_SET_OVERLAY_DISPLAY, emgd_set_overlay_display,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_QUERY_2D_CAPS_HWHINT, emgd_query_2d_caps_hwhint,
		DRM_MASTER|DRM_UNLOCKED),
#endif
	/*
	 * For PDUMP
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
#if defined(PDUMP)
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_RESERVED_6, dbgdrv_ioctl, 0),
#else
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_RESERVED_6, NULL, 0),
#endif
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_SET_ATTRS, emgd_set_attrs, DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_SET_PALETTE_ENTRY, emgd_set_palette_entry,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_SET_SURFACE, emgd_set_surface, DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_SYNC, emgd_sync, DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_DRIVER_PRE_INIT, emgd_driver_pre_init,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_DRIVER_GET_PORTS, emgd_driver_get_ports,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_OVL_INIT_PARAMS, emgd_get_ovl_init_params,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_PAGE_LIST, emgd_get_page_list,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_START_PVRSRV, emgd_start_pvrsrv,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_TEST_PVRSRV, emgd_test_pvrsrv,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_CHIPSET_INFO, emgd_get_chipset_info,
                DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_DIHCLONE_SET_SURFACE, emgd_dihclone_set_surface, DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_PREINIT_MMU, emgd_preinit_mmu, DRM_MASTER|DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_UNLOCK_PLANES, emgd_unlock_planes, DRM_MASTER|DRM_UNLOCKED),
#else
#if defined(PDUMP)
	EMGD_IOCTL_DEF_DRV(IGD_RESERVED_6, dbgdrv_ioctl, 0),
#else
	EMGD_IOCTL_DEF_DRV(IGD_RESERVED_6, NULL, 0),
#endif
	EMGD_IOCTL_DEF_DRV(IGD_SET_ATTRS, emgd_set_attrs, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_SET_PALETTE_ENTRY, emgd_set_palette_entry,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_SET_SURFACE, emgd_set_surface, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_SYNC, emgd_sync, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_DRIVER_PRE_INIT, emgd_driver_pre_init,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_DRIVER_GET_PORTS, emgd_driver_get_ports,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_OVL_INIT_PARAMS, emgd_get_ovl_init_params,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_PAGE_LIST, emgd_get_page_list,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_START_PVRSRV, emgd_start_pvrsrv,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_TEST_PVRSRV, emgd_test_pvrsrv,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_CHIPSET_INFO, emgd_get_chipset_info,
		DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_DIHCLONE_SET_SURFACE, emgd_dihclone_set_surface, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_PREINIT_MMU, emgd_preinit_mmu, DRM_MASTER|DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_UNLOCK_PLANES, emgd_unlock_planes, DRM_MASTER|DRM_UNLOCKED),
#endif
	/*
	 * For VIDEO (MSVDX/TOPAZ
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_VIDEO_CMD_BUF, emgd_video_cmd_buf,
                         DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_INIT_VIDEO, emgd_init_video,
                         DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_GET_DEVICE_INFO, emgd_get_device_info,
                         DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_VIDEO_GET_INFO, emgd_video_get_info,
                         DRM_UNLOCKED),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_VIDEO_FLUSH_TLB, emgd_video_flush_tlb,
                         DRM_UNLOCKED),
  
        /* For Buffer Class of Texture Stream */
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_BC_TS_INIT, emgd_bc_ts_cmd_init,
                        DRM_AUTH),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_BC_TS_UNINIT, emgd_bc_ts_cmd_uninit,
                        DRM_AUTH),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_BC_TS_REQUEST_BUFFERS, emgd_bc_ts_cmd_request_buffers,
                        DRM_AUTH),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_BC_TS_RELEASE_BUFFERS, emgd_bc_ts_cmd_release_buffers,
                        DRM_AUTH),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_BC_TS_SET_BUFFER_INFO, emgd_bc_ts_set_buffer_info,
                        DRM_AUTH),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_BC_TS_GET_BUFFERS_COUNT, emgd_bc_ts_get_buffers_count,
                        DRM_AUTH),
        EMGD_IOCTL_DEF(DRM_IOCTL_IGD_BC_TS_GET_BUFFER_INDEX, emgd_bc_ts_get_buffer_index,
                        DRM_AUTH),
#else
	EMGD_IOCTL_DEF_DRV(IGD_VIDEO_CMD_BUF, emgd_video_cmd_buf,
			DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_INIT_VIDEO, emgd_init_video,
			DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_GET_DEVICE_INFO, emgd_get_device_info,
			DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_VIDEO_GET_INFO, emgd_video_get_info,
			DRM_UNLOCKED),
	EMGD_IOCTL_DEF_DRV(IGD_VIDEO_FLUSH_TLB, emgd_video_flush_tlb,
			DRM_UNLOCKED),

	/* For Buffer Class of Texture Stream */
	EMGD_IOCTL_DEF_DRV(IGD_BC_TS_INIT, emgd_bc_ts_cmd_init,
		DRM_AUTH),
	EMGD_IOCTL_DEF_DRV(IGD_BC_TS_UNINIT, emgd_bc_ts_cmd_uninit,
		DRM_AUTH),
	EMGD_IOCTL_DEF_DRV(IGD_BC_TS_REQUEST_BUFFERS, emgd_bc_ts_cmd_request_buffers,
		DRM_AUTH),
	EMGD_IOCTL_DEF_DRV(IGD_BC_TS_RELEASE_BUFFERS, emgd_bc_ts_cmd_release_buffers,
		DRM_AUTH),
	EMGD_IOCTL_DEF_DRV(IGD_BC_TS_SET_BUFFER_INFO, emgd_bc_ts_set_buffer_info,
		DRM_AUTH),
	EMGD_IOCTL_DEF_DRV(IGD_BC_TS_GET_BUFFERS_COUNT, emgd_bc_ts_get_buffers_count,
		DRM_AUTH),
	EMGD_IOCTL_DEF_DRV(IGD_BC_TS_GET_BUFFER_INDEX, emgd_bc_ts_get_buffer_index,
		DRM_AUTH),
#endif
};

static int emgd_max_ioctl = DRM_ARRAY_SIZE(emgd_ioctl);



/*
 * NOTE: The next part of this file are EMGD-specific DRM functions, exported to
 * the generic DRM code via the drm_driver struct:
 */


extern igd_driver_h handle;
/** This is the dispatch table for the HAL.  It is cached for quick access. */
extern igd_dispatch_t *dispatch;


/**
 * The driver handle for talking with the HAL, within the DRM/kernel code.
 *
 * This is a "real handle" as opposed to the "fake handle" in user-space.
 * Notice that there's only one handle, as the secondary device shares this
 * handle.
 */
static igd_driver_h drm_HAL_handle = NULL;

/**
 * This is the drm_HAL_handle cast to an igd_context_t.  It is cached for quick
 * access.
 */
static igd_context_t *drm_HAL_context = NULL;

/**
 * This is the dispatch table for the HAL.  It is cached for quick access.
 */
static igd_dispatch_t *drm_HAL_dispatch = NULL;


int emgd_get_display_handle(void **display_handle, int screen_number)
{
	emgd_drm_get_display_t drm_data;
	struct drm_file file_priv;
	int ret;
	drm_emgd_priv_t *priv = NULL; 
	igd_display_pipe_t *primary_pipe = NULL;
	igd_display_pipe_t *secondary_pipe = NULL;
	igd_plane_t *primary_plane = NULL;
	igd_plane_t *secondary_plane = NULL;
	unsigned long pipe_num = 0;
	unsigned long port_num = 0;

	EMGD_TRACE_ENTER;

	memset(&drm_data, 0, sizeof(emgd_drm_get_display_t));
	memset(&file_priv, 0, sizeof(struct drm_file));

	drm_HAL_context->mod_dispatch.dsp_get_planes_pipes(&primary_plane, &secondary_plane, &primary_pipe, &secondary_pipe);

	priv = (drm_emgd_priv_t *)((struct drm_device *)drm_HAL_context->drm_dev)->dev_private;
	
	if (0 == screen_number) {
		if (NULL == priv->primary || NULL == primary_pipe) {
			EMGD_ERROR("Primary Display & Pipe does not exist!");
			return 1;
		}	
		port_num = priv->primary_port_number;
		pipe_num = primary_pipe->pipe_num;
	} else {
		if (NULL == priv->secondary || NULL == secondary_pipe ) {
			EMGD_ERROR("Secondary Display does not exist!");
			return 1;
		}
		port_num = priv->secondary_port_number;
		pipe_num = secondary_pipe->pipe_num;
	}

	drm_data.port_number = port_num;
	
	ret = emgd_get_display(drm_HAL_context->drm_dev, (void *)&drm_data, &file_priv);

	/* set the requested display handle for the caller */
	*display_handle = ret ? NULL : (void *)drm_data.display_handle;
	if (NULL != *display_handle) {
		PIPE(*display_handle)->pipe_num = pipe_num;
		EMGD_DEBUG("port_num: %lu, pipe_number: %lu", port_num, pipe_num);
	}

	EMGD_TRACE_EXIT;
	return ret;
}
EXPORT_SYMBOL(emgd_get_display_handle);

int emgd_get_screen_size(int screen_num, unsigned short *width, unsigned short *height)
{
	EMGD_TRACE_ENTER;

	if (NULL == width || NULL == height) {
		return 1;
	}
	
	*width = (unsigned short)config_drm.width;
	*height = (unsigned short)config_drm.height;

	EMGD_TRACE_EXIT;
	return 0;
}
EXPORT_SYMBOL(emgd_get_screen_size);
/*!
 * get_pre_driver_info
 *
 * Gets the mode information before the user-mode driver changes it.
 * This information can either come from the firmware or the DRM.
 * Note:  Prior to EMGD this information can only come from the firmware
 *        thus the field "fw_info."  This should really be changed
 *        to "pre_drv_info"
 *
 * @param mode_context This is where fw_info is stored
 *
 * @return -IGD_INVAL on failure
 * @return 0 on success
 */
static int get_pre_driver_info(mode_context_t *mode_context)
{
	int ret = 0;
	int seamless = FALSE;
	EMGD_TRACE_ENTER;

	if(mode_context->fw_info != NULL) {
		seamless = TRUE;

		ret = mode_context->dispatch->full->get_plane_info();
		if(ret) {
			seamless = FALSE;
		}

		ret = mode_context->dispatch->full->get_pipe_info(primary);
		if(ret) {
			seamless = FALSE;
		}

		ret = mode_context->dispatch->full->get_port_info();
		if(ret) {
			seamless = FALSE;
		}

	}

	if(seamless == FALSE) {
		/* If one of these plane/pipe/port functions
		 *  returns an error, we explicitly
		 *  turn-off seamless.
		 */
		 mode_context->seamless = FALSE;

	}
	EMGD_TRACE_EXIT;
	return 0;
}



/**
 * A helper function that prints the igd_param_t struct.
 *
 * @param params (IN) The the igd_param_t struct to print
 */
void emgd_print_params(igd_param_t *params)
{
	int i;

	EMGD_DEBUG("Values of params:");
	EMGD_DEBUG(" page_request = %lu = 0x%lx", params->page_request,
		params->page_request);
	EMGD_DEBUG(" max_fb_size = %lu = 0x%lx", params->max_fb_size,
		params->max_fb_size);
	EMGD_DEBUG(" preserve_regs = %u", params->preserve_regs);
	EMGD_DEBUG(" display_flags = %lu = 0x%lx", params->display_flags,
		params->display_flags);
	EMGD_DEBUG(" port_order:");
	for (i = 0 ; i < IGD_MAX_PORTS; i++) {
		EMGD_DEBUG("  port number %d = %lu", i, params->port_order[i]);
	}
	EMGD_DEBUG(" display_params:");
	for (i = 0 ; i < IGD_MAX_PORTS; i++) {
		int j;

		EMGD_DEBUG("  port_number = %lu",
			params->display_params[i].port_number);
		EMGD_DEBUG("   present_params = %lu = 0x%lx",
			params->display_params[i].present_params,
			params->display_params[i].present_params);
		EMGD_DEBUG("   flags = %lu = 0x%lx",
			params->display_params[i].flags,
			params->display_params[i].flags);
		EMGD_DEBUG("   edid_avail = %u = 0x%x",
			params->display_params[i].edid_avail,
			params->display_params[i].edid_avail);
		EMGD_DEBUG("   edid_not_avail = %u = 0x%x",
			params->display_params[i].edid_not_avail,
			params->display_params[i].edid_not_avail);
		EMGD_DEBUG("   ddc_gpio = %lu", params->display_params[i].ddc_gpio);
		EMGD_DEBUG("   ddc_speed = %lu", params->display_params[i].ddc_speed);
		EMGD_DEBUG("   ddc_dab = %lu", params->display_params[i].ddc_dab);
		EMGD_DEBUG("   i2c_gpio = %lu", params->display_params[i].i2c_gpio);
		EMGD_DEBUG("   i2c_speed = %lu", params->display_params[i].i2c_speed);
		EMGD_DEBUG("   i2c_dab = %lu", params->display_params[i].i2c_dab);
		EMGD_DEBUG("   fp_info.fp_width = %lu",
			params->display_params[i].fp_info.fp_width);
		EMGD_DEBUG("   fp_info.fp_height = %lu",
			params->display_params[i].fp_info.fp_height);
		EMGD_DEBUG("   fp_info.fp_pwr_method = %lu",
			params->display_params[i].fp_info.fp_pwr_method);
		EMGD_DEBUG("   fp_info.fp_pwr_t1 = %lu",
			params->display_params[i].fp_info.fp_pwr_t1);
		EMGD_DEBUG("   fp_info.fp_pwr_t2 = %lu",
			params->display_params[i].fp_info.fp_pwr_t2);
		EMGD_DEBUG("   fp_info.fp_pwr_t3 = %lu",
			params->display_params[i].fp_info.fp_pwr_t3);
		EMGD_DEBUG("   fp_info.fp_pwr_t4 = %lu",
			params->display_params[i].fp_info.fp_pwr_t4);
		EMGD_DEBUG("   fp_info.fp_pwr_t5 = %lu",
			params->display_params[i].fp_info.fp_pwr_t5);
		EMGD_DEBUG("   dtd_list:");
		EMGD_DEBUG("    num_dtds = %lu",
			params->display_params[i].dtd_list.num_dtds);
		for (j = 0 ; j < params->display_params[i].dtd_list.num_dtds; j++) {
			EMGD_DEBUG("   *dtd[%d].width = %u", j,
				params->display_params[i].dtd_list.dtd[j].width);
			EMGD_DEBUG("    dtd[%d].height = %u", j,
				params->display_params[i].dtd_list.dtd[j].height);
			EMGD_DEBUG("    dtd[%d].refresh = %u", j,
				params->display_params[i].dtd_list.dtd[j].refresh);
			EMGD_DEBUG("    dtd[%d].dclk = %lu = 0x%lx", j,
				params->display_params[i].dtd_list.dtd[j].dclk,
				params->display_params[i].dtd_list.dtd[j].dclk);
			EMGD_DEBUG("    dtd[%d].htotal = %u", j,
				params->display_params[i].dtd_list.dtd[j].htotal);
			EMGD_DEBUG("    dtd[%d].hblank_start = %u", j,
				params->display_params[i].dtd_list.dtd[j].hblank_start);
			EMGD_DEBUG("    dtd[%d].hblank_end = %u", j,
				params->display_params[i].dtd_list.dtd[j].hblank_end);
			EMGD_DEBUG("    dtd[%d].hsync_start = %u", j,
				params->display_params[i].dtd_list.dtd[j].hsync_start);
			EMGD_DEBUG("    dtd[%d].hsync_end = %u", j,
				params->display_params[i].dtd_list.dtd[j].hsync_end);
			EMGD_DEBUG("    dtd[%d].vtotal = %u", j,
				params->display_params[i].dtd_list.dtd[j].vtotal);
			EMGD_DEBUG("    dtd[%d].vblank_start = %u", j,
				params->display_params[i].dtd_list.dtd[j].vblank_start);
			EMGD_DEBUG("    dtd[%d].vblank_end = %u", j,
				params->display_params[i].dtd_list.dtd[j].vblank_end);
			EMGD_DEBUG("    dtd[%d].vsync_start = %u", j,
				params->display_params[i].dtd_list.dtd[j].vsync_start);
			EMGD_DEBUG("    dtd[%d].vsync_end = %u", j,
				params->display_params[i].dtd_list.dtd[j].vsync_end);
			EMGD_DEBUG("    dtd[%d].mode_number = %d", j,
				params->display_params[i].dtd_list.dtd[j].mode_number);
			EMGD_DEBUG("    dtd[%d].flags = %lu = 0x%lx", j,
				params->display_params[i].dtd_list.dtd[j].flags,
				params->display_params[i].dtd_list.dtd[j].flags);
			EMGD_DEBUG("    dtd[%d].x_offset = %u", j,
				params->display_params[i].dtd_list.dtd[j].x_offset);
			EMGD_DEBUG("    dtd[%d].y_offset = %u", j,
				params->display_params[i].dtd_list.dtd[j].y_offset);
			/*EMGD_DEBUG("    dtd[%d].pd_private_ptr = 0x%p", j,
				params->display_params[i].dtd_list.dtd[j].pd_private_ptr); */
			EMGD_DEBUG("    dtd[%d].private_ptr = 0x%p", j,
				params->display_params[i].dtd_list.dtd[j].private_ptr);
		}
		EMGD_DEBUG("   attr_list:");
		EMGD_DEBUG("    num_attrs = %lu",
			params->display_params[i].attr_list.num_attrs);
		for (j = 0 ; j < params->display_params[i].attr_list.num_attrs; j++) {
			EMGD_DEBUG("    attr[%d].id = %lu = 0x%lx", j,
				params->display_params[i].attr_list.attr[j].id,
				params->display_params[i].attr_list.attr[j].id);
			EMGD_DEBUG("    attr[%d].value = %lu = 0x%lx", j,
				params->display_params[i].attr_list.attr[j].value,
				params->display_params[i].attr_list.attr[j].value);
		}
	}
	EMGD_DEBUG("   display_color = %lu = 0x%lx", params->display_color,
		params->display_color);
	EMGD_DEBUG("   quickboot = %lu", params->quickboot);
	EMGD_DEBUG("   qb_seamless = %d", params->qb_seamless);
	EMGD_DEBUG("   qb_video_input = %lu", params->qb_video_input);
	EMGD_DEBUG("   qb_splash = %d", params->qb_splash);
	EMGD_DEBUG("   polling = %d", params->polling);
} /* emgd_print_params() */


/**
 * A helper function that starts, initializes and configures the HAL.  This
 * will be called during emgd_driver_load() if the display is to be initialized
 * at module load time.  Otherwise, this is deferred till the X driver loads
 * and calls emgd_driver_pre_init().
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h").
 * @param params (IN) The the igd_param_t struct to give to the HAL.
 *
 * @return 0 on Success
 * @return <0 on Error
 */
int emgd_startup_hal(struct drm_device *dev, igd_param_t *params)
{
	drm_emgd_priv_t *priv = dev->dev_private;
	int err = 0;

	EMGD_TRACE_ENTER;


	/* Initialize the various HAL modules: */
	EMGD_DEBUG("Calling igd_module_init()");

	err = igd_module_init(drm_HAL_handle, &drm_HAL_dispatch, params);
	if (err != 0) {
		return -EIO;
	}

	/* Give the dispatch table to the ioctl-handling "bridge" code: */
	emgd_set_real_dispatch(drm_HAL_dispatch);

	/* Record that the HAL is running now: */
	priv->hal_running = 1;
	/* Record that the console's state is saved: */
	priv->saved_registers = CONSOLE_STATE_SAVED;

	/* Since PVR services is running, we're restarting the HAL, which
	 * disabled the SGX & MSVDX interrupts.  Need to re-enable those
	 * interrupts:
	 */
	SysReEnableInterrupts();

	EMGD_TRACE_EXIT;

	return 0;
} /* emgd_startup_hal() */



/**
 * A helper function that initializes the display, and potentially merges the
 * module parameters with the pre-compiled parameters.
 *
 * @param merge_mod_params (IN) non-zero if the module parameters should be
 * merged with the pre-compiled parameters.
 *
 */
void emgd_init_display(int merge_mod_params, drm_emgd_priv_t *priv)
{
	int                     err         = 0;
	igd_display_info_t     *mode_list   = NULL;
	igd_display_info_t     *mode        = NULL;
	struct drm_framebuffer *framebuffer = NULL;
	emgd_framebuffer_t     *emgd_fb     = NULL;
	unsigned char          *fb          = NULL;
	int                     mode_flags = IGD_QUERY_LIVE_MODES;
	unsigned long           temp_bg_color;
	int temp_dc;
	EMGD_TRACE_ENTER;


	if (merge_mod_params) {
		EMGD_DEBUG("Checking other module parameters before initializing the "
				"display");

		/*************************************
		 * Get the desired display display config:
		 *************************************/
		if (-1 != drm_emgd_dc) {
			/* Validate and potentially use the module parameter: */
			EMGD_DEBUG("Value of module parameter \"dc\" = \"%d\"", drm_emgd_dc);
			if (IGD_DC_SINGLE(drm_emgd_dc) || IGD_DC_CLONE(drm_emgd_dc) ||
				IGD_DC_VEXT(drm_emgd_dc) || IGD_DC_EXTENDED(drm_emgd_dc)) {
				/* Use validated value to override compile-time value: */
				config_drm.dc = drm_emgd_dc;
			} else if (IGD_DC_TWIN(drm_emgd_dc)) {
				/* Use validated value to override compile-time value: */
				EMGD_DEBUG("Module parameter \"dc\" contains unsupported value "
						"%d.", drm_emgd_dc);
				EMGD_DEBUG("Overriding and making it 1 (single display).");
				drm_emgd_dc = 1;
				config_drm.dc = drm_emgd_dc;
			} else {
				/* Use compile-time value: */
				EMGD_ERROR("Module parameter \"dc\" contains invalid value "
					"%d (must be 1, 2, 5 or 8).", drm_emgd_dc);
				if (config_drm.dc == 4) {
					EMGD_DEBUG("Compile-time setting for module parameter "
						"\"dc\" contains unsupported value %d.", config_drm.dc);
					EMGD_DEBUG("Overriding and making it 1 (single display).");

					config_drm.dc = 1;
				} else {
					EMGD_ERROR("Will use compile-time setting %d instead "
						"of invalid value %d.\n", config_drm.dc, drm_emgd_dc);
				}
				drm_emgd_dc = config_drm.dc;
			}
		} else {
			/* Check and potentially use the compile-time value: */
			if (IGD_DC_SINGLE(config_drm.dc) || IGD_DC_CLONE(config_drm.dc) ||
				IGD_DC_VEXT(config_drm.dc) ||
				IGD_DC_EXTENDED(config_drm.dc)) {
				/* Report the compile-time value: */
				EMGD_DEBUG("Using compile-time setting for the module parameter"
						" \"dc\" = \"%d\"", config_drm.dc);
			} else if (IGD_DC_TWIN(config_drm.dc)) {
				EMGD_DEBUG("Compile-time setting for module parameter "
						"\"dc\" contains unsupported value %d.", config_drm.dc);
				EMGD_DEBUG("Overriding and making it 1 (single display).");
				config_drm.dc = 1;
			} else {
				EMGD_DEBUG("Compile-time setting for module parameter "
						"\"dc\" contains invalid value %d.", config_drm.dc);
				EMGD_DEBUG("Must be 1, 2, 5 or 8.  Making it 1 (single"
						" display).");
				config_drm.dc = 1;
			}
			drm_emgd_dc = config_drm.dc;
		}

		/*************************************
		 * Get the desired display "width":
		 *************************************/
		if (-1 != drm_emgd_width) {
			/* Override the compile-time value with the module parameter: */
			EMGD_DEBUG("Using the \"width\" module parameter: \"%d\"",
					drm_emgd_width);
			config_drm.width = drm_emgd_width;
		} else {
			/* Use the compile-time value: */
			drm_emgd_width = config_drm.width;
			EMGD_DEBUG("Using the compile-time \"width\" value: \"%d\"",
					drm_emgd_width);
		}

		/*************************************
		 * Get the desired display "height":
		 *************************************/
		if (-1 != drm_emgd_height) {
			/* Override the compile-time value with the module parameter: */
			EMGD_DEBUG("Using the \"height\" module parameter: \"%d\"",
					drm_emgd_height);
			config_drm.height = drm_emgd_height;
		} else {
			/* Use the compile-time value: */
			drm_emgd_height = config_drm.height;
			EMGD_DEBUG("Using the compile-time \"height\" value: \"%d\"",
					drm_emgd_height);
		}

		/*************************************
		 * Get the desired display "refresh":
		 *************************************/
		if (-1 != drm_emgd_refresh) {
			/* Override the compile-time value with the module parameter: */
			EMGD_DEBUG("Using the \"refresh\" module parameter: \"%d\"",
					drm_emgd_refresh);
			config_drm.refresh = drm_emgd_refresh;
		} else {
			/* Use the compile-time value: */
			drm_emgd_refresh = config_drm.refresh;
			EMGD_DEBUG("Using the compile-time \"refresh\" value: \"%d\"",
					drm_emgd_refresh);
		}
	}


	if (config_drm.kms) {
		priv->num_crtc = 2;


		/*************************************
		 * Initialize kernel mode setting functionality
		 *************************************/
		emgd_modeset_init(priv->ddev);

		/* Get Display context */
		drm_HAL_context->mod_dispatch.dsp_get_dc(NULL,
							(igd_display_context_t **) &primary,
							(igd_display_context_t **) &secondary);


		/*************************************
		 * Initialize primary_fb_info
		 *************************************/
		framebuffer = list_entry(priv->ddev->mode_config.fb_list.next,
				struct drm_framebuffer, head);
		if (!framebuffer) {
			EMGD_ERROR("Can't display splash screen/video as there is no fb.");
		} else {
			emgd_fb = container_of(framebuffer, emgd_framebuffer_t, base);
			primary_fb_info = &priv->initfb_info;
			if (priv->fbdev) {
				fb = priv->fbdev->screen_base;
			}
		}


		/*
		 * Update the private data structure
		 */
		priv->primary               = primary;
		priv->secondary             = secondary;
		priv->primary_port_number   = IGD_DC_PRIMARY(priv->dc);
		priv->secondary_port_number = IGD_DC_SECONDARY(priv->dc);
	} else {

		/**************************************************************************
		 * Special case handling: Since HAL doesn't know anything about Vertical
		 * extended mode, if we are in Vertical Extended (5), send HAL asking for
		 * (2)
		 *************************************************************************/
		temp_dc = drm_emgd_dc;
		if(IGD_DC_VEXT(drm_emgd_dc)) {
			temp_dc = IGD_DISPLAY_CONFIG_CLONE;
		}

		/*************************************
		 * Query the DC list (use first one):
		 *************************************/
		EMGD_DEBUG("Calling query_dc()");
		err = drm_HAL_dispatch->query_dc(drm_HAL_handle, temp_dc,
				&desired_dc, IGD_QUERY_DC_INIT);
		EMGD_DEBUG("query_dc() returned %d", err);
		if (err) {
			EMGD_ERROR_EXIT("Cannot initialize the display as requested.\n"
					"The query_dc() function returned %d.", err);
			return;
		}
		port_number = (*desired_dc & 0xf0) >> 4;
		EMGD_DEBUG("Using DC 0x%lx with port number %d",
				*desired_dc, port_number);
		if(IGD_DC_VEXT(drm_emgd_dc)) {
			drm_emgd_dc = (*desired_dc & ~IGD_DISPLAY_CONFIG_CLONE) |
					IGD_DISPLAY_CONFIG_VEXT;
		}

		/*************************************
		 * Query the mode list:
		 *************************************/
		EMGD_DEBUG("Calling query_mode_list()");
		err = drm_HAL_dispatch->query_mode_list(drm_HAL_handle, *desired_dc,
				&mode_list, mode_flags);
		EMGD_DEBUG("query_mode_list() returned %d", err);
		if (err) {
			EMGD_ERROR_EXIT("Cannot initialize the display as requested\n"
					"The query_mode_list() function returned %d.", err);
			return;
		}


		/*************************************
		 * Find the desired mode from the list:
		 *************************************/
		EMGD_DEBUG("Comparing the mode list with the desired width, height, and"
			" refresh rate...");

		mode = mode_list;
		while (mode && (mode->width != IGD_TIMING_TABLE_END)) {
			EMGD_DEBUG(" ...Found a mode with width=%d, height=%d, refresh=%d;",
					mode->width, mode->height, mode->refresh);
			if ((mode->width == drm_emgd_width) &&
					(mode->height == drm_emgd_height) &&
					(mode->refresh == drm_emgd_refresh)) {
				EMGD_DEBUG("     ... This mode is a match!");
				desired_mode = mode;
				break;
			}
			mode++;
		}
		if (NULL == desired_mode) {
			EMGD_ERROR("Cannot initialize the display as requested.");
			EMGD_ERROR("No mode matching the desired width (%d), height "
					"(%d), and refresh rate (%d) was found.",
					drm_emgd_width, drm_emgd_height, drm_emgd_refresh);
			return;
		} else {
			/* Must set this in order to get the timings setup: */
			desired_mode->flags |= IGD_DISPLAY_ENABLE;
		}

		/*************************************
		 * Call alter_displays():
		 *************************************/
		primary_fb_info   = kzalloc(sizeof(igd_framebuffer_info_t), GFP_KERNEL);
		secondary_fb_info = kzalloc(sizeof(igd_framebuffer_info_t), GFP_KERNEL);
		primary_fb_info->width = desired_mode->width;

		/*************************************
		 * Special for Vertical Extended, double the height
		 *************************************/
		if(IGD_DC_VEXT(drm_emgd_dc)) {
			primary_fb_info->height = desired_mode->height * 2;
		} else {
			primary_fb_info->height = desired_mode->height;
		}
		primary_fb_info->pixel_format = IGD_PF_ARGB32;
		primary_fb_info->flags = 0;
		primary_fb_info->allocated = 0;
		memcpy(secondary_fb_info, primary_fb_info,
				sizeof(igd_framebuffer_info_t));

		EMGD_DEBUG("Calling alter_displays()");
		err = drm_HAL_dispatch->alter_displays(drm_HAL_handle,
				&primary, desired_mode, primary_fb_info,
				&secondary, desired_mode, secondary_fb_info,
				*desired_dc, 0);
		EMGD_DEBUG("alter_displays() returned %d", err);
		if (err) {
			EMGD_ERROR_EXIT("Cannot initialize the display as requested.\n"
					"The alter_displays() function returned %d.", err);
			return;
		}

		/*
		 * Update the private data structure with the values we get
		 * back from alter displays.
		 */
		priv->dc                    = *desired_dc;
		priv->primary               = primary;
		priv->secondary             = secondary;
		priv->primary_port_number   = IGD_DC_PRIMARY(*desired_dc);
		priv->secondary_port_number = IGD_DC_SECONDARY(*desired_dc);

		/*************************************
		 * Special for Vertical Extended, pan the second display
		 *************************************/
		if(IGD_DC_VEXT(drm_emgd_dc)) {
			drm_HAL_dispatch->pan_display(secondary, 0,
					secondary_fb_info->height / 2);
		}

		/*************************************
		 * Call get_display():
		 *************************************/
		EMGD_DEBUG("Calling get_display()");
		err = drm_HAL_dispatch->get_display(primary, port_number,
				primary_fb_info, &pt_info, 0);
		EMGD_DEBUG("get_display() returned %d", err);
		if (err) {
			EMGD_ERROR_EXIT("Cannot initialize the display as requested\n"
					"The get_display() function returned %d.", err);
			return;
		}

		/*************************************
		 * Get FB virtual address
		 *************************************/
		EMGD_DEBUG("Calling full_clear_fb()");
		fb = mode_context->context->dispatch.gmm_map(
				primary_fb_info->fb_base_offset);
	}

	if (fb) {

		/*************************************
		 * Set the framebuffer to the background color:
		 *************************************/
		temp_bg_color = mode_context->display_color;
		mode_context->display_color = config_drm.ss_data->bg_color;
		if(mode_context->seamless == FALSE)
		{
            full_clear_fb(mode_context, primary_fb_info, fb);
		}
		mode_context->display_color = temp_bg_color;

		/*************************************
		 * Display a splash screen if requested by user
		 *************************************/
		if(config_drm.ss_data->width &&
				config_drm.ss_data->height) {

			/* Display a splash screen */
			printk(KERN_ERR "[EMGD] Display splash screen image.\n");
			EMGD_DEBUG("Calling disp_splash_screen()");
			display_splash_screen(primary_fb_info, fb, config_drm.ss_data);
		}

		/*************************************
		 * Display a splash video if requested by user
		 *************************************/
		if(config_drm.sv_data->pixel_format &&
				config_drm.sv_data->src_width &&
				config_drm.sv_data->src_height &&
				config_drm.sv_data->src_pitch &&
				config_drm.sv_data->dst_width &&
				config_drm.sv_data->dst_height) {

			/* Display a splash video */
			EMGD_DEBUG("Calling disp_splash_video()");
			disp_splash_video(config_drm.sv_data);
		}
	} else {
		EMGD_ERROR("framebuffer base address is 0");
	}
	mode_context->seamless = FALSE;

	if (!config_drm.kms) {
		mode_context->context->dispatch.gmm_unmap(fb);
	}
	EMGD_TRACE_EXIT;
} /* emgd_init_display() */



/**
 * Function to display a splash video to the user. The splash video data must be
 * provided to the kernel mode driver by another entity (like a driver or FPGA
 * HW). Splash video will be display after setting the mode (if requested by
 * the user through config options).
 *
 * @param sv_data (IN) a non null pointer to splash video information like
 * width, height etc.
 *
 * @return 0 on Success
 * @return <0 on Error
 */
int disp_splash_video(emgd_drm_splash_video_t *sv_data)
{
	igd_surface_t surface;
	igd_rect_t ovl_rect, surf_rect;
	igd_ovl_info_t ovl_info;
	unsigned int tmp;

	surface.offset       = sv_data->offset;
	surface.pitch        = sv_data->src_pitch;
	surface.width        = sv_data->src_width;
	surface.height       = sv_data->src_height;
	surface.pixel_format = sv_data->pixel_format;
	surface.flags        = IGD_SURFACE_OVERLAY;

	/* Set the surface rect as big as the video frame size */
	surf_rect.x1 = 0;
	surf_rect.y1 = 0;
	surf_rect.x2 = sv_data->src_width;
	surf_rect.y2 = sv_data->src_height;


	/* The x,y postion of the sprite c */
	ovl_rect.x1 = sv_data->dst_x;
	ovl_rect.y1 = sv_data->dst_y;

	/*
	 *  NOTE: This for scaling if the hardware supports it.
	 *  If no dest w x h values are set,set it the the
	 *  src w x h
	 */
	tmp = sv_data->dst_width ? sv_data->dst_width : sv_data->src_width;

	ovl_rect.x2 = ovl_rect.x1 + tmp;

	tmp = sv_data->dst_height?
		sv_data->dst_height:
	sv_data->src_height;

	ovl_rect.y2 = ovl_rect.y1 + tmp;

	/*
	 *  If no values are set, set it to default
	 */
	ovl_info.video_quality.brightness =
		config_drm.ovl_brightness ?
		config_drm.ovl_brightness : 0x8000;
	ovl_info.video_quality.contrast =
		config_drm.ovl_contrast ?
		config_drm.ovl_contrast : 0x8000;
	ovl_info.video_quality.saturation =
		config_drm.ovl_saturation ?
		config_drm.ovl_saturation : 0x8000;
	ovl_info.video_quality.hue =
		config_drm.ovl_hue ?
		config_drm.ovl_hue : 0x8000;

	/*
	 * If any values are set for gamma, turn on the gamma flags
	 */
	ovl_info.gamma.red   = config_drm.ovl_gamma_red;
	ovl_info.gamma.green = config_drm.ovl_gamma_green;
	ovl_info.gamma.blue  = config_drm.ovl_gamma_blue;

	if(ovl_info.gamma.red || ovl_info.gamma.green || ovl_info.gamma.blue) {

		ovl_info.gamma.flags = IGD_OVL_GAMMA_ENABLE;
	}

	drm_HAL_dispatch->alter_ovl(drm_HAL_handle, NULL,
		&surface,
		&surf_rect,
		&ovl_rect,
		&ovl_info, IGD_OVL_ALTER_ON);

	return 0;
}



/**
 * This is the drm_driver.load() function.  It is called when the DRM "loads"
 * (i.e. when our driver loads, it calls drm_init(), which eventually causes
 * this driver function to be called).
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param flags (IN) The last member of the pci_device_id struct that we
 * fill-in at the top of this file (e.g. CHIP_PSB_8108).
 *
 * @return 0 on Success
 * @return <0 on Error
 */
int emgd_driver_load(struct drm_device *dev, unsigned long flags)
{
	int i, err = 0;
	igd_param_t *params;
	drm_emgd_priv_t *priv = NULL;
	igd_init_info_t *init_info;
	int num_hal_params =
		sizeof(config_drm.hal_params) / sizeof(config_drm.hal_params[0]);


	EMGD_TRACE_ENTER;

	mutex_lock(&dev->struct_mutex);

	/**************************************************************************
	 *
	 * Get the compile-time/module-parameter "params" before initializing the
	 * HAL:
	 *
	 **************************************************************************/

	/*************************************
	 * Take into account the "configid" module parameter:
	 *************************************/
	if (num_hal_params <= 0) {
		EMGD_ERROR("The compile-time configuration (in \"user_config.c\")\n"
			"contains no igd_param_t structures.  Please fix and recompile.");
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}
	if ((drm_emgd_configid == 0) ||
		(drm_emgd_configid > num_hal_params)) {
		EMGD_ERROR("Module parameter \"configid\" contains invalid value "
			"%d.\nMust specify a compile-time configuration (in "
			"\"user_config.c\"),\nnumbered between 1 and %d.\n",
			drm_emgd_configid, num_hal_params);
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}

	/* Obtain the user-configurable set of parameter values: */
	if (drm_emgd_configid < 0) {
		params = config_drm.hal_params[0];
	} else {
		params = config_drm.hal_params[drm_emgd_configid-1];
	}


	/*************************************
	 * Take into account the "portorder" module parameter:
	 *************************************/
	err = 0;
	EMGD_DEBUG("Determining desired port order:");
	if (0 == drm_emgd_numports) {
		/* Set this to 1 so we use the compile-time value below: */
		err = 1;
		drm_emgd_numports = IGD_MAX_PORTS;
	} else if (drm_emgd_numports != IGD_MAX_PORTS) {
		EMGD_ERROR("Module parameter \"portorder\" specifies %d ports "
			"(must specify %d in a comma-separated list).",
			drm_emgd_numports, IGD_MAX_PORTS);
		drm_emgd_numports = IGD_MAX_PORTS;
		err = -EINVAL;
	}
	if (!err) {
		/* Validate each port within the module parameter: */
		for (i = 0 ; i < drm_emgd_numports ; i++) {
			if ((drm_emgd_portorder[i] < 0) ||
				(drm_emgd_portorder[i] > 5)) {
				EMGD_ERROR("Item %d in module parameter \"portorder\" "
					"contains invalid value %d (must be between 0 and 5).",
					i, drm_emgd_portorder[i]);
				err = -EINVAL;
			}
		}
	}
	if (!err) {
		/* Override the compile-time value with the module parameter: */
		for (i = 0 ; i < drm_emgd_numports ; i++) {
			params->port_order[i] = drm_emgd_portorder[i];
		}
		EMGD_DEBUG("Using the \"portorder\" module parameter: \"%d, %d, %d, "
			"%d, %d\"", drm_emgd_portorder[0], drm_emgd_portorder[1],
			drm_emgd_portorder[2], drm_emgd_portorder[3],
			drm_emgd_portorder[4]);
	} else {
		/* Use the compile-time value: */
		for (i = 0 ; i < drm_emgd_numports ; i++) {
			drm_emgd_portorder[i] = params->port_order[i];
		}
		EMGD_DEBUG("Using the compile-time \"portorder\" value: \"%d, %d, "
			"%d, %d, %d\"", drm_emgd_portorder[0], drm_emgd_portorder[1],
			drm_emgd_portorder[2], drm_emgd_portorder[3],
			drm_emgd_portorder[4]);
		err = 0;
	}


	emgd_print_params(params);


	/**************************************************************************
	 *
	 * Minimally initialize and configure the driver, deferring as much as
	 * possible until the X driver starts.:
	 *
	 **************************************************************************/

	/* Determine whether display initialization is desired: */
	if (-1 != drm_emgd_init) {
		/* Validate and potentially use the module parameter: */
		if (!((drm_emgd_init == 1) || (drm_emgd_init == 0))) {
			EMGD_ERROR("Module parameter \"init\" contains invalid "
				"value %d (must be 0 or 1).", drm_emgd_init);
			drm_emgd_init = config_drm.init;
			EMGD_ERROR("Using the compile-time \"init\" value: \"%d\"",
				drm_emgd_init);
		} else {
			/* Override the compile-time value with the module parameter: */
			config_drm.init = drm_emgd_init;
			EMGD_DEBUG("Using the \"init\" module parameter: \"%d\"",
				drm_emgd_init);
		}
	} else {
		/* Use the compile-time value: */
		drm_emgd_init = config_drm.init;
		EMGD_DEBUG("Using the compile-time \"init\" value: \"%d\"",
			drm_emgd_init);
	}


	/*
	 * In order for some early ioctls (e.g. emgd_get_chipset_info()) to work,
	 * we must do the following minimal initialization.
	 */
	EMGD_DEBUG("Calling igd_driver_init()");
	init_info = (igd_init_info_t *)OS_ALLOC(sizeof(igd_init_info_t));
	drm_HAL_handle = igd_driver_init(init_info);
	if (drm_HAL_handle == NULL) {
		OS_FREE(init_info);
		mutex_unlock(&dev->struct_mutex);
		return -ENOMEM;
	}

	/* Get the HAL context and give it to the ioctl-handling "bridge" code: */
	drm_HAL_context = (igd_context_t *) drm_HAL_handle;
	emgd_set_real_handle(drm_HAL_handle);

	/* Save the drm dev pointer, it's needed by igd_module_init */
	drm_HAL_context->drm_dev = dev;

	/* Create the private structure used to communicate to the IMG 3rd-party
	 * display driver:
	 */
	priv = OS_ALLOC(sizeof(drm_emgd_priv_t));
	if (NULL == priv) {
		OS_FREE(init_info);
		mutex_unlock(&dev->struct_mutex);
		return -ENOMEM;
	}


	OS_MEMSET(priv, 0, sizeof(drm_emgd_priv_t));
	priv->hal_running = 0;
	priv->context     = drm_HAL_context;
	priv->init_info   = init_info;
	priv->qb_seamless = params->qb_seamless;
	dev->dev_private  = priv;
	priv->ddev        = dev;
	priv->kms_enabled = 0;


	/* Do basic driver initialization & configuration: */
	EMGD_DEBUG("Calling igd_driver_config()");
	err = igd_driver_config(drm_HAL_handle);
	if (err != 0) {
		OS_FREE(init_info);
		OS_FREE(priv);
		OS_FREE(drm_HAL_handle);
		mutex_unlock(&dev->struct_mutex);
		return -EIO;
	}

	// PVRSRVDrmLoad() sets up an ISR routine with a pointer to drm_device to be passed every time.  This variable (gpDrmDevice) is initialized in msvdx_pre_init_plb only.    
	// Due to this reason, msvdx_pre_init_plb() is moved before PVRSRVDrmLoad().

	/* Init MSVDX and load firmware */
	msvdx_pre_init_plb(dev);

	/* Initialize the PVR services if not already initialized */
	printk(KERN_INFO "Initializing PVR Services.\n");
	PVRSRVDrmLoad(dev, 0);

	/* Decide if we can defer the rest of the initialization */
	if (config_drm.init) {

		if (config_drm.kms) {
			params->preserve_regs = 1;
			priv->kms_enabled = 1;
		}

		/* Initialize and configure the driver now */
		err = emgd_startup_hal(dev, params);
		if (err != 0) {
			OS_FREE(init_info);
			OS_FREE(priv);
			OS_FREE(drm_HAL_handle);
			mutex_unlock(&dev->struct_mutex);
			return err;
		}

		/* This will get the plane, pipe and port register values and fill up the
		 * fw_info data structure. This needs to be done at INIT time before the
		 * user-mode driver loads
		 */
		get_pre_driver_info(mode_context);

		/* Per the user's request, initialize the display: */
		emgd_init_display(TRUE, priv);
	}



#ifdef DEBUG_BUILD_TYPE
	/* Turn on KMS debug messages in the general DRM module if our OAL
	 * messages are on and it's a debug driver.
	 */
	if (emgd_debug->hal.oal) {
		drm_debug |= DRM_UT_KMS;
	}
#endif

#ifdef SUPPORT_V2G_CAMERA
	/* to start v2g camera module */
	if (1 == config_drm.v2g) {
		EMGD_DEBUG("V2G Camera Enabled.");
		if (0 == v2g_start_camera()) {
			EMGD_DEBUG("v2g camera started successfully!");
		} else {
			EMGD_ERROR("Fail to start v2g camera!");
		}
	}
#endif	
	/* can not work out how to start PVRSRV */
	/* Load Buffer Class Module*/
	emgd_bc_ts_init();

	mutex_unlock(&dev->struct_mutex);
	EMGD_TRACE_EXIT;

	return 0;
} /* emgd_driver_load() */


/**
 * This is the drm_driver.unload() function.  It is called when the DRM
 * "unloads."  That is, drm_put_dev() (in "drm_stub.c"), which is called by
 * drm_exit() (in "drm_drv.c"), calls this function.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return 0 on Success
 * @return <0 on Error
 */
int emgd_driver_unload(struct drm_device *dev)
{
	drm_emgd_priv_t *priv = dev->dev_private;
	unsigned long save_flags = 0;

	EMGD_TRACE_ENTER;

	mutex_lock(&dev->struct_mutex);

	/* Unload Buffer Class Module*/
	emgd_bc_ts_uninit();

	PVRSRVDrmUnload(dev);

	/* KMS cleanup */
	if (config_drm.init && config_drm.kms) {
		emgd_modeset_destroy(dev);
	}

	if (priv->hal_running) {
		/* igd_driver_shutdown() will restore the then-currently-saved register
		 * state.  We can't rely on any save_restore() calls before that time,
		 * because igd_driver_shutdown() does things that mess up the register
		 * state.  Thus, we must allow it to do a restore.  The only way to
		 * control the final state of the hardware is to potentially do a
		 * save_restore now.  Thus, if the X server's state is currently saved
		 * (i.e. the console's state is currently active), we must do a
		 * save_restore now, so that this state will still exist after
		 * igd_driver_shutdown().
		 */
		if (priv->saved_registers == CONSOLE_STATE_SAVED) {
			EMGD_DEBUG("Need to restore the console's saved register state");

			save_flags = IGD_REG_SAVE_ALL;
			drm_HAL_dispatch->driver_save_restore(drm_HAL_handle, save_flags);
			EMGD_DEBUG("State of saved registers is X_SERVER_STATE_SAVED");
			priv->saved_registers = X_SERVER_STATE_SAVED;
		}
		igd_driver_shutdown_hal(drm_HAL_handle);
		igd_driver_shutdown(drm_HAL_handle);
	} else {
		/* Do safe cleanup that would've been done by igd_driver_shutdown() */
		igd_driver_shutdown(drm_HAL_handle);
	}

	if (!config_drm.kms) {
		kfree(primary_fb_info);
		kfree(secondary_fb_info);
	}

	OS_FREE(priv->init_info);
	OS_FREE(priv);

	/* Note: This macro is #define'd in "oal/os/memory.h" */
#ifdef INSTRUMENT_KERNEL_ALLOCS
	emgd_report_unfreed_memory();
#endif

	mutex_unlock(&dev->struct_mutex);

	EMGD_TRACE_EXIT;

	return 0;
} /* emgd_driver_unload() */


/**
 * This is the drm_driver.open() function.  It is called when a user-space
 * process opens the DRM device file.  The DRM drm_open() (in "drm_fops.c")
 * calls drm_open_helper(), which calls this function.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param priv (IN) DRM's file private data struct (in "drmP.h")
 *
 * @return 0 on Success
 * @return <0 on Error
 */
int emgd_driver_open(struct drm_device *dev, struct drm_file *priv)
{
	int ret = 0;
	drm_emgd_priv_t *emgd_priv = dev->dev_private;

	EMGD_TRACE_ENTER;

	/*
	 * Under the latest MeeGo images, something is trying to open the DRM device
	 * while we're still inside the the load() function (possibly an updated
	 * copy of udev?).  Don't let open() calls through here until
	 * initialization is finished.  If userspace can overlap load() and open(),
	 * then it stands to reason that we might also wind up racing with close()
	 * and unload() as well, so let's protect all of those operations by
	 * grabbing the mutex.
	 *
	 * FIXME: Should we do the same for other operations like suspend/resume/etc?
	 */
	mutex_lock(&dev->struct_mutex);

	/* The is_master flag is set after the call to this function, so there needs
	 * to be manual check to determine the DRM master */
	if(priv->is_master || (!priv->minor->master && !emgd_priv->drm_master_fd)) {
		emgd_priv->drm_master_fd = priv;
	}

	ret = PVRSRVOpen(dev, priv);

	mutex_unlock(&dev->struct_mutex);

	EMGD_TRACE_EXIT;
	return ret;
} /* emgd_driver_open() */


/**
 * This is the drm_driver.lastclose() function.  It is called when the last
 * user-space process closes/releases the DRM device file.  At end of DRM
 * drm_release() (in "drm_fops.c"), it calls drm_lastclose() (in "drm_drv.c"),
 * which calls this function.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 */
void emgd_driver_lastclose(struct drm_device *dev)
{
	drm_emgd_priv_t *priv = dev->dev_private;
	igd_init_info_t *init_info = priv->init_info;
	int err = 0;
	unsigned long restore_flags = 0;

	EMGD_TRACE_ENTER;

	mutex_lock(&dev->struct_mutex);


	if (priv->hal_running) {
		if (config_drm.init) {
			if( x_started ) {

				restore_flags = (IGD_REG_SAVE_ALL & ~IGD_REG_SAVE_GTT)
									| IGD_REG_SAVE_TYPE_REG;

				if (!config_drm.kms) {
					restore_flags &= ~IGD_REG_SAVE_RB;
				}


				if (priv->saved_registers == CONSOLE_STATE_SAVED) {
					EMGD_DEBUG("Need to restore the console's saved "
						"register state");
					drm_HAL_dispatch->driver_save_restore(drm_HAL_handle,
															restore_flags);
					EMGD_DEBUG("State of saved registers is "
						"X_SERVER_STATE_SAVED");
					priv->saved_registers = X_SERVER_STATE_SAVED;
				}

				if (priv->saved_registers == X_SERVER_STATE_SAVED &&
					!config_drm.kms && !priv->qb_seamless) {
					emgd_init_display(FALSE, priv);
				}

				if (priv->saved_registers == CONSOLE_STATE_SAVED) {
					EMGD_DEBUG("Need to restore the console's saved register "
						"state");
					drm_HAL_dispatch->driver_save_restore(drm_HAL_handle,
															restore_flags);
					EMGD_DEBUG("State of saved registers is X_SERVER_STATE_SAVED");
					priv->saved_registers = X_SERVER_STATE_SAVED;
				}

				/* Since an alter_displays() was done, re-init the 3DD: */
				if (priv->reinit_3dd) {
					priv->reinit_3dd(dev);
				}

				x_started = false;
			}
		} else {
			/* The X server has quit/crashed.  If the console's state is
			 * currently saved (likely) restore that state, so that the console
			 * can be seen and work.
			 */
			context_count = 0;
			priv->dc = 0; /* Don't let the 3DD re-init too early: */
			if (priv->saved_registers == CONSOLE_STATE_SAVED) {
				EMGD_DEBUG("Need to restore the console's saved register "
					"state");
				restore_flags = IGD_REG_SAVE_ALL;

				drm_HAL_dispatch->driver_save_restore(drm_HAL_handle,
														restore_flags);
				EMGD_DEBUG("State of saved registers is X_SERVER_STATE_SAVED");
				priv->saved_registers = X_SERVER_STATE_SAVED;
			}

			/******************************************************************
			 * Shutdown and minimally-restart the HAL, so that if/when the X
			 * server starts again, the HAL will be started again.  This will
			 * allow the necessary port drivers to be loaded, all configuration
			 * parameters to be used, the EDID to be read again (e.g. because
			 * the user may have switched monitors), etc.
			 *
			 * However, keeep the DRM driver state around, so that the driver
			 * can continue to work (i.e. re-enter the state when the DRM
			 * driver was first loaded).
			 *
			 * Also, keep the PVR services up and going, since it wasn't
			 * designed to be shutdown and restarted (without doing an
			 * rmmod/insmod).
			 ******************************************************************/
			EMGD_DEBUG("Shutting down the HAL");

			/* igd_driver_shutdown() will restore the then-currently-saved
			 * register state.  We can't rely on any save_restore() calls
			 * before that time, because igd_driver_shutdown() does things that
			 * mess up the register state.  Thus, we must allow it to do a
			 * restore.  The only way to control the final state of the
			 * hardware is to potentially do a save_restore now.  Thus, if the
			 * X server's state is currently saved (i.e. the console's state is
			 * currently active), we must do a save_restore now, so that this
			 * state will still exist after igd_driver_shutdown().
			 */
			if (priv->saved_registers == X_SERVER_STATE_SAVED) {
				EMGD_DEBUG("Need to restore the console's saved register "
					"state");
				restore_flags = IGD_REG_SAVE_ALL;
				drm_HAL_dispatch->driver_save_restore(drm_HAL_handle,
														restore_flags);
				EMGD_DEBUG("State of saved registers is X_SERVER_STATE_SAVED");
				priv->saved_registers = CONSOLE_STATE_SAVED;
			}
			msvdx_shutdown_plb(drm_HAL_handle);
			topaz_shutdown_tnc(drm_HAL_handle);
			
			igd_driver_shutdown_hal(drm_HAL_handle);
			igd_driver_shutdown(drm_HAL_handle);

			EMGD_DEBUG("Minimally restarting the HAL--like load-time");
			/*
			 * In order for some early ioctls (e.g. emgd_get_chipset_info()) to
			 * work, we must do the following minimal initialization.
			 */
			EMGD_DEBUG("Calling igd_driver_init()");
			if(init_info == NULL){
				init_info = (igd_init_info_t *)OS_ALLOC(sizeof(igd_init_info_t));
			}
			drm_HAL_handle = igd_driver_init(init_info);
			if (drm_HAL_handle == NULL) {
				/* This shouldn't happen, but if it does, alert the user, as
				 * the only thing to do is to rmmod/insmod emgd.ko, or to
				 * reboot:
				 */
				OS_FREE(init_info);
				printk(KERN_ALERT "[EMGD] Failed to restart the EMGD graphics "
						"HAL\n");
				mutex_unlock(&dev->struct_mutex);
				return;
			}

			/* Get the HAL context and give it to the ioctl-handling "bridge"
			 * code:
			 */
			drm_HAL_context = (igd_context_t *) drm_HAL_handle;
			emgd_set_real_handle(drm_HAL_handle);

			/* Save the drm dev pointer, it's needed by igd_module_init */
			drm_HAL_context->drm_dev = dev;

			/* Reset part of the private structure used to communicate to the
			 * IMG 3rd-party display driver:
			 */
			priv->saved_registers = 0;
			priv->suspended_state = NULL;
			priv->must_power_on_ports = 0;
			priv->xserver_running = 0;
			priv->primary_port_number = 0;
			priv->primary = NULL;
			priv->secondary_port_number = 0;
			priv->secondary = NULL;
			priv->msvdx_private = NULL;
			priv->hal_running = 0;
			priv->context = drm_HAL_context;
			priv->init_info = init_info;


			/* Do basic driver initialization & configuration: */
			EMGD_DEBUG("Calling igd_driver_config()");
			err = igd_driver_config(drm_HAL_handle);
			if (err != 0) {
				/* This shouldn't happen, but if it does, alert the user, as
				 * the only thing to do is to rmmod/insmod emgd.ko, or to
				 * reboot:
				 */
				OS_FREE(init_info);
				printk(KERN_ALERT "[EMGD] Failed to restart the EMGD graphics "
						"HAL.\n");
				mutex_unlock(&dev->struct_mutex);
				return;
			}

			/* To ensure the devinfo->interrupt_h is NULL, call the
			 * following:
			 */
			if (priv->reinit_3dd) {
				priv->reinit_3dd(dev);
			}
		}
	}

	mutex_unlock(&dev->struct_mutex );

	EMGD_TRACE_EXIT;

} /* emgd_driver_lastclose() */


/**
 * This is the drm_driver.preclose() function.  It is called when a user-space
 * process closes/releases the DRM device file.  At the very start of DRM
 * drm_release() (in "drm_fops.c"), it calls this function, before it does any
 * real work (other than get the lock).
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param priv (IN) DRM's file private data struct (in "drmP.h")
 */
void emgd_driver_preclose(struct drm_device *dev, struct drm_file *priv)
{
	drm_emgd_priv_t *emgd_priv = dev->dev_private;

	/* Notes on what to implement in this function.  What this
	 * function does is largely influenced by when/why this can be called:
	 *
	 * - We can determine whether the connection was for the X driver vs. for a
	 *   3D application, because the X driver should be the master.
	 *
	 *   - If there's any state management we need to take care of (e.g. if the
	 *     X server crashes while there are still 3D app's running), this is
	 *     probably a good place to do it.
	 *
	 *   - Normal 3D app shutdown is probably best done here too (simply
	 *     because we can tell it's a 3D connection, and do our work before the
	 *     general drm_release() code does its work.
	 *
	 *   - We don't yet know what the IMG 3D code is going to need.
	 *
	 * We don't yet know what we need to do for the IMG code.  Thus, I'm
	 * inclined to leave this function a stub for now.
	 */

	EMGD_TRACE_ENTER;
	mutex_lock(&dev->struct_mutex);
	if ((emgd_priv->hal_running) && priv->is_master && drm_HAL_dispatch) {
		/* The X server can't call gmm_cache_flush() nor igd_driver_shutdown()
		 * after DRICloseScreen() closes the connection with the DRM.  However,
		 * we can tell on this side of the connection (because the X server is
		 * the master) that it is closing down.  There is no need to truly shut
		 * down the HAL, but we can flush the GMM cache and reset the display
		 * hardware to a known state/mode:
		 */
		drm_HAL_dispatch->gmm_flush_cache();

		/* TODO - WRITE CODE THAT PUTS THE DISPLAY HW IN A KNOWN STATE/MODE */
	}

	if (emgd_priv->drm_master_fd == priv)
		emgd_priv->drm_master_fd = NULL;

	mutex_unlock(&dev->struct_mutex);
	EMGD_TRACE_EXIT;

	/* TODO -- ADD ANYTHING WE DISCOVER WE NEED AFTER THE IMG TRAINING */

} /* emgd_driver_preclose() */


/**
 * This is the drm_driver.postclose() function.  It is called when a user-space
 * process closes/releases the DRM device file.  At the end of DRM
 * drm_release() (in "drm_fops.c"), but before drm_lastclose is optionally
 * called, it calls this function.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param priv (IN) DRM's file private data struct (in "drmP.h")
 */
void emgd_driver_postclose(struct drm_device *dev, struct drm_file *priv)
{
	int ret = 0;
	drm_emgd_priv_t *dev_priv = dev->dev_private;
	igd_context_t *context = dev_priv->context;

	EMGD_TRACE_ENTER;
	mutex_lock(&dev->struct_mutex);

	/* Calling the msvdx_postclose_check before PVRSRVRelease */
	if (priv) {
		msvdx_postclose_check(context, (void *) priv);
	}

	ret = PVRSRVRelease(dev, priv);

	mutex_unlock(&dev->struct_mutex);
	EMGD_TRACE_EXIT;
} /* emgd_driver_postclose() */


/**
 * This is the drm_driver.suspend() function.  It appears to support what the
 * Linux "pm.h" file calls "the legacy suspend framework."  The DRM
 * drm_class_suspend() (in "drm_sysfs.c", which implements the Linux
 * class.suspend() function defined in "device.h") calls this function if
 * conditions are met, such as drm_minor->type is DRM_MINOR_LEGACY and the
 * driver doesn't have the DRIVER_MODESET (i.e. KMS--Kernel Mode Setting)
 * feature set.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param state (IN) What power state to put the device in (in "pm.h")
 *
 * @return 0 on Success
 * @return <0 on Error
 */
int emgd_driver_suspend(struct drm_device *dev, pm_message_t state)
{
	int ret;
	unsigned int pwr_state;
	drm_emgd_priv_t *priv = dev->dev_private;

	EMGD_TRACE_ENTER;


	mutex_lock(&dev->struct_mutex);

	/* When the system is suspended, the X server does a VT switch, which saves
	 * the register state of the X server, and restores the console's register
	 * state.  This code saves the console's register state, so that after the
	 * system resumes, VT switches to the console can occur.
	 */
	if (priv->hal_running) {
		EMGD_DEBUG("Saving the console's register state");
		priv->suspended_state =
			drm_HAL_context->mod_dispatch.reg_alloc(drm_HAL_context,
				IGD_REG_SAVE_ALL);
		if (priv->suspended_state) {
			/*
			//Flag CDVO is not needed
			if (state.event == PM_EVENT_SUSPEND){
				if (drm_HAL_context->mod_dispatch.flag_cdvo!= NULL){
					drm_HAL_context->mod_dispatch.flag_cdvo(drm_HAL_context);
				}
			}
			*/
			drm_HAL_context->mod_dispatch.reg_save(drm_HAL_context,
				priv->suspended_state);
		}
	}

	/* Map the pm_message_t event states to HAL states: */
	switch (state.event) {
	case PM_EVENT_PRETHAW:
	case PM_EVENT_FREEZE:
		pwr_state = IGD_POWERSTATE_D1;
		break;
	case PM_EVENT_HIBERNATE:
		pwr_state = IGD_POWERSTATE_D3;
		break;
	case PM_EVENT_SUSPEND:
	default:
		pwr_state = IGD_POWERSTATE_D2;
		break;
	} /* switch (state) */

	EMGD_DEBUG("Calling pwr_alter()");
	ret = drm_HAL_dispatch->pwr_alter(drm_HAL_handle, pwr_state);
	EMGD_DEBUG("pwr_alter() returned %d", ret);

	if (0 == ret) {
		EMGD_DEBUG("Calling PVRSRVDriverSuspend()");
		ret = PVRSRVDriverSuspend(dev, state);
		EMGD_DEBUG("PVRSRVDriverSuspend() returned %d", ret);
	}

	EMGD_DEBUG("Returning %d", ret);
	mutex_unlock(&dev->struct_mutex);
	EMGD_TRACE_EXIT;
	return ret;

} /* emgd_driver_suspend() */


/**
 * This is the drm_driver.resume() function.  It appears to support what the
 * Linux "pm.h" file calls "the legacy suspend framework."  The DRM
 * drm_class_resume() (in "drm_sysfs.c", which implements the Linux
 * class.suspend() function defined in "device.h") calls this function if
 * conditions are met, such as drm_minor->type is DRM_MINOR_LEGACY and the
 * driver doesn't have the DRIVER_MODESET feature set.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return 0 on Success
 * @return <0 on Error
 */
int emgd_driver_resume(struct drm_device *dev)
{
	int ret;
	drm_emgd_priv_t *priv = dev->dev_private;

	EMGD_TRACE_ENTER;

	mutex_lock(&dev->struct_mutex);

	EMGD_DEBUG("Calling pwr_alter()");
	ret = drm_HAL_dispatch->pwr_alter(drm_HAL_handle, IGD_POWERSTATE_D0);
	EMGD_DEBUG("pwr_alter() returned %d", ret);

	if (0 == ret) {
		EMGD_DEBUG("Calling PVRSRVDriverResume()");
		ret = PVRSRVDriverResume(dev);
		EMGD_DEBUG("PVRSRVDriverResume() returned %d", ret);
	}

	/* Restore the register state of the console, so that after the X server is
	 * back up, VT switches to the console can occur.
	 */
	if (priv->hal_running) {
		EMGD_DEBUG("Restoring the console's register state");
		if (priv->suspended_state) {
			drm_HAL_context->mod_dispatch.reg_restore(drm_HAL_context,
				priv->suspended_state);
			drm_HAL_context->mod_dispatch.reg_free(drm_HAL_context,
				priv->suspended_state);
			priv->suspended_state = NULL;
		}
	}

	EMGD_DEBUG("Returning %d", ret);
	mutex_unlock(&dev->struct_mutex);
	EMGD_TRACE_EXIT;
	return ret;
} /* emgd_driver_resume() */


/**
 * This is the drm_driver.device_is_agp() function.  It is called as a part of
 * drm_init() (before the drm_driver.load() function), and is "typically used
 * to determine if a card is really attached to AGP or not" (see the Doxygen
 * comment for this function in "drmP.h").  It is actually called by the
 * inline'd DRM procedure, drm_device_is_agp() (in "drmP.h").
 *
 * The Intel open source driver states that all Intel graphics devices are
 * treated as AGP.  However, the EMGD driver provides its own management of the
 * GTT tables, and so doesn't need AGP GART driver support.  Thus, this
 * function always returns 0.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return 0 the graphics "card" is absolutely not AGP.
 * @return 1 the graphics "card" is AGP.
 * @return 2 the graphics "card" may or may not be AGP.
 */
int emgd_driver_device_is_agp(struct drm_device *dev)
{
	EMGD_DEBUG("[EMGD] Returning 0 from emgd_driver_device_is_agp()\n");
	return 0;
} /* emgd_driver_device_is_agp() */



/**
 * This is the drm_driver.get_vblank_counter() function.  It is called to get
 * the raw hardware vblank counter.  There are 4 places within "drm_irq.c" that
 * call this function.
 *
 * @param dev         (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param crtc_select (IN) Exactly how this is derived is unclear, but
 *                     for now we are assuming that it is the order
 *                     in which the CRTCs were creatd.  So 0 for
 *                     the first CRTC, 1 for the second, and so on.
 *
 * @return raw vblank counter value
 */
u32 emgd_driver_get_vblank_counter(struct drm_device *dev, int crtc_select)
{
	struct drm_crtc *crtc;
	emgd_crtc_t     *cur_emgd_crtc, *selected_emgd_crtc = NULL;


	EMGD_TRACE_ENTER;

	/* Only supported for KMS-enabled driver */
	if (!config_drm.kms) {
		/* Since we have previously returned 0 for our non-KMS driver,
		 * this is left in to prevent any unforeseen problems. */
		EMGD_TRACE_EXIT;
		return 0;
	}


	/* Find the CRTC associated with the  */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		cur_emgd_crtc = container_of(crtc, emgd_crtc_t, base);

		if ((1 << crtc_select) == cur_emgd_crtc->crtc_id) {
			selected_emgd_crtc = cur_emgd_crtc;
			break;
		}		
	}

	if (NULL == selected_emgd_crtc) {
		EMGD_ERROR_EXIT("Invalid CRTC selected.");
		return -EINVAL;
	}

	EMGD_TRACE_EXIT;

	return mode_context->kms_dispatch->kms_get_vblank_counter(
										selected_emgd_crtc);
} /* emgd_driver_get_vblank_counter() */



/**
 * This is the drm_driver.enable_vblank() function.  It is called by
 * drm_vblank_get() (in "drm_irq.c") to enable vblank interrupt events.
 * This function is only available for KMS
 *
 * @param dev         (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param crtc_select (IN) Exactly how this is derived is unclear, but
 *                     for now we are assuming that it is the order
 *                     in which the CRTCs were creatd.  So 0 for
 *                     the first CRTC, 1 for the second, and so on.
 *
 * @return 0 on Success
 * @return <0 if the given crtc's vblank interrupt cannot be enabled
 */
int emgd_driver_enable_vblank(struct drm_device *dev, int crtc_select)
{
	struct drm_crtc *crtc;
	unsigned char   *mmio;
	emgd_crtc_t     *cur_emgd_crtc, *selected_emgd_crtc = NULL;
	unsigned long    request_for;
	int              ret = 0;

	EMGD_TRACE_ENTER;

	/* Only supported for KMS-enabled driver */
	if (!config_drm.kms) {
		/* We should return an error here since this is not
		 * supported.  However, since we have previously returned 0
		 * for our non-KMS driver, this is left in to prevent any
		 * unforeseen problems. */
		EMGD_TRACE_EXIT;
		return 0;
	}


	/* Find the CRTC associated with the  */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		cur_emgd_crtc = container_of(crtc, emgd_crtc_t, base);

		if ((1 << crtc_select) == cur_emgd_crtc->crtc_id) {
			selected_emgd_crtc = cur_emgd_crtc;
			break;
		}		
	}

	if (NULL == selected_emgd_crtc) {
		EMGD_ERROR_EXIT("Invalid CRTC selected.");
		return -EINVAL;
	}


	switch (selected_emgd_crtc->igd_pipe->pipe_features & IGD_PORT_MASK) {
		case IGD_PORT_SHARE_LVDS:
			request_for = VBLANK_INT4_PORT4;
			break;

		case IGD_PORT_SHARE_DIGITAL:
			request_for = VBLANK_INT4_PORT2;
			break;

		default:
			EMGD_DEBUG("Unsupported port type");
			request_for = 0;
			ret         = -EINVAL;
			break;
	}

	if (0 == ret) {
		mmio = EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
		ret  = mode_context->dispatch->full->request_vblanks(request_for, mmio);

		if (ret) {
			EMGD_DEBUG("Failed to enable vblank");
		}
	}

	EMGD_TRACE_EXIT;
	return ret;
} /* emgd_driver_enable_vblank() */



/**
 * This is the drm_driver.disable_vblank() function.  It is called by
 * vblank_disable_fn() (in "drm_irq.c") to disable vblank interrupt events.
 *
 * @param dev         (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 * @param crtc_select (IN) Exactly how this is derived is unclear, but
 *                     for now we are assuming that it is the order
 *                     in which the CRTCs were creatd.  So 0 for
 *                     the first CRTC, 1 for the second, and so on.
 */
void emgd_driver_disable_vblank(struct drm_device *dev, int crtc_select)
{
	struct drm_crtc *crtc;
	unsigned char   *mmio;
	emgd_crtc_t     *cur_emgd_crtc, *selected_emgd_crtc = NULL;
	unsigned long    request_for;
	int              ret = 0;

	EMGD_TRACE_ENTER;


	/* Only supported for KMS-enabled driver */
	if (!config_drm.kms) {
		EMGD_TRACE_EXIT;
		return;
	}


	/* Find the CRTC associated with the  */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		cur_emgd_crtc = container_of(crtc, emgd_crtc_t, base);

		if ((1 << crtc_select) == cur_emgd_crtc->crtc_id) {
			selected_emgd_crtc = cur_emgd_crtc;
			break;
		}		
	}

	if (NULL == selected_emgd_crtc) {
		EMGD_ERROR_EXIT("Invalid CRTC selected.");
		return;
	}


	switch (selected_emgd_crtc->igd_pipe->pipe_features & IGD_PORT_MASK) {
		case IGD_PORT_SHARE_LVDS:
			request_for = VBLANK_INT4_PORT4;
			break;

		case IGD_PORT_SHARE_DIGITAL:
			request_for = VBLANK_INT4_PORT2;
			break;

		default:
			EMGD_DEBUG("Unsupported port type");
			request_for = 0;
			ret         = -EINVAL;
			break;
	}

	if (0 == ret) {
		mmio = EMGD_MMIO(mode_context->context->device_context.virt_mmadr);
		ret  = mode_context->dispatch->full->end_request(request_for, mmio);

		if (ret) {
			EMGD_DEBUG("Failed to disable vblank");
		}
	}

	EMGD_TRACE_EXIT;
	return;
} /* emgd_driver_disable_vblank() */



/**
 * This is the drm_driver.irq_preinstall() function.  It is called by
 * drm_irq_install() (in "drm_irq.c") before it installs this driver's IRQ
 * handler (i.e. emgd_driver_irq_handler()).
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 */
void emgd_driver_irq_preinstall(struct drm_device *dev)
{
	/* Notes on what to implement in this function:
	 *
	 * - Ditto the notes in emgd_driver_enable_vblank().
	 *
	 * - In addition, I'll note that I see a HAL interface that roughly
	 *   corresponds to this functionality--the interrupt_install() and
	 *   interrupt_uninstall() entry points in igd_dispatch_t (in "igd.h" and
	 *   better defined in "igd_interrupt.h").  However:
	 *
	 *    - There are no "plb" versions of this code, only "alm", "nap" & "wht".
	 *
	 *    - These install a passed-in interrupt handler.  As far as I can see
	 *      in the ssigd tree, these routines are never called, and no
	 *      interrupt handlers exist.
	 *
	 *    - Koheo doesn't contain the hal/core/interrupt code.  I'm not sure
	 *      what that does, but grep'ing through the source shows a number of
	 *      lines about irq's.
	 *
	 * - The open source Intel driver has some code that implements this.  It's
	 *   not small, so I haven't really studied it yet.  Can/should we go with
	 *   a similar approach, or keep like the current HAL approach and
	 *   structure some new entrypoints that allow future hardware to be
	 *   different?
	 */

	/* TODO -- REPLACE THIS STUB WITH A REAL IMPLEMENTATION  */
	printk(KERN_INFO "[EMGD] Inside of STUBBED %s()", __FUNCTION__);

} /* emgd_driver_irq_preinstall() */


/**
 * This is the drm_driver.irq_postinstall() function.  It is called by
 * drm_irq_install() (in "drm_irq.c") after it installs this driver's IRQ
 * handler (i.e. emgd_driver_irq_handler()).
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return 0 on Success
 * @return <0 if the given crtc's vblank interrupt cannot be enabled
 */
int emgd_driver_irq_postinstall(struct drm_device *dev)
{
	return 0;
} /* emgd_driver_irq_postinstall() */


/**
 * This is the drm_driver.irq_uninstall() function.  It is called by
 * drm_irq_install() (in "drm_irq.c") as a part of uninstalling this driver's
 * IRQ handler (i.e. emgd_driver_irq_handler()).
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 */
void emgd_driver_irq_uninstall(struct drm_device *dev)
{
	/* TODO -- REPLACE THIS STUB WITH A REAL IMPLEMENTATION THE IMG TRAINING */
	printk(KERN_INFO "[EMGD] Inside of STUBBED %s()", __FUNCTION__);

} /* emgd_driver_irq_uninstall() */


/**
 * This is the drm_driver.irq_handler() function, and is a Linux IRQ interrupt
 * handler (see the irq_handler_t in the Linux header "interrupt.h").  It is
 * installed by drm_irq_install() (in "drm_irq.c") by calling request_irq()
 * (implemented in "interrupt.h") with this function as the 2nd parameter.  The
 * return type is an enum (see the Linux header "irqreturn.h").
 *
 * Our HAL will have already installed an IRQ handler, so we do nothing here.
 *
 * @param dev (IN) DRM per-device (e.g. one GMA) struct (in "drmP.h")
 *
 * @return IRQ_NONE if the interrupt was not from this device
 * @return IRQ_HANDLED if the interrupt was handled by this device
 * @return IRQ_WAKE_THREAD if this handler requests to wake the handler thread
 */
irqreturn_t emgd_driver_irq_handler(int irq, void *arg)
{
	/* TODO -- REPLACE THIS STUB WITH A REAL IMPLEMENTATION THE IMG TRAINING */
	printk(KERN_INFO "[EMGD] Inside of STUBBED %s()", __FUNCTION__);

	return IRQ_NONE;
} /* emgd_driver_irq_handler() */

//TODO: TEST WITHOUT #if CLAUSE
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,3,0)
static int           emgd_pci_probe(struct pci_dev *pdev,
#else
static int __devinit emgd_pci_probe(struct pci_dev *pdev,
#endif
		const struct pci_device_id *ent)
{
	if (PCI_FUNC(pdev->devfn)) {
		return -ENODEV;
	}

	/*
	 * Name changed at some point in time.  2.6.35 uses drm_get_dev
	 * and 2.6.38 uses drm_get_pci_dev  Need to figure what kernel
	 * version this changed.
	 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	return drm_get_pci_dev(pdev, ent, &driver);
#else
	return drm_get_dev(pdev, ent, &driver);
#endif
}

static void emgd_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	drm_put_dev(dev);
}

static int emgd_pm_suspend(struct device *dev)
{
	return 0;
}

static int emgd_pm_resume(struct device *dev)
{
	return 0;
}

static int emgd_pm_freeze(struct device *dev)
{
	return 0;
}

static int emgd_pm_thaw(struct device *dev)
{
	return 0;
}

static int emgd_pm_poweroff(struct device *dev)
{
	return 0;
}

static int emgd_pm_restore(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops emgd_pm_ops = {
	.suspend = emgd_pm_suspend,
	.resume = emgd_pm_resume,
	.freeze = emgd_pm_freeze,
	.thaw = emgd_pm_thaw,
	.poweroff = emgd_pm_poweroff,
	.restore = emgd_pm_restore,
};

/*
 * NOTE: The remainder of this file is standard kernel module initialization
 * code:
 */


/*
 * Older kernels kept the PCI driver information directly in the
 * DRM driver structure.  Newer kernels (2.6.38-rc3 and beyond)
 * move it outside of the DRM driver structure and pass it to
 * drm_pci_init instead in order to help pave the way for
 * USB graphics devices.
 */
#define EMGD_PCI_DRIVER {    \
	.name     = DRIVER_NAME, \
	.id_table = pciidlist,   \
	.probe = emgd_pci_probe, \
	.remove = emgd_pci_remove, \
	.driver.pm = &emgd_pm_ops, \
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
static struct pci_driver emgd_pci_driver = EMGD_PCI_DRIVER;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#define IOCTL unlocked_ioctl
#else
#define IOCTL ioctl
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0))
#define EMGD_FOPS { \
    .owner   = THIS_MODULE,        \
    .open    = drm_open,           \
    .release = drm_release,        \
	.IOCTL   = drm_ioctl,		   \
    .mmap    = emgd_mmap,          \
    .poll    = drm_poll,           \
    .fasync  = drm_fasync,         \
    .read    = drm_read,           \
}
#else
#define EMGD_FOPS { \
    .owner   = THIS_MODULE,        \
    .open    = drm_open,           \
    .release = drm_release,        \
	.IOCTL   = drm_ioctl,		   \
    .mmap    = emgd_mmap,          \
    .poll    = drm_poll,           \
    .read    = drm_read,           \
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
static const struct file_operations emgd_driver_fops = EMGD_FOPS;
#endif
/**
 * DRM Sub driver entry points
 */
static struct drm_driver driver = {
	.driver_features    = DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED,
	.load               = emgd_driver_load,
	.unload             = emgd_driver_unload,
	.open               = emgd_driver_open,
	.lastclose          = emgd_driver_lastclose,
	.preclose           = emgd_driver_preclose,
	.postclose          = emgd_driver_postclose,
	.suspend            = emgd_driver_suspend,
	.resume             = emgd_driver_resume,
	.device_is_agp      = emgd_driver_device_is_agp,
	.get_vblank_counter = emgd_driver_get_vblank_counter,
	.enable_vblank      = emgd_driver_enable_vblank,
	.disable_vblank     = emgd_driver_disable_vblank,
	.irq_preinstall     = emgd_driver_irq_preinstall,
	.irq_postinstall    = emgd_driver_irq_postinstall,
	.irq_uninstall      = emgd_driver_irq_uninstall,
	.irq_handler        = emgd_driver_irq_handler,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
	.get_map_ofs        = drm_core_get_map_ofs,
	.get_reg_ofs        = drm_core_get_reg_ofs,
#endif
	.ioctls             = emgd_ioctl,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0))
    .fops                = &emgd_driver_fops,
#else
    .fops                = EMGD_FOPS,
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,38)
	.pci_driver          = EMGD_PCI_DRIVER,
#endif
	.name                = DRIVER_NAME,
	.desc                = DRIVER_DESC,
	.date                = DRIVER_DATE,
	.major               = DRIVER_MAJOR,
	.minor               = DRIVER_MINOR,
	.patchlevel          = DRIVER_PATCHLEVEL,
};

/**
 * Standard procedure to initialize this kernel module when it is loaded.
 */
static int __init emgd_init(void) {
	int ret;
	struct pci_dev *our_device;

	printk(KERN_INFO "[EMGD] Initializing Driver.\n");
	driver.num_ioctls = emgd_max_ioctl;

	/* If init == 1 then we should always set KMS to 0 for US15 */

	if(config_drm.init || drm_emgd_init == 1){
		
		/*  Detecting device */

		/* 
		 * 0x8086 is the intel vendor id and 0x8108 is the 
		 * US15 device id.
		 * pci_get_device returns NULL if it is not a PLB.
		 */

		our_device = pci_get_device(PCI_VENDOR_ID_INTEL, 
					PCI_DEVICE_ID_VGA_PLB, NULL);
		
		if(our_device){
			EMGD_ERROR("US15 detected. Setting KMS to 0 "
				"config_drm.kms = %d ", config_drm.kms);
			config_drm.kms = 0;
		}
	}

	if (config_drm.kms && (config_drm.init || drm_emgd_init == 1)) {
		driver.driver_features |= DRIVER_MODESET;
	}

	PVRDPFInit();

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	ret = drm_pci_init(&driver, &emgd_pci_driver);
#else
	ret = drm_init(&driver);
#endif
	printk(KERN_INFO "[EMGD] Driver Initialized.\n");
	EMGD_TRACE_EXIT;
	return ret;
}

/**
 * Standard procedure to clean-up this kernel module before it exits & unloads.
 */
static void __exit emgd_exit(void) {
	EMGD_TRACE_ENTER;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	drm_pci_exit(&driver, &emgd_pci_driver);
#else
	drm_exit(&driver);
#endif
	EMGD_TRACE_EXIT;
}


module_init(emgd_init);
module_exit(emgd_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL and additional rights");
