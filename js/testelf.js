print("reading elf file test.avr.elf\n");

function test() {
  load("../js/jtag.js");
  var file = readElf("test.avr.elf");
  print(Dwarf.fileAsString(file));
}

file = readElf("test.avr.elf");