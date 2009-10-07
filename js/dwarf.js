var DwarfNode = Class.create ({
  initialize: function(hash, parent, file) {
    Object.extend(this, hash);
    this.parent = parent;
    this.file = file || this.parent && this.parent.file;
    
    this.attributes = this.attributes.map(
      function (x) {
        return new DwarfAttribute(x, this);
      });

    this.children = this.children.map(
      function (x) {
        return new DwarfTag(x, this);
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

  getAttribute: function(name) {
      return this.getAttributes().find(function (x) { return x.name === name; });
  },

  getAttributeValue: function(name) {
    var attr = this.getAttribute(name);
    return attr && attr.value;
  },
  
  getChildren: function() {
    return this.children;
  },

  getNodeWithOffset: function(offset) {
    var child;

    if (this.offset && this.offset === offset) {
      return this;
    } else {
      return this.getChildren().map(function (x) {
        return x.getNodeWithOffset(offset);
      }).without(undefined)[0];
    }
  },

  getAttributeWithOffset: function(offset) {
    var res = this.getAttributes().find(fieldCompare("offset", offset));
    if (res) {
        return res;
    } else{
      return this.getChildren().map(function (x) {
        return x.getAttributeWithOffset(offset);
      }).without(undefined)[0];
    }
  },

  getObjectWithOffset: function(offset) {
    return this.getNodeWithOffset(offset) ||
      this.getAttributeWithOffset(offset);
  },
  
  matchesPC: function(pc2) {
    var low_pc = this.getAttributeValue("low_pc");
    var high_pc = this.getAttributeValue("high_pc");
    var pc = this.getAttributeValue("pc");
    
    if (low_pc && high_pc) {
      return ((pc2 >= low_pc) && (pc2 <= high_pc));
    } else if (pc) {
      return pc === pc2;
    } else {
      return false;
    }
  },

  /* pretty print */
  toString: function () {
    return "<" + this.tagName + " \"" + this.name + "\">";
  },
  
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
  initialize: function(hash, parent){
    Object.extend(this, hash);
    this.parent = parent;
    this.file = this.parent && this.parent.file;
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
  },

  toString: function () {
    return "("  + this.prettyPrint() + ")";
  }
  
});

var DwarfRefValue = Class.create(DwarfValue, {
  getReferencedObject: function (){
    return this.file && this.file.getObjectWithOffset(this.value);
  },

  prettyPrint: function () {
    return "*" + this.getReferencedObject().toString();
  }
});

var DwarfAttribute = Class.create ({
  initialize: function(hash, parent) {
    Object.extend(this, hash);
    this.parent = parent;
    this.file = this.parent && this.parent.file;
    if (this.value && this.value.formName && (this.value.formName.substr(0, 3) === "ref")) {
      this.value = new DwarfRefValue(this.value, this);
    } else{
      this.value = new DwarfValue(this.value, this);
    }
  },
  
  prettyPrint: function() {
    return this.name + ": " + this.value.prettyPrint();
  },
  
  toString: function() {
    return "{" + this.prettyPrint() + "}";
  },

  getReferencedObject: function (){
    return this.value.getReferencedObject();
  }
  
});

var DwarfTag = Class.create (DwarfNode, {
});

var DwarfDie = Class.create(DwarfNode, {
});

var DwarfCU = Class.create(DwarfNode, {
  
  getChildrenByType: function (type) {
    return this.getChildren().filter(fieldCompare("tagName", type));
  },

  getChildrenByName: function (name) {
    return this.getChildren().filter(fieldCompare("name", name));
  },
  
  getBaseTypes: function () {
    return this.getChildrenByType("base_type");
  },
  
  getSubPrograms: function () {
    return this.getChildrenByType("subprogram");
  },

  getSubPrograms: function () {
    return this.getChildrenByType("subprogram");
  },

  getLineForPC: function (pc) {
    var line = undefined;
    var i;
    for (i = 0; i < this.lines.length; i++) {
      if (pc < this.lines[i].pc)
        break;
      else
        line = this.lines[i];
    }
    return line;
  },

  getPCForLine: function (line) {
    return this.lines[line].pc;
  }
  
});

var Dwarf = Class.create ({
  initialize: function(filename) {
    this.filename = filename;
    this.fileData = readElf(filename);
    var elfSections = this.fileData.elf;

    this.cus = elfSections.flatten().map(function (x) { return new DwarfCU(x, this, this); });
  },

  getCus: function () {
    return this.cus;
  },

  getCu: function (name) {
    return this.getCus().find(fieldCompare("name", name));
  },

  prettyPrint: function () {
    var res = "ELF file " + this.filename + "\n";
    this.getCus().each(function (x) {
      res += x.prettyPrint();
    });
    return res;
  },

  toString: function () {
    return "[ elfFile \"" + this.filename + "\"]";
  },

  getSubPrograms: function () {
    return this.getCus().map(
      function (x) {
        return x.getSubPrograms();
      }).flatten();
  },

  getSubProgram: function (name) {
    return this.getCus().map(objBinder("getSubPrograms")).flatten();
  },

  getNodeWithOffset: function(offset) {
    return this.getCus().map(function (x) {
      return x.getNodeWithOffset(offset);
    }).without(undefined)[0];
  },

  getAttributeWithOffset: function(offset) {
    return this.getCus().map(function (x) {
      return x.getAttributeWithOffset(offset);
    }).without(undefined)[0];
  },

  getObjectWithOffset: function(offset) {
    return this.getCus().map(function (x) {
      return x.getObjectWithOffset(offset);
    }).without(undefined)[0];
  },
  

  getLineForPC: function (pc) {
    getCus().each(
      function (x) {
        var line = x.getLineForPC(pc);
        if (line) {
          return line;
        }
        return undefined;
      });
  }
});

