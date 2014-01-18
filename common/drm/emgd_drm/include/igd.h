/*
 *-----------------------------------------------------------------------------
 * Filename: igd.h
 * $Revision: 1.22 $
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
 *  This file contains the top level dispatch table definition and includes
 *  the common header files necessary to interface with the shingle springs
 *  graphics driver.
 *-----------------------------------------------------------------------------
 */

#ifndef _IGD_H
#define _IGD_H

#include <config.h>
#include <igd_errno.h>
#include <igd_mode.h>
#include <igd_appcontext.h>
#include <igd_render.h>
#include <igd_2d.h>
#include <igd_pd.h>
#include <igd_gmm.h>
#include <igd_rb.h>
#include <igd_ovl.h>
#include <emgd_shared.h>

/*
 * This is needed so that 16bit ports can use a far pointer on some
 * of the prototypes.
 */
#ifndef FAR
#define FAR
#endif



/*!
 * @ingroup render_group
 * @brief Dispatch table for accessing all runtime driver functions.
 *
 * This is the dispatch table for the driver. All rendering and driver
 * manipulation functionality is done by calling functions within this
 * dispatch table.
 * This dispatch table will be populated during the igd_module_init()
 * function call. Upon returning from that call all usable members
 * will be populated. Any members left as NULL are not supported in the
 * current HAL configuration and cannot be used.
 */
