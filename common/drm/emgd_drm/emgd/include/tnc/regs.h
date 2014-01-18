/*
 *-----------------------------------------------------------------------------
 * Filename: regs.h
 * $Revision: 1.16 $
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
 *  This is the register definition file for the Atom E6xx platform. This should
 *  contain device dependent register definitions. Standard register
 *  definitions (VGA, PCI, etc) should not be put in this file. For those
 *  see pci.h or vga.h.
 *-----------------------------------------------------------------------------
 */

/*
 * Note: Use _REGS_H_  instead of _TNC_REGS_H_ to insure that no file can
 * include 2 device dependent register files.
 */
#ifndef _REGS_H_
#define _REGS_H_

#include <pci.h>
#include <psb_regs.h>
#include <msvdx.h>
#include <topaz.h>

/*-----------------------------------------------------------------------------
 * PCI Register Definitions
 *---------------------------------------------------------------------------*/
#define TNC_PCI_MMADR      PCI_BAR_0
#define TNC_PCI_IOBAR      PCI_BAR_1
#define TNC_PCI_GMADR      PCI_BAR_2
#define TNC_PCI_GTTADR     PCI_BAR_3

/*
 * GVD.MGGC
 * Device: 2 Offset:50h
 * [22:20] Graphics mode select
 * [17] VGA Disable
 */
#define TNC_PCI_GC         0x52
#define TNC_PCI_BSM        0x5C

/* Atom E6xx Device 31 GPIO base address offset */
#define TNC_PCI_GBA			0x44

#define TNC_D2_MMIO_SIZE  (1024*1024)
#define TNC_D3_MMIO_SIZE  (512*1024)
#define TNC_ST_SDVO_MMIO_SIZE  (10*1024)

#define TNC_GTT_SIZE   (128*1024)



/*-----------------------------------------------------------------------------
 * General VGA Register Definitions
 *---------------------------------------------------------------------------*/
#define FEATURE_CONT_REG      0x3DA  /* Feature Control Register */
#define FEATURE_CONT_REG_MONO 0x3BA  /* Feature Control Register (Mono) */
#define FEATURE_CONT_REG_READ 0x3CA  /* Feature Control Register (Read) */
#define MSR_PORT              0x3C2  /* Miscellaneous Output Port */
#define MSR_PORT_LSB          0xC2   /* Miscellaneous Output Port LSB */
#define MSR_READ_PORT         0x3CC  /* Miscellaneous Output Reg (Read) */
#define STATUS_REG_00         0x3C2  /* Input Status Register 0 */
#define STATUS_REG_01         0x3DA  /* Input Status Register 1 */
#define STATUS_REG_01_MONO    0x3BA  /* Input Status Register 1 (Mono) */

/* DAC Register Definitions. */
#define DAC_PEL_MASK          0x3C6  /* Color Palette Pixel Mask Register */
#define DAC_READ_INDEX        0x3C7  /* Color Palette Read-Mode Index Reg */
#define DAC_STATE             0x3C7  /* Color Palette State Register */
#define DAC_WRITE_INDEX       0x3C8  /* Color Palette Index Register */
#define DAC_DATA_REG          0x3C9  /* Color Palette Data Register */


/*-----------------------------------------------------------------------------
 * Attribute Controller Register Definitions
 *---------------------------------------------------------------------------*/
#define AR_PORT_LSB  0xC0  /*Attribute Controller Index Port LSB */

#define AR00  0x00  /* Color Data Register */
#define AR01  0x01	/* Color Data Register */
#define AR02  0x02	/* Color Data Register */
#define AR03  0x03	/* Color Data Register */
#define AR04  0x04	/* Color Data Register */
#define AR05  0x05	/* Color Data Register */
#define AR06  0x06	/* Color Data Register */
#define AR07  0x07	/* Color Data Register */
#define AR08  0x08	/* Color Data Register */
#define AR09  0x09	/* Color Data Register */
#define AR0A  0x0A	/* Color Data Register */
#define AR0B  0x0B	/* Color Data Register */
#define AR0C  0x0C	/* Color Data Register */
#define AR0D  0x0D	/* Color Data Register */
#define AR0E  0x0E	/* Color Data Register */
#define AR0F  0x0F	/* Color Data Register */
#define AR10  0x10	/* Mode Control Register */
#define AR11  0x11	/* Overscan Color Register */
#define AR12  0x12	/* Color Plane Enable Register */
#define AR13  0x13	/* Horizontal Pixel Panning Register */
#define AR14  0x14	/* Pixel Pad Register */


/*-----------------------------------------------------------------------------
 * CRT Controller Register Definitions
 *---------------------------------------------------------------------------*/
#define CR_PORT_LSB     0xD4  /* CRT Controller Index Port LSB */
#define CRT_3D4	        0x3D4 /* Color CRTC Index Port */
#define CRT_3B4	        0x3B4 /* Monochrome CRTC Index Port */

