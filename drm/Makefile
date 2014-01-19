#----------------------------------------------------------------------------
# Filename: Makefile.gnu
# $Revision: 1.59 $
#----------------------------------------------------------------------------
# Copyright (c) 2002-2010, Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#----------------------------------------------------------------------------
export EGD_TOPLEVEL = DRM Driver

KERNELVER ?= $(shell uname -r)
KERNELDIR ?= /lib/modules/$(KERNELVER)/build
INSTALLDIR ?= /lib/modules/$(KERNELVER)/kernel/drivers/gpu/drm/emgd

BLUE = \033[34m
OFF = \033[0m
BUILD ?= release
CONFIG_PVR_RELEASE ?= $(BUILD)
CONFIG_DRM_EGD ?= m

# Get the include paths pointed to the right place. 
export  EMGD_MOD_DIR ?= $(CURDIR)

BUILDDATE ?= $(shell date +%Y%m%d)

PROJECT_INCLUDES = \
	   -I$(EMGD_MOD_DIR)/include \
	   -I$(EMGD_MOD_DIR)/emgd/display/mode/cmn \
	   -I$(EMGD_MOD_DIR)/emgd/video/overlay/cmn \
	   -I$(EMGD_MOD_DIR)/emgd/video/msvdx \
	   -I$(EMGD_MOD_DIR)/emgd/include \
	   -I$(EMGD_MOD_DIR)/emgd/cfg \
	   -I$(EMGD_MOD_DIR)/emgd/pal/lpd \
	   -I$(EMGD_MOD_DIR)/emgd/pal/lvds \
	   -I$(EMGD_MOD_DIR)/emgd/pal/ch7036 \
	   -I$(EMGD_MOD_DIR)/emgd/drm \
	   -I$(KERNELDIR)/include/drm \
	   -I/usr/src/linux-headers-2.6.32-5-common/include/drm \
	   -I$(EMGD_MOD_DIR)/pvr/include4 \
	   -I$(EMGD_MOD_DIR)/pvr/services4/include \
	   -I$(EMGD_MOD_DIR)/pvr/services4/include/env/linux \
	   -I$(EMGD_MOD_DIR)/pvr/services4/srvkm/env/linux \
	   -I$(EMGD_MOD_DIR)/pvr/services4/srvkm/include \
	   -I$(EMGD_MOD_DIR)/pvr/services4/srvkm/bridged \
	   -I$(EMGD_MOD_DIR)/pvr/services4/system/plb \
	   -I$(EMGD_MOD_DIR)/pvr/services4/system/tnc \
	   -I$(EMGD_MOD_DIR)/pvr/services4/system/include \
	   -I$(EMGD_MOD_DIR)/pvr/services4/srvkm/hwdefs \
	   -I$(EMGD_MOD_DIR)/pvr/services4/srvkm/bridged/sgx \
	   -I$(EMGD_MOD_DIR)/pvr/services4/srvkm/devices/sgx \
	   -I$(EMGD_MOD_DIR)/pvr/services4/3rdparty/emgd_bufferclass \
	   -I$(EMGD_MOD_DIR)/pvr/tools/intern/debug \
	   -DSUPPORT_DRI_DRM_EXT \
	   -DLINUX \
	   -DPVR_BUILD_DIR="\"emgd\"" \
	   -DPVR_BUILD_DATE="\"$(BUILDDATE)\"" \
	   -DPVR_BUILD_TYPE="\"$(CONFIG_PVR_RELEASE)\"" \
	   -DBUILD=$(CONFIG_PVR_RELEASE) \
	   -DPVR_SECURE_HANDLES \
	   -DPVR_PROC_USE_SEQ_FILE \
	   -DLDM_PCI \
	   -DSUPPORT_CACHEFLUSH_ON_ALLOC \
	   -DSUPPORT_DRI_DRM \
	   -DSGX535 \
	   -DSGX_CORE_REV=121 \
	   -UDEBUG_LOG_PATH_TRUNCATE \
	   -DDISPLAY_CONTROLLER=emgd_dc \
	   -D_XOPEN_SOURCE=600 \
	   -DSERVICES4 \
	   -DPVR2D_VALIDATE_INPUT_PARAMS \
	   -DSUPPORT_SRVINIT \
	   -DSUPPORT_SGX \
	   -DSUPPORT_PERCONTEXT_PB \
	   -DSUPPORT_LINUX_X86_WRITECOMBINE \
	   -DSUPPORT_SECURE_DRM_AUTH_EXPORT \
	   -DSUPPORT_PDUMP_DELAYED_INITPHASE_TERMINATION \
	   -DTRANSFER_QUEUE \
	   -DSYS_USING_INTERRUPTS \
	   -DSUPPORT_HW_RECOVERY \
	   -DSUPPORT_ACTIVE_POWER_MANAGEMENT \
	   -DPVR_SECURE_HANDLES \
	   -DUSE_PTHREADS \
	   -DSUPPORT_SGX_EVENT_OBJECT \
	   -DSUPPORT_SGX_HWPERF \
	   -DSUPPORT_LINUX_X86_PAT \
	   -DSUPPORT_SGX535 \
	   -DSUPPORT_CACHE_LINE_FLUSH \
	   -DSUPPORT_CPU_CACHED_BUFFERS \
	   -DDEBUG_MESA_OGL_TRACE \
	   -DSUPPORT_EGL_IMAGE_SYNC_DEPENDENCY \


ifeq "$(strip $(CONFIG_PVR_RELEASE))" "release"
	ccflags-y += -DRELEASE
else
	# FIXME: Looks like this causes conflicts in the emgd code.
	ccflags-y += -DDEBUG
	ccflags-y += -DDEBUG_BUILD_TYPE 
endif

EXTRA_CFLAGS += $(PROJECT_INCLUDES)

ifeq ($(PDUMP),1)
	EXTRA_CFLAGS += -DPDUMP=1
endif

EMGD_OBJS := \
	emgd/drm/emgd_fb.o \
	emgd/drm/emgd_fbcon.o \
	emgd/drm/emgd_crtc.o \
	emgd/drm/emgd_encoder.o \
	emgd/drm/emgd_connector.o \
	emgd/drm/emgd_mmap.o \
	emgd/drm/emgd_drv.o \
	emgd/drm/emgd_interface.o \
	emgd/drm/emgd_test_pvrsrv.o \
	emgd/drm/user_config.o \
	emgd/drm/splash_screen.o \
	emgd/display/pd/cmn/pd.o \
	emgd/display/pi/cmn/igd_pi.o \
	emgd/display/pi/cmn/displayid.o \
	emgd/display/pi/cmn/pd_init_all.o \
	emgd/display/pi/cmn/edid.o \
	emgd/display/pi/cmn/pi.o \
	emgd/display/pi/cmn/mode_table.o \
	emgd/display/pi/tnc/i2c_gmbus_tnc.o \
	emgd/display/pi/tnc/i2c_bitbash_tnc.o \
	emgd/display/pi/plb/i2c_plb.o \
	emgd/display/mode/cmn/match.o \
	emgd/display/mode/cmn/micro_mode.o \
	emgd/display/mode/cmn/vga_mode.o \
	emgd/display/mode/cmn/igd_mode.o \
	emgd/display/mode/tnc/micro_mode_tnc.o \
	emgd/display/mode/tnc/mode_tnc.o \
	emgd/display/mode/tnc/kms_mode_tnc.o \
	emgd/display/mode/tnc/clocks_tnc.o \
	emgd/display/mode/plb/micro_mode_plb.o \
	emgd/display/mode/plb/clocks_plb.o \
	emgd/display/mode/plb/mode_plb.o \
	emgd/display/mode/plb/kms_mode_plb.o \
	emgd/display/dsp/cmn/dsp.o \
	emgd/display/dsp/tnc/dsp_tnc.o \
	emgd/display/dsp/plb/dsp_plb.o \
	emgd/core/init/cmn/igd_global.o \
	emgd/core/init/cmn/igd_init.o \
	emgd/core/init/tnc/micro_init_tnc.o \
	emgd/core/init/tnc/init_tnc.o \
	emgd/core/init/plb/init_plb.o \
	emgd/core/init/plb/micro_init_plb.o \
	emgd/state/power/cmn/igd_pwr.o \
	emgd/state/power/plb/pwr_plb.o \
	emgd/state/appcontext/cmn/igd_appcontext.o \
	emgd/state/appcontext/plb/appcontext_plb.o \
	emgd/state/reg/cmn/reg.o \
	emgd/state/reg/tnc/reg_tnc.o \
	emgd/state/reg/plb/reg_plb.o \
	emgd/video/overlay/cmn/ovl_coeff.o \
	emgd/video/overlay/cmn/igd_ovl.o \
	emgd/video/overlay/cmn/micro_ovl.o \
	emgd/video/overlay/tnc/ovl_tnc_cache.o \
	emgd/video/overlay/tnc/ovl_tnc.o \
	emgd/video/overlay/tnc/ovl2_tnc.o \
	emgd/video/overlay/tnc/micro_ovl_tnc.o \
	emgd/video/overlay/plb/ovl_plb_cache.o \
	emgd/video/overlay/plb/ovl2_plb.o \
	emgd/video/overlay/plb/ovl_plb.o \
	emgd/video/overlay/plb/micro_ovl_plb.o \
	emgd/video/msvdx/msvdx_init.o \
	emgd/video/msvdx/msvdx.o \
	emgd/video/msvdx/msvdx_pvr.o \
	emgd/video/topaz/topaz_init.o \
	emgd/video/topaz/topaz.o \
	emgd/pal/sdvo/sdvo_attr.o \
	emgd/pal/sdvo/sdvo_hdmi.o \
	emgd/pal/sdvo/sdvo_port.o \
	emgd/pal/sdvo/sdvo_intf.o \
	emgd/pal/ch7036/ch7036_attr.o \
	emgd/pal/ch7036/ch7036_fw.o \
	emgd/pal/ch7036/ch7036_intf.o \
	emgd/pal/ch7036/ch7036_port.o \
	emgd/pal/ch7036/ch7036.o \
	emgd/pal/ch7036/ch7036_iic.o \
	emgd/pal/ch7036/ch7036_pm.o \
	emgd/pal/ch7036/ch7036_reg_table.o \
	emgd/pal/ch7036/lvds/lvds.o \
	emgd/pal/lvds/lvds.o \
	emgd/pal/lpd/lpd.o \
	emgd/gmm/gmm.o \
	emgd/gmm/gtt.o \
	emgd/utils/pci.o \
	emgd/utils/memmap.o \
	emgd/utils/math_fix.o \

ENVDIR = pvr/services4/srvkm/env/linux
COMMONDIR = pvr/services4/srvkm/common
BRIDGEDDIR = pvr/services4/srvkm/bridged
SYSCONFIGDIR = pvr/services4/system/common
SGXDIR = pvr/services4/srvkm/devices/sgx
DISPCLASSDIR = pvr/services4/3rdparty/emgd_displayclass
BUFFERCLASSDIR = pvr/services4/3rdparty/emgd_bufferclass

ifeq ($(PDUMP),1)
DBGDRVDIR = pvr/tools/intern/debug/dbgdriv

DBGDRV_OBJS = $(DBGDRVDIR)/linux/main.o \
              $(DBGDRVDIR)/common/dbgdriv.o \
              $(DBGDRVDIR)/common/ioctl.o \
              $(DBGDRVDIR)/linux/hostfunc.o \
              $(DBGDRVDIR)/common/hotkey.o
endif

ENV_OBJS = $(ENVDIR)/osfunc.o \
	   $(ENVDIR)/mutils.o \
	   $(ENVDIR)/mmap.o \
	   $(ENVDIR)/module.o \
	   $(ENVDIR)/pdump.o \
	   $(ENVDIR)/proc.o \
	   $(ENVDIR)/pvr_bridge_k.o \
	   $(ENVDIR)/pvr_debug.o \
	   $(ENVDIR)/mm.o \
	   $(ENVDIR)/mutex.o \
	   $(ENVDIR)/event.o \
	   $(ENVDIR)/osperproc.o \
	   $(ENVDIR)/pvr_drm.o

COMMON_OBJS = $(COMMONDIR)/buffer_manager.o \
	    $(COMMONDIR)/devicemem.o \
	    $(COMMONDIR)/deviceclass.o \
	    $(COMMONDIR)/handle.o \
	    $(COMMONDIR)/hash.o \
	    $(COMMONDIR)/metrics.o \
	    $(COMMONDIR)/pvrsrv.o \
	    $(COMMONDIR)/queue.o \
	    $(COMMONDIR)/ra.o \
	    $(COMMONDIR)/resman.o \
	    $(COMMONDIR)/power.o \
	    $(COMMONDIR)/mem.o \
	    $(COMMONDIR)/pdump_common.o \
	    $(COMMONDIR)/perproc.o \
	    $(COMMONDIR)/lists.o \
	    $(COMMONDIR)/mem_debug.o

BRIDGED_OBJS = $(BRIDGEDDIR)/bridged_support.o \
	     $(BRIDGEDDIR)/bridged_pvr_bridge.o \
	     $(BRIDGEDDIR)/sgx/bridged_sgx_bridge.o

SYSCONFIG_OBJS = $(SYSCONFIGDIR)/sysconfig.o \
	        pvr/services4/system/tnc/sysconfig.o \
	        pvr/services4/system/plb/sysconfig.o \
		$(SYSCONFIGDIR)/sysutils.o
#		 $(SYSCONFIGDIR)/sysirq.o
#	 	 $(SYSCONFIGDIR)/ospm_power.o \

SGX_OBJS = $(SGXDIR)/sgxinit.o \
	 $(SGXDIR)/sgxpower.o \
	 $(SGXDIR)/sgxreset.o \
	 $(SGXDIR)/sgxutils.o \
	 $(SGXDIR)/sgxkick.o \
	 $(SGXDIR)/sgxtransfer.o \
	 $(SGXDIR)/mmu.o \
	 $(SGXDIR)/pb.o

DC_OBJS = $(DISPCLASSDIR)/emgd_dc.o \
	  $(DISPCLASSDIR)/emgd_dc_linux.o

BC_OBJS = $(BUFFERCLASSDIR)/emgd_bc.o \
	  $(BUFFERCLASSDIR)/emgd_bc_linux.o


emgd-y := \
	$(DC_OBJS) \
	$(EMGD_OBJS) \
	$(ENV_OBJS) \
	$(COMMON_OBJS) \
	$(BRIDGED_OBJS) \
	$(SYSCONFIG_OBJS) \
	$(SGX_OBJS) \
	$(BC_OBJS) \

ifeq ($(PDUMP),1)
	emgd-y += $(DBGDRV_OBJS)
endif

obj-$(CONFIG_DRM_EGD) += emgd.o

all:: clean modules

modules::
	@echo $(CURDIR) -- $(CONFIG_PVR_RELEASE)
	@echo "$(MAKE) -C $(KERNELDIR) M=$(CURDIR) modules"
	@$(MAKE) -C $(KERNELDIR) M=$(CURDIR) modules

clean::
	@rm -f $(emgd-y)
	@rm -f emgd.o emgd.mod.* emgd.ko Module.* modules.order
	@find . -name "*.cmd" -exec rm '{}' \;

install::
	install -o root -g root -m 755 -d $(INSTALLDIR)
	install -o root -g root -m 744 emgd.ko $(INSTALLDIR)
	/sbin/depmod -a

uninstall::
	rmmod $(INSTALLDIR)/emgd.ko
	rm -rf $(INSTALLDIR)/emgd.ko
	/sbin/depmod -a

debug::
	export CONFIG_PVR_RELEASE=debug; $(MAKE) modules

package:: clean
	@echo -e "$(BLUE)Packaging $(EGD_TOPLEVEL)$(OFF)";
	mkdir -p $(EGD_PKG)
	tar -C $(EMGD_MOD_DIR) --exclude "CVS" -czf $(EGD_PKG)/emgd_drm.tgz *

