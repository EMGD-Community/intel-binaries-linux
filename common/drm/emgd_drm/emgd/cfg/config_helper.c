/*
 *-----------------------------------------------------------------------------
 * Filename: config_helper.c
 * $Revision: 1.8 $
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

#include <stdio.h>

#include <config.h>

int main(int argc, char **argv)
{
	char configs[] = ""
#ifdef CONFIG_MICRO
		"MICRO "
#else
		"FULL "
#endif
#ifdef CONFIG_WHT
		"WHT "
#endif
#ifdef CONFIG_ALM
		"ALM "
#endif
#ifdef CONFIG_NAP
		"NAP "
#endif
#ifdef CONFIG_GN4
		"GN4 "
#endif
#ifdef CONFIG_PLB
		"PLB "
#endif
#ifdef CONFIG_TNC
		"TNC "
#endif
#ifdef CONFIG_MODE
		"MODE "
#endif
#ifdef CONFIG_DSP
		"DSP "
#endif
#ifdef CONFIG_PI
		"PI "
#endif
#ifdef CONFIG_PD
		"PD "
#endif
#ifdef CONFIG_INIT
		"INIT "
#endif
#ifdef CONFIG_INTERRUPT
		"INTERRUPT "
#endif
#ifdef CONFIG_GART
		"GART "
#endif
#ifdef CONFIG_REG
		"REG "
#endif
#ifdef CONFIG_RESET
		"RESET "
#endif
#ifdef CONFIG_POWER
		"POWER "
#endif
#ifdef CONFIG_GMM
		"GMM "
#endif
#ifdef CONFIG_MICRO_GMM
		"MICRO_GMM "
#endif
#ifdef CONFIG_APPCONTEXT
		"APPCONTEXT "
#endif
#ifdef CONFIG_CMD
		"CMD "
#endif
#ifdef CONFIG_2D
		"2D "
#endif
#ifdef CONFIG_BLEND
		"BLEND "
#endif
#ifdef CONFIG_OVERLAY
		"OVERLAY "
#endif
#ifdef CONFIG_DECODE
		"DECODE "
#endif
		/*
		 * Port Driver Compile Options
		 */
#ifdef CONFIG_PD_ANALOG
		"PD_ANALOG "
#endif
#ifdef CONFIG_PD_SII164
		"PD_SII164 "
#endif
#ifdef CONFIG_PD_CH7009
		"PD_CH7009 "
#endif
#ifdef CONFIG_PD_TL955
		"PD_TL955 "
#endif
#ifdef CONFIG_PD_RGBA
		"PD_RGBA "
#endif
#ifdef CONFIG_PD_NS2501
		"PD_NS2501 "
#endif
#ifdef CONFIG_PD_TH164
		"PD_TH164 "
#endif
#ifdef CONFIG_PD_FS454
		"PD_FS454 "
#endif
#ifdef CONFIG_PD_NS387
		"PD_NS387 "
#endif
#ifdef CONFIG_PD_CX873
		"PD_CX873 "
#endif
#ifdef CONFIG_PD_LVDS
		"PD_LVDS "
#endif
#ifdef CONFIG_PD_FS460
		"PD_FS460 "
#endif
#ifdef CONFIG_PD_CH7017
		"PD_CH7017 "
#endif
#ifdef CONFIG_PD_TI410
		"PD_TI410 "
#endif
#ifdef CONFIG_PD_TV
		"PD_TV "
#endif
#ifdef CONFIG_PD_HDMI
		"PD_HDMI "
#endif
#ifdef CONFIG_PD_SDVO
		"PD_SDVO "
#endif
#ifdef CONFIG_PD_SOFTPD
		"PD_SOFTPD "
#endif
#ifdef CONFIG_PD_CH7036
		"PD_CH7036 "
#endif	
		/*
		 * Port Driver Link Options
		 */
#ifdef CONFIG_LINK_PD_ANALOG
		"LINK_PD_ANALOG "
#endif
#ifdef CONFIG_LINK_PD_SII164
		"LINK_PD_SII164 "
#endif
#ifdef CONFIG_LINK_PD_CH7009
		"LINK_PD_CH7009 "
#endif
#ifdef CONFIG_LINK_PD_TL955
		"LINK_PD_TL955 "
#endif
#ifdef CONFIG_LINK_PD_RGBA
		"LINK_PD_RGBA "
#endif
#ifdef CONFIG_LINK_PD_NS2501
		"LINK_PD_NS2501 "
#endif
#ifdef CONFIG_LINK_PD_TH164
		"LINK_PD_TH164 "
#endif
#ifdef CONFIG_LINK_PD_FS454
		"LINK_PD_FS454 "
#endif
#ifdef CONFIG_LINK_PD_NS387
		"LINK_PD_NS387 "
#endif
#ifdef CONFIG_LINK_PD_CX873
		"LINK_PD_CX873 "
#endif
#ifdef CONFIG_LINK_PD_LVDS
		"LINK_PD_LVDS "
#endif
#ifdef CONFIG_LINK_PD_FS460
		"LINK_PD_FS460 "
#endif
#ifdef CONFIG_LINK_PD_CH7017
		"LINK_PD_CH7017 "
#endif
#ifdef CONFIG_LINK_PD_TI410
		"LINK_PD_TI410 "
#endif
#ifdef CONFIG_LINK_PD_TV
		"LINK_PD_TV "
#endif
#ifdef CONFIG_LINK_PD_HDMI
		"LINK_PD_HDMI "
#endif
#ifdef CONFIG_LINK_PD_SDVO
		"LINK_PD_SDVO "
#endif
#ifdef CONFIG_LINK_PD_SOFTPD
		"LINK_PD_SOFTPD "
#endif
#ifdef CONFIG_LINK_PD_CH7036
      "LINK_PD_CH7036 "
#endif

#ifdef CONFIG_COPP
		"COPP "
#endif
		;
	printf("%s\n", configs);
	return 0;
}

