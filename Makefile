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
STD = -std=gnu11 -D_GNU_SOURCE
COMPILE = $(CC) $(STD) $(RPM_OPT_FLAGS) $(WFLAGS) $(DFLAGS) $(MFLAGS1) $(MFLAGS2)
bench-base64unpack-% : bench-base64unpack.c base64.h base64.c \
		base64pack.h base64unpack-scalar.h base64unpack-simd.h
	$(COMPILE) -include $(INC1) base64.c bench-base64unpack.c -o $@
check-base64unpack-% : bench-base64unpack-%
	./$<
clean:
	rm -f bench-base64unpack-* bench-unmet-*
dump-rpmsetcmp.so: dump-rpmsetcmp.c
	$(COMPILE) -fpic -shared $< -o $@
conv: base64unpack-scalar.h conv.c base64.c setstring-enc.c setstring-dec.c
	$(COMPILE) -Wno-override-init -include $^ -o $@
orig.setcmp.zst: dump-rpmsetcmp.so
	apt-cache unmet &>/dev/null
	LD_PRELOAD=$$PWD/dump-rpmsetcmp.so \
	apt-cache unmet |awk '/^set:/&&NF==2' |zstd -11 >$@
conv.setcmp.zst: orig.setcmp.zst conv
	zstd -d <$< |./conv |zstd -11 >$@
bench: $(TARGETS:%=bench-unmet-%) $(TARGETS:%=runbench-unmet-%)
runbench-% : bench-% conv.setcmp.zst
	zstd -d <conv.setcmp.zst |./$<
bench-unmet-% : bench-unmet.c base64.h base64.c \
		base64unpack-scalar.h base64unpack-simd.h \
		setstring.h setstring-dec.c \
		conv.setcmp.zst
	$(COMPILE) -include $(INC1) $< setstring-dec.c base64.c -o $@ -lt1ha
