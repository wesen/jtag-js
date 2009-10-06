#include <stdio.h>
#include <signal.h>
#include <list>

#include "avarice.h"

#include "jtag.h"
#include "jtag2.h"

#include "js.hh"

#include <readline/readline.h>

#include "thread.hh"
#include "terminal-io.hh"
#include "tcp.hh"

using namespace std;

TerminalIOClass terminal;
JavaScript myJS;

JavaScript *theJS = &myJS;
jtag *theJtagICE = NULL;

void signal_handler(int signal) {
	printf("SIGINT\n");
	terminal.stop();
}

ThreadSafeList<LineIOClass *> listeners;

void printListeners(const char *buf) {
	list<LineIOClass *> *l = listeners.getLockedObject();

	for (list<LineIOClass *>::iterator it = l->begin();
			 it != l->end();
			 it++) {
		LineIOClass *io = *it;
		//		printf("print %s to %p\n", buf, io);
		io->print(buf);
	}
	listeners.unlock();
}

int main(int argc, char *argv[]) {
	Console mainConsole;
	console = &mainConsole;

	Console::debugMode = true;
	Console::quietMode = true;

	myJS.init();

	terminal.start();
	listeners.push_front(&terminal);

	for (int i = 1; i < argc; i++) {
		myJS.load(argv[i]);
	}

	TCPServer server("localhost", 8181);
	server.start();
	
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

		/* locked listeners section */
		list<LineIOClass *> *l = listeners.getLockedObject();
		for (list<LineIOClass *>::iterator it = l->begin();
				 it != l->end();
				 it++) {
			LineIOClass *io = *it;
			while (io->isDataAvailable()) {
				const string *str = io->getData();
				// XXX get result as string
				myJS.eval(*str);
				delete str;
			}
	  }
		listeners.unlock();
	}
	printf("cleaning up...\n");
	rl_cleanup_after_signal();

	return 0;
}
