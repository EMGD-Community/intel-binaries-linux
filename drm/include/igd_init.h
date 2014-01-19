/*
 *-----------------------------------------------------------------------------
 * Filename: igd_init.h
 * $Revision: 1.15 $
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

#ifndef _IGD_INIT_H_
#define _IGD_INIT_H_

#include <igd.h>


/*!
 * @addtogroup init_group
 *
 * The init module contains the entry points and data structures
 * necessary to initialize the graphics driver HAL.
 * The IAL will call several initialization functions to bring the HAL
 * to a usable state, after which, the HAL is accessed through a dispatch
 * table (data structure of function pointers).
 *
 * Typical initialization will follow these steps:
 * - Initialize the HAL by calling igd_driver_init() to
 *   detect supported chipsets without altering the hardware. If a
 *   supported device is found the _igd_init_info data structure will be
 *   populated indicating the chipset name and PCI information for the device.
 *   A driver handle that should be used during the rest of the
 *   initialization process will be returned.
 *
 * - Call igd_driver_config() to prepare the hardware for use. Until this
 *   point there has been no alteration of the hardware. After the
 *   driver has been configured the igd_get_config_info() function can
 *   be used. Only a subset of the _igd_config_info data will be available
 *   at this time as the modular HAL components have not been initialized.
 *
 * - Call igd_module_init() to initialize the modular components of the
 *   driver. This function accepts a set of parameters that can alter
 *   default behavior. A provided dispatch table will be populated with
 *   the entry points used for hardware operations during the life of
 *   the driver. igd_get_config_info() may now be used to return full
 *   configuration details.
 *
 * @{
 */

/*!
 * @brief Data populated by igd_driver_init().
 *
 * The init info is used as input/output to igd_driver_init(). Any non-zero
 * members (with the exception of the name) will be used to limit the search
 * for available devices, if all members are zero the first supported
 * device found on the system will be used. In either case the data returned
 * will reflect the to-be-controlled device found by the HAL.
 *
 * The name is a ASCII string that may be used in human readable output
 * dialogs.
 */
typedef struct _igd_init_info {
	/*! @brief PCI Vendor ID */
	unsigned int vendor_id;
	/*! @brief PCI Device ID */
	unsigned int device_id;
	/*! @brief PCI Bus ID */
	unsigned int bus;
	/*! @brief PCI Slot ID */
	unsigned int slot;
	/*! @brief PCI Function ID */
	unsigned int func;
	/*! @brief ASCII chipset name */
	char *name;
	/*! @brief ASCII chipset ID */
	char *chipset;
	/*! @brief ASCII default port driver list */
	char *default_pd_list;
} igd_init_info_t;

/*!
 * @brief Function to initialize the HAL and detect supported chipset.
 *
 * Initialize the driver and determine if the hardware is supported without
 * altering the hardware state in any way. Init info is populated based on
 * collected data.
 *
 * A driver handle is returned. This handle is an opaque data structure
 * used with future HAL calls. The contents or meaning of the value are
 * not known outside the HAL; however, a NULL value should be considered
 * a failure.
 *
 * @param init_info Device details returned from the init process. This
 *   structure will be populated by the HAL during the call.
 *
 * @return igd_driver_h (Non-NULL) on Success
 * @return NULL on failure
 */
igd_driver_h igd_driver_init(igd_init_info_t *init_info);


/*!
 *  Configure the driver. This sets the driver for future operation, it
 * may alter the hardware state. Must be called after driver init and
 * before ANY other driver commands.
 *
 * @param driver_handle: A driver handle as returned by igd_driver_init()
 *
 * @return <0 on Error
 * @return 0 On Success
 */
int igd_driver_config(igd_driver_h driver_handle);



