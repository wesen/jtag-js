#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <libelf.h>

#include "avarice.h"

#include <dwarf.h>
#include <libdwarf.h>

#include "dwarfnaming.hh"
#include "dwarf.hh"

// #define DWARF_UNIMPLEMENTED() throw DwarfException()
#define DWARF_THROW(str) \
	throw (str " in file " __FILE__ " at line " TOSTRING(__LINE__))

#define DWARF_UNIMPLEMENTED(name) \
	DWARF_THROW(name ": unimplemented")

#define DWARF_CHECK(res, str)																		\
	if ((res) == DW_DLV_ERROR) { \
		DWARF_THROW(str ": got dwarf error");				\
		}

DwarfFile::DwarfFile(JSContext *_cx, JSObject *_obj, const char *_filename) :
	cx(_cx), obj(_obj), filename(_filename) {
}

DwarfFile::~DwarfFile() {
}

jsval DwarfFile::getSmallEncodingIntegerName(Dwarf_Attribute attr,
																						 const char *attr_name,
																						 encoding_type_func val_as_string) {
	JSObject *attrValueObj = JS_NEW_OBJECT(cx);
	jsval attrValueVal = OBJECT_TO_JSVAL(attrValueObj);

	Dwarf_Unsigned uval;
	int res = dwarf_formudata(attr, &uval, &error);
  DWARF_CHECK(res, "dwarf_formudata");
	const char *val = val_as_string(uval);
	return JS_NEW_STRING_VAL(val);
}

jsval DwarfFile::dwarfLocationList(Dwarf_Attribute attr) {
	Dwarf_Locdesc **llbufarray;
	Dwarf_Signed cnt;
	int res = dwarf_loclist_n(attr, &llbufarray, &cnt, &error);
	DWARF_CHECK(res, "dwarf_loclist_n");

	JSObject *locListObj = JS_NEW_ARRAY(cx);
	
	for (int i = 0; i < cnt; i++) {
		Dwarf_Locdesc *llbuf = llbufarray[i];

		jsval locVal = dwarfLocDesc(llbuf);
		JS_SetElement(cx, locListObj, i, &locVal);

		dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
	}

	dwarf_dealloc(dbg, llbufarray, DW_DLA_LIST);

	return OBJECT_TO_JSVAL(locListObj);
}

jsval DwarfFile::dwarfLocDesc(Dwarf_Locdesc *locDesc) {
	JSObject *locDescObj = JS_NEW_ARRAY(cx);

	JS_SET_PROPERTY_INT(locDescObj, "lowpc", locDesc->ld_lopc);
	JS_SET_PROPERTY_INT(locDescObj, "highpc", locDesc->ld_hipc);

	for (int i = 0; i < locDesc->ld_cents; i++) {
		Dwarf_Loc *expr = &(locDesc->ld_s[i]);

		jsval locVal = dwarfLocation(expr);
		JS_SetElement(cx, locDescObj, i, &locVal);
	}

	return OBJECT_TO_JSVAL(locDescObj);
}

jsval DwarfFile::dwarfLocation(Dwarf_Loc *expr) {
	JSObject *exprObj = JS_NEW_OBJECT(cx);
	
	Dwarf_Small op = expr->lr_atom;
	JS_SET_PROPERTY_INT(exprObj, "op", op);
	if (op > DW_OP_nop) {
		DWARF_THROW("unsupported op");
	}
	const char *op_name = get_OP_name(op);
	JS_SET_PROPERTY_STRING(exprObj, "opName", op_name);
	
	Dwarf_Unsigned opd1 = expr->lr_number;
	Dwarf_Unsigned opd2 = expr->lr_number2;

	if ((op >= DW_OP_breg0) && (op <= DW_OP_breg31)) {
		JS_SET_PROPERTY_INT(exprObj, "opd1", opd1);
	} else {
		switch (op) {
		case DW_OP_addr:
			JS_SET_PROPERTY_INT(exprObj, "opd1", opd1);
			break;

		case DW_OP_const1s:
		case DW_OP_const2s:
		case DW_OP_const4s:
		case DW_OP_consts:
		case DW_OP_skip:
		case DW_OP_bra:
		case DW_OP_fbreg:
			JS_SET_PROPERTY_INT(exprObj, "opd1", (Dwarf_Signed)opd1);
			break;

		case DW_OP_const1u:
		case DW_OP_const2u:
		case DW_OP_const4u:
		case DW_OP_const8u:
		case DW_OP_constu:
		case DW_OP_pick:
		case DW_OP_plus_uconst:
		case DW_OP_regx:
		case DW_OP_piece:
		case DW_OP_deref_size:
		case DW_OP_xderef_size:
			JS_SET_PROPERTY_INT(exprObj, "opd1", opd1);
			break;

		case DW_OP_bregx:
			JS_SET_PROPERTY_INT(exprObj, "opd1", opd1);
			JS_SET_PROPERTY_INT(exprObj, "opd2", (Dwarf_Signed)opd2);
			break;

		default:
			break;
		}
	}

	return OBJECT_TO_JSVAL(exprObj);
}

