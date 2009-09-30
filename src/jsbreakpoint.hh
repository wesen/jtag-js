#ifndef JS_BREAKPOINT_H__
#define JS_BREAKPOINT_H__

#include <js/jsapi.h>

bool jsBreakpoint_registerClass(JSContext *cx, JSObject *global);

#endif /* JS_BREAKPOINT_H__ */
