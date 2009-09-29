#include <string.h>

#include "avarice.h"
#include "jtag.h"
#include "jtag2.h"

#include "js.hh"

/* read/ write jtag stuff */

JSBool jsJtag_getProgramCounter(JSContext *cx, JSObject *obj, uintN argc,
																jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	unsigned long pc = theJtagICE->getProgramCounter();
	return JS_NewNumberValue(cx, pc, rval);
}

JSBool jsJtag_setProgramCounter(JSContext *cx, JSObject *obj, uintN argc,
																jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	uint32_t pc;
	if (!JS_ConvertArguments(cx, argc, argv, "u", &pc))
		return JS_FALSE;

	bool ret = theJtagICE->setProgramCounter(pc);

	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_resetProgram(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	JSBool possibleReset = JS_FALSE;
	if (!JS_ConvertArguments(cx, argc, argv, "/b", &possibleReset))
		return JS_FALSE;
	
	bool ret = theJtagICE->resetProgram(possibleReset); // JSBool can be used directly from C

	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_interruptProgram(JSContext *cx, JSObject *obj, uintN argc,
															 jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	bool ret = theJtagICE->interruptProgram();
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_resumeProgram(JSContext *cx, JSObject *obj, uintN argc,
														jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	bool ret = theJtagICE->resumeProgram();
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_read(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	// unsigned long addr, unsigned int numBytes, unsigned char buffer[]
	uint32_t addr;
	uint32_t numBytes;

	if (!JS_ConvertArguments(cx, argc, argv, "uu", &addr, &numBytes))
		return JS_FALSE;

	printf("read %d bytes from %x\n", numBytes, addr);
	unsigned char *buf = theJtagICE->jtagRead(addr, numBytes);
	if (buf == NULL)
		return JS_FALSE;
	jsval array[numBytes];
	for (uint32_t i = 0; i < numBytes; i++) {
		array[i] = INT_TO_JSVAL(buf[i]);
	}

	JSObject *x = JS_NewArrayObject(cx, numBytes, array);
	delete [] buf;
	
	*rval = OBJECT_TO_JSVAL(x);
	return JS_TRUE;
}

JSBool jsJtag_write(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	uint32_t addr;
	JSObject *arr;

	if (!JS_ConvertArguments(cx, argc, argv, "uo", &addr, &arr))
		return JS_FALSE;
	if (!JS_IsArrayObject(cx, arr))
		return JS_FALSE;
	jsuint jsNumBytes;
	if (!JS_GetArrayLength(cx, arr, &jsNumBytes)) {
		return JS_FALSE;
	}
	uint32_t numBytes = jsNumBytes;

	unsigned char buf[numBytes];
	for (uint32_t i = 0; i < numBytes; i++) {
		jsval val;
		if (!JS_GetElement(cx, arr, i, &val))
			return JS_FALSE;
		if (!JSVAL_IS_INT(val)) {
			return JS_FALSE;
		}
		buf[i] = JSVAL_TO_INT(val);
		printf("%x val[%d/%d] = %d\n", addr, i, numBytes, buf[i]);
	}

	bool ret = theJtagICE->jtagWrite(addr, numBytes, buf);
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_initJtagBox(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;
	
	theJtagICE->initJtagBox();
	
	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_initJtagOnChipDebugging(JSContext *cx, JSObject *obj,
																			uintN argc, jsval *argv, jsval *rval) {
	// XXX add bitrate param
	if (!theJtagICE)
		return JS_FALSE;
	
	theJtagICE->initJtagOnChipDebugging(250000);
	
	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_createJtag(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	printf("hello creation\n");
	bool isXmega = false;
	bool isDragon = false;
	bool applyNsrst = false;

	try {
		printf("before creation\n");
		theJtagICE = new jtag2("usb", NULL, false, isDragon, applyNsrst, isXmega);
		theJtagICE->dchain.units_before = 0;
		theJtagICE->dchain.units_after = 0;
		theJtagICE->dchain.bits_before = 0;
		theJtagICE->dchain.bits_after = 0;

		printf("after creation\n");
		theJtagICE->initJtagBox();
		theJtagICE->startPolling();
		
		*rval = JSVAL_VOID;
		return JS_TRUE;
	} catch (const char *msg) {
		fprintf(stderr, "%s\n", msg);
		return JS_FALSE;
	} catch (...) {
		fprintf(stderr, "Cannot initialize JTAG ICE\n");
		return JS_FALSE;
	}
}

JSBool jsJtag_enableProgramming(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_disableProgramming(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_eraseProgramMemory(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_eraseProgramPage(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_downloadToTarget(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_singleStep(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

/*
	The input parameter is a string from command-line, as produced by
	printf("%x", 0xaabbcc );
**/
JSBool jsJtag_writeFuses(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_readFuses(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_writeLockBits(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_readLockBits(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_deleteAllBreakpoints(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_deleteBreakpoint(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_addBreakpoint(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_updateBreakpoints(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_codeBreakpointAt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_codeBreakpointBetween(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_stopAt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	if (!theJtagICE)
		return JS_FALSE;

	*rval = JSVAL_VOID;
	return JS_TRUE;
}


static JSFunctionSpec jsjtag_functions[] = {
	{ "init", jsJtag_createJtag, 0, 0, 0},
	{ "connect", jsJtag_initJtagBox, 0, 0, 0 },
	{ "initOnChipDebugging", jsJtag_initJtagOnChipDebugging, 0, 0, 0 },

	{ "getPC", jsJtag_getProgramCounter, 0, 0, 0 },
	{ "setPC", jsJtag_setProgramCounter, 1, 0, 0 },
	{ "reset", jsJtag_resetProgram, 0, 0, 0 },
	{ "interrupt", jsJtag_interruptProgram, 0, 0, 0 },
	{ "resume", jsJtag_resumeProgram, 0, 0, 0 },

	{ "read", jsJtag_read, 2, 0, 0 },
	{ "write", jsJtag_write, 2, 0, 0 },

	{ "enableProgramming", jsJtag_enableProgramming, 0, 0, 0 },
	{ "disableProgramming", jsJtag_disableProgramming, 0, 0, 0 },
	{ "eraseProgramMemory", jsJtag_eraseProgramMemory, 0, 0, 0 },
	{ "eraseProgramPage", jsJtag_eraseProgramPage, 1, 0, 0 },
	{ "downloadToTarget", jsJtag_downloadToTarget, 3, 0, 0 },

	{ "writeFuses", jsJtag_writeFuses, 1, 0, 0 },
	{ "readFuses", jsJtag_readFuses, 0, 0, 0 },
	{ "writeLockBits", jsJtag_writeLockBits, 1, 0, 0 },
	{ "readLockBits", jsJtag_readLockBits, 0, 0, 0 },

	{ "deleteAllBreakpoints", jsJtag_deleteAllBreakpoints, 0, 0, 0 },
	{ "deleteBreakpoint", jsJtag_deleteBreakpoint, 3, 0, 0 },
	{ "addBreakpoint", jsJtag_addBreakpoint, 3, 0, 0 },
	{ "updateBreakpoints", jsJtag_updateBreakpoints, 0, 0, 0 },
	{ "codeBreakpointAt", jsJtag_codeBreakpointAt, 1, 0, 0 },
	{ "codeBreakpointBetween", jsJtag_codeBreakpointBetween, 2, 0, 0 },
	{ "stopAt", jsJtag_stopAt, 1, 0, 0 },
	{ 0 }
};

/* properties */

JSBool jsJtag_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	if (JSVAL_IS_STRING(idval)) {
		JSString *ustr = JSVAL_TO_STRING(idval);
		char *str = JS_GetStringBytes(ustr);
		if (!strcmp(str, "deviceName")) {
			if (!theJtagICE)
				return JS_FALSE;
	
			*vp = STRING_TO_JSVAL(JS_NewString(cx, theJtagICE->device_name,
																				 strlen(theJtagICE->device_name)));
			return JS_TRUE;
		} else if (!strcmp(str, "programmingEnabled")) {
			if (!theJtagICE)
				return JS_FALSE;
	
			*vp = BOOLEAN_TO_JSVAL(theJtagICE->programmingEnabled);
			return JS_TRUE;
		} else if (!strcmp(str, "events")) {
			if (!theJtagICE)
				return JS_FALSE;
	
			//			if (JSVAL_IS_STRING(*vp)) {
			//				JSString *ustr = JSVAL_TO_STRING(idval);
			//				char *str = JS_GetStringBytes(ustr);
			//				theJtagICE->parseEvents(str);
			//			} else {
			//				return JS_FALSE;
			//			}
		}

		return JS_TRUE;
	} else {
		return JS_TRUE;
	}
}

JSBool jsJtag_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	return JS_FALSE;
}


/* class definitions */
JSClass JavaScript::jtag_class = {
	"jtag", 0,
	JS_PropertyStub, JS_PropertyStub, jsJtag_getProperty, jsJtag_setProperty,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

bool JavaScript::jsJtag_registerClass() {
	jtagObject = JS_DefineObject(cx, global, "jtag", &jtag_class, NULL, 0);
	if (jtagObject == NULL)
		return false;
	if (!JS_DefineFunctions(cx, jtagObject, jsjtag_functions))
	  return false;
	return true;
}
