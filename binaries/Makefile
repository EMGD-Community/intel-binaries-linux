SUBDIRS = F14 MeeGo1.2 Tizen1.0
#SUBDIRS += MeeGo_Wayland

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done
	cp Tizen1.0/rpm_contents/usr/lib/libwayland-emgd.so.1.5.15.3226 MeeGo_Wayland/rpm_contents/usr/lib/
	$(MAKE) -C MeeGo_Wayland
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

