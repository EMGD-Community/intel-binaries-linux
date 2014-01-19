/*
 *-----------------------------------------------------------------------------
 * Filename: pi.c
 * $Revision: 1.25 $
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
 *  This file contains all the necessary functions for port interface
 *  module. This module abstracts all hardware port interfaces and
 *  manages them.
 *-----------------------------------------------------------------------------
 */

#define MODULE_NAME hal.dpd

#include <config.h>
#include <igd.h>
#include <igd_errno.h>
#include <igd_init.h>
#include <igd_pwr.h>

#include <io.h>
#include <pci.h>
#include <sched.h>
#include <memory.h>

#include <context.h>
#include <mode.h>
#include <utils.h>
#include <dsp.h>
#include <debug.h>
#include <pi.h>
#include <pd.h>
#include <pd_init.h>
#include <intelpci.h>
#include <dispatch.h>
#include <mode_access.h>
#include <edid.h>
#include <displayid.h>
#include <emgd_drv.h>

#include "i2c_dispatch.h"
#include <igd_vga.h>
#include <context.h>

/*!
 * @addtogroup display_group
 * @{
 */

typedef struct _pi_context {
	igd_context_t *igd_context;
	i2c_dispatch_t *i2c_dispatch;
	unsigned long num_pi_drivers;
} pi_context_t;

/* Function to filter the modes using EDID or DisplayID*/
int get_firmware_timings(igd_display_port_t *port,
	unsigned char  *firmware_data, pd_timing_t *timing_table);

int pi_pd_init(igd_display_port_t *port, unsigned long port_feature,
	unsigned long second_port_feature, int drm_load_time);
#ifndef CONFIG_MICRO
unsigned long get_magic_cookie(pd_driver_t *pd_driver);
#endif
void assign_dynamic_numbers(igd_timing_info_t *timing_table);
int update_attrs(igd_display_port_t *port);
pd_timing_t *get_user_timings(igd_param_dtd_list_t *in_list);


extern int pi_init_all(void *handle);

extern emgd_drm_config_t config_drm;
extern i2c_dispatch_t i2c_dispatch_plb;
extern i2c_dispatch_t i2c_dispatch_tnc;

int null_func( void )
{
	return -IGD_ERROR_NODEV;
}
/* Not currently used
static i2c_dispatch_t i2c_dispatch_null = {
	(void *)null_func,
	(void *)null_func,
	(void *)null_func,
	(void *)null_func
}; */


static dispatch_table_t i2c_dispatch_list[] = {

#ifdef CONFIG_PLB
	{PCI_DEVICE_ID_VGA_PLB, &i2c_dispatch_plb},
#endif
#ifdef CONFIG_TNC
	{PCI_DEVICE_ID_VGA_TNC, &i2c_dispatch_tnc},
#endif

	{0, NULL}
};

static unsigned char firmware_data[256];

static pi_context_t pi_context[1];

/*----------------------------------------------------------------------
 *                        FUNCTION DEFINITIONS
 *----------------------------------------------------------------------*/
#ifndef CONFIG_MICRO
/*!
 *
 * @param context
 *
 * @return void
 */
static void pi_shutdown(igd_context_t *context)
{
	igd_display_port_t *port;

	EMGD_TRACE_ENTER;

	if (pi_context->igd_context == NULL) {
		return;
	}

	/* Close the port drivers */
	port = NULL;
	while ((port = context->mod_dispatch.dsp_get_next_port(context, port, 0)) != NULL) {
		if (port->pd_driver) {
			port->pd_driver->pd_close(port->pd_context);
			port->pd_driver = NULL;
			/* pd_context is freed by port driver */
			port->pd_context = NULL;
			/* timing_table is freed by port driver */
			port->timing_table = NULL;
			port->num_timing = 0;
			if (port->fp_info) {
				OS_FREE(port->fp_info);
				port->fp_info = NULL;
			}
			if (port->callback) {
				OS_FREE(port->callback);
				port->callback = NULL;
			}
		}
	}

	EMGD_TRACE_EXIT;
	return;
}

/*!
 *
 * @param context
 *
 * @return 0
 */
int pi_full_init(igd_context_t *context)
{
	/* Optional Inter-module interfaces */
	context->mod_dispatch.pi_shutdown = pi_shutdown;
	return 0;
}

#endif

/*!
 *
 * @param context
 * @param config_info
 *
 * @return 0
 */
