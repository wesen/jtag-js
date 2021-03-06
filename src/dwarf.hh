#ifndef DWARF_H__
#define DWARF_H__

#include <stdexcept>
#include <exception>
#include <string>

#include "js.hh"

#include "dwarf.h"
#include "libdwarf.h"

#include "dwarfnaming.hh"

#if 0
class DwarfException : public std::runtime_error {
public:
	// 	std::string s;
	
	DwarfException() : std::runtime_error("DwarfException") {
	}
};
#endif

class DwarfFile {
protected:
	Dwarf_Debug dbg;
	Dwarf_Error error;
	Dwarf_Die die;
	JSContext *cx;
	JSObject *obj;

	JSObject *fileObj;
	JSObject *cuObj;
	JSObject *dieObj;
	JSObject *attrObj;

	char **srcfiles;
	Dwarf_Signed fileCnt;

	std::string filename;
	
public:
	DwarfFile(JSContext *_cx, JSObject *_obj, const char *filename);
	~DwarfFile();

	jsval getSmallEncodingIntegerName(Dwarf_Attribute attr,
																		const char *attr_name,
																		encoding_type_func val_as_string);

	/**
	 * functions to convert a part of the DWARF tree
	 */

	/* location lists */
	jsval dwarfLocationList(Dwarf_Attribute attr);
	jsval dwarfLocDesc(Dwarf_Locdesc *loc);
	jsval dwarfLocation(Dwarf_Loc *loc);

	/* dwarf attribute */
	jsval dwarfFormXData(Dwarf_Attribute attr);
	jsval dwarfFormDataValue(Dwarf_Attribute attr);
	jsval dwarfFormValue(Dwarf_Attribute attr);
	jsval dwarfAttribute(JSObject *parent, Dwarf_Half tag, Dwarf_Attribute attr);
	jsval dwarfAttributeValue(Dwarf_Attribute attr);

	/* dwarf lines */
	jsval dwarfLine(Dwarf_Line line);
	
	/* dwarf die */
	void dwarfDieData(Dwarf_Die die);
	void dwarfDieLines(Dwarf_Die die);
	jsval dwarfDie(JSObject *parent, Dwarf_Die die, int level = 0);

	/* dwarf compilation unit */
	jsval dwarfCu(Dwarf_Die cu_die);

	/* whole dwarf file */
	jsval dwarfElfHeader(Elf *elf);
	jsval dwarfFile();
};

JSBool myjs_readDwarf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

#endif /* DWARF_H__ */
