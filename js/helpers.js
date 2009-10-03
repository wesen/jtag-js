// helper functions

Number.prototype.hex = function () {
	return "0x" + this.toString(16);
};

// for debugging, reload the whole library
function rel() {
	load("../js/jtag.js");
}

// object stuff

Function.prototype.method = function(name, func) {
  this.prototype[name] = func;
  return this;
};

Object.method('beget', function (o) {
  var F = function () {};
  F.prototype = o;
  return new(F);
});

Function.method('inherits', function (Parent) {
  this.prototype = new Parent(  );
  return this;
});


String.method('repeat', function (reps) {
  var res = "";
  var i;
  
  for (i = 0; i < reps; i++) {
    res += this;
  }
  return res;
});