#ifndef BREAKPOINT_H__
#define BREAKPOINT_H__

class Breakpoint {
public:
	uint32_t address;
	uint32_t maskPointer;

	bpType type;

	bool iceStatus;
	bool toRemove;
	bool toAdd;
	uint8_t bpNum;

	jtag *theJtag;

	Breakpoint(bpType _type, uint32_t _address, uint32_t _maskPointer);
	~Breakpoint();
};

#endif /* BREAKPOINT_H__ */