#define CR00            0x00  /* Horizontal Total Register */
#define CR01            0x01  /* Horizontal Display Enable End Reg */
#define CR02            0x02  /* Horizontal Blank Start Register */
#define CR03            0x03  /* Horizontal Blank End Register */
#define CR04            0x04  /* Horizontal Sync Start Register */
#define CR05            0x05  /* Horizontal Sync End Register */
#define CR06            0x06  /* Vertical Total Register */
#define CR07            0x07  /* Overflow Register */
#define CR08            0x08  /* Preset Row Scan Register */
#define CR09            0x09  /* Maximum Scan Line Register */
#define DOUBLE_SCANLINE	BIT7  /* Double scan ( 1 = Enable ) */
#define LCOMP_BIT9      BIT6  /* Bit 9 of line compare register */
#define VBLANK_BIT9     BIT5  /* Bit 9 of vertical blank start */
#define CR0A            0x0A  /* Cursor Start Scan Line Register */
#define CR0B            0x0B  /* Cursor End Scan Line Register */
#define CR0C            0x0C  /* Start Address High Register */
#define CR0D            0x0D  /* Start Address Low Register */
#define CR0E            0x0E  /* Cursor Location High Register */
#define CR0F            0x0F  /* Cursor Location Low Register */
#define CR10            0x10  /* Vertical Sync Start Register */
#define CR11            0x11  /* Vertical Sync End Register */
#define CR12            0x12  /* Vertical Display Enable End Reg */
#define CR13            0x13  /* Offset Register */
#define CR14            0x14  /* Underline Row Register */
#define CR15            0x15  /* Vertical Blank Start Register */
#define CR16            0x16  /* Vertical Blank End Register */
#define CR17            0x17  /* CRT Mode Control Register */
#define CR18            0x18  /* Line Compare Register */
#define CR22            0x22  /* Memory Data Latches Register */
#define CR24            0x24  /* Attribute Controller Toggle Reg */


/*-----------------------------------------------------------------------------
 * Graphics Controller Register Definitions
 *---------------------------------------------------------------------------*/
#define GR_PORT_LSB    0xCE   /* Graphics Controller Index Port LSB */

#define GR00           0x00   /* Set/Reset Register */
#define GR01           0x01   /* Enable Set/Reset Register */
#define GR02           0x02   /* Color Compare Register */
#define GR03           0x03   /* Data Rotate Register */
#define GR04           0x04   /* Read Map Select Register */
#define GR05           0x05   /* Graphics Mode Register */
#define GR06           0x06   /* Micsellaneous Register */
#define RANGE_MAP_MASK BIT3 + BIT2  /* Address range to map mask */
#define A0_BF_RANGE    000h   /* Map to A0000h-BFFFFh range */
#define GRAF_MODE      BIT0   /* 1 = Grahics mode, 0 = Text mode */
#define GR07           0x07   /* Color Don't Care Register */
#define GR08           0x08   /* Bit Mask Register */
#define GR10           0x10   /* Address Mapping */
#define PAGING_TARGET  BIT2 + BIT1 /* 00 = Local/Stolen, 01 = Mem mapped regs */
#define PAGE_MODE      BIT0   /* Page Map allow access to all FB mem */
#define GR11           0x11   /* Page Selector */
#define	GR18           0x18   /* Software Flag */


/*-----------------------------------------------------------------------------
 * Sequencer Register Definitions
 *---------------------------------------------------------------------------*/
#define SR_PORT_DATA      0x3C5 /* Sequencer Data Port */
#define SR_PORT_LSB       0xC4  /* Sequencer Index Port LSB */

#define SR00              0x00  /* Reset Register */
#define SR01              0x01  /* Clocking Mode Register */
#define DOT_CLOCK_DIVIDE  BIT3	/* Divide pixel clock by 2 */
#define SR02              0x02  /* Plane/Map Mask Register */
#define SR03              0x03  /* Character Font Register */
#define SR04              0x04  /* Memory Mode Register */
#define SR07              0x07  /* Horizontal Character Counter Reset */

/*-----------------------------------------------------------------------------
 * Platform specific PCI Config Register
 *---------------------------------------------------------------------------*/
#define TNC_OFFSET_VGA_MSAC         0x62

/* FIXME: Core Clock should be obtained from the Message Control Register*/
#define INTEL_OFFSET_VGA_CORECLK    0xF0

/* #define INTEL_OFFSET_BRIDGE_CAPREG  0xE0 Not needed for Atom E6xx */

#define PCI_GMS_MASK      BIT6 + BIT5 + BIT4 /* GFX Mode Select Bits Mask */
#define PCI_LOCAL		  BIT4 /* Local memory enabled */
#define PCI_DVMT_512K     BIT5 /* 512KB DVMT */
#define PCI_DVMT_1M       BIT5 + BIT4 /* 1MB DVMT */
#define PCI_DVMT_8M       BIT6 /* 8MB DVMT */

#define PCI_DRB_REG       0x60 /* DRAM row boundary Register */
#define PCI_DRC_REG       0x7C /* DRAM Controller Mode Register */
#define PCI_DT_MASK       BIT0 + BIT1  /* Select SDRAM types.
                                        *  = 00:  Single data rate SDRAM
                                        *  = 01:  Dual data rate SDRAM
                                        *  = Other:  Reserved
                                        */
#define DT_SDR_SDRAM      00   /* Single data rate SDRAM */
#define DT_DDR_SDRAM      01   /* Dual data rate SDRAM */

#define PCI_ESMRAMC_REG   0x91 /* Extended System Management RAM Reg */
#define PCI_TSEG_SZ		  BIT1 /* TSEG size bit */
#define PCI_TSEG_512K	  0    /* 512K TSEG */
#define PCI_TSEG_1M       BIT1 /* 1MB TSEG */

#define PCI_GCLKIO_REG    0xC0 /* GMCH Clock and IO Control Register */
#define PCI_AGP_Bit       BIT9 /* AGP/DVO Mux Select:
								*  = 0, DVO/ZV
								*  = 1, AGP
								*/
