SUBDIRS = F14 MeeGo1.2 MeeGo_Wayland Tizen1.0

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

