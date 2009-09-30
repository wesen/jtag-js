#ifndef JS_JTAG_H__
#define JS_JTAG_H__

class JSJtag {
public:
	jtag     *origJtag;
};

bool jsJtag_registerClass(JSContext *cx, JSObject *global);

#endif /* JS_JTAG_H__ */
