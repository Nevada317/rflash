set args -c apu2 xxxx yyyy -p m162 -X -B 1200 -eU lfuse:w:0xff:m -Uhfuse:w:0xd7:m -U efuse:w:0xfb:m -U flash:w:test.hex:i -U flash:w:patch.hex:i
b _formal_breakpoint
commands
  p Executor
  continue
end
