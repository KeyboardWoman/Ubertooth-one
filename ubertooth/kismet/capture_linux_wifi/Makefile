include ../Makefile.inc

MONITOR_OBJS = \
	../interface_control.c.o \
	../wifi_ht_channels.c.o \
	linux_wireless_control.c.o \
	linux_netlink_control.c.o \
	linux_nexmon_control.c.o \
	linux_wireless_rfkill.c.o \
	capture_linux_wifi.c.o

MONITOR_BIN = kismet_cap_linux_wifi

PCAPLIBS=-lpcap
NMLIBS=-lnm -lgio-2.0 -lgobject-2.0 -lglib-2.0
NETLINKLIBS=-lnl-genl-3 -lnl-3

all: $(MONITOR_BIN)

$(MONITOR_BIN):	$(MONITOR_OBJS) $(patsubst %c.o,%c.d,$(MONITOR_OBJS)) ../libkismetdatasource.a
		$(CCLD) $(LDFLAGS) -o $(MONITOR_BIN) $(MONITOR_OBJS) ../libkismetdatasource.a $(PCAPLIBS) $(NMLIBS) $(NETLINKLIBS) $(DATASOURCE_LIBS)

clean:
	@-rm -f $(MONITOR_BIN)
	@-rm -f *.o
	@-rm -f *.d

%.c.o:	%.c
%.c.o : %.c %.c.d
	$(CC) $(CFLAGS) $(CPPFLAGS) $(GLIBCFLAGS) $(DBUSGLIBCFLAGS) -c $*.c -o $@

%.c.d:	%.c
	$(CC) -MM $(CFLAGS) $(GLIBCFLAGS) $(DBUSGLIBCFLAGS) $(CPPFLAGS) $*.c | sed -e "s/\.o/\.c.o/" > $*.c.d

.PRECIOUS: %.c.d

include $(wildcard $(patsubst %c.o,%c.d,$(MONITOR_OBJS)))