/* dwarf attribute */
jsval DwarfFile::dwarfAttribute(JSObject *parent, Dwarf_Half tag, Dwarf_Attribute attr) {
	attrObj = JS_NEW_OBJECT(cx);
#ifdef SET_PARENT_PROPERTY
	JS_SET_PROPERTY_OBJECT(attrObj, "parent", attrObj);
#endif
	
	Dwarf_Half attrNum;
	int res = dwarf_whatattr(attr, &attrNum, &error);
	DWARF_CHECK(res, "dwarf_whatattr");
	JS_SET_PROPERTY_INT(attrObj, "num", attrNum);

	const char *atname = get_AT_name(attrNum);
	JS_SET_PROPERTY_STRING(attrObj, "name", atname);

	jsval valVal = dwarfAttributeValue(attr);
	JS_SetProperty(cx, attrObj, "value", &valVal);

	Dwarf_Off off;
	res = dwarf_attr_offset(die, attr, &off, &error);
	DWARF_CHECK(res, "dwarf_attr_offset");
	JS_SET_PROPERTY_INT(attrObj, "offset", off);

	return OBJECT_TO_JSVAL(attrObj);

}

jsval DwarfFile::dwarfAttributeValue(Dwarf_Attribute attr) {
	jsval retVal = JSVAL_VOID;

	JSObject *attrValueObj = JS_NEW_OBJECT(cx);
	jsval attrValueVal = OBJECT_TO_JSVAL(attrValueObj);

	Dwarf_Half form;
	int fres = dwarf_whatform(attr, &form, &error);
	DWARF_CHECK(fres, "dwarf_whatform");

	JS_SET_PROPERTY_STRING(attrValueObj, "formName", get_FORM_name(form));
	JS_SET_PROPERTY_INT(attrValueObj, "form", form);

	Dwarf_Half directForm;
	fres = dwarf_whatform(attr, &directForm, &error);
	DWARF_CHECK(fres, "dwarf_whatform_direct");

	JS_SET_PROPERTY_STRING(attrValueObj, "directFormName", get_FORM_name(directForm));
	JS_SET_PROPERTY_INT(attrValueObj, "directForm", directForm);

	Dwarf_Half attrNum;
	fres = dwarf_whatattr(attr, &attrNum, &error);
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
				retVal = dwarfLocationList(attr);
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

	// strdup free?
	JS_SetProperty(cx, attrValueObj, "value", &retVal);
	return attrValueVal;
}

jsval DwarfFile::dwarfFormXData(Dwarf_Attribute attr) {
	Dwarf_Signed tempsd;
	Dwarf_Unsigned tempud;

	int ures = dwarf_formudata(attr, &tempud, &error);
	int sres = dwarf_formsdata(attr, &tempsd, &error);

	if (ures == DW_DLV_OK) {
		return INT_TO_JSVAL(tempud);
	}
	if (sres == DW_DLV_OK) {
		return INT_TO_JSVAL(tempsd);
	}

	throw("could get neither signed nor unsigned data value");
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
		{
			Dwarf_Unsigned tempud;
			int ures = dwarf_formudata(attr, &tempud, &error);
			DWARF_CHECK(ures, "dwarf_formudata");
			if ((srcfiles != NULL) &&
					(fileCnt >= tempud)) {
				retVal = JS_NEW_STRING_VAL(srcfiles[tempud-1]);
			}
		}
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
	jsval valVal = JSVAL_VOID;
	
	Dwarf_Half form;
	int fres = dwarf_whatform(attr, &form, &error);
	DWARF_CHECK(fres, "dwarf_whatform");

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
			JSObject *formArrayObj = JS_NEW_ARRAY(cx);
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

	return valVal;
}

