#include <string.h>

#include "avarice.h"
#include "jtag.h"
#include "jtag2.h"

#include "js.hh"

#include "jsjtag.hh"
#include "jsbreakpoint.hh"

#define JS_GET_PRIVATE_BREAKPOINT()                                                 \
	JSBreakpoint *bp = (JSBreakpoint *)JS_GetInstancePrivate(cx, obj, &jsbreakpoint_class, NULL); \
	if (!bp) {                                                                \
	JS_ReportError(cx, "Could not get private JTAG breakpoint instance");						\
	return JS_FALSE; \
	}

JSBool jsBreakpoint_enable(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_disable(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_add(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_delete(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

/* breakpoint javascript native definitions */
static JSFunctionSpec jsbreakpoint_functions[] = {
	{ "enable",  jsBreakpoint_enable,  0, JSPROP_ENUMERATE, 0 },
	{ "disable", jsBreakpoint_disable, 0, JSPROP_ENUMERATE, 0 },
	{ "add",     jsBreakpoint_add,     0, JSPROP_ENUMERATE, 0 },
	{ "delete",  jsBreakpoint_delete,  0, JSPROP_ENUMERATE, 0 },
	{ 0 }
};

enum jsbreakpoint_propID {
	JSBREAKPOINT_ADDRESS      = 1,
	JSBREAKPOINT_MASK_POINTER,
	JSBREAKPOINT_TYPE,
	JSBREAKPOINT_ENABLED,
	JSBREAKPOINT_ICESTATUS
};

static JSPropertySpec jsbreakpoint_props[] = {
	{ "enabled",     JSBREAKPOINT_ENABLED,      JSPROP_ENUMERATE | JSPROP_READONLY },
	{ "iceStatus",   JSBREAKPOINT_ICESTATUS,    JSPROP_ENUMERATE | JSPROP_READONLY },
	{ 0 }
};

JSBool jsBreakpoint_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp);
JSBool jsBreakpoint_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp);

JSClass jsbreakpoint_class = {
	"breakpoint", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	jsBreakpoint_getProperty, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

/* JTAG breakpoint definitions */

JSObject *jsBreakpoint_NewObject(JSContext *cx, JSObject *global,
																 JSJtag *theJtag, breakpoint2 *bp) {
	JSObject *obj = JS_NewObject(cx, &jsbreakpoint_class, NULL, NULL);
	if (!obj) {
		JS_ReportError(cx, "Could not create JTAG breakpoint object");
		return NULL;
	}

	JSBreakpoint *theBreakpoint;
	theBreakpoint = (JSBreakpoint *)JS_malloc(cx, sizeof(*theBreakpoint));
	if (!theBreakpoint) {
		return NULL;
	}
	memset(theBreakpoint, 0, sizeof(*theBreakpoint));
	theBreakpoint->jtag = theJtag;

	jsval addrVal = INT_TO_JSVAL(bp->address);
	jsval typeVal = INT_TO_JSVAL(bp->type);
	jsval maskVal = INT_TO_JSVAL(bp->mask_pointer);
	
	if (!JS_SetPrivate(cx, obj, theBreakpoint)) {
		JS_ReportError(cx, "Could not set private JTAG breakpoint object");
		goto error;
	}

	if (!JS_SetProperty(cx, obj, "address", &addrVal) ||
			!JS_SetProperty(cx, obj, "type", &typeVal) ||
			!JS_SetProperty(cx, obj, "mask_pointer", &maskVal)) {
		JS_ReportError(cx, "Could not initialize breakpoint properties");
		goto error;
	}

	return obj;

 error:
	JS_free(cx, theBreakpoint);
	return NULL;
}

/* JTAG breakpoint properties */
JSBool jsBreakpoint_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	JS_GET_PRIVATE_BREAKPOINT();

	jsint slot = JSVAL_TO_INT(idval);
	switch (slot) {
	case JSBREAKPOINT_ENABLED:
		//		*vp = BOOLEAN_TO_JSVAL(origBp->enabled);
		return JS_FALSE;
		return JS_TRUE;
		break;

	case JSBREAKPOINT_ICESTATUS:
		//		*vp = BOOLEAN_TO_JSVAL(origBp->icestatus);
		return JS_FALSE;
		return JS_TRUE;
		break;
		
	default:
		return JS_TRUE;
	}
}

bool jsBreakpoint_registerClass(JSContext *cx, JSObject *global) {
	JSObject *breakpointClass = JS_InitClass(cx, global, NULL, &jsbreakpoint_class,
																					 NULL, 0, jsbreakpoint_props, jsbreakpoint_functions,
																					 NULL, NULL);
	if (!breakpointClass) {
		JS_ReportError(cx, "Could not initialize JTAG breakpoint class");
		return false;
	}
	
	return true;
}
