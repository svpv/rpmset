RPM_OPT_FLAGS ?= -g -O2 -Wall
ARCH := $(shell arch)
TARGETS = scalar
MFLAGS1 =
ifeq ($(ARCH),i686)
TARGETS += sse4
MFLAGS1 = -m32
endif
ifeq ($(ARCH),x86_64)
TARGETS += sse4
endif
ifeq ($(ARCH),aarch64)
TARGETS += neon
endif
all: $(TARGETS:%=bench-base64unpack-%)
check: $(TARGETS:%=check-base64unpack-%)
MFLAGS2 =
%-sse4: MFLAGS2 = -msse4
WFLAGS = -Wextra
COMPILE = $(CC) $(RPM_OPT_FLAGS) $(WFLAGS) $(MFLAGS1) $(MFLAGS2)
bench-base64unpack-% : base64unpack-%.h base64.h base64.c base64pack.h bench-base64unpack.c
	$(COMPILE) -include $< base64.c bench-base64unpack.c -o $@
check-base64unpack-% : bench-base64unpack-%
	./$<
