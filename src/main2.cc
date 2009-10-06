#include <stdio.h>
#include <signal.h>
#include <list>
#include <map>

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
TCPServer server("localhost", 8181);
JavaScript myJS;

JavaScript *theJS = &myJS;
jtag *theJtagICE = NULL;

ThreadSafeList<LineIOClass *> listeners;
map<LineIOClass *, string *> listenerBufs;

void signal_handler(int signal) {
	printf("SIGINT\n");
	server.stop();
	list<LineIOClass *> *l = listeners.getLockedObject();

	for (list<LineIOClass *>::iterator it = l->begin();
			 it != l->end();
			 it++) {
		ThreadedClass *io = (ThreadedClass *)*it;
		// xXX igitt hackk hackk
		io->stop();
	}
	listeners.unlock();
}

void printListeners(const char *buf) {
	list<LineIOClass *> *l = listeners.getLockedObject();

	for (list<LineIOClass *>::iterator it = l->begin();
			 it != l->end();
			 it++) {
		LineIOClass *io = *it;
		io->print(buf);
	}
	listeners.unlock();
}

bool ThreadSynchronization::debug = false;

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
				TerminalIOClass::currentIO = io;
				string *buf = listenerBufs[io];
				if (buf == NULL) {
					buf = listenerBufs[io] = new string();
				}

				const string *str = io->getData();
				*buf += *str;
				delete str;
					
				if (myJS.isCompilable(*buf)) {
					const string *res = myJS.eval(*buf);
					buf->clear();
					io->print(*res);
					delete res;
				}
			}
	  }
		listeners.unlock();
	}
	printf("cleaning up...\n");
	rl_cleanup_after_signal();

	return 0;
}