#define PCI_GMCHCFG_REG   0xC6 /* GMCH Configuration Register */
#define PCI_SMFREQ_MASK   BIT10 + BIT11
						   /* System Mem Frequency Select
                            * = 00:  Intel Reserved
                            * = 01:  System Memory Frequency is 166Mhz (DDR333) - Intel Reserved
                            * = 10:  System Memory Frequency is 133Mhz (SDR133, DDR266)
                            * = 11:  System Memory Frequency is 100Mhz (DDR200)
                            */
#define SYS_MEM_FREQ_166  1  /* System Memory Frequency is 166Mhz */
#define SYS_MEM_FREQ_133  2  /* System Memory Frequency is 133Mhz */
#define SYS_MEM_FREQ_100  3  /* System Memory Frequency is 100Mhz */
#define PCI_SMFREQ_POS    10 /* System Memory Frequency position  */

#define PCI_CONFIG_LMINT  0xE0
#define PREALLOCATED_SIZE (8 * 1024 * 1024)


/*-----------------------------------------------------------------------------
 * General VGA Register Definitions
 *---------------------------------------------------------------------------*/
#define FEATURE_CONT_REG      0x3DA  /* Feature Control Register */
#define FEATURE_CONT_REG_MONO 0x3BA  /* Feature Control Register (Mono) */
#define FEATURE_CONT_REG_READ 0x3CA  /* Feature Control Register (Read) */
#define MSR_PORT              0x3C2  /* Miscellaneous Output Port */
#define MSR_PORT_LSB          0xC2   /* Miscellaneous Output Port LSB */
#define MSR_READ_PORT         0x3CC  /* Miscellaneous Output Reg (Read) */
#define STATUS_REG_00         0x3C2  /* Input Status Register 0 */
#define STATUS_REG_01         0x3DA  /* Input Status Register 1 */
#define STATUS_REG_01_MONO    0x3BA  /* Input Status Register 1 (Mono) */

/* DAC Register Definitions. */
#define DAC_PEL_MASK          0x3C6  /* Color Palette Pixel Mask Register */
#define DAC_READ_INDEX        0x3C7  /* Color Palette Read-Mode Index Reg */
#define DAC_STATE             0x3C7  /* Color Palette State Register */
#define DAC_WRITE_INDEX       0x3C8  /* Color Palette Index Register */
#define DAC_DATA_REG          0x3C9  /* Color Palette Data Register */


/*-----------------------------------------------------------------------------
 * Memory mapped I/O Registers Definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Instruction and Interrupt Control Registers (01000h - 02FFFh)
 *---------------------------------------------------------------------------*/
#define PGTBL_CTL         0x02020  /* Page Table Control Register */
#define HWS_PGA           0x02080  /* Hardware Status Page Address register */
#define HWSTAM            0x02098  /* Hardware Status Mask */
#define SCPD0             0x0209C  /* Scratch Pad 0 (Debug) */
#define IER               0x020A0  /* Interrupt Enable */
#define IIR               0x020A4  /* Interrupt Identity */
#define IMR               0x020A8  /* Interrupt Mask */
#define ISR               0x020AC  /* Interrupt Status */
#define EIR               0x020B0  /* Error Identity */
#define EMR               0x020B4  /* Error Mask */
#define ESR               0x020B8  /* Error Status */
#define FW_BLC_SELF       0x020E0  /* Display FIFO Watermark */
#define MI_ARB_STATE      0x020E4  /* Memory Interface Arbitration State */
#define G_DEBUG		  0x020FC  /* G-UNIT Debug enable register */

#if 0
/*-----------------------------------------------------------------------------
 * FENCE and Per Process GTT Control Registers (02000h - 031FFh)
 * --------------------------------------------------------------------------*/
#define FENCE0            0x02000 /* Fence table registers */
#define FENCE1            0x02004
#define FENCE2            0x02008
#define FENCE3            0x0200C
#define FENCE4            0x02010
#define FENCE5            0x02014
#define FENCE6            0x02018
#define FENCE7            0x0201C
#define FENCE8            0x03000
#define FENCE9            0x03004
#define FENCE10           0x03008
#define FENCE11           0x0300C
#define FENCE12           0x03010
#define FENCE13           0x03014
#define FENCE14           0x03018
#define FENCE15           0x0301C
#endif
/*-----------------------------------------------------------------------------
 * FIXME: For TNC, we were not able to find specs detailing the FENCE registers
 * Use the GN4 FENCE Registers for the time being as a placeholder till we can
 * nail down the FENCE registers
 * --------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * FENCE and Per Process GTT Control Registers (03000h - 031FFh)
 *
 * The FENCE registers are now 64-bits but we can only read/write 32-bits
 * at a time.  As a result, each register has aliases for the whole 64-bits,
 * the low DWORD, and the high DWORD.
 *
 * This is important for restoring the registers, since we must always write
 * the high DWORD first.
 * --------------------------------------------------------------------------*/
#define FENCE0            0x03000 /* Fence table registers */
#define FENCE0l           0x03000
#define FENCE0h           0x03004
#define FENCE1            0x03008
#define FENCE1l           0x03008
#define FENCE1h           0x0300C
#define FENCE2            0x03010
#define FENCE2l           0x03010
#define FENCE2h           0x03014
#define FENCE3            0x03018
#define FENCE3l           0x03018
#define FENCE3h           0x0301C
#define FENCE4            0x03020
#define FENCE4l           0x03020
#define FENCE4h           0x03024
#define FENCE5            0x03028
#define FENCE5l           0x03028
#define FENCE5h           0x0302C
#define FENCE6            0x03030
#define FENCE6l           0x03030
#define FENCE6h           0x03034
#define FENCE7            0x03038
#define FENCE7l           0x03038
#define FENCE7h           0x0303C
#define FENCE8            0x03040
#define FENCE8l           0x03040
#define FENCE8h           0x03044
#define FENCE9            0x03048
#define FENCE9l           0x03048
#define FENCE9h           0x0304C
#define FENCE10           0x03050
#define FENCE10l          0x03050
#define FENCE10h          0x03054
#define FENCE11           0x03058
#define FENCE11l          0x03058
#define FENCE11h          0x0305C
#define FENCE12           0x03060
#define FENCE12l          0x03060
#define FENCE12h          0x03064
#define FENCE13           0x03068
#define FENCE13l          0x03068
#define FENCE13h          0x0306C
#define FENCE14           0x03070
#define FENCE14l          0x03070
#define FENCE14h          0x03074
#define FENCE15           0x03078
#define FENCE15l          0x03078
#define FENCE15h          0x0307C

