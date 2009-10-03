#ifndef JS_H__
#define JS_H__

#include <string>
#define XP_UNIX 1
#include <js/jsapi.h>

#define JS_REPORT_UNIMPLEMENTED() JS_ReportError(cx, "Feature not implemented yet")

#define JS_NEW_STRING(s)     JS_NewString(cx, (s), strlen(s))
#define JS_NEW_STRING_VAL(s) STRING_TO_JSVAL(JS_NEW_STRING(s))

class JavaScript {
protected:
	JSRuntime *rt;
	JSContext *cx;
	JSObject *global;

	static JSClass global_class;

public:
	JSObject *jtagObject;

	JavaScript();
	~JavaScript();

	bool load(const char *filename);

	bool init();
	void eval(const std::string &str);
};

extern JavaScript *theJS;


#endif
