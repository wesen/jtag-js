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
#define DWARF_UNIMPLEMENTED() throw "unimplemented"

DwarfFile::DwarfFile(const char *_filename) : filename(_filename) {
}

DwarfFile::~DwarfFile() {
}

jsval DwarfFile::dwarfLocationList(Dwarf_Die die) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfLocation(Dwarf_Loc *loc) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfOpAddr(Dwarf_Unsigned opd) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfOpSigned(Dwarf_Unsigned opd) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfOpUnsigned(Dwarf_Unsigned opd) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfOpBreg(Dwarf_Unsigned opd1, Dwarf_Unsigned opd2) {
	DWARF_UNIMPLEMENTED();
}

/* dwarf attribute */
jsval DwarfFile::dwarfAttribute(JSObject *parent, Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfAttributeName(Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfAttributeValue(Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfAttributeType(Dwarf_Attribute attr) {
	DWARF_UNIMPLEMENTED();
}

/* different types of attribute values */
jsval DwarfFile::dwarfFormAddr(Dwarf_Attribute attrib) {            // addr (binary)
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfFormRefAddr(Dwarf_Attribute attrib) {         // die offset
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfFRef(Dwarf_Attribute attrib) {            // <%lu>
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfFormBlock(Dwarf_Attribute attribute) {       // list of hex (formblock)
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfFormData(Dwarf_Attribute attribute) {        // integer (signed / unsigned)
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfFormSignedData(Dwarf_Attribute attribute) {  // signed integer
	DWARF_UNIMPLEMENTED();
}

jsval DwarfFile::dwarfFormUnsignedData(Dwarf_Attribute attribute) { // unsigned integer
	DWARF_UNIMPLEMENTED();
}
	
jsval DwarfFile::dwarfFormString(Dwarf_Attribute attribute) {       // string
	DWARF_UNIMPLEMENTED();
}


/* dwarf die */
jsval DwarfFile::dwarfDie(JSObject *parent, Dwarf_Die die) {
	DWARF_UNIMPLEMENTED();
}

/* dwarf compilation unit */
jsval DwarfFile::dwarfCu(Dwarf_Die cu_die) {
	DWARF_UNIMPLEMENTED();
}

/* whole dwarf file */
jsval DwarfFile::dwarfFile(){
	DWARF_UNIMPLEMENTED();	
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
		JS_ReportError(cx, "%s", s);
		return JS_FALSE;
	}
}
	
