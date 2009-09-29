#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
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

/* functions stolen from spidermonkey itself */
JSBool myjs_load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    uintN i;
    JSString *str;
    const char *filename;

    for (i = 0; i < argc; i++) {
			str = JS_ValueToString(cx, argv[i]);
			if (!str)
				return JS_FALSE;
			argv[i] = STRING_TO_JSVAL(str);
			filename = JS_GetStringBytes(str);
			if (!theJS->load(filename)) {
				return JS_FALSE;
			}
    }

    return JS_TRUE;
}

static JSFunctionSpec myjs_global_functions[] = {
	{ "print", myjs_print, 1, 0, 0 },
	{ "quit", myjs_quit, 0, 0, 0 },
	{ "exit", myjs_quit, 0, 0, 0 },
	{ "load", myjs_load, 1, 0, 0 },
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
	jtagObject = NULL;
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

JSBool jsGlobal_getDebugMode(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	*vp = BOOLEAN_TO_JSVAL(debugMode);
	return JS_TRUE;
}

JSBool jsGlobal_setDebugMode(JSContext *cx, JSObject *obj, jsval idval, jsval *vp) {
	if (JSVAL_IS_BOOLEAN(*vp)) {
		debugMode = JSVAL_TO_BOOLEAN(*vp);
		return JS_TRUE;
	} else {
		return JS_FALSE;
	}
}

bool JavaScript::load(const char *filename) {
	JSScript *script;
	JSBool ok;
	jsval result;
	uint32 oldopts;
	
	oldopts = JS_GetOptions(cx);
	JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
	char cwd[512];

	char *ptr = getcwd(cwd, sizeof(cwd));
	char *dir = dirname((char *)filename);

	script = JS_CompileFile(cx, global, filename);
	if (!script) {
		ok = JS_FALSE;
	} else {
		if (ptr && dir) {
			chdir(dir);
		}
		char realpathbuf[4096];
		CONSOLE_PRINTF("loading file \"%s\"\n", realpath(filename, realpathbuf));
		ok = JS_ExecuteScript(cx, global, script, &result);
		JS_DestroyScript(cx, script);
		if (ptr && dir) {
			chdir(ptr);
		}
	}
	JS_SetOptions(cx, oldopts);
	if (!ok)
		return false;
	else
		return true;
}
	

bool JavaScript::init() {
	rt = JS_NewRuntime(8L * 1024L * 1024L);
	if (!rt)
		return false;

	cx = JS_NewContext(rt, 8192);
	if (!cx)
		return false;

	JS_SetErrorReporter(cx, reportError);

	global = JS_NewObject(cx, &global_class, NULL, NULL);
	if (!global)
		return false;
	if (!JS_InitStandardClasses(cx, global)) {
		return false;
	}
	if (!JS_DefineFunctions(cx, global, myjs_global_functions)) {
		return false;
	}
	JS_DefineProperty(cx, global, "debug", BOOLEAN_TO_JSVAL(debugMode),
										jsGlobal_getDebugMode, jsGlobal_setDebugMode, JSPROP_PERMANENT);

	jsJtag_registerClass();

	return true;
}

void JavaScript::eval(const std::string &str) {
	const char *src = str.c_str();
	jsval rval;

	JSBool ret = JS_EvaluateScript(cx, JS_GetGlobalObject(cx), src, strlen(src), __FILE__, __LINE__, &rval);

	if (!ret) {
		return;
	} else {
		JSString *str = JS_ValueToString(cx, rval);
		if (str != NULL) {
			// XXX split into chunks of TERMINAL_BUF_SIZE bytes
			CONSOLE_PRINTF("%s\n", (JS_GetStringBytes(str)));
		} else {
			TerminalIOClass::printTerminal("<undefined>\n");
		}
	}
}