/*-----------------------------------------------------------------------------
 * MISC I/0 Contol Register ( 05000h - 05FFFh )
 *---------------------------------------------------------------------------*/
#define IO_OFF          0x05000         /* Register group offset */

#define IO00            0x05000         /* Hsync / Vsync control register */
#define GPIO0           0x05010         /* GPIO register 0 (DDC1) */
#define	DDC1_SCL_PIN    GPIO0_SCL_PIN   /* DDC1 SCL GPIO pin # */
#define	DDC1_SDA_PIN    GPIO0_SDA_PIN   /* DDC1 SDA CPIO pin # */
#define GPIO1           0x05014         /* GPIO register 1 (I2C) */
#define	I2C_SCL_PIN     GPIO1_SCL_PIN   /* I2C SCL GPIO pin # */
#define	I2C_SDA_PIN     GPIO1_SDA_PIN   /* I2C SDA CPIO pin # */
#define GPIO2           0x05018         /* GPIO register 2 (DDC2) */
#define	DDC2_SCL_PIN    GPIO2_SCL_PIN   /* DDC2 SCL GPIO pin # */
#define	DDC2_SDA_PIN    GPIO2_SDA_PIN   /* DDC2 SDA CPIO pin # */
#define GPIO3           0x0501C         /* GPIO register 3 (AGP mux DVI DDC) */
#define GPIO4           0x05020         /* GPIO register 4 (AGP mux I2C) */
#define GPIO5           0x05024         /* GPIO register 5 (AGP mux DDC2/I2C) */

#define GPIOPIN0        GPIO0
#define GPIOPIN1        GPIO0+1
#define GPIOPIN2        GPIO1
#define GPIOPIN3        GPIO1+1
#define GPIOPIN4        GPIO2
#define GPIOPIN5        GPIO2+1
#define GPIOPIN6        GPIO3
#define GPIOPIN7        GPIO3+1
#define GPIOPIN8        GPIO4
#define GPIOPIN9        GPIO4+1
#define GPIOPIN10       GPIO5
#define GPIOPIN11       GPIO5+1
#define GPIOPINMAX      12

#define GMBUS0          0x5100 /* GMBUS clock/device select register */
#define GMBUS1          0x5104 /* GMBUS command/status register */
#define GMBUS2          0x5108 /* GMBUS status register */
#define GMBUS3          0x510c /* GMBUS data buffer register */
#define GMBUS4          0x5110 /* GMBUS REQUEST_INUSE register */
#define GMBUS5          0x5120 /* GMBUS 2-Byte Index register */
#define GMBUS6          0x5124 /* GMBUS Clock divider */

/*  GMBUS1 Bits */
#define SW_CLR_INT      BIT31
#define SW_RDY          BIT30
#define ENT             BIT29
#define STO             BIT27
#define ENIDX           BIT26
#define STA             BIT25

/*  GMBUS2 Bits */
#define INUSE           BIT15
#define HW_WAIT         BIT14
#define HW_TMOUT        BIT13
#define HW_INT          BIT12
#define HW_RDY          BIT11
#define HW_BUS_ERR      BIT10
#define GA              BIT9

/*-----------------------------------------------------------------------------
 * Clock Control and PM Register ( 06000h - 06FFFh )
 *---------------------------------------------------------------------------*/
#define VGA0_DIVISOR    0x06000  /* VGA 0  Divisor */
#define VGA1_DIVISOR    0x06004  /* VGA 1 Divisor */
#define VGA_PD          0x06010  /* VGA Post Divisor Select */
#define DPLLAMD         0x0601C  /* Display PLL A SDVO Multiplier/Divisor */
#define DPLLBMD         0x06020  /* Display PLL B SDVO Multiplier/Divisor */

/*-----------------------------------------------------------------------------
 * DPLL A Control Register ( 0F000h - 0FFFFh )
 *---------------------------------------------------------------------------*/
#define DPLLACNTR       0x0F014  /* Display PLL A Control */
#define FPA0            0x0F040  /* DPLL A Divisor 0 */
#define FPA1            0x0F044  /* DPLL A Divisor 1 */
#define DPLL_TEST		0x0F06C  /* DPLLA Test Register */

/*-----------------------------------------------------------------------------
 * DPLL B Control Register ( 06000h - 06FFFh )
 *---------------------------------------------------------------------------*/
#define DPLLBCNTR       0x06018  /* Display PLL B Control */
#define FPB0            0x06048  /* DPLL B Divisor 0 */
#define FPB1            0x0604C  /* DPLL B Divisor 1 */
#define TNC_DPLL_TEST	0x0606C	 /* DPLL A and DPLL B test register */

#define DREFCLK         0x0
#define TVCLKINBC       0x4000
#define CLOCK_2X        0x40000000