static int pi_get_config_info(igd_context_t *context,
	igd_config_info_t *config_info)
{

	igd_display_port_t   *port = NULL;
	igd_param_t          *init_params = NULL;
	igd_display_params_t *display_params = NULL;
	int                   i;

	EMGD_TRACE_ENTER;

	EMGD_ASSERT(context, "Null context", -IGD_ERROR_INVAL);
	EMGD_ASSERT(config_info, "Null config_info", -IGD_ERROR_INVAL);

	config_info->num_act_dsp_ports = pi_context->num_pi_drivers;

	init_params = pi_context->igd_context->mod_dispatch.init_params;

	while ((port = pi_context->igd_context->mod_dispatch.
		dsp_get_next_port(pi_context->igd_context, port, 0)) != NULL) {

		/* Get the display params to check if the user enabled EDID */
		for (i = 0; i < IGD_MAX_PORTS; i++) {
			if (init_params->display_params[i].port_number == port->port_number) {
				display_params = &init_params->display_params[i];
				break;
			}
		}

		/* If DID rotation info is available, pass it to user-space through the
		 * config_info struct */
		if ((!display_params || (display_params->flags & IGD_DISPLAY_READ_EDID))
			&& (port->firmware_type == PI_FIRMWARE_DISPLAYID
			&&	port->displayid != NULL)) {

			config_info->displayid_rotation[port->port_number - 1].rotation =
				port->displayid->rotation_info.rotation;
			config_info->displayid_rotation[port->port_number - 1].flip =
				port->displayid->rotation_info.flip;

		} else {
			config_info->displayid_rotation[port->port_number - 1].rotation = 0;
			config_info->displayid_rotation[port->port_number - 1].flip = 0;
		}
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param context
 *
 * @return 0
 */
int pi_init(igd_context_t *context)
{
	i2c_dispatch_t *i2c_dispatch;

	EMGD_TRACE_ENTER;

	OS_MEMSET(pi_context, 0, sizeof(pi_context_t));

	/* Save igd_context in local_igd_context. */
	pi_context->igd_context = context;

	/* Get I2C dispatch table */
	i2c_dispatch = (i2c_dispatch_t *)dispatch_acquire(context,
		i2c_dispatch_list);
	if(!i2c_dispatch) {
		EMGD_DEBUG("No i2c Dispatch available for PI module");
	}
	pi_context->i2c_dispatch = i2c_dispatch;

	/*
	 * If Dynamic Port drivers are not used then init the static drivers
	 * now.
	 */
#ifndef IGD_DPD_ENABLED
	{
		void *handle = NULL;
		int ret;
		ret = pi_init_all(handle);
	}
#endif

	/* Inter-module dispatch functions */
	context->mod_dispatch.i2c_read_regs = i2c_dispatch->i2c_read_regs;
	context->mod_dispatch.i2c_write_reg_list =
		i2c_dispatch->i2c_write_reg_list;
	context->mod_dispatch.pi_get_config_info = pi_get_config_info;

	OPT_MICRO_CALL(pi_full_init(context));

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 * Get a port with the requested feature set from the list. Don't allocate
 * it just return it.  Only consider ports that aren't already in use.
 *
 * @param feature
 * @param last
 *
 * @return port on success
 * @return NULL on failure
 */
igd_display_port_t *pi_get_feature_port(unsigned long feature,
	igd_display_port_t *last)
{
	igd_display_port_t *port;
	inter_module_dispatch_t *md = &pi_context->igd_context->mod_dispatch;

	while ((port = md->dsp_get_next_port(pi_context->igd_context, last, 0))) {
		if (!port->inuse) {
			if (feature) {
				if (port->port_features & feature) {
					return port;
				}
			} else {
				return port;
			}
		}
		last = port;
	}

	return NULL;
}

/*!
 * Function to register port driver with display driver
 *
 * @param pd_driver
 *
 * @return PD_SUCCESS on success
 * @return PD_ERR_NULL_PTR, PD_ERR_VER_MISMATCH, PD_ERR_DISPLAY_TYPE,
 * 	PD_ERR_NOMEM on failure
 */
int pi_pd_register(pd_driver_t *pd_driver)
{
	igd_display_port_t *port;
	igd_param_t *init_params;
	unsigned long      prev_dab = 0, prev_i2c_speed = 0;
	unsigned long      port_type, port_feature, second_port_feature = 0;
	unsigned long      cookie_sent, cookie_rcvd;
	unsigned long      num_instances = 0;
	unsigned long      prev_instance_dab = 0, prev_instance_i2c_reg = 0;
	unsigned long      dab_index = 0;
	int                ret = PD_SUCCESS;

	EMGD_TRACE_ENTER;

	if (!pd_driver) {
		EMGD_ERROR_EXIT("Null pd_driver received.");
		return PD_ERR_NULL_PTR;
	}

	/* Check the PD SDK version (interface version between main and
	 * port drivers Rightnow this check is useful only for XFree86.
	 * XP and CE already done this checking. */
	if (pd_driver->pd_sdk_version != PD_SDK_VERSION) {
		EMGD_ERROR("PD SDK version mismatch between main driver"
			"and %s. %u.%u != %u.%u", pd_driver->name,
			(unsigned short) PD_SDK_VERSION>>8,
			(unsigned short) PD_SDK_VERSION & 0xFF,
			(unsigned short) pd_driver->pd_sdk_version>>8,
			(unsigned short) pd_driver->pd_sdk_version & 0xFF);
		return PD_ERR_VER_MISMATCH;
	}

	/* Do magic cookie hand shaking */
#ifndef CONFIG_MICRO
	cookie_sent = get_magic_cookie(pd_driver);
#else
	cookie_sent = 0;
#endif
	cookie_rcvd = pd_driver->validate(cookie_sent);
	if (cookie_sent != cookie_rcvd) {
		/* TODO: Do this check once we comeup with handshake algorithm. */
		/*
		EMGD_ERROR("Error, magic cookie handshaking failed.");
		return PD_ERR_HAND_SHAKE;
		*/
	}

	init_params = pi_context->igd_context->mod_dispatch.init_params;

	port_feature = 0;
	/* Get the port features based on the display types */
	if (pd_driver->type == PD_DISPLAY_CRT) {
		/* Allocate GMCH onboard CRT port */
		port_type = IGD_PORT_ANALOG;
	} else if (pd_driver->type == PD_DISPLAY_LVDS_INT) {
		/* Allocate GMCH onboard LVDS port */
		port_type = IGD_PORT_LVDS;
	} else if (pd_driver->type == PD_DISPLAY_TVOUT_INT) {
		/* Allocate GMCH onboard TV port */
		port_type = IGD_PORT_TV;
	} else if (pd_driver->type &
			(PD_DISPLAY_TVOUT |    PD_DISPLAY_FP |      PD_DISPLAY_CRT_EXT |
			 PD_DISPLAY_LVDS_EXT | PD_DISPLAY_HDMI_EXT| PD_DISPLAY_HDMI_INT|
			 PD_DISPLAY_DRGB)) {

		/* Allocate DVO port which is the only kind of port exported to
		 * 3rd party encoders */
		port_type = IGD_PORT_DIGITAL;

		if (pd_driver->flags & PD_FLAG_GANG_MODE) {
			igd_display_port_t *portb;
			unsigned long user_gang = 0;
			/* Get DVO Port B */
			pi_context->igd_context->mod_dispatch.dsp_get_display(2,
				NULL, &portb, 0);
			if(portb) {
				if (portb->attr_list && portb->attr_list->num_attrs != 0) {
					unsigned long i;
					for (i = 0; i < portb->attr_list->num_attrs; i++) {
						if (portb->attr_list->attr[i].id==PD_ATTR_ID_GANG_MODE){
							user_gang = portb->attr_list->attr[i].value;
						}
					}
				}
			}
			/* If both user attribute and port driver flag are set to GANG MODE,
			 * then allocate a gang display port */
			if (user_gang) {
				port_feature = IGD_PORT_GANG;
				second_port_feature = IGD_PORT_GANG;
			}
		}
	} else if (pd_driver->type == PD_DISPLAY_RGBA) {
		port_type = IGD_PORT_DIGITAL;
		port_feature = IGD_RGBA_COLOR;
		second_port_feature = IGD_RGBA_ALPHA;
	} else {
		EMGD_ERROR_EXIT("Invalid display type.");
		return PD_ERR_DISPLAY_TYPE;
	}

	/* Get the port entry */
	port = NULL;
	while((port = pi_get_feature_port(port_feature, port))) {
		dab_index = 0;


		/* This port already has a port driver,
		 * don't search device on this port. */
		if (port->pd_driver || (port->port_type != port_type)) {
			continue;
		}

		/* allocate memory for callback */
		port->callback = (pd_callback_t *)OS_ALLOC(sizeof(pd_callback_t));
		if (port->callback == NULL) {
			EMGD_ERROR_EXIT("Unable to alloc memory for callback context.");
			return PD_ERR_NOMEM;
		}
		/* Fill entries in pd_callback_t */
		port->callback->callback_context = port;
		port->callback->read_regs = pi_read_regs;
		port->callback->write_regs = pi_write_regs;
		port->callback->eld = NULL; /* Insert when edid is initialize */

		/*	SDVO port driver needs the port number */
		port->callback->port_num = port->port_number;

        /*  SDVO port driver should not reset for seamless mode */
        port->callback->reset = (init_params->qb_seamless == 1) ? 0 : 1;
		/* now save the pd_driver in port entry */
		port->pd_driver = pd_driver;

		/* preference is to user specified i2c_speed */
		prev_dab = port->dab;
		prev_i2c_speed = port->i2c_speed;
		if (!port->i2c_speed) {
			port->i2c_speed = pd_driver->i2c_speed?pd_driver->i2c_speed:
				I2C_DEFAULT_SPEED;
		}

		/* Try detecting the encoder by calling port driver open() */
		if (port->dab ||
			((port->dab == 0) && (pd_driver->dab_list[0] == PD_DAB_LIST_END))) {

			/* Workaround for not to detect 2 encoders if only 1 encoder
			 * is present and both DVOB and DVOC are using same I2C bus */
			if ((init_params->display_flags & IGD_DISPLAY_MULTI_DVO) &&
				(num_instances > 0) &&
				(prev_instance_dab == port->dab) &&
				(prev_instance_i2c_reg == port->i2c_reg)) {
				/* Print this msg, because user explicitly mentioned DAB/I2C
				 * bus details which are same as previous encoder's DAB/I2C bus
				 * details. */
				EMGD_DEBUG("1+ encoders have same I2C bus and DAB");
				ret = -1;
			} else {
				/* Call open() only once if either user provides a DAB
				 *                      or
				 * no required to open an encoder. ex: analog, rgba, lvds etc.
				 */
				EMGD_DEBUG("Looking for \"%s\" on port 0x%lx with DAB 0x%lx",
					pd_driver->name, port->port_reg, port->dab);
				ret = pd_driver->open(port->callback, &(port->pd_context));
			}
		} else {

			/* Call open() for each DAB */
			while (pd_driver->dab_list[dab_index] != PD_DAB_LIST_END) {
				if(pd_driver->type == PD_DISPLAY_LVDS_INT) {
                	port->ddc_dab = pd_driver->dab_list[dab_index];
					printk ("NUHAIRI: port->ddc_dab\n" );
                } else {
					port->dab = pd_driver->dab_list[dab_index];
                }

				/* Workaround for not to detect 2 encoders if only 1 encoder
				 * is present and both DVOB and DVOC are using same I2C bus */
				if ((init_params->display_flags & IGD_DISPLAY_MULTI_DVO) &&
					(num_instances > 0) &&
					(prev_instance_dab == port->dab) &&
					(prev_instance_i2c_reg == port->i2c_reg)) {
					/* Don't print the debug msg, because this is a valid case.
					 * Example,
					 *    Algorithm is detecting for multiple encoders with
					 *    same DAB and same I2C bus on different ports.
					 * If this case arises, simply continue
					 */
					/* EMGD_DEBUG("1+ encoders have same I2C bus and DAB"); */
					if (pd_driver->flags & PD_FLAG_DUAL_DVO) {
						/* If this flag is set, that means port driver is
						 * explicityly requesting to be loaded on
						 * both DVO B & DVO C with same DAB. Example: CH7017.
						 *
						 * In this case open the port driver again. */
					} else {
						ret = -1;
						port->pd_context = NULL;
						dab_index++;
						continue;
					}
				}

				EMGD_DEBUG("Looking for \"%s\" on port 0x%lx with DAB 0x%lx",
					pd_driver->name, port->port_reg, port->dab);
				ret = pd_driver->open(port->callback, &(port->pd_context));
				if (ret == 0) {
					break;
				} else {

					dab_index++;
				}
			}
		}
#ifndef CONFIG_MICRO
		if(pi_context->igd_context->mod_dispatch.check_port_supported && ret == 0){
			ret = pi_context->igd_context->mod_dispatch.check_port_supported(port);
		}
#endif
		if (ret == 0) {

			/* Initialize our port entry */
			ret = pi_pd_init(port, port_feature, second_port_feature, TRUE);
			if (ret) {
				port->pd_driver = NULL;
				port->pd_context = NULL;
				port->dab = prev_dab;
				port->i2c_speed = prev_i2c_speed;
				port->mult_port = NULL;
				port->timing_table = NULL;
				port->num_timing = 0;
				if (port->callback) {
					OS_FREE(port->callback);
					port->callback = NULL;
				}
			} else {
				EMGD_DEBUG("Device found on %s port for \"%s\"", port->port_name,
					pd_driver->name);
				num_instances++;
				prev_instance_dab = port->dab;
				prev_instance_i2c_reg = port->i2c_reg;
			}

			/* If Multi-DVO support is enabled then detect next encoder of
			 * same kind */
			if (init_params->display_flags & IGD_DISPLAY_MULTI_DVO){
				/* Continue to find next encoder */
				continue;
			} else {
				/* Found one encoder and return to port driver */
				break;
			}
		} else {
			port->pd_driver = NULL;
			port->pd_context = NULL;
			port->dab = prev_dab;
			port->i2c_speed = prev_i2c_speed;;
			if (port->callback) {
				OS_FREE(port->callback);
				port->callback = NULL;
			}
		}
	} /* end while(port == feature_port()) */

	if (num_instances == 0) {
		EMGD_DEBUG("No device found for \"%s\"", pd_driver->name);
		return PD_ERR_NOPORT_AVAIL;
	}

	EMGD_TRACE_EXIT;
	return PD_SUCCESS;
} /* end pi_pd_register() */

/* Function to replace common timings in 1st list with 2nd list, 2nd list
 * is unchanged. */
void replace_common_dtds(igd_timing_info_t *dtds1,
	igd_timing_info_t *dtds2)
{
	igd_timing_info_t *temp;
	int index;

	if (!dtds2 || !dtds1) {
		return;
	}

	while (dtds1->width != IGD_TIMING_TABLE_END) {
		temp = dtds2;
		index = 0;

		while (temp->width != IGD_TIMING_TABLE_END && index < NUM_TIMINGS) {
			/* Replace modes that have common width, height and
			   refresh rate. Removed Dot clock comparison since
			   EDID(CEA modes) may differ in dot clock value.
			   causing duplicate mode.
			   All ial would do a match  mode by height, width
			   and refresh rate.
			*/
			if ((temp->width   == dtds1->width) &&
				(temp->height  == dtds1->height) &&
				(temp->refresh == dtds1->refresh) &&
				((temp->mode_info_flags & PD_SCAN_INTERLACE) ==
				 (dtds1->mode_info_flags & PD_SCAN_INTERLACE))) {
				dtds1->mode_info_flags &= ~PD_MODE_SUPPORTED;
			}
			temp++;
			index++;
		}
		dtds1++;
	}
}

/*!
 *
 * @param port
 *
 * @return 0
 */
int check_port_attrs(igd_display_port_t *port)
{
	int          ret;
	unsigned long         attr_value = 0;
#ifndef CONFIG_MICRO
	pd_attr_t             out_list,*temp_list;
	temp_list = &out_list;
#endif
	/* Attempt to see if the port driver has this attibutes so it can update
	  the port driver value. For now this is required for internal HDMI which
	  has a different i2c bus and port name from the standard SDVO port driver.
	  DP would most likely use this attribute assuming it uese the same port
	  number as well*/
	ret = pi_pd_find_attr_and_value(port, PD_ATTR_ID_PORT_DDC_REG,
		PD_ATTR_FLAG_GENERAL, NULL, &attr_value);
	if(!ret){
		EMGD_DEBUG("ddr_reg value unique = %ld.", attr_value);
		port->ddc_reg = attr_value;
	}
#ifndef CONFIG_MICRO
	ret = pi_pd_find_attr_and_value(port, PD_ATTR_ID_PORT_NAME,
		PD_ATTR_FLAG_GENERAL, &(temp_list), &attr_value);
	if(!ret){
		EMGD_DEBUG("ddr_reg value unique = %ld.", attr_value);
		pd_strcpy(port->port_name, temp_list->name);
	}
#endif
	return 0;

}

/*!
 * Function to initialize port driver related members in port table entry
 *
 * @param port
 * @param port_feature
 * @param second_port_feature
 * @param drm_load_time
 *
 * @return PD_SUCCESS on success
 * @return 1 on failure
 */
int pi_pd_init(igd_display_port_t *port,
	unsigned long port_feature,
	unsigned long second_port_feature,
	int drm_load_time)
{
	igd_display_port_t *second_port;
	pd_timing_t        *user_timings = NULL;
	pd_timing_t        *std_timings = NULL;
	pd_timing_t        *firmware_timings = NULL;
	pd_timing_t        *final_timings = NULL;
	pd_timing_t        *pd_timing_table = NULL;
	mode_state_t       *mstate;
	int                i, ret = PD_SUCCESS;
	unsigned long      edid_flags;
	unsigned char      num_firmware_timings = 0;
	igd_display_params_t *display_params = NULL;
	igd_param_t *init_params;

	EMGD_TRACE_ENTER;

	mstate = NULL;

	/* If the display device is a ganged mode device or RGBA mode, then hook
	 * up second port pointer in first port */
	if (second_port_feature) {
		second_port = pi_get_feature_port(second_port_feature, port);

		/* If second_port is N/A, or second port was already taken by
		 * other port driver, then release main port and return error */
		if (second_port == NULL || second_port->pd_driver) {
			EMGD_ERROR_EXIT("Second ganged/RGBA port N/A or already allocated.");
			return PD_ERR_NOPORT_AVAIL;
		}
		/* now link second port to first one */
		port->mult_port = second_port;
	}

	/* Check port attributes to overwrite port value is any available */
	check_port_attrs(port);
	/* Implementation notes to get the timing list:
	 *   Any port timing table consists of
	 *          EDID DTDS
	 *          USER DTDS
	 *          STD TIMINGS
	 *   based on edid_flags.
	 *
	 * If there are no flags, then it defaults to use STD TIMINGS + EDID DTDs
	 */

	/* Get the display params for this port */
	init_params = pi_context->igd_context->mod_dispatch.init_params;

	for (i = 0; i < 5; i++) {
		if (port->port_number == init_params->display_params[i].port_number) {
			display_params = &init_params->display_params[i];
			break;
		}
	}

	/* Start with STD TIMINGS */
	edid_flags = IGD_DISPLAY_USE_STD_TIMINGS;

	/* If there is EDID, then default to use EDID */
	if (!display_params || (display_params->flags & IGD_DISPLAY_READ_EDID)) {
		/* Read firmware (EDID/DisplayID) on I2C */
		ret = pi_context->i2c_dispatch->i2c_read_regs(
			pi_context->igd_context,
			port->ddc_reg,      /* DDC register */
			port->ddc_speed,    /* DDC speed */
			port->ddc_dab,      /* Data Addr Byte*/
			0,                  /* Register */
			firmware_data,      /* Values */
			128,               /* Num bytes to read */
			0);

		/* If EDID is present then use EDID.
		 * edid_flags will be corrected later if display_params are present */
		if (ret == 0) {
			edid_flags |= IGD_DISPLAY_USE_EDID;
		}
	}

	/* Check for display params */
	if (display_params) {
		if (edid_flags & IGD_DISPLAY_USE_EDID) {
			/* Adjust edid_flags to use edid_avail
			 * if both edid is present and edid_avail is not 0 */
			if (display_params->edid_avail) {
				edid_flags = display_params->edid_avail;
				EMGD_DEBUG("EDID_Avail: 0x%lx", edid_flags);
			}
		} else {
			/* Adjust edid_flags to use edid_not_avail
			 * if edid is not present and edid_not_avail is not 0 */
			if (display_params->edid_not_avail) {
				edid_flags =
					display_params->edid_not_avail & ~IGD_DISPLAY_USE_EDID;
				EMGD_DEBUG("EDID_Not_Avail: 0x%lx", edid_flags);
			}
		}
	}

	/* Make a copy of crt_timing_table */
	/* All crt timings are already enabled in mode_table.c */
	std_timings = (igd_timing_info_t *) OS_ALLOC(crt_timing_table_size);
	OS_MEMCPY(std_timings, crt_timing_table, crt_timing_table_size);

	/* Include Standard built-in modes */
	if (edid_flags & IGD_DISPLAY_USE_STD_TIMINGS) {
		EMGD_DEBUG("Using STD TIMINGS ");
		final_timings = std_timings;
	}

	/* Include user DTDs */
	if (edid_flags & IGD_DISPLAY_USE_USERDTDS) {
		EMGD_DEBUG("Using USER-DTDs ");
		user_timings = get_user_timings(port->dtd_list);

		if (user_timings) {
			/* Add user DTDs at the begining of the final timings */
			user_timings[port->dtd_list->num_dtds].extn_ptr = final_timings;
			final_timings = user_timings;
		}
	}

	/* Include EDID timings and filter modes */
	if (edid_flags & IGD_DISPLAY_USE_EDID) {
		EMGD_DEBUG("Using EDID-DTDs ");
		ret = get_firmware_timings(port, firmware_data, final_timings);
		if (port->firmware_type == PI_FIRMWARE_EDID) {
			firmware_timings = port->edid->timings;
			num_firmware_timings = port->edid->num_timings;
		} else if (port->firmware_type == PI_FIRMWARE_DISPLAYID) {
			firmware_timings = port->displayid->timings;
			num_firmware_timings = port->displayid->num_timings;
		}
		if (ret == 0 && num_firmware_timings) {
			/* Add EDID DTDs at the begining of the final timings */
			firmware_timings[num_firmware_timings].extn_ptr =
				(void *)final_timings;
			final_timings = firmware_timings;
		}
	}

	/* Replace any common timings */
	replace_common_dtds(std_timings, firmware_timings);
	replace_common_dtds(std_timings, user_timings);
	replace_common_dtds(firmware_timings, user_timings);

	/* Count the number of timings in final_timings.  If the above functions
	 * result in an empty timing list, then use std_timings as default.
	 */
	if (!get_native_dtd(final_timings, PI_SUPPORTED_TIMINGS, NULL, 0)) {
		EMGD_DEBUG("User options resulted in 0 timings; using std timings.");
		final_timings = std_timings;
		enable_disable_timings(final_timings, 1);
	}

	/* Update port driver attributes */
	update_attrs(port);

	/* Now get the timing list filtered by PORT DRIVER */
	ret = port->pd_driver->get_timing_list(port->pd_context,
			final_timings, &pd_timing_table);

	if (ret || !pd_timing_table) {
		EMGD_ERROR_EXIT("port driver: get timing list error.");
		return PD_ERR_NO_TIMINGS;
	}

	/* Delete temporary lists and buffers */
	if (user_timings) {
		OS_FREE(user_timings);
	}
	if (std_timings) {
		OS_FREE(std_timings);
	}

	/* Filter modes based on chipset type */
	pi_context->igd_context->mod_dispatch.filter_modes(pi_context->igd_context,
		port, pd_timing_table);

	/* Now save the timings in port */
	port->timing_table = pd_timing_table;
	port->num_timing = get_native_dtd(pd_timing_table,
			PI_SUPPORTED_TIMINGS, &port->fp_native_dtd, PD_MODE_DTD_FP_NATIVE);

	assign_dynamic_numbers(port->timing_table);

#ifdef DEBUG_FIRMWARE
	{
		int ti;
		EMGD_DEBUG("Supported timings for \"%s\" (%lu)",
			port->pd_driver->name, port->num_timing);
		ti = 0;
		while (port->timing_table[ti].width != PD_TIMING_LIST_END) {
			if (port->timing_table[ti].mode_info_flags & PD_MODE_SUPPORTED) {
				EMGD_DEBUG("\t%ux%u@%u dclk=%lu mode_num=%d hsync=%luKHz "
							"vsync=%luHz flags=0x%lx",
					port->timing_table[ti].width,
					port->timing_table[ti].height,
					port->timing_table[ti].refresh,
					port->timing_table[ti].dclk,
					port->timing_table[ti].mode_number,
					port->timing_table[ti].dclk/port->timing_table[ti].htotal,
					((port->timing_table[ti].dclk * 1000)/
					port->timing_table[ti].htotal)/
					port->timing_table[ti].vtotal,
					port->timing_table[ti].mode_info_flags);
			}
			ti++;
		}
	}
#endif


	/*
	 * Exit early when called by emgd_driver_pre_init to poke the X driver's
	 * (i.e. "xorg.conf") DTDs and attr's into the port drivers (done above).
	 */
	if (!drm_load_time) {
		EMGD_TRACE_EXIT;
		return PD_SUCCESS;
	}


#ifndef CONFIG_MICRO
	/*
	 * There is only two states that need to be saved; one is the regular state
	 * and the other is for the console.
	 */
	ret = pi_save_mode_state(port, REG_MODE_STATE_REG);
	if (config_drm.init) {
		ret = pi_save_mode_state(port, REG_MODE_STATE_CON);
	}
#endif

	if(port->displayid != NULL){
		/* Driver to init audio if cea extension available */
		if(port->firmware_type == PI_FIRMWARE_EDID){
			port->callback->eld = &(port->edid->cea);
		}
		/* Displayid unsupported for now. Uncomment this code when audio
		information is available for Display ID
		else if(port->firmware_type == PI_FIRMWARE_EDID){
			port->callback->eld = &(port->displayid->cea);
		}
		*/
		else{
			port->callback->eld = NULL;
		}
	}
	ret = port->pd_driver->init_device(port->pd_context);
	if (ret) {
#ifndef CONFIG_MICRO
		/* TODO: Restore the pd state? */
		EMGD_ERROR_EXIT("port driver: init_device error. ret = %d", ret);
		if (mstate) {
			mstate->pd_state[pi_context->num_pi_drivers].port = NULL;
			mstate->pd_state[pi_context->num_pi_drivers].state = NULL;
		}
#endif
		return ret;
	}

	/* Increment the number of port drivers */
	pi_context->num_pi_drivers++;

	/* save the port driver display type & flags in port. These additions are
	 * required to support different types displays by same port driver. */
	port->pd_type = port->pd_driver->type;
	port->pd_flags = port->pd_driver->flags;

	EMGD_TRACE_EXIT;
	return PD_SUCCESS;
} /* end pi_pd_init */

/*!
 * Function to read registers
 *
 * @param context
 * @param list
 * @param type
 *
 * @return PD_SUCCESS on success
 * @return PD_ERR_NULL_PTR, PD_ERR_I2C_READ, PD_ERR_UNSUCCESSFUL on failure
 */
int pi_read_regs(void *callback_context, pd_reg_t *list, unsigned long type)
{
	int ret;
	igd_display_port_t *port = callback_context;
	unsigned char      *mmio;

	/*EMGD_TRACE_EXIT;*/

	if (!port) {
		EMGD_ERROR_EXIT("Null callback context passed.");
		return PD_ERR_NULL_PTR;
	}

	if (!port->pd_driver) {
		EMGD_ERROR_EXIT("Null pd_driver in port entry.");
		return PD_ERR_NULL_PTR;
	}

	mmio = EMGD_MMIO(pi_context->igd_context->device_context.virt_mmadr);

	/* Based on the port type either read GMCH registers or I2C registers */
	switch (type) {
	case PD_REG_I2C:
		ret = 0;
		while (list->reg != PD_REG_LIST_END) {
			ret = pi_context->i2c_dispatch->i2c_read_regs(
				pi_context->igd_context,
				port->i2c_reg,
				port->i2c_speed,
				port->dab,
				(unsigned char)list->reg,
				(unsigned char *)&list->value, 1, 0);
			if (ret) {
				EMGD_DEBUG("i2c_read_reg: 0x%lx failed.", list->reg);
				break;
			}
			list++;
		}
		if (ret) {
			return PD_ERR_I2C_READ;
		}
		break;
	case PD_REG_DDC_FW:
		ret = 0;
		while (list->reg != PD_REG_LIST_END) {
			ret = pi_context->i2c_dispatch->i2c_read_regs(
				pi_context->igd_context,
				port->ddc_reg,
				port->ddc_speed,
				port->ddc_dab,
				(unsigned char)list->reg,
				(unsigned char *)&list->value, 1,
				IGD_I2C_WRITE_FW);
			if (ret) {
				EMGD_DEBUG("i2c_read_reg: 0x%lx failed.", list->reg);
				break;
			}
			list++;
		}
		if (ret) {
			return PD_ERR_I2C_READ;
		}
		break;
	case PD_REG_DDC:
		ret = 0;
		while (list->reg != PD_REG_LIST_END) {
			ret = pi_context->i2c_dispatch->i2c_read_regs(
				pi_context->igd_context,
				port->ddc_reg,
				port->ddc_speed,
				port->ddc_dab,
				(unsigned char)list->reg,
				(unsigned char *)&list->value, 1,
				0);
			if (ret) {
				EMGD_DEBUG("i2c_read_reg: 0x%lx failed.", list->reg);
				break;
			}
			list++;
		}
		if (ret) {
			return PD_ERR_I2C_READ;
		}
		break;
	case PD_REG_PIO8:
		while (list->reg != PD_REG_LIST_END) {
			list->value = EMGD_READ_PORT8(list->reg);
			list++;
		}
		break;
	case PD_REG_PIO16:
		while (list->reg != PD_REG_LIST_END) {
			list->value = EMGD_READ_PORT16(list->reg);
			list++;
		}
		break;
	case PD_REG_PIO32:
		while (list->reg != PD_REG_LIST_END) {
			list->value = EMGD_READ_PORT32(list->reg);
			list++;
		}
		break;
	case PD_REG_MIO :
	case PD_REG_MIO8 :
		if ((port->port_type == IGD_PORT_ANALOG) ||
			(port->port_type == IGD_PORT_TV)     ||  /* For Integrated TV */
			(port->port_type == IGD_PORT_LVDS)	 ||
			(port->port_type == IGD_PORT_DIGITAL)) {
			while (list->reg != PD_REG_LIST_END) {
				if (type == PD_REG_MIO) {
					if (BIT31 & list->reg) {
#ifdef CONFIG_TNC
						/* Atom E6xx si hack: Si folks defined LVDS (0:2:0)
						 * related register in 0:3:0 (sdvo device) as they
						 * are afraid to touch Lincroft hardmacro.
						 * This triggered LVDS port driver to touch 0:3:0
						 * registers for its operation. As this is done for
						 * LVDS operation and LVDS port driver is internal,
						 * BIT31 is defined to access 0:3:0 device. */
						list->value = READ_MMIO_REG_TNC(IGD_PORT_SDVO,
							list->reg);
#endif
					} else {
						list->value = EMGD_READ32(EMGD_MMIO(mmio) + list->reg);
					}
				} else {
					list->value = EMGD_READ8(EMGD_MMIO(mmio) + list->reg);
				}
				list++;
			}
		}
		break;
#ifdef CONFIG_TNC
	case PD_REG_LPC:
		if (port->port_type == IGD_PORT_LVDS) {
			while (list->reg != PD_REG_LIST_END) {
				list->value = READ_MMIO_REG_TNC(IGD_PORT_LPC, list->reg);
				list++;
			}
		}
		break;
#endif
	case PD_REG_PCI:
		/* Rightnow this is only to provide the device id */
		while (list->reg != PD_REG_LIST_END) {
#if 0
			/* Assume IGD at bus=0, dev=2, func=0 */
			EMGD_WRITE_PORT32(0xCF8,
				(0x80000000 | (0L << 16) | (2L << 11) | (0L << 8) |
					(list->reg & 0xFC)));
			list->value = EMGD_READ_PORT32(0xCFC + (list->reg & 0x03));
#endif
			list->value = pi_context->igd_context->device_context.did;
			list++;
		}
		break;
	case PD_REG_BRIDGE_OPCODE:
		/* right now, we only return the graphics frequency to calculate the
		 * PWM Backlight modulation frequency. This is only available for pouslbo */
		while (list->reg != PD_REG_LIST_END) {
			list->value = pi_context->igd_context->device_context.gfx_freq;
			list++;
		}

		break;
	default:
		EMGD_ERROR_EXIT("Unknown reg type (0x%lx).", type);
		return PD_ERR_UNSUCCESSFUL;
		break;
	}

	/*EMGD_TRACE_EXIT;*/
	return PD_SUCCESS;
} /* end pi_read_regs */

/*!
 * Function to write registers
 *
 * @param context
 * @param list
 * @param type
 *
 * @return 0 on success
 * @return PD_ERR_NULL_PTR, PD_ERR_I2C_WRITE, PD_ERR_UNSUCCESSFUL on failure
 */
extern unsigned short io_base_sdvo;
extern unsigned short io_base;
extern unsigned short io_base_sdvo_st;
extern unsigned short io_base_sdvo_st_gpio;

int pi_write_regs(void *callback_context, pd_reg_t *list, unsigned long type)
{
	igd_display_port_t *port = callback_context;
	int           ret;
	unsigned char *mmio;

	EMGD_TRACE_ENTER;

	if (!port) {
		EMGD_ERROR_EXIT("Null callback context passed.");
		return PD_ERR_NULL_PTR;
	}

	if (!port->pd_driver) {
		EMGD_ERROR_EXIT("Null pd_driver.");
		return PD_ERR_NULL_PTR;
	}

	EMGD_DEBUG("Getting mmio");
	mmio = EMGD_MMIO(pi_context->igd_context->device_context.virt_mmadr);
	EMGD_DEBUG("mmio = 0x%lx", (unsigned long)mmio);

	/* Based on the port type either write GMCH registers or I2C registers */
	switch (type) {
	case PD_REG_DDC_FW:
		/*This will use shorter delay than PD_REG_DDC*/
		ret = pi_context->i2c_dispatch->i2c_write_reg_list(
			pi_context->igd_context,
			port->ddc_reg,
			port->ddc_speed,
			port->ddc_dab,
			list,
			IGD_I2C_WRITE_FW);
		if (ret) {
        	EMGD_DEBUG("i2c_write_reg: 0x%lx = 0x%lx failed.",
       		list->reg, list->value);
        	return PD_ERR_I2C_WRITE;
        }
    	break;
	case PD_REG_DDC:
		ret = pi_context->i2c_dispatch->i2c_write_reg_list(
			pi_context->igd_context,
			port->ddc_reg,
			port->ddc_speed,
			port->ddc_dab,
			list,
			0);
		if (ret) {
        	EMGD_DEBUG("i2c_write_reg: 0x%lx = 0x%lx failed.",
       		list->reg, list->value);
        	return PD_ERR_I2C_WRITE;
        }
    	break;
	case PD_REG_I2C:
		ret = pi_context->i2c_dispatch->i2c_write_reg_list(
			pi_context->igd_context,
			port->i2c_reg,
			port->i2c_speed,
			port->dab,
			list,
			0);
		if (ret) {
			EMGD_DEBUG("i2c_write_reg: 0x%lx = 0x%lx failed.",
				list->reg, list->value);
			return PD_ERR_I2C_WRITE;
		}
		EMGD_DEBUG("i2c_write_reg success");
		break;
	case PD_REG_PIO8:
		while (list->reg != PD_REG_LIST_END) {
			EMGD_WRITE_PORT8(list->reg, list->value);
			list++;
		}
		EMGD_DEBUG("EMGD_WRITE_PORT8 seemed successful");
		break;
	case PD_REG_PIO16:
		while (list->reg != PD_REG_LIST_END) {
			EMGD_WRITE_PORT16(list->reg, list->value);
			list++;
		}
		EMGD_DEBUG("EMGD_WRITE_PORT16 seemed successful");
		break;
	case PD_REG_PIO32:
		while (list->reg != PD_REG_LIST_END) {
			EMGD_WRITE_PORT32(list->reg, list->value);
			list++;
		}
		EMGD_DEBUG("EMGD_WRITE_PORT32 seemed successful");
		break;
	case PD_REG_MIO :
	case PD_REG_MIO8 :
		if ((port->port_type == IGD_PORT_ANALOG) ||
			(port->port_type == IGD_PORT_TV)     ||  /* For Integrated TV */
			(port->port_type == IGD_PORT_LVDS)	 ||
			(port->port_type == IGD_PORT_DIGITAL)) {
			while (list->reg != PD_REG_LIST_END) {
				if (type == PD_REG_MIO) {
					if (BIT31 & list->reg) {
#ifdef CONFIG_TNC
						/* BIT31 indicates write to 0:3:0 SDVO device */
						WRITE_MMIO_REG_TNC(IGD_PORT_SDVO, list->reg,
							list->value);
#endif
					} else {
						EMGD_WRITE32(list->value, EMGD_MMIO(mmio) + list->reg);
					}
				} else {
					EMGD_WRITE8(list->value, EMGD_MMIO(mmio) + list->reg);
				}
				list++;
			}
			EMGD_DEBUG("complicated write seemed successful");
		}
		break;
#ifdef CONFIG_TNC
	case PD_REG_LPC:
		if (port->port_type == IGD_PORT_LVDS) {
			while (list->reg != PD_REG_LIST_END) {
				WRITE_MMIO_REG_TNC(IGD_PORT_LPC, list->reg, list->value);
				list++;
			}
			EMGD_DEBUG("Write to IGD_PORT_LPC seemed successful");
		}
		break;
#endif
	default:
		EMGD_ERROR_EXIT("Unknown reg type (0x%lx).", type);
		return PD_ERR_UNSUCCESSFUL;
		break;
	}

	EMGD_TRACE_EXIT;
	return 0;
} /* end pi_write_regs */

/*!
 * Depending on the parameters, this function does multiple things.  It always
 * counts and returns the number of [supported] timings.  If desired, it also
 * finds the timing with a desired "mode_info_flags" that has the largest value
 * of width, height, OR refresh rate, which it sets to the "native_dtd"
 * parameter.
 *
 * @param timing
 * @param flags
 * @param native_dtd
 * @param nflags
 *
 * @return 0 on failure
 * @return native dtd on success
 */
unsigned long get_native_dtd(igd_timing_info_t *timing,
	unsigned long flags, pd_timing_t **native_dtd, unsigned long nflags)
{
	unsigned long entries = 0;

	EMGD_TRACE_EXIT;

	if (!timing) {
		return 0;
	}
	if (native_dtd) {
		*native_dtd = NULL;
	}
	while (timing->width != IGD_TIMING_TABLE_END) {
		if (flags & PI_SUPPORTED_TIMINGS) {
			if (timing->mode_info_flags & PD_MODE_SUPPORTED) {
				entries++;
			}
			if ((native_dtd) &&
				(timing->mode_info_flags & nflags)) {
				/* Native Resolution is defined as the largest resolution the
				 * panel can display. However, some panels contain more than one
				 * DTD in its EDID. We will choose the largest resolution
				 * available from EDID */
				if(((*native_dtd) && (nflags == PD_MODE_DTD)) &&
					(((pd_timing_t*)(*native_dtd))->width > timing->width ||
					((pd_timing_t*)(*native_dtd))->height > timing->height ||
					((pd_timing_t*)(*native_dtd))->refresh > timing->refresh)){
						/* do nothing */
				} else {
					*native_dtd = timing;
				}
			}
		} else {
			entries++;
		}

		timing++;
		if ((timing->width == PD_TIMING_LIST_END) && timing->extn_ptr) {
			timing = timing->extn_ptr;
		}
	}

	EMGD_TRACE_EXIT;
	return entries;
}

#ifndef CONFIG_MICRO
unsigned long get_magic_cookie(pd_driver_t *pd_driver)
{
	/* FIXME: Implement cookie checking */
	return 0;
}
#endif

/*!
 * Function to filter modes based on EDID or DisplayID
 *
 * @param port
 * @param firmware_data
 * @param timing_table
 *
 * @return 0 on success
 * @return -IGD_ERROR_EDID, -IGD_ERROR_NOMEM on failure
 */
int get_firmware_timings(igd_display_port_t *port,
	unsigned char *firmware_data, igd_timing_info_t *timing_table)
{
	edid_t         *edid;
	displayid_t    *displayid;
	int            ret = -1;

	EMGD_TRACE_ENTER;

	if (!firmware_data) {
		return -IGD_ERROR_EDID;
	}

	if (!port->displayid) {
		/* EDID and DisplayID use same memory */
		displayid = (displayid_t *) OS_ALLOC(sizeof(displayid_t));
		if (!displayid) {
			return -IGD_ERROR_NOMEM;
		}
		edid = (edid_t *) displayid;
	} else {
		displayid = port->displayid;
		edid = (edid_t *) displayid;
	}
	OS_MEMSET(displayid, 0, sizeof(displayid_t));

	/* Now parse the EDID or DisplayID */
	/* Check the header to determine whether the data is EDID or DisplayID */
	/* EDID header first 8 bytes =
	 *     byte 0, 1, 2, 3: 00 ff ff ff = unsigned long 0xFFFFFF00
	 *     byte 4, 5, 6, 7: ff ff ff 00 = unsigned long 0x00FFFFFF */
	if (*(unsigned long *) &firmware_data[0] == 0xFFFFFF00 &&
		*(unsigned long *) &firmware_data[4] == 0x00FFFFFF) {
#ifdef DEBUG_FIRMWARE
		firmware_dump(firmware_data, 256);
#endif
		/* This is EDID data */
		ret = edid_parse(firmware_data, edid, timing_table, 0,
			(unsigned char) (port->pd_driver->flags&PD_FLAG_UP_SCALING?1:0));
		if (ret == EDID_READ_AGAIN) {
			/* Check to see if there is an extension block */
			if((firmware_data[0x7e] == 0x1)){
				ret = pi_context->i2c_dispatch->i2c_read_regs(
					pi_context->igd_context,
					port->ddc_reg,      /* DDC register */
					port->ddc_speed,    /* DDC speed */
					port->ddc_dab,      /* Data Addr Byte*/
					0x80,                /* Register */
					&firmware_data[128], /* Values */
					128,
					0);				 /* next 128 bytes include extension */
				ret = edid_ext_parse(&firmware_data[128], edid, timing_table,0,
					(unsigned char)(port->pd_driver->flags&
					PD_FLAG_UP_SCALING?1:0));
				/* Parse next 128 bytes of EDID block */
			}else{
				ret = 0;
			}
		} else if (ret) {
			OS_FREE(edid);
			return -IGD_ERROR_EDID;
		}

		port->firmware_type = PI_FIRMWARE_EDID;
#ifdef DEBUG_FIRMWARE
		edid_print(edid);
#endif

#ifndef CONFIG_NO_DISPLAYID
	} else {
		/* size  = payload + 5 */
		/* +5 is for the 5 mandatory bytes not included in payload */
		unsigned short displayid_size = firmware_data[1] + 5;
		if (displayid_size > 256) {
			EMGD_DEBUG("Invalid DisplayID size = %u (incl 5 mand bytes) > 256",
				displayid_size);
			return -IGD_ERROR_EDID;
		}

		/* If the DisplayID is greater than 128 bytes */
		if (displayid_size > 128) {
			ret = pi_context->i2c_dispatch->i2c_read_regs(
				pi_context->igd_context,
				port->ddc_reg,      /* DDC register */
				port->ddc_speed,    /* DDC speed */
				port->ddc_dab,      /* Data Addr Byte*/
				0,                  /* Register */
				firmware_data,      /* Values */
				displayid_size,    /* Num bytes to read */
				0);
		}

#ifdef DEBUG_FIRMWARE
		firmware_dump(firmware_data, 256);
#endif
		/* This is DisplayID data */
		ret = displayid_parse(firmware_data, displayid, timing_table, 0,
			(unsigned char) (port->pd_driver->flags&PD_FLAG_UP_SCALING?1:0));
		if (!ret) {
			port->firmware_type = PI_FIRMWARE_DISPLAYID;
		} else {
			OS_FREE(displayid);
		}
#ifdef DEBUG_FIRMWARE
		displayid_print(firmware_data, displayid);
#endif
#else
	/* If DisplayID isn't enabled then print a debug message and return error */
	} else {
		EMGD_ERROR_EXIT("EDID header is wrong! Will ignore");
		return -IGD_ERROR_EDID;
#endif
	}

	port->edid = edid;

	EMGD_TRACE_EXIT;
	return ret;
} /* end get_firmware_timings() */

/*!
 * Update port driver attributes with incoming values from IAL
 *
 * @param in_list attributes in this list are used to update corresponding
 *	attributes in out_list
 * @param fp_info flat panel attributes used to update ocrresponding
 *	attributes in out_list
 * @param out_num_attrs size of out_list
 * @param out_list contains a list of attributes to be updated
 *
 * @return 0
 */
int update_attrs(igd_display_port_t *port)
{
	int          ret;
	unsigned int i = 0;

	igd_param_attr_list_t *in_list = port->attr_list;
	igd_param_fp_info_t   *fp_info = port->fp_info;

	/* Initial 5 attributes are for fp_info,
	 * Note: If both igd_param_fp_info_t and fp_info attributes
	 * were specified then fp_info values takes the precedence */
	unsigned long         out_num_attrs = 5;
	pd_attr_t             *out_list;

	EMGD_TRACE_ENTER;

	/* if: there's something in in_list */
	if (in_list) {
		out_num_attrs += in_list->num_attrs;
	}

	out_list = OS_ALLOC(sizeof(pd_attr_t) * out_num_attrs);
	if (!out_list) {
		EMGD_DEBUG("No memory to make attr_list.");
		return 0;
	}
	OS_MEMSET(out_list, 0, sizeof(pd_attr_t) * out_num_attrs);

	/* Pass user specified attributes */
	if (in_list) {
		/* For every incoming attr, make a pd_attr_t */
		for (i = 0; i < in_list->num_attrs; i++) {
			/* This will work for all kinds of attributes:
			 * in_list->attr[i].value =
			 *      actual value for range attributes
			 *      index for list  attributes
			 *      value for boolean attributes.
			 * This works because
			 *   pd_attr_t, pd_range_attr_t, pd_list_attr_t,
			 *   pd_boolean_attr_t all have save offsets for
			 *   default values. */
			out_list[i].id = in_list->attr[i].id;
			out_list[i].flags = PD_ATTR_FLAG_VALUE_CHANGED;
			out_list[i].current_value = in_list->attr[i].value;
		}
	}  /* if: there's something in in_list */

	/* Pass flat panel attributes to the port driver, if necessary */
	if (fp_info) {
		/* Initialize flat panel attributes */
		/* Update FP attributes */
		if (fp_info->fp_pwr_method == IGD_PARAM_FP_PWR_METHOD_PD) {
			/* The only thing remaining is the FP_PWR_Tx, so check
			 * to ensure it is for an FP_PWR_METHOD_PD */
			out_list[i].id = PD_ATTR_ID_FP_PWR_T1;
			out_list[i].flags = PD_ATTR_FLAG_VALUE_CHANGED;
			out_list[i++].current_value = fp_info->fp_pwr_t1;

			out_list[i].id = PD_ATTR_ID_FP_PWR_T2;
			out_list[i].flags = PD_ATTR_FLAG_VALUE_CHANGED;
			out_list[i++].current_value = fp_info->fp_pwr_t2;

			out_list[i].id = PD_ATTR_ID_FP_PWR_T3;
			out_list[i].flags = PD_ATTR_FLAG_VALUE_CHANGED;
			out_list[i++].current_value = fp_info->fp_pwr_t3;

			out_list[i].id = PD_ATTR_ID_FP_PWR_T4;
			out_list[i].flags = PD_ATTR_FLAG_VALUE_CHANGED;
			out_list[i++].current_value = fp_info->fp_pwr_t4;

			out_list[i].id = PD_ATTR_ID_FP_PWR_T5;
			out_list[i].flags = PD_ATTR_FLAG_VALUE_CHANGED;
			out_list[i++].current_value = fp_info->fp_pwr_t5;
		}
	}

#ifndef CONFIG_NO_DISPLAYID
	/* Based on our architecture, any user-defined config option will override
	 * firmware options. i.e., First send the DisplayID attributes
	 * then send the config attributes to port drivers. */
	if (port->firmware_type == PI_FIRMWARE_DISPLAYID) {
		ret = port->pd_driver->set_attrs(port->pd_context,
			port->displayid->num_attrs,
			port->displayid->attr_list);
	}
#endif
	ret = port->pd_driver->set_attrs(port->pd_context, i, out_list);
	if (ret) {
		EMGD_DEBUG("Attribute update failed. ret = %d.", ret);
	}
	OS_FREE(out_list);

	EMGD_TRACE_EXIT;
	return 0;
} /* update_attrs */

/* Add user defined timings to big timing table */
pd_timing_t *get_user_timings(igd_param_dtd_list_t *in_list)
{
	pd_timing_t        *t = NULL, *timing = NULL;
	igd_display_info_t *dtd;
	unsigned long i;
	int ret = 0;

	EMGD_TRACE_ENTER;

	if (!in_list || !(in_list->num_dtds) || !(in_list->dtd)) {
		return NULL;
	}

	t = (pd_timing_t *)OS_ALLOC((in_list->num_dtds + 1)
			* sizeof(pd_timing_t));
	if (!t) {
		return NULL;
	}

	OS_MEMSET(t, 0, (in_list->num_dtds + 1) * sizeof(pd_timing_t));
	timing = t;
	dtd = in_list->dtd;

	/*
	 * OPTIMIZEME: When igd_display_info_t goes away there will be no reason
	 * to copy this data. A Rule can be imposed that anything passed to
	 * the HAL during init must remain in scope until the HAL is shut
	 * down. The HAL can then just use this directly.
	 */
	for (i = 0; i < in_list->num_dtds; i++) {
		OS_MEMCPY(t, dtd, sizeof(igd_display_info_t));
		t->mode_info_flags = dtd->flags | PD_MODE_DTD_USER | PD_MODE_SUPPORTED;
#if 0
		/* Assume there is no border, then htotal and vtotal are the same as
		 * hblank_end and vblank_end */
		t->htotal = t->hblank_end;
		t->vtotal = t->vblank_end;
		if (dtd->refresh) {
			t->refresh = (unsigned short)dtd->refresh;
		} else if (t->htotal && t->vtotal) {
			t->refresh = (unsigned short)
				((unsigned long)((unsigned long)(t->dclk) * 1000) /
					((unsigned long)(t->htotal) *
						(unsigned long)(t->vtotal)));
		}
#endif

#ifndef CONFIG_MICRO
               /*
                * If the VESA flag is set, set the mode mode number to VESA.
                * DTD may not contain mode number but the VESA flag is set
                * This will cause program pipe VGA to be executed and fail
                * Need to make sure that if USER wants to use VGA mode, that the
                * mode_number is entered in the user DTD
                */
               if((t->mode_info_flags & PD_MODE_VESA) && (!t->mode_number)){
                       t->mode_number = VGA_MODE_NUM_MAX + 1;
               }

               /*
                * Handle the corner case where user DTD is derived from std timing.
                * Two std timings have border (htotal != h_blank_end).
                * Compare the timing attribute with the std timing and use the
                * total and refresh rate from std timing.
                * Only happens when using Harmonic tool so the change is limited
                * to Atom E6xx through the dispatch function
                */
                if(pi_context->igd_context->mod_dispatch.get_refresh_in_border){
                       /* returns 1 if a refresh was obtained */
                       ret = pi_context->igd_context->mod_dispatch.get_refresh_in_border(t);
               }
#endif
               if(!ret){
                       /* Assume there is no border, then htotal and vtotal are the same as
                        * hblank_end and vblank_end */
                       t->htotal = t->hblank_end;
                       t->vtotal = t->vblank_end;
                       if (dtd->refresh) {
                               t->refresh = (unsigned short)dtd->refresh;
                       } else if (t->htotal && t->vtotal) {
                               /*
                                * Refresh is used mainly for esthetic, mainly in GUI.
                                * The compiler will truncate the decimals, not rounding
                                * UP the value (in this case, 59.7 will be 59Hz and not 60Hz
                                * Need to manually handle the rounding.
                                *
                                * We multiply the dclk by 10, therefore shifting the final
                                * decimal place by one, then check if the last digit is
                                * >4 to round up the refresh by 1 after dividing by 10.
                                */
                               unsigned short temp_refresh = (unsigned short)
                                       ((unsigned long)((unsigned long)(t->dclk) * 10000) /
                                               ((unsigned long)(t->htotal) *
                                                       (unsigned long)(t->vtotal)));
                               if((temp_refresh % 10) > 4){
                                       t->refresh = (temp_refresh / 10 ) + 1;
                               } else {
                                       t->refresh = (temp_refresh / 10);
                               }
                       }
               }

		/* t->pd_extn_ptr = NULL; */
		t->extn_ptr = NULL;

		t++;
		dtd++;
	}

	/* End the table with end marker */
	t->width = IGD_TIMING_TABLE_END;

	EMGD_TRACE_EXIT;

	return timing;
}

/*!
 * Assign dynamic VBE numbers to the modes that do not already have
 * VESA defined numbers.
 *
 * @param timing_table
 *
 * @return void
 */
#define FIRST_DYNAMIC_MODE_NUMBER 0x120
void assign_dynamic_numbers(igd_timing_info_t *timing_table)
{
	unsigned short next_number = FIRST_DYNAMIC_MODE_NUMBER;
	unsigned int i;
	unsigned short vesa_mode_table[] = {
		640, 480, 0x101,
		800, 600, 0x103,
		1024, 768, 0x105,
		1280, 1024, 0x107,
		0xffff, 0xffff, 0xffff,
	};

	EMGD_TRACE_ENTER;

	while(timing_table->width != IGD_TIMING_TABLE_END) {
		if((timing_table->mode_info_flags & IGD_MODE_SUPPORTED) &&
			!(timing_table->mode_info_flags & IGD_MODE_VESA)) {

			for (i=0; vesa_mode_table[i] != 0xffff; i+=3) {
				if ((timing_table->width == vesa_mode_table[i]) &&
					(timing_table->height == vesa_mode_table[i+1])) {
					/* This is a VESA Standard mode, so assign it to the
					 * correct VESA mode number.  This can occur with
					 * modes added either through User Defined DTDs or
					 * potentially EDID. */
					timing_table->mode_number = vesa_mode_table[i+2];
					timing_table->mode_info_flags |= IGD_MODE_VESA;
					break;
				}
			}
			if (vesa_mode_table[i] == 0xffff) {
				/* Assign this mode a Dynamic number, if it is not
				 * a VESA Standard mode. */
				timing_table->mode_number = next_number;
				/* VBE modes use lower 2 bits for depth so next mode is += 4 */
				next_number += 4;
				timing_table->mode_info_flags |= IGD_MODE_VESA;
			}
		}
		timing_table++;

		/* If reached the first table END,
		 * then check for the added modes */
		if (timing_table->width == IGD_TIMING_TABLE_END &&
			timing_table->extn_ptr) {
			timing_table = timing_table->extn_ptr;
		}
	}

	EMGD_TRACE_EXIT;
	return;
}

/*!
 * This is a utility function that can be used througout the HAL.
 * It can be used to get a ptr to an attr structure and/or the
 * actual current_value of that attribute.
 * According to usage modal at time of creation of this function:
 *	- the *caller_pd_attr must be NULL if the attr was not found.
 *	- the *attr_value is only changed if the attr was found
 *
 * @param port
 * @param attr_id
 * @param flag
 * @param caller_pd_attr
 * @param attr_value
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
int pi_pd_find_attr_and_value(igd_display_port_t *port,
		unsigned long attr_id,
		unsigned long flag,
		pd_attr_t   **caller_pd_attr,
		unsigned long *attr_value)
{
	unsigned long pd_attr_length   = 0;
	pd_attr_t    *pd_attr_list     = NULL;
	pd_attr_t    *found_pd_attr    = NULL;

	EMGD_TRACE_ENTER;

	if(!port || !(port->pd_driver)) {
		return -IGD_ERROR_INVAL;
	}

	if(flag == PD_ATTR_FLAG_GENERAL){
		pd_attr_length = PD_QUERY_GENERAL_ATTR;
	}
	port->pd_driver->get_attrs(port->pd_context, &pd_attr_length,
		&pd_attr_list);
	if(!pd_attr_length) {
		return -IGD_ERROR_INVAL;
	}

	found_pd_attr = pd_get_attr(pd_attr_list, pd_attr_length, attr_id, flag);

	if (!found_pd_attr) {
		if(caller_pd_attr) {
			*caller_pd_attr = NULL;
		}
		return -IGD_INVAL;
	}
	if(caller_pd_attr) {
		*caller_pd_attr = found_pd_attr;
	}
	if(attr_value) {
		*attr_value = found_pd_attr->current_value;
	}

	EMGD_TRACE_EXIT;
	return 0;
}

/*!
 *
 * @param port
 * @param id
 * @param value
 *
 * @return 0 on success
 * @return -IGD_ERROR_INVAL on failure
 */
int pi_get_port_init_attr(igd_display_port_t *port,
		unsigned long id,
		unsigned long *value)
{
	unsigned short i;

	EMGD_TRACE_ENTER;

	if (!port || !port->attr_list) {
		return -IGD_ERROR_INVAL;
	}

	for (i = 0; i < (unsigned short) port->attr_list->num_attrs; i++) {
		if (port->attr_list->attr[i].id == id) {
			*value = port->attr_list->attr[i].value;
			EMGD_TRACE_EXIT;
			return 0;
		}
	}

	EMGD_DEBUG("Attribute (0x%ld) Not Found", id);
	EMGD_TRACE_EXIT;
	return -IGD_ERROR_INVAL;
}



int pi_save_mode_state(igd_display_port_t *port, reg_state_id_t reg_state_id)
{
	int ret = PD_SUCCESS;
	mode_state_t  *mstate = NULL;
	if(pi_context->igd_context->mod_dispatch.reg_get_mod_state) {
		module_state_h     *state = NULL;
		unsigned long *flags = NULL;
		pi_context->igd_context->mod_dispatch.reg_get_mod_state(
			reg_state_id,
			&state,
			&flags);

		if (state) {
			mstate = (mode_state_t *)(*state);
		}
	}

	/* If mode state is present in register context,
	 * then call save() function to save the port driver's state */
	if (mstate) {
		ret = port->pd_driver->pd_save(port->pd_context,
				&(mstate->pd_state[pi_context->num_pi_drivers].state), 0);
		if (ret) {
			EMGD_ERROR_EXIT("port driver: reg saving error. ret = %d", ret);
			return ret;
		}
		mstate->pd_state[pi_context->num_pi_drivers].port = port;
	}

	return ret;
}

