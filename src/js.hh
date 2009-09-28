#ifndef JS_H__
#define JS_H__

#include <string>
#define XP_UNIX 1
#include <js/jsapi.h>

class JavaScript {
protected:
	JSRuntime *rt;
	JSContext *cx;
	JSObject *global;

	static JSClass global_class;

public:
	JavaScript();
	~JavaScript();

	bool init();
	void eval(const std::string &str);
};

#endif
