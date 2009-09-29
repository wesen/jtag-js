#include <stdio.h>
#include <signal.h>

#include "avarice.h"

#include "jtag.h"
#include "jtag2.h"

#include "js.hh"

#include <readline/readline.h>
#include "terminal-io.hh"

TerminalIOClass terminal;
JavaScript myJS;

JavaScript *theJS = &myJS;
jtag *theJtagICE = NULL;

void signal_handler(int signal) {
	printf("SIGINT\n");
	terminal.stop();
}

int main(int argc, char *argv[]) {
	debugMode = true;

	myJS.init();

	for (int i = 1; i < argc; i++) {
		printf("loading %s\n", argv[i]);
		myJS.load(argv[i]);
	}
	terminal.go();
	
	for (;;) {
		bool interrupt = false;
		bool breakpoint = false;

		if (theJtagICE) {
			theJtagICE->pollDevice(&interrupt, &breakpoint);
		} else {
			usleep(10000);
		}
	
		if (!terminal.isRunning()) {
			break;
		}

	  if (terminal.isDataAvailable()) {
	    const string *str = terminal.getData();
			myJS.eval(*str);
	    delete str;
	  }
	}
	rl_cleanup_after_signal();

	return 0;
}
