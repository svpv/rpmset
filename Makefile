RPM_OPT_FLAGS ?= -g -O2 -Wall
ARCH := $(shell arch)
AVX2 := $(shell grep -wc avx2 /proc/cpuinfo)
TARGETS = scalar
MFLAGS1 =
ifeq ($(ARCH),i686)
TARGETS += sse4
MFLAGS1 = -m32
endif
ifeq ($(ARCH),x86_64)
TARGETS += sse4
endif
ifneq ($(AVX2),0)
TARGETS += avx2 zen1
endif
ifeq ($(ARCH),aarch64)
TARGETS += neon
endif
all: $(TARGETS:%=bench-base64unpack-%)
check: $(TARGETS:%=check-base64unpack-%)
MFLAGS2 =
%-sse4: MFLAGS2 = -msse4
%-avx2: MFLAGS2 = -mavx2
%-zen1: MFLAGS2 = -march=znver1
INC1 = base64unpack-simd.h
%-scalar: INC1 = base64unpack-scalar.h
WFLAGS = -Wextra
DFLAGS =
COMPILE = $(CC) $(RPM_OPT_FLAGS) $(WFLAGS) $(DFLAGS) $(MFLAGS1) $(MFLAGS2)
bench-base64unpack-% : bench-base64unpack.c base64.h base64.c \
		base64pack.h base64unpack-scalar.h base64unpack-simd.h
	$(COMPILE) -include $(INC1) base64.c bench-base64unpack.c -o $@
check-base64unpack-% : bench-base64unpack-%
	./$<
clean:
	rm -f bench-base64unpack-*
