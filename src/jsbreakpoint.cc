#include "avarice.h"
#include "jtag.h"
#include "jtag2.h"

#include "js.hh"

#include "jsjtag.hh"
#include "jsbreakpoint.hh"

#define JS_GET_PRIVATE_BREAKPOINT()                                                 \
	breakpoint2 *bp = (breakpoint2 *)JS_GetInstancePrivate(cx, obj, &jsbreakpoint_class, NULL); \
	if (!bp) {                                                                \
	JS_ReportError(cx, "Could not get private JTAG breakpoint instance");						\
	return JS_FALSE; \
	}


JSBool jsBreakpoint_update(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_enable(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_disable(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_add(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
	JS_REPORT_UNIMPLEMENTED();
	return JS_FALSE;
}

JSBool jsBreakpoint_delete(JSContext *cx, JSObject *obj, uintN argc,
													 jsval *argv, jsval *rval) {
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

static JSClass jsbreakpoint_class = {
	"breakpoint", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	jsBreakpoint_getProperty, jsBreakpoint_setProperty,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

/* JTAG breakpoint definitions */

JSObject *jsBreakpoint_NewObject(JSContext *cx, JSObject *global, breakpoint2 *bp) {
	JSObject *obj = JS_NewObject(cx, &jsbreakpoint_class, NULL, NULL);
	if (!obj) {
		JS_ReportError(cx, "Could not create JTAG breakpoint object");
		return NULL;
	}
	if (!JS_SetPrivate(cx, obj, bp)) {
		JS_ReportError(cx, "Could not set private breakpoint object");
		return NULL;
	}

	return obj;
}

/* JTAG breakpoint properties */
JSBool jsBreakpoint_getProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	JS_GET_PRIVATE_BREAKPOINT();
	
	jsint slot = JSVAL_TO_INT(idval);
	switch (slot) {
	default:
		return JS_TRUE;
	}
}

JSBool jsBreakpoint_setProperty(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	JS_GET_PRIVATE_BREAKPOINT();
	
	jsint slot = JSVAL_TO_INT(idval);
	switch (slot) {
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
