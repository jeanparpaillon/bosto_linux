RULES = 99-bosto-pad.rules

RULESDIR = /etc/udev/rules.d
DESTDIR ?= /

all:

install: $(RULES)
	install -m 644 -o root -g root $(RULES) $(DESTDIR)$(RULESDIR)/$(RULES)
	udevadm control --reload-rules && udevadm trigger

uninstall:
	rm -f $(DESTDIR)$(RULESDIR)/$(RULES)
	udevadm control --reload-rules && udevadm trigger

clean:

.PHONY: all install uninstall clean
