sub_dirs = \
	libwacom \
	linux \
	udev

top_targets = all install uninstall clean

define top_template
$1:
	@for d in $(sub_dirs); do \
		$(MAKE) -C $$$$d $$@; \
	done
endef

$(foreach target,$(top_targets),$(eval $(call top_template,$(target))))

.PHONY: $(top_targets)
