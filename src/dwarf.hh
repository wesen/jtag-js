#ifndef DWARF_H__
#define DWARF_H__

#include <stdexcept>
#include <exception>
#include <string>

#include "js.hh"

#include "dwarf.h"
#include "libdwarf.h"

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

	std::string filename;
	
public:
	DwarfFile(JSContext *_cx, const char *filename);
	~DwarfFile();

	jsval getSmallEncodingIntegerName();

	/**
	 * functions to convert a part of the DWARF tree
	 */

	/* location lists */
	jsval dwarfLocationList(Dwarf_Die die);
	jsval dwarfLocation(Dwarf_Loc *loc);
	jsval dwarfOpAddr(Dwarf_Unsigned opd);
	jsval dwarfOpSigned(Dwarf_Unsigned opd);
	jsval dwarfOpUnsigned(Dwarf_Unsigned opd);
	jsval dwarfOpBreg(Dwarf_Unsigned opd1, Dwarf_Unsigned opd2);

	/* dwarf attribute */
	jsval dwarfAttribute(JSObject *parent, Dwarf_Attribute attr);
	jsval dwarfAttributeName(Dwarf_Attribute attr);  
	jsval dwarfAttributeValue(Dwarf_Attribute attr);
	jsval dwarfAttributeType(Dwarf_Attribute attr);

	/* different types of attribute values */
	jsval dwarfFormAddr(Dwarf_Attribute attrib);            // addr (binary)
	jsval dwarfFormRefAddr(Dwarf_Attribute attrib);         // die offset 
	jsval dwarfFormRef(Dwarf_Attribute attrib);             // <%lu>
	jsval dwarfFormBlock(Dwarf_Attribute attribute);        // list of hex (formblock)
	jsval dwarfFormData(Dwarf_Attribute attribute);         // integer (signed / unsigned)
	jsval dwarfFormSignedData(Dwarf_Attribute attribute);   // signed integer
	jsval dwarfFormUnsignedData(Dwarf_Attribute attribute); // unsigned integer
	jsval dwarfFormString(Dwarf_Attribute attribute);       // string

	/* dwarf die */
	jsval dwarfDie(JSObject *parent, Dwarf_Die die);

	/* dwarf compilation unit */
	jsval dwarfCu(Dwarf_Die du_die);

	/* whole dwarf file */
	jsval dwarfElfHeader(Elf *elf);
	jsval dwarfFile();
};

JSBool myjs_readDwarf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

#endif /* DWARF_H__ */
