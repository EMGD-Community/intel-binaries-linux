/*
 *-----------------------------------------------------------------------------
 * Filename: igd_pwr.c
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

#define MODULE_NAME hal.power

#include <config.h>
#include <io.h>

#include <igd_init.h>
#include <igd_errno.h>
#include <igd_pwr.h>
#include <igd_mode.h>

#include <module_init.h>
#include <mode.h>
#include <dsp.h>
#include <intelpci.h>
#include <dispatch.h>
#include "pwr_dispatch.h"           /* contains the power dispatch functions */

/*!
 * @addtogroup state_group
 * @{
 */

static dispatch_table_t pwr_dispatch_list[] = {
#ifdef CONFIG_PLB
	{PCI_DEVICE_ID_VGA_PLB, &pwr_dispatch_plb},
#endif
#ifdef CONFIG_TNC
	{PCI_DEVICE_ID_VGA_TNC, &pwr_dispatch_plb},
#endif
	{0, NULL}
};

typedef struct _power_context {
	pwr_dispatch_t *dispatch;
	void *power_state;
} power_context_t;

static power_context_t power_context;

static int igd_pwr_query(igd_driver_h driver_handle, unsigned int power_state)
{                                                           /* igd_pwr_query */
	return power_context.dispatch->pwr_query(power_state);
}                                                           /* igd_pwr_query */

/*!
 * This function alters the power state of the graphics device.  This
 * function saves and restores registers as necessary and changes the
 * power states of the mode and overlay module.
 *
 * This function will convert a request for D2 to a request for D1.
 *
 * @param driver_handle driver context
 * @param dwPowerState power state to set the device to.
 *
 * @return < 0 on Error
 * @return 0 on Success
 */
static int igd_pwr_alter(igd_driver_h driver_handle, unsigned int dwPowerState)
{                                                           /* igd_pwr_alter */
	unsigned char *mmio;
	int           retval   = 0;
	igd_context_t *context = (igd_context_t *) driver_handle;

	EMGD_DEBUG("in igd_pwr_alter");

	EMGD_DEBUG("Power State requested: 0x%x", dwPowerState);

	if(context == NULL) {
		EMGD_ERROR("In igd_pwr_alter:-Device context is null");
		return -IGD_ERROR_INVAL;
	}

	if(power_context.power_state == NULL) {
		EMGD_DEBUG("In igd_pwr_alter:- Memory not allocated yet."
				" Power Module Not Initialised");
		return -IGD_ERROR_INVAL;
	}

	mmio = context->device_context.virt_mmadr;

	if(context->device_context.power_state == dwPowerState ) {
		EMGD_DEBUG("Already in the present state");
		return -IGD_ERROR_INVAL;
	}

	switch(dwPowerState) {
	case IGD_POWERSTATE_D0:
		/* Do any chipset specific power management */
		retval = power_context.dispatch->pwr_set(context, dwPowerState);

		/* restore the registers */
		retval = context->mod_dispatch.reg_restore(context,
			power_context.power_state);
		if (retval) {
			return retval;
		}

		/* Officially change the power state after registers are restored */
		context->device_context.power_state = IGD_POWERSTATE_D0;

		/* Power up mode module */
		if(context->mod_dispatch.mode_pwr) {
			retval = context->mod_dispatch.mode_pwr(context, dwPowerState);
			if (retval) {
				return retval;
			}
		}

		/* enable overlay */
		if(context->mod_dispatch.overlay_pwr) {
			context->mod_dispatch.overlay_pwr(context, IGD_POWERSTATE_D0);
		}

		/* enable msvdx */
		if(context->mod_dispatch.msvdx_pwr) {
			context->mod_dispatch.msvdx_pwr(context, IGD_POWERSTATE_D0);
		}
		break;

	case IGD_POWERSTATE_D1:
	case IGD_POWERSTATE_D2:
		/* Standby - ACPI S1 */

		if (IGD_POWERSTATE_D2 == dwPowerState) {
			dwPowerState = IGD_POWERSTATE_D1;
		}


		/* Turn off the msvdx */
		if(context->mod_dispatch.msvdx_pwr) {
			context->mod_dispatch.msvdx_pwr(context, dwPowerState);
		}

		/* Turn off the overlay */
		if(context->mod_dispatch.overlay_pwr) {
			context->mod_dispatch.overlay_pwr(context, dwPowerState);
		}

		/* save registers */
		retval = context->mod_dispatch.reg_save(context,
			power_context.power_state);
		if (retval) {
			return retval;
		}

		/* Change the state of the device to Dx.  This is required so the
		 * plane/pipe/port all use this.  This needs to happen after "reg_save"
		 * because igd_sync will timeout if power state is not D0
		 */
		context->device_context.power_state = dwPowerState;

		if(context->mod_dispatch.mode_pwr) {
			retval = context->mod_dispatch.mode_pwr(context, dwPowerState);
			if (retval) {
				return retval;
			}
		}

		/* Do any chipset specific power management */
		retval = power_context.dispatch->pwr_set(context, dwPowerState);

		break;

	case IGD_POWERSTATE_D3:
		/* Suspend to memory - ACPI S3 */

		/* diable msvdx */
		if(context->mod_dispatch.msvdx_pwr) {
			context->mod_dispatch.msvdx_pwr(context, dwPowerState);
		}

		/* disable overlay */
		if(context->mod_dispatch.overlay_pwr) {
			context->mod_dispatch.overlay_pwr(context, dwPowerState);
		}

		/* save registers */
		retval= context->mod_dispatch.reg_save(context,
			power_context.power_state);
		if (retval) {
			return retval;
		}

		/* Change the state of the device to Dx.  This is required so the
		 * plane/pipe/port all use this.  This needs to happen after "reg_save"
		 * because igd_sync will timeout if power state is not D0
		 */
		context->device_context.power_state = dwPowerState;

		/* Mode module Power */
		if(context->mod_dispatch.mode_pwr) {
			retval = context->mod_dispatch.mode_pwr(context, dwPowerState);
			if (retval) {
				return retval;
			}
		}


		/* Do any chipset specific power management */
		retval = power_context.dispatch->pwr_set(context, dwPowerState);

		break;

	default:
		/* state undefined */
		EMGD_ERROR("In igd_pwr_alter:-Undefined Power State");
		break;

	}

	return retval;
}                                                           /* igd_pwr_alter */

