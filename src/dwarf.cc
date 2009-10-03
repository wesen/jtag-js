#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <libelf.h>

#include <dwarf.h>
#include <libdwarf.h>

#include "dwarf.hh"

// #define DWARF_UNIMPLEMENTED() throw DwarfException()
#define DWARF_UNIMPLEMENTED(name) throw (name ": unimplemented")

DwarfFile::DwarfFile(const char *_filename) : filename(_filename) {
}

DwarfFile::~DwarfFile() {
}

jsval DwarfFile::dwarfLocationList(Dwarf_Die die) {
	DWARF_UNIMPLEMENTED("dwarfLocationList");
}

jsval DwarfFile::dwarfLocation(Dwarf_Loc *loc) {
	DWARF_UNIMPLEMENTED("dwarfLocation");
}

jsval DwarfFile::dwarfOpAddr(Dwarf_Unsigned opd) {
	DWARF_UNIMPLEMENTED("dwarfOpAddr");
}

jsval DwarfFile::dwarfOpSigned(Dwarf_Unsigned opd) {
	DWARF_UNIMPLEMENTED("dwarfOpSigned");
}

jsval DwarfFile::dwarfOpUnsigned(Dwarf_Unsigned opd) {
	DWARF_UNIMPLEMENTED("dwarfOpUnsigned");
}

jsval DwarfFile::dwarfOpBreg(Dwarf_Unsigned opd1, Dwarf_Unsigned opd2) {
	DWARF_UNIMPLEMENTED("dwarfOpBreg");
}

/* dwarf attribute */
jsval DwarfFile::dwarfAttribute(JSObject *parent, Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED("dwarfAttribute");
}

jsval DwarfFile::dwarfAttributeName(Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED("dwarfAttributeName");
}

jsval DwarfFile::dwarfAttributeValue(Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED("dwarfAttributeValue");
}

jsval DwarfFile::dwarfAttributeType(Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED("dwarfAttributeType");
}

/* different types of attribute values */
jsval DwarfFile::dwarfFormAddr(Dwarf_Attribute attrib) {            // addr (binary)
	DWARF_UNIMPLEMENTED("dwarfFormAddr");
}

jsval DwarfFile::dwarfFormRefAddr(Dwarf_Attribute attrib) {         // die offset
	DWARF_UNIMPLEMENTED("dwarfFormRefAddr");
}

jsval DwarfFile::dwarfFormRef(Dwarf_Attribute attrib) {            // <%lu>
	DWARF_UNIMPLEMENTED("dwarfFormRef");
}

jsval DwarfFile::dwarfFormBlock(Dwarf_Attribute attribute) {       // list of hex (formblock)
	DWARF_UNIMPLEMENTED("dwarfFormBlock");
}

jsval DwarfFile::dwarfFormData(Dwarf_Attribute attribute) {        // integer (signed / unsigned)
	DWARF_UNIMPLEMENTED("dwarfFormData");
}

jsval DwarfFile::dwarfFormSignedData(Dwarf_Attribute attribute) {  // signed integer
	DWARF_UNIMPLEMENTED("dwarfFormSignedData");
}

jsval DwarfFile::dwarfFormUnsignedData(Dwarf_Attribute attribute) { // unsigned integer
	DWARF_UNIMPLEMENTED("dwarfFormUnsignedData");
}
	
jsval DwarfFile::dwarfFormString(Dwarf_Attribute attribute) {       // string
	DWARF_UNIMPLEMENTED("dwarfFormString");
}


/* dwarf die */
jsval DwarfFile::dwarfDie(JSObject *parent, Dwarf_Die die) {
	DWARF_UNIMPLEMENTED("dwarfDie");
}

/* dwarf compilation unit */
jsval DwarfFile::dwarfCu(Dwarf_Die cu_die) {
	DWARF_UNIMPLEMENTED("dwarfCu");
}

/* single dwarf section */
jsval DwarfFile::dwarfElfHeader(Elf *elf) {
	DWARF_UNIMPLEMENTED("dwarfElfHeader");
}

/* whole dwarf file */
jsval DwarfFile::dwarfFile(){
	(void) elf_version(EV_NONE);
	if (elf_version(EV_CURRENT) == EV_NONE) {
		throw "libelf.a out of date";
	}

	int fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0) {
		throw "could not open file";
	}

	Elf_Cmd cmd;
	Elf *arf, *elf;
	jsval ret = JSVAL_VOID;

	try {
		cmd = ELF_C_READ;
		arf = elf_begin(fd, cmd, (Elf *)0);

		if (arf == NULL) {
			throw "could not read elf file";
		}

		try {
			while ((elf = elf_begin(fd, cmd, arf)) != 0) {
				Elf32_Ehdr *eh32;
				eh32 = elf32_getehdr(elf);
				if (!eh32) {
					throw "could not get ELF header";
				} else {
					try {
						dwarfElfHeader(elf);
					} catch (...) {
						elf_end(elf);
						throw;
					}
				}
				cmd = elf_next(elf);
			}
		} catch (...) {
			elf_end(arf);
			throw;
		}
		
	} catch (...) {
		close(fd);
		throw;
	}

	return ret;
	
}



JSBool myjs_readDwarf(JSContext*cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	const char *str;
	if (!JS_ConvertArguments(cx, argc, argv, "s", &str)) {
		return JS_FALSE;
	}

	DwarfFile dwarfFile(str);
	try {
		*rval = dwarfFile.dwarfFile();
		return JS_TRUE;
	} catch (const char *s) {
		JS_ReportError(cx, "readDwarf: %s", s);
		return JS_FALSE;
	}
}
	