#define P2D_CG_DIS      0x06200  /* Clock Gating Disable */
/* Not available in Atom E6xx */
/* #define P3D_CG_DIS      0x06204 */ /* Clock Gating Disable */


/*-----------------------------------------------------------------------------
 * Display Palette Register Definitions (0A000h - 0AFFFh)
 *---------------------------------------------------------------------------*/
#define DPALETTE_A      0x0A000  /* Display Pipe A Palette */
#define DPALETTE_B      0x0A800  /* Display Pipe B Palette */


/*-----------------------------------------------------------------------------
 * Display Pipeline / Port Register Definitions (60000h - 6FFFFh)
 *---------------------------------------------------------------------------*/
#define DP_OFFSET       0x60000  /* register group offset */
#define PIPEA_TIMINGS   0x60000
#define HTOTAL_A        0x60000  /* Pipe A Horizontal Total Register */
#define ACTIVE_DISPLAY  0x7FF    /* bit [ 10:0 ] */
#define HBLANK_A        0x60004  /* Pipe A Horizontal Blank Register */
#define HSYNC_A         0x60008  /* Pipe A Horizontal Sync Register */
#define VTOTAL_A        0x6000C  /* Pipe A Vertical Total Register */
#define VBLANK_A        0x60010  /* Pipe A Vertical Blank Register */
#define VSYNC_A         0x60014  /* Pipe A Vertical Sync Register */
#define PIPEASRC        0x6001C  /* Pipe A Source Image Size Register */
#define BCLRPAT_A       0x60020  /* Pipe A Border Color Pattern Register */
#define CRCCTRLREDA     0x60050  /* Pipe A CRC Red Control Register */
#define CRCCTRLGREENA   0x60054  /* Pipe A CRC Green Control Register */
#define CRCCTRLBLUEA    0x60058  /* Pipe A CRC Blue Control Register */
#define CRCCTRLRESA     0x6005C  /* Pipe A CRC Alpha Control Register */

/*-----------------------------------------------------------------------------
 * Pipe B registers are shared between Device 2 and Device 3
 * Atom E6xx silicon design specifies that both device 2 and device 3
 * Pipe B registers need to be set sequencially. Device 2, followed by
 * Device 3
 *---------------------------------------------------------------------------*/
#define PIPEB_TIMINGS   0x61000
#define HTOTAL_B        0x61000  /* Pipe B Horizontal Total Register */
#define HBLANK_B        0x61004  /* Pipe B Horizontal Blank Register */
#define HSYNC_B         0x61008  /* Pipe B Horizontal Sync Register */
#define VTOTAL_B        0x6100C  /* Pipe B Vertical Total Register */
#define VBLANK_B        0x61010  /* Pipe B Vertical Blank Register */
#define VSYNC_B         0x61014  /* Pipe B Vertical Sync Register */
#define PIPEBSRC        0x6101C  /* Pipe B Source Image Size Register */
#define BCLRPAT_B       0x61020  /* Pipe B Border Color Pattern Register */
#define VSYNCSHIFT_B	0x61028  /* Pipe B Vertical Sync Shift Register */
#define CRCCTRLREDB     0x61050  /* Pipe B CRC Red Control Register */
#define CRCCTRLGREENB   0x61054  /* Pipe B CRC Green Control Register */
#define CRCCTRLBLUEB    0x61058  /* Pipe B CRC Blue Control Register */
#define CRCCTRLRESB     0x6105C  /* Pipe B CRC Alpha Control Register */

/* These registers are in Device 3 */
#define PORT_HPLUG_EN		0x61110  /* Port Hot Plug Enable */
#define PORT_HPLUG_STAT 	0x61114  /* Port Hot Plug Status */
#define SDVOBCNTR       	0x61140  /* Digital Display Port B Control */
#define LVDSCNTR        	0x61180  /* Digital Display Port Control */
#define SDVO_BUFF_CTRL_REG	0x61170  /* SDVO Buffer Control */

/* Panel Power Sequencing */
#define LVDS_PNL_PWR_STS  0x61200  /* LVDS Panel Power Status Register */
#define LVDS_PNL_PWR_CTL  0x61204  /* LVDS Panel Power Control Register */
#define PP_ON_DELAYS      0x61208  /* Panel Power On Sequencing Delays */
#define PP_OFF_DELAYS     0x6120C  /* Panel Power Off Sequencing Delays */
#define PP_DIVISOR        0x61210  /* Panel Power Cycle Delay and Reference */

/* Panel Fitting */
#define PFIT_CONTROL      0x61230  /* Panel Fitting Control */
#define PFIT_PGM_RATIOS   0x61234  /* Programmed Panel Fitting Ratios */

/* Backlight control */
#define BLC_PWM_CTL2      0x61250  /* Backlight PWM Control 2 */
#define BLC_PWM_CTL       0x61254  /* Backlight PWM Control */
#define BLM_HIST_CTL      0x61260  /* Image BLM Histogram Control */

#define PORT_EN           BIT31
#define PORT_PIPE_SEL     BIT30
#define PORT_PIPE_SEL_POS 30
#define PORT_PIPE_A       0      /* 0 = Pipe A */
#define PORT_PIPE_B       BIT30  /* 1 = Pipe B */
#define STALL_MASK        BIT29 + BIT28
#define STALL_ENABLE      BIT28
#define SYNC_MASK         BIT15 + BIT11 + BIT10 + BIT4 + BIT3
#define SYNC_POLARITY     BIT15  /* 1 = Use VGA register */
#define VSYNC_OUTPUT      BIT11
#define HSYNC_OUTPUT      BIT10
#define VSYNC_POLARITY    BIT4
#define HSYNC_POLARITY    BIT3
#define FP_DATA_ORDER     BIT14
#define SUBDATA_ORDER     BIT6
#define BORDER_EN         BIT7
#define DISPLAY_EN        BIT2
#define INTERLACED_BIT    0x00100000
#define RGBA_BITS         0x00030000



