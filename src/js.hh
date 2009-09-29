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
	JSObject *jtagObject;

	static JSClass global_class;
	static JSClass jtag_class;

public:
	JavaScript();
	~JavaScript();

	bool load(const char *filename);

	bool init();
	void eval(const std::string &str);
	bool jsJtag_registerClass();
};

extern JavaScript *theJS;


#endif