/*!
 * @defgroup init_param Initialization Parameters
 *
 * Initialization parameters are passed to igd_module_init() to control
 * configurable behavior of the HAL. Some parameters are global in that
 * they have an effect on the entire driver. In addition, there are
 * parameters that are specific to a display. These parameters are provided
 * in a list with each list entry representing a defined "port number" as
 * follows shown here:
 *
<PRE>
                                        /|-----------|
                                       / | Fp Info   |
                                      /  |-----------|
        |----------|   /|---------|  /
        | Params   |  / | Display | /    |-----------|
        |          | /  | Param 1 |/     | DTD Array |
        |          |/   |---------|\     |-----------|
        |----------|\               \
                     \  |---------|  \   |-----------|
                      \ | Display |   \  | Attribute |
                       \| Param N |    \ | Array     |
                        |---------|     \|-----------|
</PRE>
 *
 * The port numbers are mapped to hardware specific outputs as defined
 * here:
 *
 * 830M and later Port numbers:
 *   - 1 DVO A port
 *   - 2 DVO B port
 *   - 3 DVO C port
 *   - 4 Internal LVDS port
 *   - 5 Analog port
 *
 *   On 835: If RGBA is used (DVO B & C together), then use DVO B number
 *   to specify any parameter for it.
 *
 * 810/815 Port numbers:
 *   - 3 DVO port
 *   - 5 Analog port
 *
 * NOTE: The permanence of the Params data structure that is passed from
 * the IAL to the HAL in igd_module_init API is not guaranteed by the IAL
 * Therefore, the different HAL modules should use the data out of this
 * data structure only until igd_module_init and not after that.
 * @{
 */

/*!
 * This parameter, when set, will cause the driver to save the current
 * register state of the device prior to altering it in any way. This
 * will allow the state of the device to be reapplied at exit time.
 */
#define IGD_DRIVER_SAVE_RESTORE   0x01

/*!
 * @name Per-Display Present Params Flags
 * @anchor present_params_flags
 *
 * These flags are used to identify which parameters are being provided to
 * the HAL. If a bit is not set in the _igd_display_params::present_params
 * variable the parameters will not be read by the HAL and the default
 * behavior will be used instead.
 *
 * - IGD_PARAM_DDC_GPIO Parameter to select non-standard GPIO pair for DDC.
 * - IGD_PARAM_DDC_SPEED Parameter to select non-standard DDC speed.
 * - IGD_PARAM_DDC_DAB Parameter to select the Device Address Byte to use for
 *     DDC communication
 * - IGD_PARAM_I2C_GPIO Parameter to select non-standard GPIO pair for i2c.
 * - IGD_PARAM_I2C_SPEED Parameter to select non-standard I2C speed.
 * - IGD_PARAM_DAB Parameter to select the Device Address Byte to use for I2C
 *     communication.
 * - IGD_PARAM_FP_INFO Parameter to provide non-detectable Flat Panel
 *     configuration data.
 * - IGD_PARAM_DTD_LIST Parameter to provide a Detailed Timing List for a
 *     display.
 * - IGD_PARAM_ATTR_LIST Parameter to provide a set of Port Driver Attributes
 *     to use during init.
 *
 * @{
 */
#define IGD_PARAM_DDC_GPIO        0x00000001
#define IGD_PARAM_DDC_SPEED       0x00000002
#define IGD_PARAM_DDC_DAB         0x00000004
#define IGD_PARAM_I2C_GPIO        0x00000008
#define IGD_PARAM_I2C_SPEED       0x00000010
#define IGD_PARAM_DAB             0x00000020
#define IGD_PARAM_FP_INFO         0x00000040
#define IGD_PARAM_DTD_LIST        0x00000080
#define IGD_PARAM_ATTR_LIST       0x00000100
/*! @} */


/*!
 *
 *
 *
 * @note Any changes to the above number assignment will break the driver.
 *       If change is required, make sure to change the port numbers in
 *       port tables.
 *
 */

/* The method for controlling the flat panel power. The options include:
 * no support or the Port Driver handles flat panel power. */
#define IGD_PARAM_FP_PWR_METHOD_NONE 0
#define IGD_PARAM_FP_PWR_METHOD_PD   1

