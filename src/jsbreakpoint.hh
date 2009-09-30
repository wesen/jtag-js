#ifndef JS_BREAKPOINT_H__
#define JS_BREAKPOINT_H__

#include <js/jsapi.h>

class JSJtag;

/* private instance data */
class JSBreakpoint {
public:
	JSJtag *jtag;
};

JSObject *jsBreakpoint_NewObject(JSContext *cx, JSObject *global,
																 JSJtag *theJtag, breakpoint2 *bp);

bool jsBreakpoint_registerClass(JSContext *cx, JSObject *global);

extern JSClass jsbreakpoint_class;

#endif /* JS_BREAKPOINT_H__ */