/*!
 * This function should be called from context manager.
 * To delete the memory allocated ealier.
 *
 * @param context
 *
 * @return void
 */
static void _pwr_shutdown(igd_context_t *context)
{

	if(power_context.power_state) {
		context->mod_dispatch.reg_free(context, power_context.power_state);
	}
	power_context.power_state = NULL;
	context->device_context.power_state = IGD_POWERSTATE_UNDEFINED;
	return;
}

/*!
 * This function should only be called only once from context manager .
 * It initializes the power module.
 *
 * @param context
 *
 * @return < 0 on Error
 * @return 0 on Success
 */
int _pwr_init(igd_context_t *context)
{                                                               /* _pwr_init */
	EMGD_ASSERT(context, "Null Context", -IGD_ERROR_INVAL);

	/* Hook up the IGD dispatch table entires for power */
	context->dispatch.pwr_alter = igd_pwr_alter;
	context->dispatch.pwr_query = igd_pwr_query;

	/* Inter-module dispatch */
	context->mod_dispatch.pwr_shutdown = _pwr_shutdown;

	power_context.dispatch = (pwr_dispatch_t *)dispatch_acquire(context,
		pwr_dispatch_list);

	if(!power_context.dispatch) {
		return -IGD_ERROR_NODEV;
	}

	/* Initialize chipset specific default power behavior */
	power_context.dispatch->pwr_init(context);

	power_context.power_state = context->mod_dispatch.reg_alloc(context,
		IGD_REG_SAVE_VGA |
		IGD_REG_SAVE_DAC | IGD_REG_SAVE_MMIO | IGD_REG_SAVE_RB |
		IGD_REG_SAVE_MODE | IGD_REG_SAVE_3D );

	if(!power_context.power_state) {
		return -IGD_ERROR_NOMEM;
	}

	return 0;
}                                                               /* _pwr_init */