/*!
 * @brief Port-specific color correction information
 *
 * Initialization parameter passed as part of igd_module_init() to provide
 * information about an attached display device. One color_correct_info
 * is provided for each display port.
 *
 */
typedef struct _igd_param_color_correct_info {
	/*! @brief RED value for GAMMA */
	unsigned short gamma_r;
	/*! @brief GREEN value for GAMMA */
	unsigned short gamma_g;
	/*! @brief BLUE value for GAMMA */
	unsigned short gamma_b;
	/*! @brief RED value for BRIGHTNESS */
	unsigned short brightness_r;
	/*! @brief GREEN value for BRIGHTNESS */
	unsigned short brightness_g;
	/*! @brief BLUE value for BRIGHTNESS */
	unsigned short brightness_b;
	/*! @brief RED value for CONTRAST */
	unsigned short constrast_r;
	/*! @brief GREEN value for CONTRAST */
	unsigned short constrast_g;
	/*! @brief BLUE value for CONTRAST */
	unsigned short constrast_b;
} igd_param_color_correct_info_t;



/*!
 * @brief Flat Panel Information
 *
 * Initialization parameter passed as part of igd_module_init() to provide
 * information about an attached Flat Panel (typically LVDS). One fp_info
 * is provided for each display port, and is only used when the
 * IGD_PARAM_FP_INFO flag is set in the _igd_display_params::present_params
 *
 */
typedef struct _igd_param_fp_info {
	/*! @brief Flat panel width */
	unsigned long fp_width;
	/*! @brief Flat panel height */
	unsigned long fp_height;
	/*! @brief Flat Panel Power Method */
	unsigned long fp_pwr_method;
	/*!
	 * @brief Min time delay in miliseconds between VDD active and clock/data
	 * active.
	 */
	unsigned long fp_pwr_t1;
	/*!
	 * @brief Min time delay in miliseconds between clock/data active and
	 * backlight enable.
	 */
	unsigned long fp_pwr_t2;
	/*!
	 * @brief Min time delay in miliseconds between Backlight disable and
	 * clock/data inactive
	 */
	unsigned long  fp_pwr_t3;
	/*!
	 * @brief Min time delay in miliseconds between clock/data inactive and
	 * VDD inactive.
	 */
	unsigned long  fp_pwr_t4;
	/*!
	 * @brief Min time delay in miliseconds between VDD inactive and
	 * VDD active.
	 */
	unsigned long  fp_pwr_t5;
} igd_param_fp_info_t;

/*!
 * @brief Per-display init-time list of DTDs.
 *
 * In the case of EDID-less display device, this parameter provides
 * the DTD (Detailed Timing Descriptor) list to the HAL during initialization.
 */
typedef struct _igd_param_dtd_list {
	/*! @brief number of DTDs */
	unsigned long  num_dtds;
	/*! @brief DTD list */
	igd_display_info_t  *dtd;
} igd_param_dtd_list_t;

/*!
 * @brief Port driver attribute
 */
typedef struct _igd_param_attr {
	/*! @brief See @ref attr_id_defs for predefined IDs */
	unsigned long  id;
	/*! @brief value or index(incase of list type attr) */
	unsigned long  value;
} igd_param_attr_t;

/*!
 * @brief Port driver init-time attribute list
 *
 * This data structure is used during HAL initialization. It should be
 * populated with any port driver attributes that were modified and saved
 * in a prior use of the HAL.
 */
typedef struct _igd_param_attr_list {
	/*! @brief  number of attributes in the list */
	unsigned long  num_attrs;
	/*! @brief IAL allocated attribute list */
	igd_param_attr_t *attr;
} igd_param_attr_list_t;



/*!
 * @brief Per-Display Init-time configuration parameters.
 *
 * These parameters are provided as an array during igd_module_init().
 * They allow the default behavior of the driver to be altered on a
 * per-display basis. They are input-only to provide data
 * about non-standard hardware configurations and persistant driver state.
 */
