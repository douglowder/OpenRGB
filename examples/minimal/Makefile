#-----------------------------------------------------------------------------#
# Builds the minimal OpenRGB consumer against an installed prefix.            #
#                                                                             #
#   make                        use openrgb from the default pkg-config path  #
#   make PREFIX=/tmp/staged     use a staged install                          #
#-----------------------------------------------------------------------------#
PREFIX ?=

ifeq ($(PREFIX),)
  CFLAGS  := $(shell pkg-config --cflags openrgb)
  LDFLAGS := $(shell pkg-config --libs openrgb)
else
  CFLAGS  := -std=c++17 -I$(PREFIX)/include -I$(PREFIX)/include/openrgb/internal \
             $(shell pkg-config --cflags hidapi)
  LDFLAGS := -L$(PREFIX)/lib -lopenrgb -Wl,-rpath,$(PREFIX)/lib
endif

list_devices: list_devices.cpp
	$(CXX) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f list_devices

.PHONY: clean
