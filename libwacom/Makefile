libwacom_dir = /etc/libwacom

libwacom_data = \
	bosto-13hd.tablet \
	bosto.stylus \
	layouts/bosto-13hd.svg

all:

install:
	@for f in $(libwacom_data); do \
		install -D -m644 $$f $(DESTDIR)$(libwacom_dir)/$$f; \
	done
	libwacom-update-db $(DESTDIR)$(libwacom_dir)

uninstall:
	@for file in $(libwacom_data); do \
		rm -f $(DESTDIR)$(libwacom_dir)/$$file; \
	done

clean:

.PHONY: all install uninstall clean