typedef struct _igd_display_params {
	/*! @brief Port to which these parameters apply. See @ref init_param */
	unsigned long port_number;
	/*! @brief parameters present bitfield: See @ref present_params_flags */
	unsigned long present_params;
	unsigned long flags;
	/*!
	 * @brief Timing sources to use with Edid displays.
	 * See @ref advanced_edid
	 */
	unsigned short edid_avail;
	/*!
	 * @brief Timings sources to use with Edid-less displays.
	 * See @ref advanced_edid
	 */
	unsigned short edid_not_avail;
	unsigned long ddc_gpio;       /* DDC GPIO pin pair number 0..6 */
	unsigned long ddc_speed;      /* DDC speed in KHz to read EDID */
	unsigned long ddc_dab;        /* DDC DAB to read EDID from display device */
	unsigned long i2c_gpio;       /* I2C GPIO pin pair number 0..6 */
	unsigned long i2c_speed;      /* I2C speed in KHz to read EDID */
	unsigned long i2c_dab;        /* I2C DAB to communicate with DVO device */
	igd_param_fp_info_t fp_info;  /* Connected FP Info */
	/*!
	 * DTD parameter: Incase of EDID-less display device, this parameter
	 * provides the DTD (Detailed Timing Descriptor) list
	 */
	igd_param_dtd_list_t dtd_list;

	igd_param_attr_list_t attr_list;
} igd_display_params_t;

/*!
 * @name Advanced Edid Flags
 * @anchor advanced_edid
 *
 * Flags to use with edid_avail and edid_not_avail:
 * perport edid_avail/edid_not_avail
 *
 * IGD_DISPLAY_USE_STD_TIMINGS:
 *   If not set: Do not use driver built-in standard timings
 *   If set:     Use driver built-in standard timings
 *
 * IGD_DISPLAY_USE_EDID:  not applicable to edid_not_avail
 *   If not set: Do not use EDID block
 *   If set:     Use EDID block and filter modes
 *
 * IGD_DISPLAY_USE_USERDTDS:
 *   If not set: Do not use user-DTDs
 *   If set:     Use user-DTDs.
 *
 * @{
 */
#define IGD_DISPLAY_USE_STD_TIMINGS    0x1
#define IGD_DISPLAY_USE_EDID           0x2
#define IGD_DISPLAY_USE_USERDTDS       0x4
/*! @} */


/* Flags for igd_display_params_t->flags*/
#define IGD_DISPLAY_READ_EDID     0x00000001  /* igd_display_params_t.flags */

/*!
 * @anchor display_flags_def
 *
 * These flags may be used in the display_flags member of the
 * igd_params_t data structure.
 *
 */
#define IGD_DISPLAY_MULTI_DVO     0x00000002  /* igd_param_t.display_flags  */
#define IGD_DISPLAY_DETECT        0x00000004  /* igd_param_t.display_flags  */
#define IGD_DISPLAY_FB_BLEND_OVL  0x00000008  /* igd_param_t.display_flags  */
#define IGD_DISPLAY_BATCH_BLITS   0x00000010
	/*! @brief enable dynamic blending of display frame buffer with overlay */



/*!
 * @brief Init-Time Driver Parameters
 *
 * The igd_param_t data structure contains  global init-time static parameters
 * to impact the behavior of the whole driver. They are provided to the driver
 * during module initialization to control overall driver behavior.
 *
 *
 *           VERY IMPORTANT!!! CHANGES TO DEFINITION OF THIS STRUCTURE AND
 *           ITS MEMBER STRUCTURES MUST BE FOLLOWED BY UPDATES TO THE
 *           FOLLOWING COMPONENTS / FILES:
 *                       1. ssigd/ial/vbios/usrbld/igd_uinit.h
 *                       2. ssigd/ial/vbios/src/core/user_config.c
 *                       3. pcf2iegd tool (gens user_config.c/h for usr-bld)
 *                       4. CED tool (generates user_config.c/h for usr-bld)
 *
 *
 *
 * See igd_module_init()
 */
