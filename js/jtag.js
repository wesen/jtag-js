load("helpers.js");
load("prototype.js");
load("dwarf.js");
load("repl.js");

// avr specific functions

avr = {
  FLASH_SPACE_OFFSET:      0x000000,
  RAM_SPACE_OFFSET:        0x800000,
  EEPROM_SPACE_OFFSET:     0x810000,
  FUSE_SPACE_OFFSET:       0x820000,
  LOCK_SPACE_OFFSET:       0x830000,
  SIG_SPACE_OFFSET:        0x840000,
  BREAKPOINT_SPACE_OFFSET: 0x900000,

  LOW_FUSE      : 0,
  HIGH_FUSE     : 1,
  EXTENDED_FUSE : 2,

  BREAKPOINT_NONE : 0,
  BREAKPOINT_CODE : 1,
  BREAKPOINT_SOFTCODE : 2,
  BREAKPOINT_WRITE_DATA : 3,
  BREAKPOINT_READ_DATA : 4,
  BREAKPOINT_ACCESS_DATA : 5
};

avr.init = function () {
  avr.ADDR_SPACE_MASK = 
    avr.FLASH_SPACE_OFFSET |
    avr.RAM_SPACE_OFFSET   |
    avr.EEPROM_SPACE_OFFSET |
    avr.FUSE_SPACE_OFFSET   |
    avr.LOCK_SPACE_OFFSET   |
    avr.SIG_SPACE_OFFSET    |
    avr.BREAKPOINT_SPACE_OFFSET;
};

avr.ram = function (x) {
  return x + avr.RAM_SPACE_OFFSET;
}

avr.fuse = function (x) {
  return x + avr.FUSE_SPACE_OFFSET;    
}

// jtag helper functions

jtag.samplePC = function () {
  jtag.interrupt();
  var pc = jtag.getPC();
  jtag.resume();
      return pc;
};

jtag.writeFuse = function (fuse, fuseValue) {
  return jtag.write(avr.fuse(fuse), [ fuseValue ]);
}

jtag.writeHighFuse = function (value) {
  return jtag.writeFuse(avr.HIGH_FUSE, value);
};

jtag.writeLowFuse = function (value) {
  return jtag.writeFuse(avr.LOW_FUSE, value);
};

jtag.writeExtendedFuse = function (value) {
  return jtag.writeFuse(avr.EXTENDED_FUSE, value);
};
