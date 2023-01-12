#!/bin/bash

EXE=$1
shift
ARGS=$@

echo "Executing $EXE with args $ARGS"
${EXE} ${ARGS}

echo "Finished"
