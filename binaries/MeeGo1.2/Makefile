EMGD_BUILD_REV=3398
EMGD_BUILD_VER=1.18
EMGD_BUILD_ARCH=i586

all: unpak link

unpak: clean
	mkdir -p rpm_contents
	for package in bin devel gui ; do \
		cd rpm_contents ; \
		rpm2cpio ../emgd-$$package-$(EMGD_BUILD_REV)-$(EMGD_BUILD_VER).$(EMGD_BUILD_ARCH).rpm    | cpio -idmv ; \
		cd .. ; \
	done

link:
	cd rpm_contents/usr/lib/ && ln -s libEGL.so.1.5.15.3226                libEGL.so.1
	cd rpm_contents/usr/lib/ && ln -s libEMGD2d.so.1.5.15.3226             libEMGD2d.so
	cd rpm_contents/usr/lib/ && ln -s libEMGD2d.so.1.5.15.3226             libEMGD2d.so.1
	cd rpm_contents/usr/lib/ && ln -s libEMGDegl.so.1.5.15.3226            libEMGDegl.so
	cd rpm_contents/usr/lib/ && ln -s libEMGDegl.so.1.5.15.3226            libEMGDegl.so.1
	cd rpm_contents/usr/lib/ && ln -s libemgdglslcompiler.so.1.5.15.3226   libemgdglslcompiler.so
	cd rpm_contents/usr/lib/ && ln -s libemgdglslcompiler.so.1.5.15.3226   libemgdglslcompiler.so.1
	cd rpm_contents/usr/lib/ && ln -s libEMGDOGL.so.1.5.15.3226            libEMGDOGL.so
	cd rpm_contents/usr/lib/ && ln -s libEMGDOGL.so.1.5.15.3226            libEMGDOGL.so.1
	cd rpm_contents/usr/lib/ && ln -s libemgdPVR2D_DRIWSEGL.so.1.5.15.3226 libemgdPVR2D_DRIWSEGL.so
	cd rpm_contents/usr/lib/ && ln -s libemgdPVR2D_DRIWSEGL.so.1.5.15.3226 libemgdPVR2D_DRIWSEGL.so.1
	#cd rpm_contents/usr/lib/ && ln -s libemgdPVR2D_GBMWSEGL.so.1.5.15.3226 libemgdPVR2D_GBMWSEGL.so
	#cd rpm_contents/usr/lib/ && ln -s libemgdPVR2D_GBMWSEGL.so.1.5.15.3226 libemgdPVR2D_GBMWSEGL.so.1
	#cd rpm_contents/usr/lib/ && ln -s libemgdPVR2D_WAYLANDWSEGL.so.1.5.15.3226 libemgdPVR2D_WAYLANDWSEGL.so
	#cd rpm_contents/usr/lib/ && ln -s libemgdPVR2D_WAYLANDWSEGL.so.1.5.15.3226 libemgdPVR2D_WAYLANDWSEGL.so.1
	#cd rpm_contents/usr/lib/ && ln -s libgbm.so.1.5.15.3226                libgbm.so
	#cd rpm_contents/usr/lib/ && ln -s libgbm.so.1.5.15.3226                libgbm.so.1
	cd rpm_contents/usr/lib/ && ln -s libEMGDScopeServices.so.1.5.15.3226  libEMGDScopeServices.so
	cd rpm_contents/usr/lib/ && ln -s libEMGDScopeServices.so.1.5.15.3226  libEMGDScopeServices.so.1
	cd rpm_contents/usr/lib/ && ln -s libemgdsrv_init.so.1.5.15.3226       libemgdsrv_init.so
	cd rpm_contents/usr/lib/ && ln -s libemgdsrv_init.so.1.5.15.3226       libemgdsrv_init.so.1
	cd rpm_contents/usr/lib/ && ln -s libemgdsrv_um.so.1.5.15.3226         libemgdsrv_um.so
	cd rpm_contents/usr/lib/ && ln -s libemgdsrv_um.so.1.5.15.3226         libemgdsrv_um.so.1
	#cd rpm_contents/usr/lib/ && ln -s libGLES_CM.so.1.5.15.3226            libGLES_CM.so
	cd rpm_contents/usr/lib/ && ln -s libGLES_CM.so.1.5.15.3226            libGLES_CM.so.1
	cd rpm_contents/usr/lib/ && ln -s libGLES_CM.so.1.5.15.3226            libGLESv1_CM.so.1.5.15.3226
	cd rpm_contents/usr/lib/ && ln -s libGLESv1_CM.so.1.5.15.3226	       libGLESv1_CM.so.1
	#cd rpm_contents/usr/lib/ && ln -s libGLESv2.so.1.5.15.3226             libGLESv2.so
	cd rpm_contents/usr/lib/ && ln -s libGLESv2.so.1.5.15.3226             libGLESv2.so.1
	#cd rpm_contents/usr/lib/ && ln -s libOpenVG.so.1.5.15.3226              libOpenVG.so
	cd rpm_contents/usr/lib/ && ln -s libOpenVG.so.1.5.15.3226             libOpenVG.so.1
	#cd rpm_contents/usr/lib/ && ln -s libOpenVGU.so.1.5.15.3226            libOpenVGU.so
	cd rpm_contents/usr/lib/ && ln -s libOpenVGU.so.1.5.15.3226            libOpenVGU.so.1
	#cd rpm_contents/usr/lib/ && ln -s libwayland-egl.so.1.5.15.3226        libwayland-egl.so
	#cd rpm_contents/usr/lib/ && ln -s libwayland-egl.so.1.5.15.3226        libwayland-egl.so.1
	#cd rpm_contents/usr/lib/ && ln -s libwayland-emgd.so.1.5.15.3226 libwayland-emgd.so
	#cd rpm_contents/usr/lib/ && ln -s libwayland-emgd.so.1.5.15.3226 libwayland-emgd.so.1
	
clean:
	rm -rfv rpm_contents/
