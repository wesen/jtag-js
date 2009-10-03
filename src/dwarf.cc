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
	if ((res) == DW_DLV_ERROR) { \
	   throw (str ": got dwarf error in file " __FILE__	\
						" at line " TOSTRING(__LINE___)); \
	}


DwarfFile::DwarfFile(JSContext *_cx, JSObject *_obj, const char *_filename) :
	cx(_cx), obj(_obj), filename(_filename) {
}

DwarfFile::~DwarfFile() {
}

jsval DwarfFile::getSmallEncodingIntegerName(Dwarf_Attribute attr,
																						 const char *attr_name,
																						 encoding_type_func val_as_string) {
	return JSVAL_VOID;
}

jsval DwarfFile::dwarfLocationList(Dwarf_Attribute attr) {
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
jsval DwarfFile::dwarfAttribute(JSObject *parent, Dwarf_Half tag, Dwarf_Attribute attr) {
	JSObject *attrObj = JS_NewObject(cx, NULL, NULL, NULL);
	jsval parentVal = OBJECT_TO_JSVAL(parent);
	JS_SetProperty(cx, attrObj, "parent", &parentVal);
	
	Dwarf_Half attrNum;
	int res = dwarf_whatattr(attr, &attrNum, &error);
	DWARF_CHECK(res, "dwarf_whatattr");
	jsval attrNumVal = INT_TO_JSVAL(attrNum);
	JS_SetProperty(cx, attrObj, "num", &attrNumVal);

	const char *atname = get_AT_name(attrNum);
	jsval atnameVal = JS_NEW_STRING_VAL(atname);
	JS_SetProperty(cx, attrObj, "name", &atnameVal);

	jsval valVal = dwarfAttributeValue(attr);
	JS_SetProperty(cx, attrObj, "value", &valVal);

	Dwarf_Off off;
	res = dwarf_attr_offset(die, attr, &off, &error);
	DWARF_CHECK(res, "dwarf_attr_offset");
	jsval attrOffVal = INT_TO_JSVAL(off);
	JS_SetProperty(cx, attrObj, "offset", &attrOffVal);

	return OBJECT_TO_JSVAL(attrObj);

}

jsval DwarfFile::dwarfAttributeValue(Dwarf_Attribute attr) {
	jsval retVal = JSVAL_VOID;

	Dwarf_Half attrNum;
	int fres = dwarf_whatattr(attr, &attrNum, &error);
	DWARF_CHECK(fres, "dwarf_whatattr");
	if (fres == DW_DLV_NO_ENTRY){
		throw "could not get data value";
	}

	switch (attrNum) {
	case DW_AT_language:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_language", get_LANG_name);
		break;

	case DW_AT_accessibility:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_accessibility", get_ACCESS_name);
		break;

	case DW_AT_visibility:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_visibility", get_VIS_name);
		break;

	case DW_AT_virtuality:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_virtuality", get_VIRTUALITY_name);
		break;

	case DW_AT_identifier_case:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_identifier", get_ID_name);
		break;

	case DW_AT_inline:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_inline", get_INL_name);
		break;

	case DW_AT_encoding:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_encoding", get_ATE_name);
		break;

	case DW_AT_ordering:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_ordering", get_ORD_name);
		break;

	case DW_AT_calling_convention:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_calling_convention", get_CC_name);
		break;

	case DW_AT_discr_list:
		retVal = getSmallEncodingIntegerName(attr, "DW_AT_discr_list", get_DSC_name);
		break;

	case DW_AT_location:
	case DW_AT_data_member_location:
	case DW_AT_vtable_elem_location:
	case DW_AT_string_length:
	case DW_AT_return_addr:
	case DW_AT_use_location:
	case DW_AT_static_link:
	case DW_AT_frame_base:
		retVal = dwarfLocationList(attr);
		break;

	case DW_AT_upper_bound:
		{
			Dwarf_Half form;
			int res = dwarf_whatform(attr, &form, &error);
			DWARF_CHECK(res, "whatform_attr");
			if (res == DW_DLV_NO_ENTRY)
				break;

			if (form == DW_FORM_block1) {
				dwarfLocationList(attr);
			} else {
				retVal = dwarfFormValue(attr);
			}
		}
		break;

	case DW_AT_high_pc:
		retVal = dwarfFormValue(attr);
		break;

	case DW_AT_ranges:
		{
			Dwarf_Half form;
			int res = dwarf_whatform(attr, &form, &error);
			DWARF_CHECK(res, "dwarf_whatform");
			if (res == DW_DLV_NO_ENTRY)
				break;

			retVal = dwarfFormValue(attr);

			if ((form == DW_FORM_data4) ||
					(form == DW_FORM_data8)) {
				Dwarf_Unsigned off;
				Dwarf_Ranges *rangeset;
				Dwarf_Signed rangecount;

				int ures = dwarf_formudata(attr, &off, &error);
				DWARF_CHECK(res, "dwarf_formudata");
				// form data ranges
			}
		}
		break;

	default:
		retVal = dwarfFormValue(attr);
		break;
	}

	return retVal;
}

jsval DwarfFile::dwarfFormXData(Dwarf_Attribute attr) {
	return JSVAL_VOID;
}

jsval DwarfFile::dwarfFormDataValue(Dwarf_Attribute attr) {
	jsval retVal = JSVAL_VOID;

	Dwarf_Half attrNum;
	int fres = dwarf_whatattr(attr, &attrNum, &error);
	DWARF_CHECK(fres, "dwarf_whatattr");
	if (fres == DW_DLV_NO_ENTRY){
		throw "could not get data value";
	}

	switch (attrNum) {
	case DW_AT_ordering:
	case DW_AT_byte_size:
	case DW_AT_bit_offset:
	case DW_AT_inline:
	case DW_AT_language:
	case DW_AT_visibility:
	case DW_AT_virtuality:
	case DW_AT_accessibility:
	case DW_AT_address_class:
	case DW_AT_calling_convention:
	case DW_AT_discr_list:
	case DW_AT_encoding:
	case DW_AT_identifier_case:
	case DW_AT_MIPS_loop_unroll_factor:
	case DW_AT_MIPS_software_pipeline_depth:
	case DW_AT_decl_column:
	case DW_AT_decl_line:
	case DW_AT_call_column:
	case DW_AT_call_line:
	case DW_AT_start_scope:
	case DW_AT_byte_stride:
	case DW_AT_bit_stride:
	case DW_AT_count:
	case DW_AT_stmt_list:
	case DW_AT_MIPS_fde:
		retVal = dwarfFormXData(attr);
		break;

	case DW_AT_decl_file:
	case DW_AT_call_file:
		break;
		
	case DW_AT_const_value:
		retVal = dwarfFormXData(attr);
		break;

	case DW_AT_upper_bound:
	case DW_AT_lower_bound:
		retVal = dwarfFormXData(attr);
		break;
	}

	return retVal;
}

jsval DwarfFile::dwarfFormValue(Dwarf_Attribute attr) {
	JSObject *attrValueObj = JS_NewObject(cx, NULL, NULL, NULL);
	jsval attrValueVal = OBJECT_TO_JSVAL(attrValueObj);
	
	Dwarf_Half form;
	int fres = dwarf_whatform(attr, &form, &error);
	DWARF_CHECK(fres, "dwarf_whatform");

	jsval formNameVal = JS_NEW_STRING_VAL(get_FORM_name(form));
	JS_SetProperty(cx, attrValueObj, "formName", &formNameVal);
	jsval formVal = INT_TO_JSVAL(form);
	JS_SetProperty(cx, attrValueObj, "form", &formVal);

	Dwarf_Half directForm;
	fres = dwarf_whatform(attr, &directForm, &error);
	DWARF_CHECK(fres, "dwarf_whatform_direct");

	jsval directFormNameVal = JS_NEW_STRING_VAL(get_FORM_name(directForm));
	JS_SetProperty(cx, attrValueObj, "directFormName", &directFormNameVal);
	jsval directFormVal = INT_TO_JSVAL(directForm);
	JS_SetProperty(cx, attrValueObj, "directForm", &directFormVal);

	jsval valVal = JSVAL_VOID;
	
	switch (form) {
	case DW_FORM_addr:
		{
			Dwarf_Addr addr;
			int bres = dwarf_formaddr(attr, &addr, &error);
			DWARF_CHECK(bres, "dwarf_formaddr");
			valVal = INT_TO_JSVAL(addr);
		}
		break;

	case DW_FORM_ref_addr:
		{
			Dwarf_Off off;
			int bres = dwarf_global_formref(attr, &off, &error);
			DWARF_CHECK(bres, "dwarf_global_formref");
			valVal = INT_TO_JSVAL(off);
		}
		break;

	case DW_FORM_ref1:
	case DW_FORM_ref2:
	case DW_FORM_ref4:
	case DW_FORM_ref8:
	case DW_FORM_ref_udata:
		{
			Dwarf_Off off;
			int bres = dwarf_formref(attr, &off, &error);
			DWARF_CHECK(bres, "dwarf_formref");
			valVal = INT_TO_JSVAL(off);
		}
		break;

	case DW_FORM_block:
	case DW_FORM_block1:
	case DW_FORM_block2:
	case DW_FORM_block4:
		{
			Dwarf_Block *tempb;
			int fres = dwarf_formblock(attr, &tempb, &error);
			DWARF_CHECK(fres, "dwarf_formblock");
			JSObject *formArrayObj = JS_NewArrayObject(cx, 0, NULL);
			valVal = OBJECT_TO_JSVAL(formArrayObj);
			
			for (int i = 0; i < tempb->bl_len; i++) {
				jsval blDataVal = INT_TO_JSVAL(*(i + (unsigned char *)tempb->bl_data));
				JS_SetElement(cx, formArrayObj, i, &blDataVal);
			}
			dwarf_dealloc(dbg, tempb, DW_DLA_BLOCK);
		}
		break;

	case DW_FORM_data1:
	case DW_FORM_data2:
	case DW_FORM_data4:
	case DW_FORM_data8:
		valVal = dwarfFormDataValue(attr);
		break;

	case DW_FORM_sdata:
		{
			Dwarf_Signed tempsd;
			int wres = dwarf_formsdata(attr, &tempsd, &error);
			DWARF_CHECK(wres, "dwarf_formsdata");
			valVal = INT_TO_JSVAL(tempsd);
		}
		break;

	case DW_FORM_udata:
		{
			Dwarf_Unsigned tempud;
			int wres = dwarf_formudata(attr, &tempud, &error);
			DWARF_CHECK(wres, "dwarf_formudata");
			valVal = INT_TO_JSVAL(tempud);
		}
		break;

	case DW_FORM_string:
	case DW_FORM_strp:
		{
			char *temps;
			int wres = dwarf_formstring(attr, &temps, &error);
			DWARF_CHECK(wres, "dwarf_formstring");
			valVal = JS_NEW_STRING_VAL(temps);
			dwarf_dealloc(dbg, temps, DW_DLA_STRING);
		}
		break;

	case DW_FORM_flag:
		{
			Dwarf_Bool tempbool;
			int wres = dwarf_formflag(attr, &tempbool, &error);
			DWARF_CHECK(wres, "dwarf_formflag");
			valVal = BOOLEAN_TO_JSVAL(tempbool);
		}
		break;

	case DW_FORM_indirect:
		{
			valVal = JS_NEW_STRING_VAL(get_FORM_name(form));
		}
		break;

	default:
		throw ("unknown attribute");
		break;
		
		
	}

	// strdup free?
	JS_SetProperty(cx, attrValueObj, "value", &valVal);
	return attrValueVal;
}

void DwarfFile::dwarfDieData(JSObject *dieObj, Dwarf_Die _die) {
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

	Dwarf_Off dieOff;
	res = dwarf_dieoffset(die, &dieOff, &error);
	DWARF_CHECK(res, "dwarf_dieoffset");
	jsval dieOffsetVal = INT_TO_JSVAL(dieOff);
	JS_SetProperty(cx, dieObj, "offset", &dieOffsetVal);

	Dwarf_Off cuOff;
	Dwarf_Off cuOffLen;
	res = dwarf_die_CU_offset_range(die, &cuOff, &cuOffLen, &error);
	DWARF_CHECK(res, "dwarf_die_CU_offset");
	jsval dieCuOffsetVal = INT_TO_JSVAL(cuOff);
	jsval dieCuOffsetLenVal = INT_TO_JSVAL(cuOffLen);
	JS_SetProperty(cx, dieObj, "cuOffset", &dieCuOffsetVal);
	JS_SetProperty(cx, dieObj, "cuOffsetLen", &dieCuOffsetLenVal);

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
				jsval attrVal = dwarfAttribute(dieObj, tag, attrs[i]);
				JS_SetElement(cx, attrArrayObj, i, &attrVal);
			} catch (const char *s) {
				jsval val = JS_NEW_STRING_VAL(s);
				JS_SetElement(cx, attrArrayObj, i, &val);
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

	die = in_die;
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
						jsval dieVal = dwarfDie(dieObj, sib_die, level + 1);
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
	
