include ../Makefile.inc

MONITOR_OBJS = \
	capture_nrf_mousejack.c.o 
MONITOR_BIN = kismet_cap_nrf_mousejack

all: $(MONITOR_BIN)

LIBUSBCFLAGS=-I/usr/include/libusb-1.0
LIBUSBLIBS=-lusb-1.0

$(MONITOR_BIN):	$(MONITOR_OBJS) $(patsubst %c.o,%c.d,$(MONITOR_OBJS)) ../libkismetdatasource.a
		$(CCLD) $(LDFLAGS) -o $(MONITOR_BIN) $(MONITOR_OBJS) ../libkismetdatasource.a $(DATASOURCE_LIBS) $(LIBUSBLIBS)

clean:
	@-rm -f $(MONITOR_BIN)
	@-rm -f *.o
	@-rm -f *.d

%.c.o:	%.c
%.c.o : %.c %.c.d
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LIBUSBCFLAGS) -c $*.c -o $@

%.c.d:	%.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $*.c | sed -e "s/\.o/\.c.o/" > $*.c.d

.PRECIOUS: %.c.d

include $(wildcard $(patsubst %c.o,%c.d,$(MONITOR_OBJS)))

