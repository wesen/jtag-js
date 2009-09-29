Number.prototype.hex = function () {
	return "0x" + this.toString(16);
};

// for debugging, reload the whole library
function rel() {
	load("../js/jtag.js");
}
