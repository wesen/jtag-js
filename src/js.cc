#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

#include "avarice.h"
#include "jtag.h"

#include "js.hh"
#include "terminal-io.hh"

/* exported functions */
JSBool myjs_print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	const char *str;
	if (!JS_ConvertArguments(cx, argc, argv, "s", &str)) {
		return JS_FALSE;
	}

	TerminalIOClass::printTerminal(str);
	*rval = JSVAL_VOID;
	return JS_TRUE;
}

JSBool myjs_quit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	*rval = JSVAL_VOID;
	printf("Exiting...\n");
	TerminalIOClass::terminalIO->stop();
	kill(getpid(), SIGINT);
	rl_cleanup_after_signal();
	return JS_TRUE;
}

JSBool myjs_readpc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
	*rval = JSVAL_VOID;
	theJtagICE->jtagReadFuses();
	//	unsigned long pc = theJtagICE->getProgramCounter();
	//	printf("pc: %ul\n", pc);
	return JS_TRUE;
}

static JSFunctionSpec myjs_global_functions[] = {
	{ "print", myjs_print, 1, 0, 0 },
	{ "quit", myjs_quit, 0, 0, 0 },
	{ "exit", myjs_quit, 0, 0, 0 },
	{ "readPC", myjs_readpc, 0, 0, 0},
	{ 0 }
};

/* class definitions */
JSClass JavaScript::global_class = {
	"global", JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

/* helpers */
void reportError(JSContext *cx, const char *message, JSErrorReport *report) {
	CONSOLE_PRINTF("%s:%u:%s\n", report->filename ? report->filename : "<no filename>",
								 (unsigned int)report->lineno, message);
}

/* Javascript class */
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

	if (!JS_DefineFunctions(cx, global, myjs_global_functions)) {
		return false;
	}
}

void JavaScript::eval(const std::string &str) {
	const char *src = str.c_str();
	jsval rval;
	if (!JS_EvaluateScript(cx, JS_GetGlobalObject(cx), src, strlen(src), __FILE__, __LINE__, &rval)) {
		return;
	} else {
		JSString *str = JS_ValueToString(cx, rval);
		if (str != NULL) {
			CONSOLE_PRINTF("%s\n", (JS_GetStringBytes(str)));
		} else {
			TerminalIOClass::printTerminal("<undefined>\n");
		}
	}
}

