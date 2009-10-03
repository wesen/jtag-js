#ifndef JS_H__
#define JS_H__

#include <string.h>
#include <string>
#define XP_UNIX 1
#include <js/jsapi.h>

#define JS_REPORT_UNIMPLEMENTED() JS_ReportError(cx, "Feature not implemented yet")

#define JS_NEW_STRING(s)     JS_NewString(cx, (char *)(s), strlen((char *)s))
#define JS_NEW_STRING_VAL(s) STRING_TO_JSVAL(JS_NEW_STRING(s))

#define JS_SET_PROPERTY_STRING(obj, name, str) \
	{ const char *_str = str; jsval _strVal = JS_NEW_STRING_VAL(_str); \
		JS_SetProperty(cx, obj, name, &_strVal); }

#define JS_SET_PROPERTY_INT(obj, name, i) \
	{ jsval _intVal = INT_TO_JSVAL(i); \
		JS_SetProperty(cx, obj, name, &_intVal); }

#define JS_SET_PROPERTY_BOOLEAN(obj, name, b) \
	{ jsval _bVal = BOOLEAN_TO_JSVAL(b); \
		JS_SetProperty(cx, obj, name, &_bVal); }

#define JS_SET_PROPERTY_OBJECT(obj, name, obj2) \
	{ jsval _objVal = OBJECT_TO_JSVAL(obj2); \
		JS_SetProperty(cx, obj, name, &_objVal); }

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