void DwarfFile::dwarfDieData(JSObject *dieObj, Dwarf_Die _die) {
	/* die naming information */
	char *name;
	int res = dwarf_diename(die, &name, &error);
	DWARF_CHECK(res, "dwarf_diename");
	
	if (res == DW_DLV_OK) {
		JS_SET_PROPERTY_STRING(dieObj, "name", name);
		dwarf_dealloc(dbg, name, DW_DLA_STRING);
	}

	Dwarf_Half tag;
	res = dwarf_tag(die, &tag, &error);
	DWARF_CHECK(res, "dwarf_tag");
	if (res == DW_DLV_OK) {
		JS_SET_PROPERTY_INT(dieObj, "tag", tag);
		
		JS_SET_PROPERTY_STRING(dieObj, "tagName", get_TAG_name(tag));
	}
	
	/* die offset and die cu offset */
	Dwarf_Off dieOff;
	res = dwarf_dieoffset(die, &dieOff, &error);
	DWARF_CHECK(res, "dwarf_dieoffset");
	JS_SET_PROPERTY_INT(dieObj, "offset", dieOff);

	Dwarf_Off cuOff;
	Dwarf_Off cuOffLen;
	res = dwarf_die_CU_offset_range(die, &cuOff, &cuOffLen, &error);
	DWARF_CHECK(res, "dwarf_die_CU_offset");
	JS_SET_PROPERTY_INT(dieObj, "cuOffset", cuOff);
	JS_SET_PROPERTY_INT(dieObj, "cuOffsetLength", cuOffLen);

	/* go through die attributes */
	JSObject *attrArrayObj = JS_NEW_ARRAY(cx);
	JS_SET_PROPERTY_OBJECT(dieObj, "attributes", attrArrayObj);

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
				JSObject *attrObj = JS_NEW_OBJECT(cx);
				jsval attrVal = OBJECT_TO_JSVAL(attrObj);

				JS_SET_PROPERTY_STRING(attrObj, "name", "error");
				JS_SET_PROPERTY_STRING(attrObj, "type", "error");
				JS_SET_PROPERTY_INT(attrObj, "num", 0);

				JSObject *attrValObj = JS_NEW_OBJECT(cx);
				JS_SET_PROPERTY_STRING(attrValObj, "value", s);
				JS_SET_PROPERTY_INT(attrValObj, "form", DW_FORM_string);
				JS_SET_PROPERTY_STRING(attrValObj, "formName", get_FORM_name(DW_FORM_string));

				JS_SET_PROPERTY_OBJECT(attrObj, "value", attrValObj);

				JS_SetElement(cx, attrArrayObj, i, &attrVal);
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

jsval DwarfFile::dwarfLine(Dwarf_Line line) {
	JSObject *lineObj = JS_NEW_OBJECT(cx);

	char *filename = NULL;
	int sres = dwarf_linesrc(line, &filename, &error);
	DWARF_CHECK(sres, "dwarf_linesrc");
	if (sres == DW_DLV_NO_ENTRY) {
		filename = (char *)"<unknown>";
	}
	JS_SET_PROPERTY_STRING(lineObj, "filename", filename);
	if (sres == DW_DLV_OK) {
		dwarf_dealloc(dbg, filename, DW_DLA_STRING);
	}

	Dwarf_Addr pc;
	int ares = dwarf_lineaddr(line, &pc, &error);
	DWARF_CHECK(ares, "dwarf_lineaddr");
	if (ares == DW_DLV_OK) {
		JS_SET_PROPERTY_INT(lineObj, "pc", pc);
	}

	Dwarf_Unsigned lineno;
	int res = dwarf_lineno(line, &lineno, &error);
	DWARF_CHECK(res, "dwarf_lineno");
	if (res == DW_DLV_NO_ENTRY) {
		JS_SET_PROPERTY_INT(lineObj, "lineno", -1);
	} else {
		JS_SET_PROPERTY_INT(lineObj, "lineno", lineno);
	}

	Dwarf_Signed column;
	res = dwarf_lineoff(line, &column, &error);
	DWARF_CHECK(res, "dwarf_lineoff");
	if (res == DW_DLV_NO_ENTRY) {
		column = -1;
	}
	JS_SET_PROPERTY_INT(lineObj, "column", column);

	Dwarf_Bool newstatement;
	res = dwarf_linebeginstatement(line, &newstatement, &error);
	DWARF_CHECK(res, "dwarf_linebeginstatement");
	JS_SET_PROPERTY_BOOLEAN(lineObj, "beginStatement", newstatement);

	Dwarf_Bool newblock;
	res = dwarf_lineblock(line, &newblock, &error);
	DWARF_CHECK(res, "dwarf_lineblock");
	JS_SET_PROPERTY_BOOLEAN(lineObj, "beginBlock", newblock);

	Dwarf_Bool endsequence;
	res = dwarf_lineendsequence(line, &endsequence, &error);
	DWARF_CHECK(res, "dwarf_lineendsequence");
	JS_SET_PROPERTY_BOOLEAN(lineObj, "endSequence", endsequence);
	
	return OBJECT_TO_JSVAL(lineObj);
}

void DwarfFile::dwarfDieLines(JSObject *dieObj, Dwarf_Die die) {
	JSObject *linesObj = JS_NEW_ARRAY(cx);
	
	Dwarf_Line *linebuf;
	Dwarf_Signed linecount;
	
	int res = dwarf_srclines(die, &linebuf, &linecount, &error);
	DWARF_CHECK(res, "dwarf_srclines");
	if (res == DW_DLV_NO_ENTRY) {
		return;
	}

	for (int i = 0; i < linecount; i++) {
		Dwarf_Line line = linebuf[i];
		jsval lineVal = dwarfLine(line);
		JS_SetElement(cx, linesObj, i, &lineVal);
	}

	dwarf_srclines_dealloc(dbg, linebuf, linecount);

	JS_SET_PROPERTY_OBJECT(dieObj, "lines", linesObj);
}

/* dwarf die */
jsval DwarfFile::dwarfDie(JSObject *parent, Dwarf_Die in_die, int level) {
	//	DWARF_UNIMPLEMENTED("dwarfDie");
	JSObject *dieObj = JS_NEW_OBJECT(cx);
#ifdef SET_PARENT_PROPERTY
	JS_SET_PROPERTY_OBJECT(dieObj, "parent", parent);
#endif
	JS_SET_PROPERTY_INT(dieObj, "level", level);

	JSObject *childArrayObj = JS_NEW_ARRAY(cx);
	JS_SET_PROPERTY_OBJECT(dieObj, "children", childArrayObj);

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
	srcfiles = NULL;

	int sres = dwarf_srcfiles(in_die, &srcfiles, &fileCnt, &error);
	DWARF_CHECK(sres, "dwarf_srcfiles");
	if (sres == DW_DLV_OK) {
		printf("%d files\n", fileCnt);
	} else {
		fileCnt = 0;
	}

	printf("%s\n", srcfiles[0]);
	
	jsval retVal =  dwarfDie(NULL, in_die);

	if (sres == DW_DLV_OK) {
		for (int i = 0; i < fileCnt; i++) {
			dwarf_dealloc(dbg, srcfiles[i], DW_DLA_STRING);
		}
		dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
	}

	return retVal;
}

/* single dwarf section */
jsval DwarfFile::dwarfElfHeader(Elf *elf) {
	int res = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &error);
	DWARF_CHECK(res, "dwarf_elf_init");

	JSObject *cuArrayObj = JS_NEW_ARRAY(cx);
	
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

	JSObject *dwarfFileObj = JS_NEW_OBJECT(cx);
	JSObject *elfArrayObj = JS_NEW_ARRAY(cx);
	JS_SET_PROPERTY_OBJECT(dwarfFileObj, "elf", elfArrayObj);
	JS_SET_PROPERTY_STRING(dwarfFileObj, "filename", filename.c_str());
	
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
	