typedef struct _igd_param {
	/*! @brief Maximum pages taken by the driver for offscreen memory */
	unsigned long        page_request;
	/*! @brief Maximum pages reserved for the framebuffer. */
	unsigned long        max_fb_size;
	/*! @brief Boolean option to preserve initial hardware state */
	unsigned char        preserve_regs;
	/*! @ref display_flags_def "Display Flags" */
	unsigned long        display_flags;
	/*!
	 * @brief Port detection order.
	 *
	 * HAL will detect the displays in the  order mentioned and also allocate
	 * in the same order if found.
	 *
	 * Example: To detect/allocate in the order on 855:
	 * -# DVOB PORT
	 * -# ANALOG PORT
	 * -# LVDS PORT
	 * then specify port_number[] = {2, 5, 4, 0};
	 *
	 * @note Set port_number[] = {0, 0, 0, 0}; to work in the default order.
	 *
	 * @note If a invalid number is specified then it will be skipped.
	 */
	unsigned long port_order[IGD_MAX_PORTS];

	/*! Contains the Per-Display init-time parameters. */
	igd_display_params_t display_params[IGD_MAX_PORTS];

	/*! @brief RGB color that will be used while clearing the framebuffer */
	unsigned long        display_color;

    unsigned long       quickboot;
    int                 qb_seamless;
    unsigned long       qb_video_input;
    int                 qb_splash;

	/*! Override interrupt support and revert to polling */
	int                 polling;
	unsigned long 		ref_freq;
	int 				tuning_wa;
	unsigned long 		clip_hw_fix;

	
	/* Async flip flickering workaround enable */
	unsigned long		async_flip_wa;
	
	/*
	 * Enable override of following registers when en_reg_override=1.
	 * Display Arbitration, FIFO Watermark Control, GVD HP_CONTROL,
	 * Bunit Chickenbits, Bunit Write Flush, Display Chickenbits
	 */
	unsigned long		en_reg_override;
	unsigned long		disp_arb;
	unsigned long		fifo_watermark1;
	unsigned long		fifo_watermark2;
	unsigned long		fifo_watermark3;
	unsigned long		fifo_watermark4;
	unsigned long		fifo_watermark5;
	unsigned long		fifo_watermark6;
	unsigned long		gvd_hp_control;
	unsigned long		bunit_chicken_bits;
	unsigned long		bunit_write_flush;
	unsigned long		disp_chicken_bits;
	int					punt_to_3dblit;

} igd_param_t;

typedef struct {
	unsigned debug : 1;
	unsigned ddk_version : 16;
	unsigned emgd_version : 15;
} igd_build_config_t;

/*! @} */

/*!
 *
 *  Initializes individual modules to a runable state. Init time parameters
 * may be provided to alter the default behavior of the driver.
 * See @ref init_param
 *
 * The dispatch table for all graphics operations is returned. The dispatch
 * table may return NULL pointers for unsupported functions due to
 * optional modules. This dispatch table is used to access HAL functionality
 * throughout the life of the driver.
 * See @ref _igd_dispatch
 *
 * @param driver_handle as returned from igd_driver_init().
 * @param dsp dispatch table to be populated during the call.
 * @param params Input parameters to alter default behavior.
 *   See @ref init_param
 *
 * @return 0 Success
 * @return <0 on Error
 */
int igd_module_init(igd_driver_h driver_handle,
	igd_dispatch_t **dsp,
	igd_param_t *params);



/*!
 * @name Framebuffer Capabilities
 * @anchor fb_caps
 *
 * FB caps are an indication of large feature sets that will or will not
 * be available based on FB pixel format. This allows an IAL to intelligently
 * configure itself to the appropriate setup before setting a display
 * mode.
 * FB caps are returned from a call to igd_get_config_info().
 *
 * - IGD_CAP_BASIC_2D This capability bit indicates that the device is capable
 *    of performing basic 2d acceleration.
 * - IGD_CAP_FULL_2D This capability bit indicates that the device is capable
 *    of performing full 2d acceleration.
 * - IGD_CAP_BLEND This capability bit indicates that the device is capable of
 *    performing the blend() function to the framebuffer.
 *
 * @{
 */