/*-----------------------------------------------------------------------------
 * Display Pipeline A Register ( 70000h - 70024h )
 *---------------------------------------------------------------------------*/
#define PIPEA_SCANLINE_COUNT   0x70000  /* Pipe A Scan Line Count (RO) */
#define PIPEA_SCANLINE_COMPARE 0x70004  /* Pipe A SLC Range Compare (RO) */
#define PIPEA_CONF             0x70008  /* Pipe A Configuration */
#define PIPE_STATUS_OFFSET     0x1C
#define PIPEAGCMAXRED          0x70010  /* Pipe A Gamma Correct. Max Red */
#define PIPEAGCMAXGRN          0x70014  /* Pipe A Gamma Correct. Max Green */
#define PIPEAGCMAXBLU          0x70018  /* Pipe A Gamma Correct. Max Blue */
#define PIPEA_STAT             0x70024  /* Pipe A Display Status */
#define PIPEA_DISP_ARB_CTRL    0x70030  /* Display Arbitration Control */
#define FW_1                   0x70034
#define FW_2                   0x70038
#define FW_3                   0x7003C
#define PIPEA_FRAME_HIGH       0x70040  /* Pipe A Frame Count High */
#define PIPEA_FRAME_PIXEL      0x70044  /* Pipe A Frame Cnt Low & pixel count */
#define FW_4                   0x70050
#define FW_5                   0x70054
#define FW_6                   0x70058

#define PIPE_PIXEL_MASK        0x00ffffff
#define PIPE_FRAME_HIGH_MASK   0x0000ffff
#define PIPE_FRAME_LOW_MASK    0xff000000
#define PIPE_FRAME_LOW_SHIFT   24


/*-----------------------------------------------------------------------------
 * Display Pipeline B Register ( 71000h - 71024h )
 *---------------------------------------------------------------------------*/
#define PIPEB_SCANLINE_COUNT   0x71000 /* Pipe B Disp Scan Line Count Reg */
#define PIPEB_SCANLINE_COMPARE 0x71004 /* Pipe B Disp Scan Line Cnt Range Cmp */
#define PIPEB_CONF             0x71008 /* Pipe B Pixel Pipeline Config Reg */
#define PIPEBGCMAXRED          0x71010 /* Pipe B Gamma Correct. Max Red */
#define PIPEBGCMAXGRN          0x71014 /* Pipe B Gamma Correct. Max Green */
#define PIPEBGCMAXBLU          0x71018 /* Pipe B Gamma Correct. Max Blue */
#define PIPEB_STAT             0x71024 /* Display Status Select Register */
#define PROG_STALL             0x6102C /* Programmable cDVO Stall Register */
#define PIPEB_FRAME_HIGH       0x71040 /* Pipe B Frame Count High */
#define PIPEB_FRAME_PIXEL      0x71044 /* Pipe B Frame Cnt Low and pixel cnt */


#define VBLANK_EVN_STS_EN   BIT20
#define VBLANK_ODD_STS_EN   BIT21
#define VBLANK_EVN_STS      BIT4
#define VBLANK_ODD_STS      BIT5


/* following bit flag defs can be re-used for Pipe-B */
#define PIPE_ENABLE       BIT31
#define PIPE_LOCK         BIT29
#define GAMMA_MODE        BIT24
#define HOT_PLUG_EN       BIT26
/* Setting this bit to a 1 in the PIPE{A|B}_STAT register, enables the
 * consideration of vertical sync interrupts.
 */
#define VSYNC_STS_EN      BIT25
#define INTERLACE_EN	  BIT23|BIT22
/* Setting this bit to a 1 in the PIPE{A|B}_STAT register, enables the
 * consideration of vertical blank interrupts.
 */
#define VBLANK_STS_EN     BIT17
#define HOT_PLUG_STS      BIT10
#define VSYNC_STS         BIT9
#define VBLANK_STS        BIT1
/* The following are "sticky" status bits in the PIPE{A|B}_STAT register.  They
 * are cleared by writing a 1 to them.  Theres is code that reads a
 * PIPE{A|B}_STAT register into a variable, then modifies the variable, and
 * writes it back to the register.  These bits should be treated specially, so
 * as not to inadvertantly clear the status bits, so that other code looking
 * for those bits to be set won't miss it.
 */
#define PIPESTAT_STS_BITS ((unsigned long) BIT31 | BIT13 | BIT12 | BIT11 | \
	VSYNC_STS | BIT8 | BIT5 | BIT4 | VBLANK_STS | BIT0)

/*-----------------------------------------------------------------------------
 * cDVO Registers
 * --------------------------------------------------------------------------*/
#define CDVO_CTRL	  0x07000  /* cDVO Control Register */
#define CDVO_SLEW_RATE	  0x07004  /* cDVO Slew Rate Register */
#define CDVO_STRENGTH	  0x07008  /* cDVO Strength Register */
#define CDVO_RCOMP_UPDATE 0x0700C  /* cDVO RCOMP update Register */

/*-----------------------------------------------------------------------------
 * TNC_SDVOFAM Registers
 * --------------------------------------------------------------------------*/
#define DPLL_ANACORE_CTRL 	  0x06300
#define DPLL_MISC	  	  0x06304
#define DPLL_MONITOR	  	  0x06308
#define DPLL_MONITOR2	  	  0x0630C
#define DPLL_TEST_COMPARATOR      0x06310
#define SDVOB_TX_LSKEW	          0x06314

