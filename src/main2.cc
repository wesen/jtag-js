#include <stdio.h>

#include "avarice.h"

#include "jtag.h"
#include "jtag2.h"

#include "js.hh"

#include "terminal-io.hh"

TerminalIOClass terminal;
JavaScript myJS;

jtag *theJtagICE = NULL;

int main(int argc, char *argv[]) {
	unsigned long jtagBitrate = 0;
	const char *jtagDeviceName = NULL;
	char *deviceName = 0;

	bool isXmega = false;
	bool isDragon = false;
	bool applyNsrst = false;
	const char *eventList = "none,run,target_power_on,target_sleep,target_wakeup";
	
	enum {
		MKI, MKII, MKII_DW
	} protocol = MKII;

	debugMode = true;

	myJS.init();
	
	jtagBitrate = 250000;
	jtagDeviceName = "usb";
	//	deviceName = "at90usb1287";

	try {
		theJtagICE = new jtag2(jtagDeviceName, deviceName, false, isDragon, applyNsrst, isXmega);
		theJtagICE->dchain.units_before = 0;
		theJtagICE->dchain.units_after = 0;
		theJtagICE->dchain.bits_before = 0;
		theJtagICE->dchain.bits_after = 0;
		theJtagICE->parseEvents(eventList);
		
		theJtagICE->initJtagBox();

	} catch (const char *msg) {
		fprintf(stderr, "%s\n", msg);
		return 1;
	} catch (...) {
		fprintf(stderr, "Cannot initialize JTAG ICE\n");
		return 1;
	}

	printf("init jtag\n");

	terminal.go();

	for (;;) {
	  if (terminal.isDataAvailable()) {
	    const string *str = terminal.getData();
			myJS.eval(*str);
	    delete str;
	  }
	}
}
