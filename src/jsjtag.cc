#include "avarice.h"
#include "jtag.h"

#include "js.hh"


JSBool jsJtag_getProgramCounter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	unsigned long pc = theJtagICE->getProgramCounter();
	return JS_NewNumberValue(cx, pc, rval);
}

JSBool jsJtag_setProgramCounter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	uint32_t pc;
	if (!JS_ConvertArguments(cx, argc, argv, "u", &pc))
		return JS_FALSE;

	bool ret = theJtagICE->setProgramCounter(pc);

	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_resetProgram(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	JSBool possibleReset = JS_FALSE;
	if (!JS_ConvertArguments(cx, argc, argv, "/b", &possibleReset))
		return JS_FALSE;
	
	bool ret = theJtagICE->resetProgram(possibleReset); // JSBool can be used directly from C

	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_interruptProgram(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	bool ret = theJtagICE->interruptProgram();
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_resumeProgram(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	bool ret = theJtagICE->resumeProgram();
	
	*rval = BOOLEAN_TO_JSVAL(ret);
	return JS_TRUE;
}

JSBool jsJtag_read(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
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

static JSFunctionSpec jsjtag_functions[] = {
	{ "getPC", jsJtag_getProgramCounter, 0, 0, 0 },
	{ "setPC", jsJtag_setProgramCounter, 1, 0, 0 },
	{ "reset", jsJtag_resetProgram, 0, 0, 0 },
	{ "interrupt", jsJtag_interruptProgram, 0, 0, 0 },
	{ "resume", jsJtag_resumeProgram, 0, 0, 0 },
	{ "read", jsJtag_read, 2, 0, 0 },
	{ "write", jsJtag_write, 2, 0, 0 },
	{ 0 }
};

/* class definitions */
JSClass JavaScript::jtag_class = {
	"jtag", 0,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
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
