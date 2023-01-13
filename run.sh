#!/bin/bash

EXE=$1
shift
# ARGS=$@
ARGS="-c apu2 -p m162 -B 1200 -eU lfuse:w:0xff:m -Uhfuse:w:0xd7:m -U efuse:w:0xfb:m"

echo "Executing $EXE with args $ARGS"
${EXE} ${ARGS}

echo "Finished"
