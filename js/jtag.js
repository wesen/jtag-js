avr = {
    RAM_ADDR: 0x800000
};

function ram(x) {
    return x + avr.RAM_ADDR;
}

function samplePC() {
    jtag.interrupt();
    var pc = jtag.getPC();
    jtag.resume();
    return pc;
}