/*-----------------------------------------------------------------------------
 * Hardware Cursor Register Definitions (70080h - 7009Ch)
 *---------------------------------------------------------------------------*/
#define CUR_A_CNTR        0x70080  /*Cursor A Control */
#define CUR_B_CNTR        0x700C0
#define CUR_BASE_OFFSET   0x4
#define CUR_POS_OFFSET    0x8
#define CUR_PAL0_OFFSET   0x10
#define CUR_PAL1_OFFSET   0x14
#define CUR_PAL2_OFFSET   0x18
#define CUR_PAL3_OFFSET   0x1C

/* Define these for ease of reference */
#define CURSOR_A_BASE     CUR_A_CNTR + CUR_BASE_OFFSET
#define CURSOR_A_POS      CUR_A_CNTR + CUR_POS_OFFSET
#define CURSOR_A_PAL0     CUR_A_CNTR + CUR_PAL0_OFFSET
#define CURSOR_A_PAL1     CUR_A_CNTR + CUR_PAL1_OFFSET
#define CURSOR_A_PAL2     CUR_A_CNTR + CUR_PAL2_OFFSET
#define CURSOR_A_PAL3     CUR_A_CNTR + CUR_PAL3_OFFSET
#define CURSOR_B_BASE     CUR_B_CNTR + CUR_BASE_OFFSET
#define CURSOR_B_POS      CUR_B_CNTR + CUR_POS_OFFSET
#define CURSOR_B_PAL0     CUR_B_CNTR + CUR_PAL0_OFFSET
#define CURSOR_B_PAL1     CUR_B_CNTR + CUR_PAL1_OFFSET
#define CURSOR_B_PAL2     CUR_B_CNTR + CUR_PAL2_OFFSET
#define CURSOR_B_PAL3     CUR_B_CNTR + CUR_PAL3_OFFSET

/*-----------------------------------------------------------------------------
 * Display Plane A Register Definitions (70180h - 70188h)
 *---------------------------------------------------------------------------*/
#define DSPAAFLIP    	0x7017C
#define DSPACNTR        0x70180  /* Display Plane A */
#define DSPALINOFF      0x70184  /* Display A Linear Offset */
#define DSPASTRIDE      0x70188  /* Display A Stride */
#define DSPAKEYVAL      0x70194  /* Sprite color key value */
#define DSPAKEYMASK     0x70198  /* Sprite color key mask */
#define DSPASURF        0x7019C  /* Display A Suface base address */
#define DSPATILEOFF     0x701A4  /* Display A Tiled Offset */

/*-----------------------------------------------------------------------------
 * Display Plane B Register Definitions (71180h - 71188h)
 *---------------------------------------------------------------------------*/
#define DSPBFLIP        0x7117C  /* Display B Async flip */
#define DSPBCNTR        0x71180  /* Display Plane B */
#define DSPBLINOFF      0x71184  /* Display B Linear Offset */
#define DSPBSTRIDE      0x71188  /* Display B Stride */
#define DSPBKEYVAL      0x71194  /* Sprite color key value */
#define DSPBKEYMASK     0x71198  /* Sprite color key mask */
#define DSPBSURF        0x7119C  /* Display B Suface base address */
#define DSPBTILEOFF     0x711A4  /* Display B Tiled Offset */

/*-----------------------------------------------------------------------------
 * Source Format Definition for DSPxCNTR
 *---------------------------------------------------------------------------*/
#define DSPxCNTR_ARGB_8888			0x1C000000
#define DSPxCNTR_RGB_8888			0x18000000
#define DSPxCNTR_RGB_565			0x14000000
#define DSPxCNTR_RGB_555			0x10000000
#define DSPxCNTR_RGB_8				0x08000000
#define DSPxCNTR_SRC_FMT_MASK			0x3C000000 /*mask for above*/

/*-----------------------------------------------------------------------------
 * Display Plane C Register Definitions (72180h - 72188h)
 *---------------------------------------------------------------------------*/
#define DSPCCNTR        0x72180  /* Display Plane C */
#define DSPCLINOFF      0x72184  /* Display C Linear Offset */
#define DSPCSTRIDE      0x72188  /* Display C Stride */
#define DSPCPOS         0x7218C  /* Display C Position */
#define DSPCSIZE        0x72190  /* Display C Size */
#define DSPCKEYMINVAL   0x72194  /* Sprite color key Min */
#define DSPCKEYMASK     0x72198  /* Sprite color key mask */
#define DSPCSURF        0x7219C  /* Display C Suface base address */
#define DSPCKEYMAXVAL   0x721A0  /* Display C Sprint color key Max */
#define DSPCTILEOFF     0x721A4  /* Display C Tiled Offset */
#define DSPCCONTALPHA	0x721A8	 /* Display C Constant Alpha */

#define DCLRC0          0x721D0  /* Display C Color Correction 0 */
#define DCLRC1          0x721D4  /* Display C Color Correction 1 */
#define DPYC_GAMC5      0x721E0  /* Display C Gamma Correction 5 */
#define DPYC_GAMC4      0x721E4  /* Display C Gamma Correction 4 */
#define DPYC_GAMC3      0x721E8  /* Display C Gamma Correction 3 */
#define DPYC_GAMC2      0x721EC  /* Display C Gamma Correction 2 */
#define DPYC_GAMC1      0x721F0  /* Display C Gamma Correction 1 */
#define DPYC_GAMC0      0x721F4  /* Display C Gamma Correction 0 */