typedef struct _igd_dispatch {
	/*!
	 * Save the register state of the graphics engine.
	 * This function is optional in the HAL. The caller must check that it
	 * is non-null before calling.
	 *
	 * @param driver_handle The driver handle returned from igd_driver_init().
	 *
	 * @param flags Any combination of the @ref driver_save_flags
	 *  flags which control the types of state to be saved. Also used to
	 *  specify where we save the register data.
	 *
	 * @return 0 on Success
	 * @return <0 on Error
	 */
	int (*driver_save)(igd_driver_h driver_handle,
		const unsigned long flags);

	/*!
	 * Restore the graphics engine to a previously saved state.
	 * This function is optional in the HAL. The caller must check that it
	 * is non-null before calling.
	 *
	 * @param driver_handle The driver handle returned from igd_driver_init().
	 *
	 * @param flags Used to specify where we are restoring from,
	 *  @ref driver_save_flags
	 *
	 * @return 0 on Success
	 * @return <0 on Error
	 */
	int (*driver_restore)(igd_driver_h driver_handle,
		const unsigned long flags);

	/*!
	 * Save all driver registers and then restore all
	 *   registers from a previously saved set.
	 * This function is optional in the HAL. The caller must check that it
	 * is non-null before calling.
	 *
	 * @param driver_handle The driver handle returned from igd_driver_init().
	 * @param flags Flags indicating what registers to save
	 *
	 * @return 0 on Success
	 * @return <0 on Error
	 */
	int (*driver_save_restore)(igd_driver_h driver_handle, unsigned long flags);

	/*!
	 * Gets the value of a runtime driver parameter. These parameters are
	 * each defined with a unique ID and may be altered at runtime.
	 *
	 * @note: There is a wrapper for this function in the dispatch table that
	 * takes a display instead of a driver handle. This version is for use
	 * when displays are not yet available.
	 *
	 * @return 0 Success
	 * @return -IGD_INVAL Error
	 */
	int (*get_param)(igd_display_h display_handle, unsigned long id,
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
	int (*set_param)(igd_display_h display_handle, unsigned long id,
		unsigned long value);

	/*!
	 *  This function returns the list of available pixel formats for the
	 *  framebuffer and the list of available pixel formats for the cursor.
	 *
	 *  Both lists end with NULL.  They are read only and should
	 *  not be modified.
	 *
	 *  @bug To be converted to take a driver handle for IEGD 5.1
	 *
	 *  @param display_handle A igd_display_h type returned from a previous
	 *    display->alter_displays() call.
	 *
	 *  fb_list_pfs  - Returns the list of pixel formats for the framebuffer.
	 *
	 *  cu_list_pfs - Returns the list of pixel formats for the cursor.
	 *
	 *
	 * Returns:
	 *  0: Success
	 *  -IGD_INVAL: Error;
	 */
	int (*get_pixelformats)(igd_display_h display_handle,
		unsigned long **fb_list_pfs, unsigned long **cu_list_pfs,
		unsigned long **overlay_pfs, unsigned long **render_pfs,
		unsigned long **texture_pfs);

	/*!
	 * @brief Return the list of available DCs.
	 *
	 * query_dc() returns the live zero terminated Display Configuration list.
	 *  All usable display configurations are returned from the HAL. The IAL
	 *  must select one from the list when calling alter_displays().
	 *
	 * @param driver_handle The driver handle returned from igd_driver_init()
	 *
	 * @param dc_list The returned display configuration(s) list. The IAL
	 *  should use a dc from the list when calling alter_displays(). The
	 *  dc_list is zero terminated and live. It should not be altered by the
	 *  IAL. See @ref dc_defines
	 *
	 * @param flags modifies the behavior of the function.
	 *   See: @ref query_dc_flags
	 *
	 * @return 0: Success.
	 * @return -IGD_INVAL:  Otherwise
	 */
	int (*query_dc)(igd_driver_h driver_handle, unsigned long request,
		unsigned long **dc_list, unsigned long flags);

	/*!
	 * Returns a live copy of the current mode list
	 * for the requested display. This mode list will be for the master
	 * port on the pipe and therefore may be the mode list for a TWIN
	 * of the display requested.
	 *
	 * @param driver_handle The driver handle returned from igd_driver_init()
	 *
	 * @param dc The display configuration data to use when determining the
	 *  available modes. See @ref dc_defines
	 *
	 * @param mode_list The returned mode list. This data should be freed by
	 *   calling free_modes() unless the QUERY_LIVE_MODES flag was passed, in
	 *   that case the live, in-use, data structure is returned. It should not
	 *   be modified or freed.
	 *
	 * @param flags Flags to modify the operation of the function. Flags
	 *   may contain any combination of the IGD_QUERY_* flags.
	 *   See: @ref query_mode_list_flags
	 *
	 * @return 0: Success.
	 * @return -IGD_ERROR_INVAL:  Otherwise
	 */
	int (*query_mode_list)(igd_driver_h driver_handle, unsigned long dc,
		igd_display_info_t **mode_list, unsigned long flags);

	/*!
	 * Free modes that were returned from a previous call to query mode list.
	 *
	 * @param mode_list The mode list to be free. This data was returned
	 *   from an earlier call to query_modes.=
	 */
	void (*free_mode_list)(igd_display_info_t *mode_list);

	/*!
	 * alter_displays() Modifies the modes associated with one or both display
	 *  pipes according to the dc provided. The primary and secondary
	 *  display handles are returned for use when rendering to these displays.
	 *
	 *  In extended or DIH modes only one fb_info needs to be provided at a
	 *  time. In this manner two calls can be used one per framebuffer, as
	 *  may be required by the OS.
	 *
	 *  @param driver_handle - required.  This is returned from a call to
	 *   igd_init_driver().
	 *
	 *  @param primary A pointer to a display handle that will be populated
	 *   during the call. This handle should be used for all rendering
	 *   tasks directed to the primary framebuffer and pipe.
	 *
	 *  @param primary_pt_info The display timing information to be used for
	 *    the primary display pipe.
	 *
	 *  @param primary_fb_info The framebuffer parameters to be used for the
	 *   primary display. This data may be larger or smaller than the display
	 *   timings to allow for centered/panned or scaled modes.
	 *
	 *  @param secondary A pointer to a display handle that will be populated
	 *   during the call. This handle should be used for all rendering
	 *   tasks directed to the secondary framebuffer or pipe.
	 *
	 *  @param secondary_pt_info The display timing information to be used for
	 *   the secondary display pipe.
	 *
	 *  @param secondary_fb_info The framebuffer parameters to be used for the
	 *   secondary display. This data may be larger or smaller than the display
	 *   timings to allow for centered/panned or scaled modes.
	 *
	 *  @param dc A unique identifer to describe the configuration of the
	 *   displays to be used. This identifier should be one returned from
	 *   the _igd_dispatch::query_dc() call. See @ref dc_defines.
	 *
	 *  @param flags Bitfield to alter the behavior of the call.
	 */
	int (*alter_displays)(igd_driver_h driver_handle,
		igd_display_h *primary,
		igd_display_info_t *primary_pt_info,
		igd_framebuffer_info_t *primary_fb_info,
		igd_display_h *secondary,
		igd_display_info_t *secondary_pt_info,
		igd_framebuffer_info_t *secondary_fb_info,
		unsigned long dc,
		unsigned long flags);

	/*!
	 * igd_configure_display() Modifies the modes associated with one display
	 *  pipes according to the dc provided.
	 *
	 *  @param driver_handle - required.  This is returned from a call to
	 *   igd_init_driver().
	 *
	 *  @param display A pointer to a display handle that will be used
	 *   during the call. This handle should be used for all rendering
	 *   tasks directed to the secondary framebuffer or pipe.
	 *
	 *  @param pt_info The display timing information to be used for
	 *    the display pipe.
	 *
	 *  @param fb_info The framebuffer parameters to be used for the
	 *   display. This data may be larger or smaller than the display
	 *   timings to allow for centered/panned or scaled modes.
	 *
	 *  @param dc A unique identifer to describe the configuration of the
	 *   displays to be used. This identifier should be one returned from
	 *   the _igd_dispatch::query_dc() call. See @ref dc_defines.
	 *
	 *  @param fb_index What is the 0-based framebuffer index
	 *
	 *  @param flags Bitfield to alter the behavior of the call.
	 */
	int (*igd_configure_display)(
	    igd_driver_h driver_handle,
	    igd_display_h *display,
	    igd_display_info_t *pt_info,
	    igd_framebuffer_info_t *fb_info,
	    unsigned long dc,
	    int fb_index,
		unsigned long flags);

	/*!
	 *  pan_display() pans the display on the display device.
	 *  It takes a @a x_offset, @a y_offset into the frame buffer and
	 *  sets the display from (x_offset, y_offset) to
	 *  (x_offset+width, y_offset+height).
	 *  If x_offset+width, y_offset+height crosses frame buffer
	 *  width and heigth, then it will return error.
	 *
	 * @param display_handle pointer to an IGD_DISPLAY pointer returned
	 *    from a successful call to dispatch->alter_displays().
	 *
	 * @param x_offset these are frame buffer offsets from (0, 0).
	 * @param y_offset these are frame buffer offsets from (0, 0).
	 *
	 * @return 0: The paning was successfull.
	 * @return -IGD_INVAL:  Otherwise
	 */
	long (*pan_display)(igd_display_h display_handle,
		unsigned long x_offset,
		unsigned long y_offset);

	/*!
	 * Alters the current power state for the display. This does not
	 * change the power state for the graphics hardware device.
	 *
	 * @bug Needs to be modified to power all displays on a pipe
	 *
	 * @param driver_handle - handle to a driver handle returned from a
	 *    previous call to igd_driver_init()
	 *
	 * @param port_number - specific display (port) to change.
	 *
	 * @param power_state - D state to change to.
	 *
	 * @returns 0 on Success
	 * @returns <0 on Error
	 */
	int (*power_display)(igd_driver_h driver_handle,
			unsigned short port_number,
			unsigned int power_state);

	/*!
	 *  This function sets one palette entry for the framebuffer when the
	 *  pixel format for the framebuffer indicates a palette is used.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @param palette_color A 32bit ARGB color
	 *
	 * @param palette_entry The palette index to set.
	 *
	 * @returns
	 *  - 0: Success
	 *  - -IGD_INVAL:  Otherwise
	 */
	int (*set_palette_entry)(igd_display_h display_handle,
		unsigned long palette_entry,
		unsigned long palette_color);
	/*!
	 *  This function gets the requested palette entry from the hardware.
	 *  The results are undefined when not in a paletted mode or in a
	 *  mode that uses palette for color correction.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @param palette_color A 32bit ARGB color
	 *
	 * @param palette_entry The palette index to return.
	 *
	 * @returns 0 on Success
	 * @returns -IGD_ERROR_INVAL Otherwise
	 */
	int (*get_palette_entry)(igd_display_h display_handle,
		unsigned long palete_entry,
		unsigned long *palette_color);

	/*!
	 *  This function sets "count" palette entries starting with "count"
	 *  offset into the palette_colors array.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 *  @param palette_colors A 32bit ARGB color array
	 *
	 *  @param start_index The first palette index to program.
	 *
	 *  @param count The number of palette entries to program.
	 *
	 * @returns 0 on Success
	 * @return -IGD_ERROR_INVAL Otherwise
	 */
	int (*set_palette_entries)(igd_display_h display_handle,
		unsigned long *palette_colors, unsigned int start_index,
		unsigned int count);


	/*!
	 * Gets attributes for a display. SS will allocate the memory required to
	 * return the *attr_list. This is a live copy of attributes used by both
	 * IAL and HAL. Don't deallocate this memory. This will be freed by the
	 * HAL.
	 *
	 * @param driver_handle Driver handle returned from a call to
	 *   igd_driver_init()
	 *
	 * @param num_attrs pointer to return the number of attributes
	 *    returned in attr_list.
	 *
	 * @param attr_list pointer to return the attributes.
	 *
	 * @returns 0 onSuccess
	 * @returns -IGD_ERROR_NOATTR No attributes defined for this display
	 * @returns -IGD_INVAL otherwise
	 */
	int (*get_attrs)(igd_driver_h driver_handle,
			unsigned short port_number,
			unsigned long *num_attrs,
			igd_attr_t **attr_list);

	/*!
	 * set attributes for a display.
	 *
	 * @param driver_handle Driver handle returned from a call to
	 *   igd_driver_init()
	 *
	 * @param num_attrs pointer to return the number of attributes
	 *   returned in attr_list. This is equal to the num_attrs returned by
	 *   igd_get_attrs().
	 *
	 * @param attr_list pointer returned from igd_get_attrs(). Change
	 *   the attributes to desired values.
	 *
	 * @returns 0 on Success
	 * @returns -IGD_ERROR_NOATTR No attributes defined for this display
	 * @returns -IGD_INVAL Otherwise
	 */
	int (*set_attrs)(igd_driver_h driver_handle,
			unsigned short port_number,
			unsigned long num_attrs,
			igd_attr_t *attr_list);

	/*!
	 * set flags for a display.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @param port_number of the port to modify.  (use zero to change
	 *   all ports associated with a display?)
	 *
	 * @param flag to set in the port's pt_info before calling program
	 *   port.
	 *
	 * @returns 0 on Success
	 * @returns -IGD_INVAL Otherwise
	 */
	int (*enable_port)(igd_display_h display_handle,
			unsigned short port_number,
			unsigned long flag,
			unsigned long test);

	/*!
	 * This functions returns the current scanline for the display handle
	 * provided. The scanline will be accurate when possible, or equal
	 * to IGD_IN_VBLANK, or IGD_IN_VSYNC during the vblank/vsync periods.
	 * See @ref get_scanline_defs
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @param scanline An unsigned long pointer which will be populated by
	 *    the HAL during the call.
	 *
	 * @returns 0 on Success
	 * @returns <0 on Error
	 */
	int (*get_scanline)(igd_display_h display_handle, int *scanline);

	/*!
	 *  This function alters the parameters associated with a cursor.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @param cursor_info An igd_cursor_info_t data structure with the
	 *   parameters to be applied to the cursor.
	 *
	 * @param image A pointer to cursor image data. The image data
	 *   will only be programmed with the cursor flag has either
	 *   IGD_CURSOR_LOAD_ARGB_IMAGE or IGD_CURSOR_LOAD_XOR_IMAGE.
	 *
	 * @param 0 on Success
	 * @param <0 on Error
	 */
	int (*alter_cursor)(igd_display_h display_handle,
		igd_cursor_info_t *cursor_info, unsigned char *image);

	/*!
	 *  This function alters the position parameters associated with a cursor.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @param cursor_info An igd_cursor_info_t data structure with the
	 *   parameters to be applied to the cursor. Only the x_offset and y_offset
	 *   parameters will be used.
	 *
	 * @param 0 on Success
	 * @param <0 on Error
	 */
	int (*alter_cursor_pos)(igd_display_h display_handle,
		igd_cursor_info_t *cursor_info);

	/*!
	 * This function will block until the start of the next vblank/vsync
	 * period.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @returns 0 on Success
	 * @returns <0 on Error
	 */
	int (*wait_vblank)(igd_display_h display_handle);

	/*!
	 * This function will block until the start of the next vblank/vsync
	 * period.
	 *
	 * @param display_handle Display handle returned from a call to
	 *   _igd_dispatch::alter_displays()
	 *
	 * @returns 0 on Success
	 * @returns <0 on Error
	 */
	int (*wait_vsync)(igd_display_h display_handle);

	int (*query_in_vblank)(igd_display_h display_handle);

	/*!
	 * This function is to access I2C register values for the specified I2C
	 * bus. Memory for 'igd_i2c_reg_t->buffer' should be allocated and freed
	 * by caller.
	 * This function is valid only for display connected via DIGITAL (DVO)
	 * ports.
	 *
	 * @bug Documentation needs update or Remove
	 *
	 *  @param driver IGD driver handle.
	 *
	 *  @param i2c_reg pointer to the i2c register structure.
	 *
	 *  @param flags If the flags is
	 *    IGD_I2C_WRITE - then igd_i2c_reg_t->buffer should point
	 *       to array of 'num_bytes' number of valid I2C registers values.
	 *    IGD_I2C_READ  - then igd_i2c_reg_t->buffer pointer should have space
	 *       for 'num_bytes' number of valid I2C registers.
	 *
	 * @returns 0 on Success
	 * @returns <0 on Error
	 */
	int (*access_i2c)(igd_display_h display, igd_i2c_reg_t *i2c_reg,
			unsigned long flags);

	/*!
	 * This function is to get the EDID information (not the actual block) for
	 * the display device associated with the 'display_handle'.
	 *
	 * @param driver_handle The driver handle returned from igd_driver_init().
	 * @param port_number display port number connected to display.
	 * @param edid_version EDID version number.
	 * @param edid_revision EDID revision number.
	 * @param edid_size Tells the caller what size of buffer to allocate
	 *   for the transfer.
	 *
	 * @returns 0 on Success
	 * @returns -IGD_ERROR_EDID Error while reading EDID or no EDID
	 * @return -IGD_INVAL Other error
	 */
	int (*get_EDID_info)(igd_driver_h driver_handle,
			unsigned short port_number,
			unsigned char *edid_version,
			unsigned char *edid_revision,
			unsigned long *edid_size);

	/*!
	 * This function is to get the EDID block of the specified size. The size
	 * is returned from previous call to igd_get_EDID_info().
	 *
	 * @note The return value indicates success/failure, blocksize should be
	 *  set by the API to the actual number of bytes transferred upon return.
	 * @param driver_handle The driver handle returned from igd_driver_init().
	 * @param port_number Specific display to query for EDID block data.
	 * @param edid_ptr Buffer to hold the EDID block data, must be 128bytes.
	 * @param block_number Tells the API which EDID block to read.
	 *
	 *  @returns 0 on Success
	 *  @returns -IGD_ERROR_EDID Error while reading EDID or no EDID
	 *  @returns -IGD_INVAL Other error
	 */
	int (*get_EDID_block)(igd_driver_h driver_handle,
		unsigned short port_number,
		unsigned char FAR *edid_ptr,
		unsigned char block_number);

	/*!
	 * This function returns the information about the port/display
	 *
	 * @param driver_handle pointer to an IGD_DRIVER_H pointer returned
	 *    from a successful call to igd_driver_init().
	 *
	 * @param port_number Specific port to get information for.
	 *
	 * @param port_info Port/display information
	 *
	 * @returns 0 on Success
	 * @returns -IGD_INVAL Otherwise
	 */
	int (*get_port_info)(igd_driver_h driver_handle,
			unsigned short port_number,
			igd_port_info_t *port_info);

	/*
	 * Sync is used to insert and check the status of synchronization
	 * points in the command queue. A sync inserted into the queue and
	 * then check insures that all rendering commands inserted before the
	 * sync are now complete.
	 *
	 * Sync should be called with the IGD_RENDER_NONBLOCK flag to insert
	 * a new sync without waiting for completion. When called with
	 * IGD_RENDER_BLOCK the call will wait for completion but may return
	 * due to a timeout. The caller should determine if calling again is
	 * prudent or if some other action should be taken insead of busy
	 * waiting.
	 *
	 * @param display_handle pointer to an IGD_DISPLAY pointer returned
	 *    from a successful call to dispatch->alter_displays().
	 *
	 * @param priority The command queue to use. IGD_PRIORITY_NORMAL is
	 *    correct for most circumstances.
	 *
	 * @param sync The sync identifier that will be populated and returned
	 *    during the call. To insert a new sync, this should be passed
	 *    containing 0 (A pointer to a zero). To check the status of an
	 *    existing sync pass the value returned from a previous call to
	 *    this function.
	 *
	 * @param flags Sync flags.
	 *
	 * @returns
	 *   0: On Success
	 *   -IGD_ERROR_BUSY: When the sync is not yet complete
	 */
	int (*sync)(igd_display_h display_handle, int priority,
		unsigned long *sync, unsigned long flags);
	/*
	 * Idle stalls until the entire engine has been idled.
	 */
	int (*idle)(igd_driver_h driver_handle);

	/* igd_appcontext.h */
	igd_appcontext_h (*appcontext_alloc)(igd_display_h display_handle,
		int priority, unsigned int flags);
	void (*appcontext_free)(igd_display_h display_handle,
		int priority, igd_appcontext_h context_handle);

	/* igd_pwr.h */
	int (*pwr_alter)(igd_driver_h driver_handle, unsigned int power_state);
	int (*pwr_query)(igd_driver_h driver_handle, unsigned int power_state);

	/* igd_reset.h */
	int (*reset_alter)(igd_driver_h driver_handle);

	/* igd_gmm.h */

	/*!
	 * This function is used by igd client drivers to allocate surfaces from
	 * graphics memory. A driver must use this function to allocate all
	 * surfaces, and must in turn free the surfaces with dispatch->gmm_free()
	 * before exit. Calls to this function are only valid after the gmm_init()
	 * function has been called by the igd init module.
	 *
	 * @param offset The offset into the Gtt memory space that the surface
	 *   begins. This is an output only. Each element of this array can be
	 *   added to the base physical or virtual address to obtain a full
	 *   virtual or physical address. Therefore this parameter should be
	 *   accessed as offset[x]. The number of offset elements is dependent on
	 *   the surface type. Normal and Buffer surfaces return 1 element, Mip Map
	 *   surfaces return NumMips level elements. Cube Map surfaces return
	 *   NumMips level elements. Volume Map surfaces return NumSlices elements.
	 *   For Volume Maps: The number of planes decreases by half for each
	 *   mip level just as the number of X and Y pixels decreases by half.
	 *   All plane offsets for the first mip are returned first followed
	 *   by all planes for the second mip and so-forth.
	 *
	 * @param pixel_format The pixel format id. See @ref pixel_formats
	 *
	 * @param width The width in pixels of the surface. This may be modified
	 *   to be larger than the requested value.
	 *
	 * @param height Height in pixels of the surface. This may be modified to
	 *   be larger than the requested value.
	 *
	 * @param pitch The pitch in bytes of the surface. This value is returned
	 *   by the driver.
	 *
	 * @param size The size of the surface in bytes. This value is returned by
	 *   the driver. In Planar formats this will be greater than height*pitch
	 *
	 * @param type The defined type of the surface to be allocated.
	 *   See @ref alloc_surface_types
	 *
	 * @param flags A bitfied of potential uses for the surface. These
	 *   potentially impact the requried alignment of the surface offset.
	 *   See @ref surface_info_flags
	 *
	 * @returns 0 on Success
	 * @returns <0 Error
	 */
	int (*gmm_alloc_surface)(unsigned long *offset,
		unsigned long pixel_format,
		unsigned int *width,
		unsigned int *height,
		unsigned int *pitch,
		unsigned long *size,
		unsigned int type,
		unsigned long *flags);


	int (*gmm_map_ci)(unsigned long *gtt_offset,
			unsigned long ci_param,	/*virtaddr or v4l2_offset*/
			unsigned long *virt_addr,
			unsigned int map_method,
			unsigned long size);


  	int (*gmm_unmap_ci)(unsigned long virt_addr);


	/*!
	 * This function maps an existing list of pages into the GTT.
	 *
	 * @param pagelist The live list of pages to be mapped.  The GMM
	 *   should not modify or release this list.
	 * @param gtt_offset The offset into the Gtt memory space at which the
	 *   pages begin.  This is an output only.
	 *
	 * @param numpages The number of pages to map (i.e., length of pagelist).
	 *
	 * @returns 0 on Success
	 * @returns <0 Error
	 */
	int (*gmm_import_pages)(void **pagelist,
			unsigned long *gtt_offset,
			unsigned long numpages);

	int (*gmm_get_num_surface)(unsigned long *count);
	int (*gmm_get_surface_list)(unsigned long allocated_size,
		unsigned long *list_size,
		igd_surface_list_t **surface_list);

	/*!
	 * This function is used by igd client drivers to allocate regions from
	 * graphics memory. Regions are portions of video memory that do not have
	 * width and height parameters, only a linear size. A driver must use this
	 * function to allocate all regions, and must in turn free the regions with
	 * igd_mm_free() before exit.
	 * Calls to this function are only valid after the gmm_init() function
	 * has been called by the igd init module.
	 *
	 * @param offset The offset into the Gtt memory space that the region
	 *   begins. This is an output only. This value can be added to the base
	 *   physical or virtual address to obtain a full virtual or physical
	 *   address.
	 *
	 * @param size The size of the region, this value may be modified to be
	 *   larger or smaller than the requested value.
	 *
	 * @param type The defined type of the surface to be allocated.
	 *   See @ref alloc_region_types
	 *
	 * @param flags A bitfied of potential uses for the region. These
	 *   potentially impact the requried alignment of the region offset.
	 *   See @ref alloc_region_flags
	 *
	 * @returns 0 on Success
	 * @returns <0 on Error
	 */
	int (*gmm_alloc_region)(unsigned long *offset,
		unsigned long *size,
		unsigned int type,
		unsigned long flags);

	/*!
	 * This function is used to find the actual physical address of the
	 * system memory page given an offset into Gtt memory. This function
	 * will only be successful if the offset provided was allocated to a
	 * cursor or overlay register type.
	 *
	 * @param offset The offset as provided by the allocation function.
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_virt_to_phys)(unsigned long offset,
		unsigned long *physical);

	/*!
	 *  This function should be used to free any regions or surfaces allocated
	 *  during igd use. Calling with offsets that were not obtained via a
	 *  prior call to _igd_dispatch::gmm_alloc_surface() or
	 *  _igd_dispatch::gmm_alloc_region() are invalid and will produce
	 *  undefined results.
	 *  Calls to this function are only valid after the igd_module_init()
	 *  function has been called.
	 *
	 * @param offset The offset as provided by the allocation function.
	 *
	 * @returns void
	 */
	void (*gmm_free)(unsigned long offset);

	/*!
	 *  This function should be used to release externally-allocated
	 *  page lists that have been imported into the GMM.  This will
	 *  simply unmap the pages from the GTT; the pages themselves
	 *  should subsequently be freed by the external source.
	 *  Calling with offsets that were not obtained via a
	 *  prior call to _igd_dispatch::gmm_import_pages() or
	 *  are invalid and will produce undefined results.
	 *  Calls to this function are only valid after the igd_module_init()
	 *  function has been called.
	 *
	 * @param offset The offset as provided by the allocation function.
	 *
	 * @returns void
	 */
	void (*gmm_release_import)(unsigned long offset);

	/*!
	 * This function returns current memory statistics.
	 *
	 * @param memstat An _igd_memstat structure to be populated during the call
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_memstat)(igd_memstat_t *memstat);

	/*!
	 * Allocates a surface similar to _igd_dispatch::gmm_alloc_surface();
	 * however, in this case the surface is allocated from a cached pool of
	 * similar surfaces to improve performance. The surface cache shrinks and
	 * grows automatically based on usage model. The surface cache should
	 * be used when many surfaces of similar format and size are allocated
	 * and freed repeatedly and the highest performance is required. The
	 * tradeoff is that memory will be consumed by the cache and which
	 * can lead to the need to flush the cache when non-cached surfaces
	 * fail due to out-of-memory conditions.
	 *
	 * Surface is passed in, and populated during call.
	 * All inputs are the same as with gmm_alloc_surface.
	 *
	 * @param display_handle Display used with this surface
	 * @param surface Input/Output structure containing surface information
	 * @param flags used to modify the behavior of the function.
	 *     See @ref gmm_alloc_cached_flags
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_alloc_cached)(igd_display_h display_handle,
		igd_surface_t *surface, unsigned int flags);

	/*!
	 * Free a surface previously allocated with the
	 * _igd_dispatch::gmm_alloc_cached() dispatch function. When freeing
	 * a cached surface it is not necessary for rendering to be complete. In
	 * this manner better performance can be achieved because it is likely
	 * that the rendering will be complete before the surface could be reused.
	 * When freeing a cached surface a sync ID obtained from
	 * _igd_dispatch::sync() after all rendering commands to the surface
	 * should be provided such that the cache manager can be sure rendering
	 * is complete before the surface is reused or freed.
	 *
	 * @param display_handle Display used with this surface
	 * @param surface structure containing surface information
	 * @param sync_id obtained after all rendering to/from this surface
	 */
	void (*gmm_free_cached)(igd_display_h display_handle,
		igd_surface_t *surface,
		unsigned long sync_id);

	/*!
	 * Allocates a region similar to _igd_dispatch::gmm_alloc_region();
	 * however, in this case the region is allocated from a cached pool of
	 * similar regions to improve performance. The region cache shrinks and
	 * grows automatically based on usage model. The region cache should
	 * be used when many regions of similar format and size are allocated
	 * and freed repeatedly and the highest performance is required. The
	 * tradeoff is that memory will be consumed by the cache and which
	 * can lead to the need to flush the cache when non-cached regions
	 * fail due to out-of-memory conditions.
	 *
	 * Region information is passed in, and populated during call.
	 * All inputs are the same as with gmm_alloc_region.
	 *
	 * @param display_handle Display used with this region
	 *
	 * @param offset The offset into the Gtt memory space that the region
	 *   begins. This is an output only. This value can be added to the base
	 *   physical or virtual address to obtain a full virtual or physical
	 *   address.
	 *
	 * @param size The size of the region, this value may be modified to be
	 *   larger or smaller than the requested value.
	 *
	 * @param type The defined type of the region to be allocated.
	 *   See @ref alloc_region_types
	 *
	 * @param region_flags A bitfied of potential uses for the region. These
	 *   potentially impact the requried alignment of the region offset.
	 *   See @ref alloc_region_flags
	 *
	 * @param flags used to modify the behavior of the function.
	 *     See @ref gmm_alloc_cached_flags
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_alloc_cached_region)(igd_display_h display_handle,
		unsigned long *offset,
		unsigned long *size,
		unsigned int type,
		unsigned int region_flags,
		unsigned int flags);

	/*!
	 * Free a region previously allocated with the
	 * _igd_dispatch::gmm_alloc_cached_region() dispatch function. When freeing
	 * a cached region it is not necessary for rendering to be complete. In
	 * this manner better performance can be achieved because it is likely
	 * that the rendering will be complete before the region could be reused.
	 * When freeing a cached region a sync ID obtained from
	 * _igd_dispatch::sync() after all rendering commands to the region
	 * should be provided such that the cache manager can be sure rendering
	 * is complete before the region is reused or freed.
	 *
	 * @param display_handle Display used with this region
	 *
	 * @param offset The offset into the Gtt memory space that the region
	 *   begins. This is an output only. This value can be added to the base
	 *   physical or virtual address to obtain a full virtual or physical
	 *   address.
	 *
	 * @param size The size of the region, this value may be modified to be
	 *   larger or smaller than the requested value.
	 *
	 * @param type The defined type of the region to be allocated.
	 *   See @ref alloc_region_types
	 *
	 * @param region_flags A bitfied of potential uses for the region. These
	 *   potentially impact the requried alignment of the region offset.
	 *   See @ref alloc_region_flags
	 *
	 * @param sync_id_write obtained after all rendering to this region
	 * @param sync_id_read obtained after all rendering from this region
	 */
	void (*gmm_free_cached_region)(igd_display_h display_handle,
		unsigned long offset,
		unsigned long size,
		unsigned int type,
		unsigned int region_flags,
		unsigned long sync_id_write,
		unsigned long sync_id_read);

	/*!
	 *  Flushes surfaces out of the internal surface cache. During normal
	 * operation an IAL will not need to call this. Only when an IAL is
	 * making use of the surface cache for some operations and direct
	 * gmm operations for others would it be necessary to flush the cache.
	 *
	 *
	 * @returns 0 No surfaces flushed
	 * @returns >0 Surfaces flushed
	 * @returns <0 Error
	 */
	int (*gmm_flush_cache)(void);

	/*!
	 * Allocates a heap of the requested size.  The heap allocated is managed
	 * by the GMM through "gmm_alloc_heapblock", "gmm_free_heapblock", and
	 * "gmm_free_heap" functions.
	 *
	 * @param display_handle Used to access other GMM dispatch functions
	 *
	 * @param heap_offset The offset into the Gtt memory space that the heap
	 *   begins.  This also serves as a "heap ID"
	 *
	 * @param heap_size The size of the heap.
	 *
	 * @param type Set to zero
	 *
	 * @param alignment The alignment of the blocks in this heap
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_alloc_heap)(igd_display_h display_handle,
		unsigned long *heap_offset,
		unsigned long *heap_size,
		unsigned int   type,
		unsigned long  alignment);

	/*!
	 * Frees the heap allocated by gmm_alloc_heap
	 *
	 * @param heap_offset The offset into the Gtt memory space that the heap
	 *   begins
	 */
	void (*gmm_free_heap)(igd_display_h display_handle,
				unsigned long heap_offset);

	unsigned long (*gmm_get_pvtheap_size)(void);
	unsigned long (*gmm_get_cache_mem)(void);
	/*!
	 * Allocates a block of the requested size from the heap indicated by
	 * heap_offset.
	 *
	 * @param display_handle Used to access other GMM dispatch functions
	 *
	 * @param heap_offset The heap ID
	 *
	 * @param block_offset
	 *
	 * @param size The size of the block.
	 *
	 * @param type Set to zero
	 *
	 * @param flags Set to zero
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_alloc_heap_block)(igd_display_h display_handle,
		unsigned long heap_offset,
		unsigned long *block_offset,
		unsigned long *size,
		unsigned long flags);

	/*!
	 * Frees a block of previously allocated by gmm_alloc_heap_block
	 *
	 * @param display_handle Used to access other GMM dispatch functions
	 *
	 * @param block_offset Offset given by gmm_alloc_heap_block
	 */
	void (*gmm_free_heap_block)(igd_display_h display_handle,
		unsigned long heap_offset,
		unsigned long block_offset,
		unsigned long sync_id_write,
		unsigned long sync_id_read);

	/*!
	 * Gets the heap id/offset from which the block
	 *
	 * @param block_offset Offset given by gmm_alloc_heap_block
	 * @param heap_offset Head ID/Offset associated with the block offset
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_get_heap_from_block)(unsigned long block_offset,
		unsigned long *heap_offset);

	/*!
	 * Allocates a reservation of the requested size. A reservation is a
	 * memory range that is dedicated for the callers use but it not
	 * populated with usable memory. The caller must make a single call to
	 * gmm_alloc_surface() or gmm_alloc_region() passing in the reservation
	 * address to allocate a surface within the reservation. Only a single
	 * surface or region may be placed in a reservation.
	 *
	 * @param offset The offset into the Gtt memory space that the reservation
	 *   begins. This is an output only. This value can be added to the base
	 *   physical or virtual address to obtain a full virtual or physical
	 *   address.
	 *
	 * @param size The size of the reservation.
	 *
	 * @param flags A bitfied of potential uses for the reservation. These
	 *   potentially impact the requried alignment of the reservation.
	 *   See @ref alloc_reservation_flags
	 */
	int (*gmm_alloc_reservation)(unsigned long *offset,
		unsigned long size,
		unsigned long flags);

	/*!
	 * Allocates a region similar to _igd_dispatch::gmm_alloc_region();
	 * however, in this case the region is allocated from a pool of
	 * persisent regions.  The pool can only grow as additional regions
	 * are allocated. The pool can be flushed but this should only be
	 * done when GMM is being shutdown.
	 *
	 * Region information is passed in, and populated during call.
	 * All inputs are the same as with gmm_alloc_region.
	 *
	 * @param display_handle Display used with this region
	 *
	 * @param offset The offset into the Gtt memory space that the region
	 *   begins. This is an output only. This value can be added to the base
	 *   physical or virtual address to obtain a full virtual or physical
	 *   address.
	 *
	 * @param size The size of the region, this value may be modified to be
	 *   larger or smaller than the requested value.
	 *
	 * @param type The defined type of the region to be allocated.
	 *   See @ref alloc_region_types
	 *
	 * @param region_flags A bitfied of potential uses for the region. These
	 *   potentially impact the requried alignment of the region offset.
	 *   See @ref alloc_region_flags
	 *
	 * @param flags used to modify the behavior of the function.
	 *     See @ref gmm_alloc_cached_flags
	 *
	 * @returns 0 on Success
	 * @returns  <0 on Error
	 */
	int (*gmm_alloc_persistent_region)(igd_display_h display_handle,
		unsigned long *offset,
		unsigned long *size,
		unsigned int type,
		unsigned int region_flags,
		unsigned int flags);

	/*!
	 * Free a region previously allocated with the
	 * _igd_dispatch::gmm_alloc_persistent_region() dispatch function.
	 * The region is marked as available and may be reused by the next
	 * allocation request.  It is the callers responsibilty to make sure
	 * rendering to the region is complete before freeing it.
	 *
	 * @param display_handle Display used with this region
	 *
	 * @param offset The offset into the Gtt memory space that the region
	 *   begins. This is an input only.
	 */
	int (*gmm_free_persistent_region)(unsigned long offset);

	/*!
	 * Flushes regions out of the internal persistent list. During normal
	 * operation an IAL will not need to call this. Only when an IAL is
	 * exiting should it flush the list.
	 *
	 *
	 * @returns 0 regions flushed
	 * @returns <0 Error
	 */
	int (*gmm_flush_persistent_regions)(igd_display_h display_handle);

	/*!
	 * Creates a linear mapping of a video memory region into the
	 * current process address space.
	 *
	 * @param offset Offset used to identifiy the memory region
	 *
	 * @returns address to mapping
	 * @returns NULL Error
	 */
	void *(*gmm_map)(unsigned long offset);

	/*!
	 * Unmaps a linear mapping created by gmm_map.
	 *
	 * @param address pointer to the mapping
	 */
	void (*gmm_unmap)(void *address);

	/*!
	 * Export the list of physical pages allocated to a memory
	 * retion.
	 *
	 * @param offset Offset used to identify the memory region
	 * @param pages  Array of page structures holding the physical page
	 *               addresses.
	 * @param page_cnt Number of pages in the page array.
	 *
	 * @returns 0 if successful
	 *          -IGD_ERROR_NOMEM if offset is invalid.
	 */
	int (*gmm_get_page_list)(unsigned long offset,
			unsigned long **pages,
			unsigned long *page_cnt);

	void (*gmm_dump)(void);
	void (*gmm_dump_v)(void);
	char *gmm_debug_desc;

	/* igd_2d.h */
	int (*setup_clip_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect,
		igd_appcontext_h appcontext_handle, unsigned int flags);
	int (*setup_blt)(igd_display_h display_handle, int priority,
		igd_surface_t *dest_surf, igd_rect_t *dest_rect,
		unsigned long raster_ops, unsigned long bg_color,
		unsigned long fg_color, igd_appcontext_h appcontext_handle,
		unsigned int flags);
	int (*color_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, unsigned int byte_mask,
		unsigned int color, unsigned int raster_ops,
		igd_appcontext_h appcontext, unsigned int flags);
	int (*rgb_color_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, unsigned int byte_mask,
		unsigned int color, unsigned int raster_ops,
		igd_appcontext_h appcontext, unsigned int flags);
	int (*pat_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, unsigned int byte_mask,
		igd_pat_t *pat, igd_chroma_t *chroma, unsigned int raster_ops,
		igd_appcontext_h appcontext, unsigned int flags);
	int (*mono_pat_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, unsigned int byte_mask,
		igd_mono_pat_t *pat, unsigned int raster_ops,
		igd_appcontext_h appcontext, unsigned int flags);
	int (*src_copy_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, igd_surface_t *src,
		igd_coord_t *src_coord, unsigned int byte_mask, igd_chroma_t *chroma,
		unsigned int raster_ops, igd_appcontext_h appcontext,
		unsigned int flags);
	int (*mono_src_copy_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, igd_mono_src_t *src,
		unsigned int byte_mask, unsigned int raster_ops,
		igd_appcontext_h appcontext, unsigned int flags);
	int (*mono_src_copy_immed_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, igd_mono_src_t *src,
		igd_surface_t *src_surface,	igd_rect_t *src_rect,
		unsigned int byte_mask, unsigned int raster_ops,
		igd_appcontext_h appcontext, unsigned int flags);
	int (*full_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, igd_surface_t *src,
		igd_coord_t *src_coord, unsigned int byte_mask, igd_pat_t *pat,
		unsigned int raster_ops, igd_appcontext_h appcontext,
		unsigned int flags);
	int (*full_mono_src_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, igd_mono_src_t *src,
		unsigned int byte_mask, igd_pat_t *pat, unsigned int raster_ops,
		igd_appcontext_h appcontext, unsigned int flags);
	int (*full_mono_pat_blt)(igd_display_h display_h, int priority,
		igd_surface_t *dest, igd_rect_t *dest_rect, igd_coord_t *src_coord,
		igd_surface_t *src,	unsigned int byte_mask, igd_mono_pat_t *pat,
		unsigned int raster_ops, igd_appcontext_h appcontext,
		unsigned int flags);
	int (*full_mono_pat_mono_src_blt)(igd_display_h display_h,
		int priority, igd_surface_t *dest, igd_rect_t *dest_rect,
		igd_mono_src_t *src, unsigned int byte_mask, igd_mono_pat_t *pat,
		unsigned int raster_ops, igd_appcontext_h appcontext,
		unsigned int flags);
	int (*text_immed_blt)(igd_display_h display_h,
		int priority, igd_surface_t *dest_surf,
		igd_rect_t *dest_rect,
		unsigned char *glyph_data, unsigned int num_glyph_bytes,
		igd_appcontext_h appcontext_handle, unsigned long raster_ops,
		unsigned long bg_color,	unsigned long fg_color,
		unsigned int flags);

	/* igd_blend.h */
	/*!
	 * Blend and stretch and color convert a stack of input surfaces into a
	 * destination surface.
	 *
	 * Blend takes N input surfaces and N corresponding input rectangles
	 * and output rectangles. This allows for any subset of an input surface
	 * to be output to any rectangle on the destination surface. The input
	 * and output rectangles need not have any correlation between inputs.
	 * Also, the input and output rectangle for any given surface need not
	 * be the same size or aspect ratio, full up and down scaling is possible.
	 * The destination surface is provided with a clip rectangle. All
	 * rendering will be clipped to this rectangle regardless of the
	 * provided destination rectangles.
	 * Each input surface has a set of render_ops that will be respected
	 * during the blend operation.
	 * The IGD_RENDER_OP_BLEND render operation will allow the surface to
	 * blend with the surfaces below it in the stack. The bottom surface
	 * in the stack will then optionally blend with the existing contents
	 * of the destination surface.
	 *
	 * All input surface must be allocated with the IGD_SURFACE_TEXTURE
	 * flag and all output surfaces must be allocated with the
	 * IGD_SURFACE_RENDER flag.
	 *
	 * When a stretch blit is performed (No blending) the surface should not
	 * have the IGD_RENDER_OP_BLEND bit set in the render_ops to insure
	 * that the fastest possible method is used and that no format
	 * conversion takes place.
	 *
	 * Due to the number of possible permutations of this API it is
	 * necessary to constrain the parameters that can be expected to work.
	 * Specific hardware implementations may support more, however there is
	 * no guarentee of functionality beyond those listed here.
	 *
	 * Blend will accept 1 or 2 input surfaces.
	 * Blend will accept a maximum of 1 input surface with a palette.
	 * Blend will accept a maximum of 1 input surface in planar format.
	 * All Surfaces can output to ARGB format.
	 * Only YUV inputs may output to YUV format.
	 * xRGB formats will output ARGB with Alpha of 1.0 when
	 *  IGD_RENDER_OP_BLEND is set on the surface. Otherwise the x will
	 *  be retained from the original source.
	 * Surfaces that are not pre-multipled cannot have a global alpha.
	 *
	 */
	int (*blend)(igd_display_h display, int priority,
		igd_appcontext_h appcontext,
		igd_surface_t *src_surface, igd_rect_t *src_rect,
		igd_surface_t *mask_surface, igd_rect_t *mask_rect,
		igd_rect_t *dest_rect, igd_surface_t *dest_surface,
		igd_rect_t *clip_rect, unsigned long flags);

	/* igd_ovl.h */
	/*!
	 * Alter the overlay associated with the given display_h.
	 *  Only 1 video surface can be associated with a given display_h
	 *  (i.e. you can not have 2 video surfaces using the same display).
	 *  This function should be called once for each unique framebuffer.
	 *  Therefore, single, twin, clone, and vertical extended  modes
	 *  should call this function once, and the HAL will properly display
	 *  video using the primary overlay and second overlay if necessary.
	 *  Whereas extended should call this function twice (once for each
	 *  display_h) if the video spans multiple displays.
	 *  In DIH, this function can be called once for each display, since a
	 *  video surface can not span displays in DIH.
	 *
	 * The primary display in clone, the primary display in vertical extended,
	 *  and the primary display in extended will always use the primary
	 *  overlay.  The secondary display in clone, the secondary display in
	 *  vertical extended, and the secondary display in extended will always
	 *  use the secondary overlay.  The hardware overlay resources (excluding
	 *  any video memory) will be allocated internal to the HAL during the
	 *  _igd_dispatch::alter_displays() call.  Video memory surfaces required
	 *  internal to the HAL, for stretching or pixel format conversions, will
	 *  be dynamically allocated and freed as necessary.
	 *
	 * @param display_h Input display used with this overlay
	 * @param appcontext_h Input appcontext which may be used for blend.
	 *     This can be the same appcontext which is used for blend and
	 *     2d (DD and XAA/EXA).
	 * @param src_surf Input src surface information
	 * @param src_rect Input src surface rectangle.
	 *     This is useful for clone, extended, and vertical extended modes
	 *     where the entire src_surf may not be displayed.
	 * @param dest_rect Input dest surface rectangle.  This is relative to the
	 *     framebuffer not the display (so clone mode works properly).
	 * @param ovl_info Input overlay information to display.
	 *     The color key, video quality, and gamma must be valid
	 *     and correct when the overlay is on.  That means NO passing in NULL
	 *     values to use previous settings.
	 * @param flags Input to turn on/off the overlay and set other flags.
	 *   See: @ref alter_ovl_flags
	 *
	 * @returns 0 (IGD_SUCCESS) on Success
	 * @returns <0 (-igd_errno) on Error.  The overlay will be off, no need
	 *     for the IAL to call the HAL to turn the overlay off.
	 *     If -IGD_ERROR_INVAL is returned, something is either to big or
	 *      to small for the overlay to handle, or it is panned off of the
	 *      displayed.  This is likely not critical, since it may be stretched
	 *      or panned back, so the overlay can support it.  The IAL
	 *      should not return an error to the application, or else the
	 *      application will likely exit.
	 *     If -IGD_ERROR_HWERROR is returned, something is outside of what
	 *      the overlay can support (pitch to large, dot clock to large,
	 *      invalid pixel format, ring or flip not happening).  In this case,
	 *      the IAL should return an error to the application, since
	 *      additional calls will always fail as well.
	 */
	int (*alter_ovl)(igd_display_h display_h,
		igd_appcontext_h     appcontext_h,
		igd_surface_t       *src_surf,
		igd_rect_t          *src_rect,
		igd_rect_t          *dest_rect,
		igd_ovl_info_t      *ovl_info,
		unsigned int         flags);

	int (*alter_ovl2)(igd_display_h display_h,
		igd_surface_t       *src_surf,
		igd_rect_t          *src_rect,
		igd_rect_t          *dest_rect,
		igd_ovl_info_t      *ovl_info,
		unsigned int         flags);

/*	int (*alter_ovl2_dihclone)(igd_display_h display_h,
		igd_surface_t       *src_surf,
		igd_rect_t          *src_rect,
		igd_rect_t          *dest_rect,
		igd_ovl_info_t      *ovl_info,
		unsigned int         flags);*/


	/* igd_ovl.h */
	/*!
	 *  Retrieve the kernel mode initialization parameters for overlay.
	 *  This function should be called by user-mode drivers as they are
	 *  initialized, to retrieve overlay initialization parameters
	 *  discovered and held by the kernel driver.
	 *
	 * @returns 0 (IGD_SUCCESS) on Success
	 * @returns <0 (-igd_errno) on Error.
	 */
	int (*get_ovl_init_params)(igd_driver_h driver_handle,
				   ovl_um_context_t *ovl_um_context);

	/*!
	 * Query the overlay to determine if an event is complete or if a given
	 * capability is present.
	 *
	 * @param display_h Display used with this overlay
	 * @param flags Used to check for which event is complete or which
	 *     capability is present.
	 *     See @ref query_ovl_flags
	 *
	 * @returns TRUE if event has occured or capability is available
	 * @returns FALSE if event is pending or capability is not available
	 */
	int (*query_ovl)(igd_display_h display_h,
		unsigned int flags);

	/* User mode only query_ovl */
	int (*query_ovl2)(igd_display_h display_h,
		unsigned int flags);
	/*!
	 * Query the overlay for the maximum width and height given the input
	 * src video pixel format.
	 *
	 * @param display_h Display used with this overlay
	 * @param pf Input src video pixel format
	 * @param max_width Output maximum overlay width supported (in pixels)
	 * @param max_height Output maximum overlay height supported (in pixels)
	 *
	 * @returns 0 (IGD_SUCCESS) on Success - will return success
	 *    even if overlay is currently in use.
	 * @returns <0 (-igd_errno) on Error
	 */
	int (*query_max_size_ovl)(igd_display_h display_h,
		unsigned long pf,
		unsigned int *max_width,
		unsigned int *max_height);

	/*!
	 * Alter the sprite C plane with the associated osd/subpicture data
	 */
    int (*alter_ovl_osd)(igd_display_h display_h,
        igd_appcontext_h     appcontext_h,
        igd_surface_t *sub_surface,
        igd_rect_t *sub_src_rect,
        igd_rect_t *sub_dest_rect,
        igd_ovl_info_t      *ovl_info,
        unsigned int         flags);

	int (*alter_ovl2_osd)(igd_display_h display_h,
		igd_surface_t *sub_surface,
		igd_rect_t *sub_src_rect,
		igd_rect_t *sub_dest_rect,
		igd_ovl_info_t      *ovl_info,
		unsigned int         flags);

	/* igd_render.h */
	_igd_get_surface_fn_t get_surface;
	_igd_set_surface_fn_t set_surface;
	_igd_query_event_fn_t query_event;

	/* These functions are only to be used by priveledged IALs */
	_igd_alloc_ring_fn_t alloc_ring;
	_igd_exec_buffer_fn_t execute_buffer;
	_igd_rb_reserve_fn_t rb_reserve;
	_igd_rb_update_fn_t rb_update;
	_igd_get_sync_slot_fn_t get_sync_slot;
	_igd_query_buffer_fn_t query_buffer;

	/*!
	 *  dispatch->get_display() returns the current framebuffer and
	 *  display information.
	 *
	 * @param display_handle required.  The display_handle contains the
	 *  display information to return.  This parameter was returned from a
	 *  previous call to dispatch->alter_displays().
	 *
	 * @param port_number required. The port number will determine which
	 *  port's display info data to return.
	 *
	 * @param fb_info required and allocated by the caller.  The fb_info
	 *  struct is returned to the caller describing the current
	 *  frame buffer.
	 *
	 * @param pt_info required and allocated by the caller.  The
	 *  pt_info struct is returned to caller describing the
	 *  requested display parameters.
	 *
	 * @param flags - Currently not used
	 *
	 * @returns 0 The mode was successfully returned for all displays.
	 * @returns -IGD_INVAL: Was not able to return the display information.
	 */
	int (*get_display)(igd_display_h display_handle,
			unsigned short port_number, igd_framebuffer_info_t *fb_info,
			igd_display_info_t *pt_info, unsigned long flags);

#ifdef D3D_DPM_ALLOC
   /* FIXEME: Somehow this D3D_DPM_ALLOC is always invisible to display dll.
    * To avoid the whole structure corruption, the following delcaration
    * appended at the very end.
    */
   unsigned long* (*gmm_map_sgx)(unsigned long size, unsigned long offset,
      unsigned long flag);
#endif
	int (*get_golden_htotal)(igd_display_info_t *drm_mode_in, igd_display_info_t *drm_mode_out  );

	/*!
	 *  Registers a VBlank interrupt callback function (and its parameter) to
	 *  call when a VBlank interrupt occurs for a given port.
	 *
	 * @param callback (IN).  A callback (function pointer) to a non-HAL
	 * function that processes a VBlank interrupt.
	 *
	 * @param priv (IN).  An opaque pointer to a non-HAL data structure.
	 *  This pointer is passed as a parameter of the callback function.
	 *
	 * @param port_number (IN).  The EMGD port number to register a VBlank
	 *  interrupt callback for.
	 *
	 * @return A handle that uniquely identifies this callback/port
	 *  combination, or NULL if a failure.
	 */
	emgd_vblank_callback_h (*register_vblank_callback)(
		emgd_process_vblank_interrupt_t callback,
		void *priv,
		unsigned long port_number);

	/*!
	 *  Unregisters a previously-registered VBlank interrupt callback function
	 *  for a given port.
	 *
	 * @param callback_h (IN).  The handle that uniquely identifies the VBlank
	 *  interrupt callback to unregister.
	 */
	void (*unregister_vblank_callback)(
		emgd_vblank_callback_h callback_h);

	/*!
	 *  Enable delivering VBlank interrupts to the callback function for the
	 *  registered callback/port combination.
	 *
	 * @param callback_h (IN).  The handle that uniquely identifies which
	 *  VBlank interrupt callback/port combination to enable.
	 *
	 * @return Zero if successful, non-zero if a failure.
	 */
	int (*enable_vblank_callback)(emgd_vblank_callback_h callback_h);

	/*!
	 *  Disable delivering VBlank interrupts to the callback function for the
	 *  registered function/port combination.
	 *
	 * @param callback_h (IN).  The handle that uniquely identifies which
	 *  VBlank interrupt callback/port combination to disable.
	 */
	void (*disable_vblank_callback)(
		emgd_vblank_callback_h callback_h);

	/*!
	 * Shutdown MSVDX before the X exits.
	 */
	void (*video_shutdown)(void);


	/*!
	 * Query the hardware on for given 2D Caps.
	 */ 
	void (*query_2d_caps_hwhint)(unsigned long caps_val,
		unsigned long *status);

	int (*dihclone_set_surface)(
	unsigned long display_number,
	unsigned long mode);

	/*!
	 *  Allows a user mode application to change pixel format of a display
	 *  plane from XRGB to ARGB and vice versa.
	 *
	 * @param enable (IN). Whether to turn on transparency or not with the
	 *  assumption that XRGB turns off transparency and ARGB turns it on.
	 *  0 = disable
	 *  1 = enable
	 * @param display_plane (IN). Which display plane to change pixel_format of
	 *  0 = Plane A
	 *  1 = Plane B
	 */
	int (*control_plane_format)(int enable, igd_display_h display_handle);

	/*!
	 * Overlay Plane assignment override.
	 */ 
	int (*set_ovl_display)(igd_display_h ovl_displays[]);

/*!
	 * Get MSVDX status.
	 */ 
	int (*msvdx_status)(igd_driver_h driver_handle, unsigned long *queue_status, unsigned long *mtx_msg_status);
	/* show_desktop calls this function so, the planes now show the desktop instead of the splas screen */
	int (*unlock_planes)(igd_display_h display_handle , unsigned int scrn_num);
} igd_dispatch_t;

#endif
