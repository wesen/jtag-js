Dwarf = {
  attributeAsString : function(attribute) {
    return "[" + attribute.name + "]";
  },

  dieAsString : function(die) {
    print("die " + die);
    var res =  " ".repeat(die.level) + "<" + die.tagName + " \"" + die.name + "\">\n";
    var child;
    var i;

    for (i = 0; i < die.attributes.length; i++) {
      res += " ".repeat(die.level) + "  " + this.attributeAsString(die.attributes[i]) + "\n";
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

