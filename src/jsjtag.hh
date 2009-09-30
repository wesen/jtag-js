#ifndef JS_JTAG_H__
#define JS_JTAG_H__

#include "jtag2.h"

bool jsJtag_registerClass(JSContext *cx, JSObject *global);

/* private instance data */
class JSJtag {
public:
	jtag     *origJtag;
};


#endif /* JS_JTAG_H__ */
