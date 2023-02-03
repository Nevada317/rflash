# GNU Makefile

PROG=imitator
CC?=gcc
CCLD?=gcc
STRIP?=strip

BUILD_DIR?=build

HEADS=$(shell find src -name "*.h")
SRCS=$(shell find src -name "*.c")
OBJS=$(patsubst src/%.c,${BUILD_DIR}/%.o,${SRCS})

CLEAR_TARGETS=${BUILD_DIR} build/
EXTRADEPS=Makefile

# Set warnings varbosity
CFLAGS  += -Wall
CFLAGS  += -Wextra

CFLAGS  += -pthread

# Add all search paths for headers on order of prefrence
CFLAGS += -Isrc/
CFLAGS += -I../src/

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
  DEFAULT_TARGET=${BUILD_DIR}/${BINARY}
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
  DEFAULT_TARGET=${BUILD_DIR}/${BINARY_UNSTRIPPED}
endif

BINARY=${PROG}
BINARY_UNSTRIPPED?=${BINARY}_prod_unstripped

${BUILD_DIR}/${BINARY}: ${BUILD_DIR}/${BINARY_UNSTRIPPED}
	@echo "Stripping main binary"
	mkdir -p $(dir $@)
	$(STRIP) -o $@ $<
	@echo -e "Done\n"

${BUILD_DIR}/${BINARY_UNSTRIPPED}: ${OBJS}
	@echo "Making main binary"
	mkdir -p $(dir $@)
	$(CC) ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}
	@echo -e "Done\n"

${BUILD_DIR}/%.o: src/%.c ${HEADS} ${EXTRADEPS}
	@echo "Making $@ form $<"
	mkdir -p $(dir $@)
	$(CC) ${CFLAGS} -c -o $@ $< ${LIBS}
	@echo -e "Done\n"

clean:
	rm -rf ${CLEAR_TARGETS}

# UNKNOWN TARGET HANDLER
unknown_target:
	@echo "WARNING: Build type flags not provided!"
	@echo "  DEBUG=0    - for release"
	@echo "  DEBUG=1    - for debug with gdb"
	@echo "Making release version with no debug info"
	@echo ""
	@DEBUG=0 $(MAKE) clean
	@DEBUG=0 $(MAKE)

DEFAULT_TARGET?=unknown_target
.DEFAULT_GOAL := $(DEFAULT_TARGET)
