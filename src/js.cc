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

/* functions stolen from spidermonkey itself */
JSBool myjs_load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    uintN i;
    JSString *str;
    const char *filename;
    JSScript *script;
    JSBool ok;
    jsval result;
    uint32 oldopts;

    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return JS_FALSE;
        argv[i] = STRING_TO_JSVAL(str);
        filename = JS_GetStringBytes(str);
        oldopts = JS_GetOptions(cx);
        JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
        script = JS_CompileFile(cx, obj, filename);
        if (!script) {
            ok = JS_FALSE;
        } else {
					ok = JS_ExecuteScript(cx, obj, script, &result);
					JS_DestroyScript(cx, script);
        }
        JS_SetOptions(cx, oldopts);
        if (!ok)
            return JS_FALSE;
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


#if 0
  /** Clear out the breakpoints. */
  virtual void deleteAllBreakpoints(void) = 0;

  /** Delete breakpoint at the specified address. */
  virtual bool deleteBreakpoint(unsigned int address, bpType type, unsigned int length) = 0;

  /** Add a code breakpoint at the specified address. */
  virtual bool addBreakpoint(unsigned int address, bpType type, unsigned int length) = 0;

  /** Send the breakpoint details down to the JTAG box. */
  virtual void updateBreakpoints(void) = 0;

  /** True if there is a breakpoint at address */
  virtual bool codeBreakpointAt(unsigned int address) = 0;

  /** True if there is a breakpoint between start (inclusive) and 
      end (exclusive) */
  virtual bool codeBreakpointBetween(unsigned int start, unsigned int end) = 0;

  virtual bool stopAt(unsigned int address) = 0;

  /** Parse a list of event names to *not* cause a break. */
  virtual void parseEvents(const char *) = 0;


  /** Switch to faster programming mode, allows chip erase */
  virtual void enableProgramming(void) = 0;

  /** Switch back to normal programming mode **/
  virtual void disableProgramming(void) = 0;

  /** Erase all chip memory **/
  virtual void eraseProgramMemory(void) = 0;

  virtual void eraseProgramPage(unsigned long address) = 0;

  /** Download an image contained in the specified file. */
  virtual void downloadToTarget(const char* filename, bool program, bool verify) = 0;

  // Running, single stepping, etc
  // -----------------------------

  /** Retrieve the current Program Counter value, or PC_INVALID if fails */
  virtual unsigned long getProgramCounter(void) = 0;

  /** Set program counter to 'pc'. Return true iff successful **/
  virtual bool setProgramCounter(unsigned long pc) = 0;

  /** Reset AVR. Return true iff successful **/
  virtual bool resetProgram(bool possible_nSRST) = 0;

  /** Interrupt AVR. Return true iff successful **/
  virtual bool interruptProgram(void) = 0;

  /** Resume program execution. Return true iff successful.
      Note: the gdb 'continue' command is handled by jtagContinue,
      this is just the low level command to resume after interruptProgram
  **/
  virtual bool resumeProgram(void) = 0;

  /** Issue a "single step" command to the JTAG box. 
      Return true iff successful **/
  virtual bool jtagSingleStep(bool useHLL = false) = 0;

#endif

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

	jtagObject = JS_DefineObject(cx, global, "jtag", &jtag_class, NULL, 0);
	if (jtagObject == NULL)
		return false;
	if (!JS_DefineFunctions(cx, jtagObject, jsjtag_functions))
		return false;

	return true;
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

