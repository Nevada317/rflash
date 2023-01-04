# GNU Makefile

PROG=rflash
CC?=gcc
CCLD?=gcc
STRIP?=strip

HEADS=$(shell find . -name "*.h")
SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %.c,%.o,${SRCS})

CLEAR_TARGETS=${OBJS} ${BINARY} ${BINARY}_*
EXTRADEPS=Makefile

# Set warnings varbosity
CFLAGS  += -Wall
CFLAGS  += -Wextra

# Add all search paths for headers on order of prefrence
CFLAGS += -I.
CFLAGS += -Isrc/
CFLAGS += -Iihex/

# Optimize-out unused functions
CFLAGS  += -fdata-sections
CFLAGS  += -ffunction-sections
LDFLAGS += -Wl,--gc-sections

ifeq "$(DEBUG)" "0"
  # Optimizer configuration
  $(info ### PRODUCTION mode. Optimizer enabled)
  $(info )
  CFLAGS += -O3
  CFLAGS += -flto
  DEBUG := 0
  PROF=0
  DEFAULT_TARGET=${BINARY}
endif

ifeq "$(PROF)" "prof"
  # Generate lots of debug info
  $(info ### PROFILER: prof)
  $(info )
  CFLAGS += -p
  DEBUG := 0
  BINARY_UNSTRIPPED=${PROG}_prof
  DEFAULT_TARGET=${BINARY_UNSTRIPPED}
endif
ifeq "$(PROF)" "gprof"
  # Generate lots of debug info
  $(info ### PROFILER: gprof)
  $(info )
  CFLAGS += -pg
  DEBUG := 0
  BINARY_UNSTRIPPED=${PROG}_prof
  DEFAULT_TARGET=${BINARY_UNSTRIPPED}
endif
ifeq "$(PROF)" "arcs"
  # Generate lots of debug info
  $(info ### PROFILER: prof)
  $(info )
  CFLAGS += -fprofile-arcs
  DEBUG := 0
  BINARY_UNSTRIPPED=${PROG}_arcs
  DEFAULT_TARGET=${BINARY_UNSTRIPPED}
endif

ifeq "$(DEBUG)" "1"
  # Generate lots of debug info
  $(info ### DEBUG INFO enabled)
  $(info )
  $(info Found headers: ${HEADS})
  $(info Found sources: ${SRCS})
  $(info Target objects: ${OBJS})
  $(info )
  CFLAGS += -g
  CFLAGS += -ggdb
  CFLAGS += -gdwarf
  BINARY_UNSTRIPPED=${PROG}_debug
  DEFAULT_TARGET=${BINARY_UNSTRIPPED}
endif

BINARY=${PROG}
BINARY_UNSTRIPPED?=${BINARY}_prod_unstripped

${BINARY}: ${BINARY_UNSTRIPPED}
	@echo "Stripping main binary"
	$(STRIP) -o $@ $<
	@echo -e "Done\n"

${BINARY_UNSTRIPPED}: ${OBJS}
	@echo "Making main binary"
	$(CC) ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}
	@echo -e "Done\n"

%.o: %.c ${HEADS} ${EXTRADEPS}
	@echo "Making $@ form $<"
	$(CC) ${CFLAGS} -c -o $@ $< ${LIBS}
	@echo -e "Done\n"

clean:
	rm -f ${CLEAR_TARGETS}

# UNKNOWN TARGET HANDLER
unknown_target:
	@echo "WARNING: Build type flags not provided!"
	@echo "  DEBUG=0    - for release"
	@echo "  DEBUG=1    - for debug with gdb"
	@echo "  PROF=arcs  - for profiling"
	@echo "  PROF=prof  - for profiling"
	@echo "  PROF=gprof - for profiling"
	@echo "Making release version with no debug info"
	@echo ""
	@DEBUG=0 $(MAKE) clean
	@DEBUG=0 $(MAKE)

DEFAULT_TARGET?=unknown_target
.DEFAULT_GOAL := $(DEFAULT_TARGET)