#define IGD_CAP_BASIC_2D  0x01
#define IGD_CAP_FULL_2D  (0x02 | IGD_CAP_BASIC_2D)
#define IGD_CAP_BLEND     0x04
/*! @} */


/*!
 * @brief Device Capabilities based on FB mode
 *
 * This structure reports the Framebuffer capabilities on a pixel format
 * basis. An IAL should check these capabilities before using acceleration
 * features. This structure is returned as part of the _igd_config_info
 * information from the igd_get_config_info() call.
 *
 * @see pixel_formats
 */
typedef struct _igd_fb_caps {
	/*! @brief The framebuffer pixel format that these caps reference */
	unsigned long pixel_format;
	/*! @brief The capability bits as defined by @ref fb_caps */
	unsigned long caps;
} igd_fb_caps_t;

/*!
 * @brief Static HAL configuration data
 *
 * Config info contains the static configuration information for the device
 * that is found during the call to igd_driver_config(). It is obtained
 * with a call to igd_get_config_info() which may be done before or after
 * module configuration with limited success.
 */
typedef struct _igd_config_info {
	/*! @brief The Base MMIO physical (Bus) address. */
	unsigned long mmio_base_phys;
	/*! @brief The Base MMIO virtual address. */
	unsigned char *mmio_base_virt;
	/*! @brief The Base Video Memory physical (Bus) address. */
	unsigned long gtt_memory_base_phys;
	/*! @brief The Base Video Memory virtual address. */
	unsigned char *gtt_memory_base_virt;
	/*! @brief The Video Memory Size. */
	unsigned long gtt_memory_size;
	/*! @brief The number of display planes supported by the device. */
	unsigned long num_dsp_planes;
	/*! @brief The number of display pipes supported by the device. */
	unsigned long num_dsp_pipes;
	/*! @brief The number of currently active display ports. */
	unsigned long num_act_dsp_ports;
	/*! @brief Caps, terminated with PF = 0 */
	igd_fb_caps_t *fb_caps;
	/*! @brief The device revision id. */
	unsigned long revision_id;
	/*! @brief HW status page offset (priviledged use only) */
	unsigned long hw_status_offset;
	/*! @brief The base of stolen memory */
	unsigned long stolen_memory_base_virt;
	/* pixel format that matches the bpp that was passed in */
	unsigned long pixel_format;
	/* port-specific rotation read from DisplayID */
	igd_DID_rotation_info_t displayid_rotation[IGD_MAX_PORTS];
} igd_config_info_t;


/*!
 * @brief Static GTT configuration data
 *
 * This holds the configuration of the GTT and the information
 * required to handle video memory allocation/free.
 *
 * FIXME: There is a lot of duplication with information stored
 * else were (like igd_config_info_t) that needs to be cleaned up.
 */
typedef struct _igd_gtt_info_t {
	unsigned long gatt_start;
	unsigned long gatt_pages;
	unsigned long gtt_start;
	unsigned long gtt_pages;
	unsigned long gtt_phys_start;
	unsigned long stolen_mem_base;
	unsigned long stolen_mem_size;
	unsigned long stolen_pages;
	unsigned long pge_ctl;
	unsigned short gmch_ctl;
	unsigned long *gtt_mmap;
	void *vram_virt;
	unsigned long initialzied;
	void *scratch_page;
} igd_gtt_info_t;



/*!
 * The igd_get_config_info() function call can be used to get static
 * configuration information details that the IAL may need. This
 * function may be called prior to igd_module_init() with limited
 * success; however, it is recommeneded that it be called after the
 * modular HAL components have been initialized.
 *
 * In addition to static configuration there may be dynamic configuration
 * details that can be queried at any time after igd_driver_config().
 * These are defined with unique identifiers and are queried one at a
 * time with igd_get_param().
 *
 *  @param driver_handle as retuned from igd_driver_init()
 *  @param config_info Populated with hardware information during the call..
 *
 *  @return 0 Success
 *  @return -IGD_INVAL Error
 */
