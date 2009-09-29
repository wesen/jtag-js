#ifndef JS_JTAG_H__
#define JS_JTAG_H__

class JSJtagBreakpoint {
public:
	/*
	 * unsigned int address
	 * unsigned int mask_pointer
	 * bpType type
	 * bool enabled
	 * bool last
	 * bool icestatus -> true if enable in actual device
	 * bool toremove
	 * bool toadd
	 * uchar bpnum (ICE's breakpoint number)
	 */
};

class JSJtag {
protected:
	jtag     *origJtag;
	JSObject *jsObject;
};

#endif /* JS_JTAG_H__ */
