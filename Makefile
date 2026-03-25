VERSION := $(shell sed -n 's/^## \(.*\)/\1/p' CHANGELOG.md | head -1)

sub_dirs = \
	libwacom \
	linux \
	udev

top_targets = all install uninstall clean

all: dkms.conf

dkms.conf: dkms.conf.in
	sed -e 's/@VERSION@/$(VERSION)/g' dkms.conf.in > dkms.conf

clean: clean-dkms clean-debian

clean-dkms:
	rm -f dkms.conf

clean-debian:
	rm -rf debian/bosto-dkms
	rm -rf debian/bosto-tablet
	rm -rf debian/bosto-tools
	rm -f debian/*.log
	rm -f debian/*.substvars
	rm -f debian/files
	rm -f debian/debhelper-build-stamp

debian:
	dpkg-buildpackage -us -uc

define top_template
$1: $1-subdirs

$1-subdirs:
	@for d in $(sub_dirs); do \
		$(MAKE) -C $$$$d $1; \
	done
endef

$(foreach target,$(top_targets),$(eval $(call top_template,$(target))))

.PHONY: $(top_targets) $(patsubst %,%-subdirs,$(top_targets))
.PHONY: debian clean-dkms clean-debian
