CFLAGS   += -I in_xsf_framework -fPIC -fvisibility=hidden -Wall
CXXFLAGS += -I in_xsf_framework -fPIC --std=c++11 -fvisibility=hidden -Wall

SRCS := snsf9x/drvimpl.cpp snsf9x/snes9x/apu/apu.cpp snsf9x/snes9x/apu/SNES_SPC.cpp snsf9x/snes9x/apu/SNES_SPC_misc.cpp snsf9x/snes9x/apu/SPC_DSP.cpp snsf9x/snes9x/cpu.cpp snsf9x/snes9x/cpuexec.cpp snsf9x/snes9x/cpuops.cpp snsf9x/snes9x/dma.cpp snsf9x/snes9x/globals.cpp snsf9x/snes9x/memmap.cpp snsf9x/snes9x/ppu.cpp snsf9x/snes9x/sdd1.cpp xsfc/xsfdrv.c
OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCS)))

libsnsf9x.so: $(OBJS)
	$(CXX) -shared -o $@ $^ -lz

install: libsnsf9x.so
	install -Dm 755 -t "$(PREFIX)"/lib libsnsf9x.so
	install -Dm 644 -t "$(PREFIX)"/include/snsf9x snsf9x.h pversion.h
	install -Dm 644 -t "$(PREFIX)"/include/snsf9x/xsfc xsfc/tagget.h xsfc/xsfdrv.h

clean:
	rm -f libsnsf9x.so $(OBJS)

.PHONY: clean install
