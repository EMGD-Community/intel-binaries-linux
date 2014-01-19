/*-----------------------------------------------------------------------------
* Copyright (c) Chrontel Inc.
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
* @file  config_.h
* @version 1.2.3
*-----------------------------------------------------------------------------
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_


#define T_LVDS_OPEN
#define T_LVDS_INIT
#define T_POST_SETMODE
#define T_LVDS_CLOSE

#define T_SET_TIMING

#define T_PD_DISPLAY_LVDS_INT 

#define T_DEVICE_DETECTION 
#define T_RES_DEALLOC

#define T_TEST_HW
#define T_RESET


#define T_HDMI_TIMING_TABLE
#define T_VGA_TIMING_TABLE
#define T_PANEL_NATIVE_DTD


#define T_LH_POWER_ON_OFF

#define T_CH7036_USE_LVDS_ATTR_TABLE

#define T_CH7036_INIT_ATTR_TABLE


#define T_CH7036_USE_FIRMWARE

#define T_CH7036_EDID_DUMP
#undef T_CH7036_EDID_DUMP

#define T_SHOW_EDID_DISPLAY_ATTR
#undef T_SHOW_EDID_DISPLAY_ATTR

#define T_LINUX
//#undef T_LINUX

#define T_CONFIG_PLB

#endif