int igd_get_config_info(igd_driver_h driver_handle,
	igd_config_info_t *config_info);



/*!
 * @defgroup runtime_param Runtime Parameter Control
 *
 * HAL parameters may be queried and set at runtime using the
 * _igd_dispatch::get_param() and _igd_dispatch::set_param() dispatch functions
 * (when called after initialization) or the
 * igd_get_param() and igd_set_param() functions when called during
 * initialization.
 * Some implemtations may not support all parameters so IAL's must handle
 * errors. Additionally, some parameters are hardware specific and of no use to
 * general purpose IALs. These should only be used by IALs with extensive
 * hardware knowledge (OpenGL, D3D, etc)
 *
 * @{
 */

/*!
 * @name Runtime Parameter Defines
 *
 * - IGD_PARAM_PANEL_ID Primary Panel ID
 *    This is a unique Flat Panel Identifier that is obtained from firmware.
 *    The IAL may use this information to send in Detailed Timing Descriptors
 *    to igd_module_init(). This facilitates the use of multiple local Flat
 *    panel configurations without EDID. When available this parameter may
 *    be queried prior to igd_module_init().
 * - IGD_PARAM_MEM_RESERVATION Bios memory reservation.
 *    This is the amount (in bytes?) of memory that the bios has already
 *    installed in the Gart prior to driver load. It is used when a
 *    non-standard bios or firmware has pre-allocated an undetectable amount
 *    of video memory for splash screen use. When available this parameter can
 *    be queried prior to igd_module_init().
 * - IGD_PARAM_DEBUG_MASK Debug Printing Mask.
 *    This parameter is read/write and may be altered at any time after
 *    igd_driver_init(). It is a bitfield to control different printing
 *    groups for debug builds.
 * - IGD_PARAM_PORT_LIST A bitfield used by the vBIOS to override the
 *    display detect capabilities in the driver.
 * - IGD_PARAM_HW_CONFIG Hardware configuration Bitfield
 *    contains one of these dependeing on platform:
 *    IGD_ALM_HW_CONFIG_BIN or IGD_NAP_HW_CONFIG_BIN both of which indicate
 *    the presence and availability of the hardware binner. This value should
 *    be queried and set (removing the bit) by an IAL claiming use of the
 *    binner.
 * - IGD_PARAM_INTR_STATUS Unknown FIXME document this
 *
 * @{
 */
#define IGD_PARAM_PANEL_ID        0x01
#define IGD_PARAM_MEM_RESERVATION 0x02
#define IGD_PARAM_DEBUG_MASK 0x03
#define IGD_PARAM_PORT_LIST  0x04
#define IGD_PARAM_GFX_FREQ  0x05
#define IGD_PARAM_SET_LVDS  0x06
#define IGD_PARAM_HW_CONFIG 0x1000
#define  IGD_ALM_HW_CONFIG_BIN 0x1
#define  IGD_NAP_HW_CONFIG_BIN 0x1
#define IGD_PARAM_INTR_STATUS 0x1001

/*! @} */

/*!
 * Gets the value of a runtime driver parameter. These parameters are
 * each defined with a unique ID and may be altered at runtime.
 *
 * Note: There is a wrapper for this function in the dispatch table that
 * takes a display instead of a driver handle. This version is for use
 * when displays are not yet available.
 *
 * @bug Runtime parameter documentation needs updates
 *
 * @return 0 Success
 * @return -IGD_INVAL Error
 */
int igd_get_param(igd_driver_h driver_handle,
	unsigned long id,
	unsigned long *value);

