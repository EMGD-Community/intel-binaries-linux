SUBDIRS = F14 MeeGo1.2 Tizen1.0 MeeGo_Wayland

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

