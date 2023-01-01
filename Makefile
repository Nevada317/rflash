# GNU Makefile

PROG=rflash
CC?=gcc
CCLD?=gcc

HEADS=$(shell find . -name "*.h")
SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %.c,%.o,${SRCS})

BINARY=${PROG}
CLEAR_TARGETS=${OBJS} ${BINARY}
EXTRADEPS=Makefile

$(info Found headers: ${HEADS})
$(info Found sources: ${SRCS})
$(info Target objects: ${OBJS})
$(info )

CFLAGS+="-Wall"

${BINARY}: ${OBJS}
	@echo "Making main binary"
	$(CC) ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LIBS}
	@echo -e "Done\n"

%.o: %.c ${HEADS} ${EXTRADEPS}
	@echo "Making $@ form $<"
	$(CC) ${CFLAGS} -c -o $@ $< ${LIBS}
	@echo -e "Done\n"

clean:
	rm -f ${CLEAR_TARGETS}