#define PLANE_ENABLE    BIT31
#define GAMMA_ENABLE    BIT30
#define BPP_MASK        BIT29 + BIT28 + BIT27 + BIT26
#define BPP_POS         26
#define STEREO_ENABLE   BIT25
#define PIPE_SEL        BIT24
#define PIPE_SEL_POS    24
#define PIXEL_MULTIPLY  BIT21 + BIT20
#define STEREO_POLARITY BIT18

/* Common offsets for all Display Pipeline registers */
#define DSP_LINEAR_OFFSET 0x04  /* Offset from the control reg */
#define DSP_STRIDE_OFFSET 0x08  /* Offset from the control reg */
#define DSP_SIZE_OFFSET   0x10  /* Offset from the control reg */
#define DSP_START_OFFSET  0x1c  /* Offset from the control reg */
#define DSP_TOFF_OFFSET   0x24  /* Offset from the control reg */

/*-----------------------------------------------------------------------------
 * VGA Display Plane Control Register Definitions (71400h)
 *---------------------------------------------------------------------------*/
#define VP00             0x71400
#define VGACNTRL         0x71400  /* VGA Display Plane Control Register */
#define VGA_DOUBLE       BIT30
#define VGA_PIPE         BIT29
#define VGA_CENTER_MASK  BIT25 + BIT24
#define VGA_CENTER_1     BIT25
#define VGA_CENTER_0     BIT24
#define VGA_PAL_READ     BIT23
#define VGA_PAL_MASK     BIT22 + BIT21
#define VGA_PALA_DISABLE BIT22
#define VGA_PALB_DISABLE BIT21
#define DAC_8_BIT	     BIT20
#define VGA_8_DOT	     BIT18

#define ADD_ID           0x71408  /* ADD Card ID Register*/

/*-----------------------------------------------------------------------------
 * Overlay Plane Control Register Definitions (30000h)
 *---------------------------------------------------------------------------*/
#define OVADD            0x30000  /* Overlay Control */

/*-----------------------------------------------------------------------------
 * VBIOS Software flags  00h - 0Fh
 *---------------------------------------------------------------------------*/
#define DSP_CHICKENBITS	0x70400  /* Chicken Bit */
#define SWFABASE        0x70410  /* Software flags A Base Addr */
#define SWF00           0x70410
#define SWF01           0x70414
#define SWF02           0x70418
#define SWF03           0x7041C
#define SWF04           0x70420
#define SWF05           0x70424
#define SWF06           0x70428
#define SWF07           0x7042C
#define SWF08           0x70430
#define SWF09           0x70434
#define SWF0A           0x70438
#define SWF0B           0x7043C
#define SWF0C           0x70440
#define SWF0D           0x70444
#define SWF0E           0x70448
#define SWF0F           0x7044C

/*-----------------------------------------------------------------------------
 * VBIOS Software flags  10h - 1Fh
 *---------------------------------------------------------------------------*/
#define SWFBBASE        0x71410  /* Software flags B Base Addr */
#define SWF10           0x71410
#define SWF11           0x71414
#define SWF12           0x71418
#define SWF13           0x7141C
#define SWF14           0x71420
#define SWF15           0x71424
#define SWF16           0x71428
#define SWF17           0x7142C
#define SWF18           0x71430
#define SWF19           0x71434
#define SWF1A           0x71438
#define SWF1B           0x7143C
#define SWF1C           0x71440
#define SWF1D           0x71444
#define SWF1E           0x71448
#define SWF1F           0x7144C

#define	SWF30			0x72414
#define SWF31			0x72418
#define SWF32			0x7241C

/*-----------------------------------------------------------------------------
 * Software Flag Registers (71410h - 71428h)
 *---------------------------------------------------------------------------*/
/* Map old software flag names to their new Gen4 names */
#define SF00  SWF10
#define SF01  SWF11
#define SF02  SWF12
#define SF03  SWF13
#define SF04  SWF14
#define SF05  SWF15
#define SF06  SWF16

/*-----------------------------------------------------------------------------
 * GPIO Control Registers(05000h - 05023h)
 *---------------------------------------------------------------------------*/
/* GPIO registers 0x0500 - 0x500C is reserved */
#define GPIOCTL_1	0x05014
#define GPIOCTL_2	0x05018
#define GPIOCTL_4	0x05020

/*-----------------------------------------------------------------------------
 * Scratch register to be used as additional parameter in SMI
 *---------------------------------------------------------------------------*/
#define SCRATCH_SWF6         0x71428

/*-----------------------------------------------------------------------------
 * Miscellaneous registers
 ----------------------------------------------------------------------------*/
#define HW_ST_PAGE_ADDR      0x02080

/*-----------------------------------------------------------------------------
 * GMBUS : I2C Bus Types
 ----------------------------------------------------------------------------*/
//#define GMBUS_ANALOG_DDC    1

//#define GMBUS_INT_LVDS_DDC  2
#define I2C_INT_LVDS_DDC  2

#define GMBUS_DVO_REG       3

#define GMBUS_DVOB_DDC      4
//#define GMBUS_DVOC_DDC      5

/*-----------------------------------------------------------------------------
 * LPC Register Offsets. Used for LVDS GPIO Bit Bashing. Registers are part
 * Atom E6xx [D31:F0]
 ----------------------------------------------------------------------------*/
#define RGEN    0x20
#define RGIO    0x24
#define RGLVL   0x28
#define RGTPE   0x2C
#define RGTNE   0x30
#define RGGPE   0x34
#define RGSMI   0x38
#define RGTS    0x3C


#endif /* _REGS_H_ */

