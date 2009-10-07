// helper functions

Number.prototype.hex = function () {
	return "0x" + this.toString(16);
};

// for debugging, reload the whole library
function rel() {
	load("../js/jtag.js");
}



function objBinder(funcname) {
    return function (x) { return x[funcname](); };
}

function fieldCompare(field, val) {
    return function (x) { return x[field] == val; };
}

