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

#include "dwarfnaming.hh"
#include "dwarf.hh"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// #define DWARF_UNIMPLEMENTED() throw DwarfException()
#define DWARF_UNIMPLEMENTED(name) \
	throw (name ": unimplemented in file " __FILE__ " at line " TOSTRING(__LINE__))

#define DWARF_CHECK(res, str)																		\
	if ((res) == DW_DLV_ERROR) throw (str ": got dwarf error in file " __FILE__ \
																		" at line " TOSTRING(__LINE___))


DwarfFile::DwarfFile(JSContext *_cx, JSObject *_obj, const char *_filename) :
	cx(_cx), obj(_obj), filename(_filename) {
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

void DwarfFile::dwarfDieData(JSObject *dieObj, Dwarf_Die die) {
	char *name;
	int res = dwarf_diename(die, &name, &error);
	DWARF_CHECK(res, "dwarf_diename");
	
	if (res == DW_DLV_OK) {
		jsval nameVal = JS_NEW_STRING_VAL(name);
		dwarf_dealloc(dbg, name, DW_DLA_STRING);
		JS_SetProperty(cx, dieObj, "name", &nameVal);
	}

	Dwarf_Half tag;
	res = dwarf_tag(die, &tag, &error);
	DWARF_CHECK(res, "dwarf_tag");
	if (res == DW_DLV_OK) {
		jsval tagVal = INT_TO_JSVAL(tag);
		JS_SetProperty(cx, dieObj, "tag", &tagVal);
		
		const char *tagname = get_TAG_name(tag);
		jsval tagnameVal = JS_NEW_STRING_VAL(tagname);
		JS_SetProperty(cx, dieObj, "tagName", &tagnameVal);
	}

	JSObject *attrArrayObj = JS_NewArrayObject(cx, 0, NULL);
	jsval attrArrayVal = OBJECT_TO_JSVAL(attrArrayObj);
	JS_SetProperty(cx, dieObj, "attributes", &attrArrayVal);

	Dwarf_Attribute *attrs;
	Dwarf_Signed cnt;
	res = dwarf_attrlist(die, &attrs, &cnt, &error);
	DWARF_CHECK(res, "dwarf_attrlist");
	try {
		for (int i = 0; i < cnt; i++) {
			try {
				jsval attrVal = dwarfAttribute(dieObj, attrs[i]);
				JS_SetElement(cx, dieObj, i, &attrVal);
			} catch (const char *s) {
				jsval val = JS_NEW_STRING_VAL(s);
				JS_SetElement(cx, dieObj, i, &val);
				dwarf_dealloc(dbg, attrs[i], DW_DLA_ATTR);
			}
			dwarf_dealloc(dbg, attrs[i], DW_DLA_ATTR);
		}
	} catch(...) {
		dwarf_dealloc(dbg, attrs, DW_DLA_LIST);
		throw;
	}

	dwarf_dealloc(dbg, attrs, DW_DLA_LIST);
}

void DwarfFile::dwarfDieLines(JSObject *dieObj, Dwarf_Die die) {
	//	DWARF_UNIMPLEMENTED("dwarfDieLines");
}

/* dwarf die */
jsval DwarfFile::dwarfDie(JSObject *parent, Dwarf_Die in_die, int level) {
	//	DWARF_UNIMPLEMENTED("dwarfDie");
	JSObject *dieObj = JS_NewObject(cx, NULL, NULL, NULL);
	jsval parentVal = OBJECT_TO_JSVAL(parent);
	JS_SetProperty(cx, dieObj, "parent", &parentVal);
	jsval levelVal = INT_TO_JSVAL(level);
	JS_SetProperty(cx, dieObj, "level", &levelVal);
	JSObject *childArrayObj = JS_NewArrayObject(cx, 0, NULL);
	jsval childArrayVal = OBJECT_TO_JSVAL(childArrayObj);
	JS_SetProperty(cx, dieObj, "children", &childArrayVal);

	dwarfDieData(dieObj, in_die);
	dwarfDieLines(dieObj, in_die);

	/* get child */
	Dwarf_Die child;
	int res = dwarf_child(in_die, &child, &error);
	DWARF_CHECK(res, "dwarf_child");
	if (res == DW_DLV_OK) {
		try {
			jsval val = dwarfDie(dieObj, child, level + 1);
			int i = 0;
			JS_SetElement(cx, childArrayObj, i, &val);

			i++;
			Dwarf_Die cur_die = child;
			for (;;i++) {
				Dwarf_Die sib_die;
				
				int res = dwarf_siblingof(dbg, cur_die, &sib_die, &error);
				DWARF_CHECK(res, "dwarf_siblingof");
				try {
					if (res == DW_DLV_OK) {
						jsval dieVal = dwarfDie(dieObj, sib_die, level);
						JS_SetElement(cx, childArrayObj, i, &dieVal);
					} else if (res == DW_DLV_NO_ENTRY) {
						break;
					}
				} catch (...) {
					if (cur_die != child) {
						dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
					}
					throw;
				}
				
				if (cur_die != child) {
					dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
				}
				
				cur_die = sib_die;
			}
		} catch (...) {
			dwarf_dealloc(dbg, child, DW_DLA_DIE);
			throw;
		}
		dwarf_dealloc(dbg, child, DW_DLA_DIE);
	}

	/* walk siblings */
	return OBJECT_TO_JSVAL(dieObj);
}

/* dwarf compilation unit */
jsval DwarfFile::dwarfCu(Dwarf_Die in_die) {
	return dwarfDie(NULL, in_die);
}

/* single dwarf section */
jsval DwarfFile::dwarfElfHeader(Elf *elf) {
	int res = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &error);
	DWARF_CHECK(res, "dwarf_elf_init");

	JSObject *cuArrayObj = JS_NewArrayObject(cx, 0, NULL);
	
	for (int cu_number = 0; ; cu_number++) {
		Dwarf_Die no_die = 0;
		Dwarf_Die cu_die = 0;

		Dwarf_Unsigned cu_header_length;
		Dwarf_Half version_stamp;
		Dwarf_Unsigned abbrev_offset;
		Dwarf_Half address_size;
		Dwarf_Unsigned next_cu_header;

		int res = dwarf_next_cu_header(dbg, &cu_header_length,
																	 &version_stamp, &abbrev_offset, &address_size,
																	 &next_cu_header, &error);
		DWARF_CHECK(res, "dwarf_next_cu_header");
		if (res == DW_DLV_NO_ENTRY) {
			break;
		}

		res = dwarf_siblingof(dbg, no_die, &cu_die, &error);
		DWARF_CHECK(res, "dwarf_siblingof");

		if (res == DW_DLV_NO_ENTRY) {
			continue;
		}

		try {
			jsval val = dwarfCu(cu_die);
			JS_SetElement(cx, cuArrayObj, cu_number, &val);
			
		} catch (...) {
			dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
			throw;
		}

		dwarf_dealloc(dbg, cu_die, DW_DLA_DIE);
	}

	res = dwarf_finish(dbg, &error);
	DWARF_CHECK(res, "dwarf_finish");

	return OBJECT_TO_JSVAL(cuArrayObj);
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

	JSObject *dwarfFileObj = JS_NewObject(cx, NULL, NULL, NULL);
	JSObject *elfArrayObj = JS_NewArrayObject(cx, 0, NULL);
	jsval val = OBJECT_TO_JSVAL(elfArrayObj);
	jsval nameVal = JS_NEW_STRING_VAL(filename.c_str());
	JS_SetProperty(cx, dwarfFileObj, "filename", &nameVal);
	JS_SetProperty(cx, dwarfFileObj, "elf", &val);
	
	try {
		cmd = ELF_C_READ;
		arf = elf_begin(fd, cmd, (Elf *)0);

		if (arf == NULL) {
			throw "could not read elf file";
		}

		try {
			int elfHeaderCnt = 0;
			while ((elf = elf_begin(fd, cmd, arf)) != 0) {
				Elf32_Ehdr *eh32;
				eh32 = elf32_getehdr(elf);
				if (!eh32) {
					throw "could not get ELF header";
				} else {
					try {
						jsval val = dwarfElfHeader(elf);
						JS_SetElement(cx, elfArrayObj, elfHeaderCnt, &val);
					} catch (...) {
						elf_end(elf);
						throw;
					}
				}
					elfHeaderCnt++;
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

	return OBJECT_TO_JSVAL(dwarfFileObj);
}



JSBool myjs_readDwarf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	const char *str;
	if (!JS_ConvertArguments(cx, argc, argv, "s", &str)) {
		return JS_FALSE;
	}

	DwarfFile dwarfFile(cx, obj, str);
	try {
		*rval = dwarfFile.dwarfFile();
		return JS_TRUE;
	} catch (const char *s) {
		JS_ReportError(cx, "readDwarf: %s", s);
		return JS_FALSE;
	}
}
	
