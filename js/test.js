jtag.init();
jtag.addBreakpoint(0, 1, 1);
print(jtag.getBreakpoints())
