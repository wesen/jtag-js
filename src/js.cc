#include "js.hh"
#include "terminal-io.hh"

JavaScript::JavaScript() {
	rt = NULL;
	cx = NULL;
	global = NULL;
}

JavaScript::~JavaScript() {
	if (cx != NULL) {
		JS_DestroyContext(cx);
		cx = NULL;
	}
	if (rt != NULL) {
		JS_DestroyRuntime(rt);
		rt = NULL;
	}
	JS_ShutDown();
}

JSClass JavaScript::global_class = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

void reportError(JSContext *cx, const char *message, JSErrorReport *report) {
	CONSOLE_PRINTF("%s:%u:%s\n", report->filename ? report->filename : "<no filename>",
								 (unsigned int)report->lineno, message);
}

bool JavaScript::init() {
	rt = JS_NewRuntime(8L * 1024L * 1024L);
	if (!rt)
		return false;

	cx = JS_NewContext(rt, 8192);
	if (!cx)
		return false;

	global = JS_NewObject(cx, &global_class, NULL, NULL);
	if (!global)
		return false;

	JS_SetErrorReporter(cx, reportError);
	
	if (!JS_InitStandardClasses(cx, global)) {
		return false;
	}
}

void JavaScript::eval(const std::string &str) {
	const char *src = str.c_str();
	jsval rval;
	if (!JS_EvaluateScript(cx, JS_GetGlobalObject(cx), src, strlen(src), __FILE__, __LINE__, &rval)) {
		return;
	} else {
		jsdouble d;
		if (JS_ValueToNumber(cx, rval, &d)) {
			CONSOLE_PRINTF("got number %f\n", d);
		}
	}
}
