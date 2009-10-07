var DwarfNode = Class.create ({
  initialize: function(cuhash) {
    Object.extend(this, cuhash);
    this.attributes = this.attributes.map(
      function (x) {
        return new DwarfAttribute(x);
      });

    this.children = this.children.map(
      function (x) {
        return new DwarfTag(x);
      });
  },

  /* accessors */
  getTagName: function () {
    return this.tagName;
  },

  getName: function () {
    return this.name;
  },

  getLines: function () {
    return this.lines;
  },

  getDies: function() {
    return this.children;
  },

  getAttributes: function() {
    return this.attributes;
  },

  getChildren: function() {
    return this.children;
  },

  /* pretty print */
  prettyPrint : function() {
    var res =   "  ".times(this.level) + "<" + this.tagName + " \"" + this.name +
      "\" (o: " + this.offset + ")>\n";

    var child;
    var i;

    for (i = 0; i < this.attributes.length; i++) {
      res += " ".times(10) + "  " + (this.attributes[i].prettyPrint()) + "\n";
    }
    res += "\n";
    
    for (i = 0; i < this.children.length; i++) {
      child = this.children[i];
      res += child.prettyPrint();
    }

    return res;
  }
  
});

var DwarfValue = Class.create ({
  initialize: function(hash){
      Object.extend(this, hash);
  },

  getType: function () {
    return this.formName;
  },

  getDirectType: function () {
    return this.directFormName;
  },

  getValue: function() {
    return this.value;
  },

  prettyPrint: function () {
    var value = this.getValue();
    return ((value && (typeof value.prettyPrint === "function")) ?
            value.prettyPrint() :
            value)
      + " (" + this.getType() + ")";
  }
});

var DwarfAttribute = Class.create (
  {
  initialize: function(hash) {
    Object.extend(this, hash);
    this.value = new DwarfValue(this.value);
  },

  prettyPrint: function() {
    return this.name + ": " + this.value.prettyPrint();
  }
});

var DwarfTag = Class.create (DwarfNode, {
});

var DwarfDie = Class.create(DwarfNode, {
});

var DwarfCU = Class.create(DwarfNode, {
});

var Dwarf = Class.create ({
  initialize: function(filename) {
    this.fileData = readElf(filename);
    var elfSections = this.fileData.elf;

    this.cus = elfSections.flatten().map(function (x) { return new DwarfCU(x); });
  },

  getCus: function () {
    return this.cus;
  },

  valueAsString : function (type, value) {
    switch (type) {
     case "addr":
      return value.hex();
      
    default:
      return value;
    }
  },
  
  attributeAsString : function(attribute) {
    var res = "[" + attribute.name + " " + attribute.num + " (o:" + attribute.offset + ") ]: ";
    var valString = (attribute.value ? (this.valueAsString(attribute.value.formName,
                                                           attribute.value.value) +
                                        " (" + attribute.value.formName + ", " +
                                        attribute.value.form + ")") : "undefined");
    res += (" ".times(25 - res.length)) + valString;
    return res;
  },

  dieAsString : function(die) {
    print("die " + die);
    var res =   "  ".times(die.level) + "<" + die.tagName + " \"" + die.name +
      "\" (o: " + die.offset + ")>\n";  var file = readElf("test.avr.elf");

    var child;
    var i;

    for (i = 0; i < die.attributes.length; i++) {
      res += " ".times(10) + "  " + this.attributeAsString(die.attributes[i]) + "\n";
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

});

