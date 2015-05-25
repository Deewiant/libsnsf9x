CFLAGS   += -I in_xsf_framework -fPIC -Wall
CXXFLAGS += -I in_xsf_framework -fPIC --std=c++11 -Wall

SRCS := snsf9x/drvimpl.cpp snsf9x/snes9x/apu/apu.cpp snsf9x/snes9x/apu/SNES_SPC.cpp snsf9x/snes9x/apu/SNES_SPC_misc.cpp snsf9x/snes9x/apu/SPC_DSP.cpp snsf9x/snes9x/cpu.cpp snsf9x/snes9x/cpuexec.cpp snsf9x/snes9x/cpuops.cpp snsf9x/snes9x/dma.cpp snsf9x/snes9x/globals.cpp snsf9x/snes9x/memmap.cpp snsf9x/snes9x/ppu.cpp snsf9x/snes9x/sdd1.cpp xsfc/xsfdrv.c
OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCS)))

libsnsf9x.so: $(OBJS)
	$(CXX) -shared -o $@ $^ -lz

clean:
	rm -f libsnsf9x.so $(OBJS)

.PHONY: clean