/*!
 * Sets the value of a runtime driver parameter. These parameters are
 * each defined with a unique ID and may be altered at runtime.
 *
 * Note: There is a wrapper for this function in the dispatch table that
 * takes a display instead of a driver handle. This version is for use
 * when displays are not yet available.
 *
 * @return 0 Success
 * @return  -IGD_INVAL Error
 */
int igd_set_param(igd_driver_h driver_handle,
	unsigned long id,
	unsigned long value);

/*! @} Runtime Param Group */
/*! @} Init Group */


/*!
 * @ingroup cleanup
 * @brief Shuts down the HAL
 *
 * This function shuts down the HAL and frees and remaining resources.
 * It should be called at driver exit to leave the hardware in a safe
 * configuration.
 *
 * @param driver_handle The driver handle returned from igd_driver_init().
 *
 * @return void
 */
void igd_driver_shutdown(igd_driver_h driver_handle);

/*!
 * @ingroup cleanup
 * @brief Shuts down the HAL
 *
 * This function shuts down the HAL and frees and remaining resources.
 * It should be called at driver exit to leave the hardware in a safe
 * configuration.
 *
 * @param driver_handle The driver handle returned from igd_driver_init().
 *
 * @return void
 */
void igd_driver_shutdown_hal(igd_driver_h driver_handle);


/*!
 * @ingroup cleanup
 * @brief Query 2D capability of the hardware.
 *
 * This function query the hardware whether given capabilities value 
 * (caps_val) is supported by hardware.
 *
 * @param driver_handle The driver handle returned from igd_driver_init().
 *
 * @return void
 */
void igd_query_2d_caps_hwhint(igd_driver_h driver_handle, 
			unsigned long caps_val,
			unsigned long *status);



/* 2D capabilities to query */
#define IGD_2D_CAPS_BLT                 0
  	 
/* Output of 2D capabilities to query */
#define IGD_2D_HW_DISABLE                       0
#define IGD_2D_HW_ENABLE                        1
#define IGD_2D_CAPS_UNKNOWN                     2
  	 


/*!
 * @addtogroup power_group
 * @{
 */

/*!
 * @name Driver Save Flags
 * @anchor driver_save_flags
 *
 * Flags for use with dispatch->driver_save()
 * @{
 */
#define IGD_REG_SAVE_VGA       0x00001
#define IGD_REG_SAVE_DAC       0x00002
#define IGD_REG_SAVE_MMIO      0x00004
#define IGD_REG_SAVE_RB        0x00008
#define IGD_REG_SAVE_VGA_MEM   0x00010
#define IGD_REG_SAVE_MODE      0x00020
#define IGD_REG_SAVE_BACKLIGHT 0x00040
#define IGD_REG_SAVE_3D        0x00080
#define IGD_REG_SAVE_GTT       0x00100
#define IGD_REG_SAVE_TYPE_REG  0x10000
#define IGD_REG_SAVE_TYPE_CON  0x20000
#define IGD_REG_SAVE_TYPE_MISC 0x40000
#define IGD_REG_SAVE_TYPE_MASK 0xF0000

#define IGD_REG_SAVE_ALL (IGD_REG_SAVE_VGA | IGD_REG_SAVE_DAC |  \
		IGD_REG_SAVE_MMIO | IGD_REG_SAVE_RB | IGD_REG_SAVE_VGA_MEM | \
		IGD_REG_SAVE_MODE | IGD_REG_SAVE_BACKLIGHT | IGD_REG_SAVE_3D | \
		IGD_REG_SAVE_GTT )

/*!
 * @note: This macro does not save the mode (i2c) regs. You have to
 * explicitly ask for that too if you want it.
 */
#define IGD_REG_SAVE_STATE (IGD_REG_SAVE_VGA | IGD_REG_SAVE_DAC | \
		IGD_REG_SAVE_MMIO | IGD_REG_SAVE_RB | IGD_REG_SAVE_VGA_MEM | \
		IGD_REG_SAVE_GTT )
/*! @} */


/*! @} */

#endif /* _IGD_INIT_H_ */
