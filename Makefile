# GNU Makefile

PROG=rflash
CC?=gcc
CCLD?=gcc
STRIP?=strip

HEADS=$(shell find . -name "*.h")
SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %.c,%.o,${SRCS})

BINARY=${PROG}
BINARY_UNSTRIPPED=${PROG}_unstripped
CLEAR_TARGETS=${OBJS} ${BINARY}
EXTRADEPS=Makefile

$(info Found headers: ${HEADS})
$(info Found sources: ${SRCS})
$(info Target objects: ${OBJS})
$(info )

# Set warnings varbosity
CFLAGS  += -Wall
CFLAGS  += -Wextra

# Optimizer configuration
CFLAGS += -O3

# Generate lots of debug info
CFLAGS += -g
CFLAGS += -ggdb
CFLAGS += -gdwarf

# Add all search paths for headers on order of prefrence
CFLAGS += -I.
CFLAGS += -Isrc/
CFLAGS += -Iihex/

# Optimize-out unused functions
CFLAGS  += -fdata-sections
CFLAGS  += -ffunction-sections
LDFLAGS += -Wl,--gc-sections

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
