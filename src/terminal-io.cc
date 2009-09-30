#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "terminal-io.hh"

using namespace std;

TerminalIOClass::TerminalIOClass(const char *name) {
  terminalIO = this;
  rl_event_hook = &TerminalIOClass::readlinePoll;
  rl_readline_name = name;
}

TerminalIOClass::~TerminalIOClass() {
}

void TerminalIOClass::print(const string &str) {
  inputQueue.putData(str);
}

TerminalIOClass *TerminalIOClass::terminalIO = NULL;

int TerminalIOClass::readlinePoll() {
	if (terminalIO->stopRequested) {
		rl_done = 1;
		return 0;
	}
	
  while (terminalIO->inputQueue.isDataAvailable()) {
    const string *str = terminalIO->inputQueue.getData();
    char buf[TERMINAL_BUF_SIZE];
    if (rl_point > 0) {
      snprintf(buf, sizeof(buf), "...\n\n%s", str->c_str());
    } else {
      snprintf(buf, sizeof(buf), "\r%s", str->c_str());
    }
    
    printf("%s", buf);

		delete str;

		fflush(stdout);
		rl_on_new_line();
		rl_redisplay();
	}
	
		
	return 0;
}

void TerminalIOClass::doWork() {
	while (!stopRequested) {
    char *line = readline("> ");
    if (!line || stopRequested)
      break;
		if (line && *line) {
			add_history(line);
		}
    outputQueue.putData(line);
  }
	rl_reset_after_signal();
}

bool TerminalIOClass::isDataAvailable() {
  return outputQueue.isDataAvailable();
}

const string *TerminalIOClass::getData() {
  return outputQueue.getData();
}


