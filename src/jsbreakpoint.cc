#include <string.h>

#include "avarice.h"
#include "jtag.h"
#include "jtag2.h"

#include "js.hh"

#include "jsjtag.hh"
#include "jsbreakpoint.hh"

#define JS_GET_PRIVATE_BREAKPOINT()                                                 \
	JSBreakpoint *bp = (JSBreakpoint *)JS_GetInstancePrivate(cx, obj, &jsbreakpoint_class, NULL); \
	breakpoint2 *origBp = bp->bp; \
	if (!bp) {                                                                \
	JS_ReportError(cx, "Could not get private JTAG breakpoint instance");						\
	return JS_FALSE; \
	}

#define JS_BREAKPOINT_AVAILABLE_CHECK() \
		if (!origBp) {																							\
      JS_ReportError(cx, "JTAG breakpoint not available yet, call jtag.init() first"); \
      return JS_FALSE;																									\
		}

JSBool jsBreakpoint_update(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	JS_BREAKPOINT_AVAILABLE_CHECK();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_enable(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	JS_BREAKPOINT_AVAILABLE_CHECK();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_disable(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	JS_BREAKPOINT_AVAILABLE_CHECK();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_add(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	JS_BREAKPOINT_AVAILABLE_CHECK();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_delete(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_GET_PRIVATE_BREAKPOINT();
	JS_BREAKPOINT_AVAILABLE_CHECK();
	
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

/* breakpoint javascript native definitions */
static JSFunctionSpec jsbreakpoint_functions[] = {
	{ "update",  jsBreakpoint_update,  0, JSPROP_ENUMERATE, 0 },
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
	{ "address",     JSBREAKPOINT_ADDRESS,      JSPROP_ENUMERATE },
	{ "maskPointer", JSBREAKPOINT_MASK_POINTER, JSPROP_ENUMERATE },
	{ "type",        JSBREAKPOINT_TYPE,         JSPROP_ENUMERATE },
	{ "enabled",     JSBREAKPOINT_ENABLED,      JSPROP_ENUMERATE | JSPROP_READONLY },
	{ "iceStatus",   JSBREAKPOINT_ICESTATUS,    JSPROP_ENUMERATE | JSPROP_READONLY },
	{ 0 }
};

JSBool jsBreakpoint_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp);
JSBool jsBreakpoint_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp);

JSClass jsbreakpoint_class = {
	"breakpoint", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	jsBreakpoint_getProperty, jsBreakpoint_setProperty,
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
	theBreakpoint->bp = bp;

	if (!JS_SetPrivate(cx, obj, theBreakpoint)) {
		JS_ReportError(cx, "Could not set private JTAG breakpoint object");
		JS_free(cx, theBreakpoint);
		return NULL;
	}

	return obj;
}

/* JTAG breakpoint properties */
JSBool jsBreakpoint_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	JS_GET_PRIVATE_BREAKPOINT();
	JS_BREAKPOINT_AVAILABLE_CHECK();
	
	jsint slot = JSVAL_TO_INT(idval);
	switch (slot) {
	case JSBREAKPOINT_ADDRESS:
		*vp = INT_TO_JSVAL((uint32_t)(origBp->address));
		return JS_TRUE;
		break;

	case JSBREAKPOINT_MASK_POINTER:
		*vp = INT_TO_JSVAL((uint32_t)(origBp->mask_pointer));
		return JS_TRUE;
		break;

	case JSBREAKPOINT_TYPE:
		*vp = INT_TO_JSVAL((uint32_t)(origBp->type));
		return JS_TRUE;
		break;

	case JSBREAKPOINT_ENABLED:
		*vp = BOOLEAN_TO_JSVAL(origBp->enabled);
		return JS_TRUE;
		break;

	case JSBREAKPOINT_ICESTATUS:
		*vp = BOOLEAN_TO_JSVAL(origBp->icestatus);
		return JS_TRUE;
		break;
		
	default:
		return JS_TRUE;
	}
}

JSBool jsBreakpoint_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	JS_GET_PRIVATE_BREAKPOINT();
	JS_BREAKPOINT_AVAILABLE_CHECK();
	
	jsint slot = JSVAL_TO_INT(idval);
	switch (slot) {
	case JSBREAKPOINT_ADDRESS:
		return JS_TRUE;
		break;

	case JSBREAKPOINT_MASK_POINTER:
		return JS_TRUE;
		break;

	case JSBREAKPOINT_TYPE:
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
