Dwarf = {
  valueAsString : function (type, value) {
    switch (type) {
      case "addr":
      return value.hex();

      default:
      return value;
    }
  },
  
  attributeAsString : function(attribute) {
    var res = "[" + attribute.name + " (o:" + attribute.offset + ") ]: ";
    var valString = (attribute.value ? (this.valueAsString(attribute.value.formName,
                                                           attribute.value.value) +
                                        " (" + attribute.value.formName + ", " +
                                        attribute.value.form + ")") : "undefined");
    res += (" ".repeat(25 - res.length)) + valString;
    return res;
  },

  dieAsString : function(die) {
    print("die " + die);
    var res =   "  ".repeat(die.level) + "<" + die.tagName + " \"" + die.name +
      "\" (o: " + die.offset + ")>\n";  var file = readElf("test.avr.elf");

    var child;
    var i;

    for (i = 0; i < die.attributes.length; i++) {
      res += " ".repeat(10) + "  " + this.attributeAsString(die.attributes[i]) + "\n";
    }
    res += "\n";
    
    for (i = 0; i < die.children.length; i++) {
      child = die.children[i];
      res += this.dieAsString(child);
    }

    return res;
  },

  fileAsString : function (file) {
    var res;
    var die;
    var elf;
    var i, j;

    res = file.filename + "\n";
    for (i = 0; i < file.elf.length; i++) {
      elf = file.elf[i];
      for (j = 0; j < elf.length; j++) {
        die = elf[j];
        res += this.dieAsString(die);
      }
    }
    return res;
  }

};

