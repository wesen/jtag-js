#include <string.h>

#include "avarice.h"
#include "jtag.h"
#include "jtag2.h"

#include "js.hh"
#include "jsjtag.hh"
#include "jsbreakpoint.hh"

extern JSClass jtag_class;

/* read/ write jtag stuff */
#define JS_GET_PRIVATE_JTAG()                                                 \
	JSJtag *privJtag = (JSJtag *)JS_GetInstancePrivate(cx, obj, &jtag_class, NULL); \
	jtag *origJtag = privJtag->origJtag;                                          \
	if (!privJtag) {                                                                \
	JS_ReportError(cx, "Could not get private jtag instance");						\
	return JS_FALSE; \
	}

#define JS_JTAGICE_AVAILABLE_CHECK() \
		if (!origJtag) {																							\
      JS_ReportError(cx, "JTAG ICE not created yet, call jtag.init() first"); \
      return JS_FALSE;																									\
		}

JSBool jsJtag_getProgramCounter(JSContext *cx, JSObject *obj, uintN argc,
																jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
	unsigned long pc = origJtag->getProgramCounter();
	return JS_NewNumberValue(cx, pc, rval);
}

JSBool jsJtag_setProgramCounter(JSContext *cx, JSObject *obj, uintN argc,
																jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
	uint32_t pc;
	if (!JS_ConvertArguments(cx, argc, argv, "u", &pc))
		return JS_FALSE;

	bool ret = origJtag->setProgramCounter(pc);

	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_resetProgram(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
	JSBool possibleReset = JS_FALSE;
	if (!JS_ConvertArguments(cx, argc, argv, "/b", &possibleReset))
		return JS_FALSE;
	
	bool ret = origJtag->resetProgram(possibleReset); // JSBool can be used directly from C

	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_interruptProgram(JSContext *cx, JSObject *obj, uintN argc,
															 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
	bool ret = origJtag->interruptProgram();
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_resumeProgram(JSContext *cx, JSObject *obj, uintN argc,
														jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
	bool ret = origJtag->resumeProgram();
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_read(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
	// unsigned long addr, unsigned int numBytes, unsigned char buffer[]
	uint32_t addr;
	uint32_t numBytes;

	if (!JS_ConvertArguments(cx, argc, argv, "uu", &addr, &numBytes))
		return JS_FALSE;

	printf("read %d bytes from %x\n", numBytes, addr);
	unsigned char *buf = origJtag->jtagRead(addr, numBytes);
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
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
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

	bool ret = origJtag->jtagWrite(addr, numBytes, buf);
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_initJtagBox(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();
	
	origJtag->initJtagBox();
	
	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_initJtagOnChipDebugging(JSContext *cx, JSObject *obj,
																			uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	uint32_t bitRate;

	if (!JS_ConvertArguments(cx, argc, argv, "u", &bitRate))
		return JS_FALSE;
	
	origJtag->initJtagOnChipDebugging(bitRate);
	
	*rval = JSVAL_VOID;
	return JS_TRUE;
}

void jsJtag_releaseNativeJtag(JSContext *cx, JSJtag *theJtag) {
	if (theJtag->origJtag != NULL) {
		delete theJtag->origJtag;

		// clear breakpoints
		for (int i = 0; i < MAX_TOTAL_BREAKPOINTS2; i++) {
			JSObject *bpObj = theJtag->breakpoints[i];
			if (bpObj != NULL) {
				JSBreakpoint *bp = (JSBreakpoint *)JS_GetInstancePrivate(cx, bpObj, &jsbreakpoint_class, NULL);
				if (!bp)
					continue;
				bp->bp = NULL;
			}
		}

		theJtag->origJtag = theJtagICE = NULL;
	}
}

JSBool jsJtag_createJtag(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
	
	bool isXmega = false;
	bool isDragon = false;
	bool applyNsrst = false;

	try {
		theJtagICE = origJtag = privJtag->origJtag =
			new jtag2("usb", NULL, false, isDragon, applyNsrst, isXmega);
		origJtag->dchain.units_before = 0;
		origJtag->dchain.units_after = 0;
		origJtag->dchain.bits_before = 0;
		origJtag->dchain.bits_after = 0;
		
		origJtag->initJtagBox();
		origJtag->startPolling();

		for (int i = 0; i < MAX_TOTAL_BREAKPOINTS2; i++) {
			JSObject *bpObj = jsBreakpoint_NewObject(cx, obj, privJtag, ((jtag2 *)origJtag)->bp + i);
			privJtag->breakpoints[i] = bpObj;
		}
		
		*rval = JSVAL_VOID;
		return JS_TRUE;
	} catch (const char *msg) {
		JS_ReportError(cx, "Cannot initialize JTAG: %p, %s\n", theJtagICE, msg);
		jsJtag_releaseNativeJtag(cx, privJtag);
		return JS_FALSE;
	} catch (...) {
		JS_ReportError(cx, "Cannot initialize JTAG: Unknown error\n");
		jsJtag_releaseNativeJtag(cx, privJtag);
		return JS_FALSE;
	}
}

JSBool jsJtag_enableProgramming(JSContext *cx, JSObject *obj,
																uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	origJtag->enableProgramming();

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_disableProgramming(JSContext *cx, JSObject *obj,
																 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	origJtag->disableProgramming();
	
	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_eraseProgramMemory(JSContext *cx, JSObject *obj,
																 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	origJtag->eraseProgramMemory();

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_eraseProgramPage(JSContext *cx, JSObject *obj,
															 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	uint32_t addr;

	if (!JS_ConvertArguments(cx, argc, argv, "u", &addr)) {
		return JS_FALSE;
	}
	origJtag->eraseProgramPage(addr);
	
	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_downloadToTarget(JSContext *cx, JSObject *obj,
															 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();

  JS_JTAGICE_AVAILABLE_CHECK();
	JS_REPORT_UNIMPLEMENTED(); // XXX

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_singleStep(JSContext *cx, JSObject *obj,
												 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	JS_REPORT_UNIMPLEMENTED(); // XXX goes into eventloop

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_deleteAllBreakpoints(JSContext *cx, JSObject *obj,
																	 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	origJtag->deleteAllBreakpoints();

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_deleteBreakpoint(JSContext *cx, JSObject *obj,
															 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	uint32_t breakpointAddress;
	uint32_t breakpointType;
	uint32_t length;

	if (!JS_ConvertArguments(cx, argc, argv, "uuu", &breakpointAddress, &breakpointType, &length))
		return JS_FALSE;

	bool ret = origJtag->deleteBreakpoint(breakpointAddress, (bpType)breakpointType, length);
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_addBreakpoint(JSContext *cx, JSObject *obj,
														uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	uint32_t breakpointAddress;
	uint32_t breakpointType;
	uint32_t length;

	if (!JS_ConvertArguments(cx, argc, argv, "uuu", &breakpointAddress, &breakpointType, &length))
		return JS_FALSE;

	bool ret = origJtag->addBreakpoint(breakpointAddress, (bpType)breakpointType, length);

	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_updateBreakpoints(JSContext *cx, JSObject *obj,
																uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	origJtag->updateBreakpoints();

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool jsJtag_codeBreakpointAt(JSContext *cx, JSObject *obj,
															 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	uint32_t breakpointAddress;

	if (!JS_ConvertArguments(cx, argc, argv, "u", &breakpointAddress))
		return JS_FALSE;

	bool ret = origJtag->codeBreakpointAt(breakpointAddress);
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_codeBreakpointBetween(JSContext *cx, JSObject *obj,
																		uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	uint32_t addr1;
	uint32_t addr2;

	if (!JS_ConvertArguments(cx, argc, argv, "uu", &addr1, &addr2))
		return JS_FALSE;

	bool ret = origJtag->codeBreakpointBetween(addr1, addr2);
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_stopAt(JSContext *cx, JSObject *obj,
										 uintN argc, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	uint32_t breakpointAddress;

	if (!JS_ConvertArguments(cx, argc, argv, "u", &breakpointAddress))
		return JS_FALSE;

	bool ret = origJtag->stopAt(breakpointAddress);
	return JS_TRUE;
}

JSObject *jsJtag_getBreakpointForNativeBp(JSContext *cx, JSJtag *theJtag, breakpoint2 *origBp) {
	for (int i = 0; i < MAX_TOTAL_BREAKPOINTS2; i++) {
		JSObject *bpObj = theJtag->breakpoints[i];
		if (bpObj != NULL) {
			JSBreakpoint *bp = (JSBreakpoint *)JS_GetInstancePrivate(cx, bpObj, &jsbreakpoint_class, NULL);
			if (!bp)
				continue;
			if (bp->bp == origBp)
				return bpObj;
		}
	}

	return NULL;
}

JSBool jsJtag_getBreakpoints(JSContext *cx, JSObject *obj,
														 uintN arg, jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_JTAG();
  JS_JTAGICE_AVAILABLE_CHECK();

	JSObject *arr = JS_NewArrayObject(cx, 0, NULL);
	if (arr == NULL)
    return JS_FALSE;

	for (int i = 0; i < MAX_TOTAL_BREAKPOINTS2; i++) {
		JSObject *bpObj = privJtag->breakpoints[i];
		printf("i: %d %p\n:", i, bpObj);
		if (bpObj != NULL) {
			JSBreakpoint *bp = (JSBreakpoint *)JS_GetInstancePrivate(cx, bpObj, &jsbreakpoint_class, NULL);
			printf("i: %d %p %p\n:", i, bp, bp);
			if (!bp)
				continue;
			breakpoint2 *origBp = bp->bp;
			if (!origBp)
				continue;
			if (origBp->last)
				break;
			jsval objVal = OBJECT_TO_JSVAL(bpObj);
			JS_SetElement(cx, arr, i, &objVal);
		}
	}

	*rval = OBJECT_TO_JSVAL(arr);
	return JS_TRUE;
}

static JSFunctionSpec jsjtag_functions[] = {
	{ "init",                  jsJtag_createJtag,              0, JSPROP_ENUMERATE, 0},
	{ "connect",               jsJtag_initJtagBox,             0, JSPROP_ENUMERATE, 0},
	{ "initOnChipDebugging",   jsJtag_initJtagOnChipDebugging, 0, JSPROP_ENUMERATE, 0},

	{ "getPC",                 jsJtag_getProgramCounter,       0, JSPROP_ENUMERATE, 0},
	{ "setPC",                 jsJtag_setProgramCounter,       1, JSPROP_ENUMERATE, 0},
	{ "reset",                 jsJtag_resetProgram,            0, JSPROP_ENUMERATE, 0},
	{ "interrupt",             jsJtag_interruptProgram,        0, JSPROP_ENUMERATE, 0},
	{ "resume",                jsJtag_resumeProgram,           0, JSPROP_ENUMERATE, 0},

	{ "read",                  jsJtag_read,                    2, JSPROP_ENUMERATE, 0},
	{ "write",                 jsJtag_write,                   2, JSPROP_ENUMERATE, 0},

	{ "enableProgramming",     jsJtag_enableProgramming,       0, JSPROP_ENUMERATE, 0},
	{ "disableProgramming",    jsJtag_disableProgramming,      0, JSPROP_ENUMERATE, 0},
	{ "eraseProgramMemory",    jsJtag_eraseProgramMemory,      0, JSPROP_ENUMERATE, 0},
	{ "eraseProgramPage",      jsJtag_eraseProgramPage,        1, JSPROP_ENUMERATE, 0},
	{ "downloadToTarget",      jsJtag_downloadToTarget,        3, JSPROP_ENUMERATE, 0},

	{ "deleteAllBreakpoints",  jsJtag_deleteAllBreakpoints,    0, JSPROP_ENUMERATE, 0},
	{ "deleteBreakpoint",      jsJtag_deleteBreakpoint,        3, JSPROP_ENUMERATE, 0},
	{ "addBreakpoint",         jsJtag_addBreakpoint,           3, JSPROP_ENUMERATE, 0},
	{ "updateBreakpoints",     jsJtag_updateBreakpoints,       0, JSPROP_ENUMERATE, 0},
	{ "codeBreakpointAt",      jsJtag_codeBreakpointAt,        1, JSPROP_ENUMERATE, 0},
	{ "codeBreakpointBetween", jsJtag_codeBreakpointBetween,   2, JSPROP_ENUMERATE, 0},
	{ "stopAt",                jsJtag_stopAt,                  1, JSPROP_ENUMERATE, 0},

	{ "getBreakpoints",        jsJtag_getBreakpoints,          0, JSPROP_ENUMERATE, 0 },
	{ 0 }
};

enum jsjtag_propID {
	JSJTAG_DEVICENAME = 1,
	JSJTAG_EVENTS,
	JSJTAG_PROGRAMMING_ENABLED,
	JSJTAG_BREAKPOINTS
};

static JSPropertySpec jsjtag_props[] = {
	{ "deviceName",         JSJTAG_DEVICENAME,          JSPROP_ENUMERATE | JSPROP_READONLY },
	{ "programmingEnabled", JSJTAG_PROGRAMMING_ENABLED, JSPROP_ENUMERATE | JSPROP_READONLY },
	{ "events",             JSJTAG_EVENTS,              JSPROP_ENUMERATE },
	{ 0 }
};

JSBool jsJtag_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp);
JSBool jsJtag_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp);

/* class definitions */
JSClass jtag_class = {
	"jtag", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, jsJtag_getProperty, jsJtag_setProperty,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};


/* JTAG definitions */
JSJtag *jsjtag_init(JSContext *cx, JSObject *obj) {
	JSJtag *theJtag = new JSJtag();
	theJtag = (JSJtag *)JS_malloc(cx, sizeof(*theJtag));
	if (!theJtag) {
		return NULL;
	}
	memset(theJtag, 0, sizeof(*theJtag));
	theJtag->origJtag = NULL;

	for (int i = 0; i < MAX_TOTAL_BREAKPOINTS2; i++) {
		theJtag->breakpoints[i] = NULL;
	}

	if (!JS_SetPrivate(cx, obj, theJtag)) {
		JS_ReportError(cx, "Could not set private JTAG object");
		JS_free(cx, theJtag);
		return NULL;
	}

	return theJtag;
}

/* JTAG properties */
JSBool jsJtag_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	JS_GET_PRIVATE_JTAG();

	jsint slot = JSVAL_TO_INT(idval);
	switch (slot) {
	case JSJTAG_DEVICENAME:
		JS_JTAGICE_AVAILABLE_CHECK();
	
		*vp = STRING_TO_JSVAL(JS_NewString(cx, origJtag->device_name,
																			 strlen(origJtag->device_name)));
		return JS_TRUE;
		break;

	case JSJTAG_PROGRAMMING_ENABLED:
		JS_JTAGICE_AVAILABLE_CHECK();
		*vp = BOOLEAN_TO_JSVAL(origJtag->programmingEnabled);
		return JS_TRUE;
		break;

	case JSJTAG_EVENTS:
		JS_JTAGICE_AVAILABLE_CHECK();
		JS_REPORT_UNIMPLEMENTED(); // XXX
		return JS_FALSE;
		break;

	case JSJTAG_BREAKPOINTS:
		JS_JTAGICE_AVAILABLE_CHECK();
		JS_REPORT_UNIMPLEMENTED(); // XXX
		return JS_FALSE;
		break;

	default:
		return JS_TRUE;
	}
}

JSBool jsJtag_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	JS_GET_PRIVATE_JTAG();

	jsint slot = JSVAL_TO_INT(idval);

	switch (slot) {
	case JSJTAG_EVENTS:
		JS_JTAGICE_AVAILABLE_CHECK();
			if (JSVAL_IS_STRING(*vp)) {
				JSString *ustr = JSVAL_TO_STRING(idval);
				char *str = JS_GetStringBytes(ustr);
				origJtag->parseEvents(str);
			} else {
				return JS_FALSE;
			}
		return JS_TRUE;
		break;

	default:
		return JS_TRUE;
	}
}


/* Register the whole shebang */

bool jsJtag_registerClass(JSContext *cx, JSObject *global) {
	if (!jsBreakpoint_registerClass(cx, global)) {
		return false;
	}
	
	JSObject *jtagClass = JS_InitClass(cx, global, NULL, &jtag_class,
																		 NULL, 0, jsjtag_props, jsjtag_functions,
																		 NULL, NULL);
	if (!jtagClass) {
		JS_ReportError(cx, "Could not initialize JTAG class");
		return false;
	}
												 
 	theJS->jtagObject = JS_DefineObject(cx, global, "jtag", &jtag_class, NULL, JSPROP_ENUMERATE);
	if (theJS->jtagObject == NULL) {
		JS_ReportError(cx, "Could not create JTAG Object");
		return false;
	}
	JSJtag *jtag = jsjtag_init(cx, theJS->jtagObject);
	if (jtag == NULL) {
		return false;
	}
	/* XXX define breakpoints property */
	
	return true;
}
