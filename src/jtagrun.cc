/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2001 Scott Finneran
 *      Copyright (C) 2002 Intel Corporation
 *	Copyright (C) 2005, 2007 Joerg Wunsch
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License Version 2
 *      as published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * This file contains functions for interfacing with the JTAG box.
 *
 * $Id: jtagrun.cc,v 1.7 2008/06/12 20:21:44 joerg_wunsch Exp $
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

#include "avarice.h"
#include "jtag.h"
#include "jtag1.h"
#include "remote.h"

unsigned long jtag1::getProgramCounter(void)
{
    uint8_t *response = NULL;
    uint8_t command[] = {'2', JTAG_EOM };
    unsigned long result = 0;

    response = doJtagCommand(command, sizeof(command), 4);

    if (response[3] != JTAG_R_OK)
	result = PC_INVALID;
    else
    {
	result = decodeAddress(response);

	result--; // returned value is PC + 1 as far as GDB is concerned

	// The JTAG box sees program memory as 16-bit wide locations. GDB
	// sees bytes. As such, double the PC value.
	result *= 2;
    }

    delete [] response;
    return result;
}

bool jtag1::setProgramCounter(unsigned long pc)
{
    uint8_t *response = NULL;
    uint8_t command[] = {'3', 0, 0, 0, JTAG_EOM };
    bool result;

    // See decoding in getProgramCounter
    encodeAddress(&command[1], pc / 2 + 1);

    response = doJtagCommand(command, sizeof(command), 1);

    result = response[0] == JTAG_R_OK;

    delete [] response;

    return result;
}

bool jtag1::resetProgram(bool possible_nSRST)
{
  bool result;

  if (possible_nSRST && apply_nSRST) {
    setJtagParameter(JTAG_P_EXTERNAL_RESET, 0x01);
  }
  result = doSimpleJtagCommand('x', 1);
  if (possible_nSRST && apply_nSRST) {
    setJtagParameter(JTAG_P_EXTERNAL_RESET, 0x00);
  }
  return result;
}

bool jtag1::interruptProgram(void)
{
    // Just ignore the returned PC. It appears to be wrong if the most
    // recent instruction was a branch.
    return doSimpleJtagCommand('F', 4);
}

bool jtag1::resumeProgram(void)
{
    return doSimpleJtagCommand('G', 0);
}

bool jtag1::jtagSingleStep(bool useHLL)
{
    return doSimpleJtagCommand('1', 1);
}

void jtag1::parseEvents(const char *)
{
    // current no event name parsing in mkI
}

void jtag1::startPolling() {
}

bool jtag1::pollDevice(bool *gdbInterrupt, bool *breakpoint) {
#ifdef GDB_FUNCTIONALITY_XXX
	// Now that we are "going", wait for either a response from the JTAG
	// box or a nudge from GDB.
	console->debugOut("Waiting for input.\n");

	// Check for input from JTAG ICE (breakpoint, sleep, info, power)
	// or gdb (user break)
	FDSelect fds;
	fds.add(gdbFileDescriptor);
	fds.add(jtagBox);

	int numfds = fds.waitRead();
	unixCheck(numfds, "GDB/JTAG ICE communications failure");

	if (fds.isSet(gdbFileDescriptor))
	{
	    int c = getDebugChar();
	    if (c == 3) // interrupt
	    {
				console->debugOut("interrupted by GDB\n");
				*gdbInterrupt = true;
	    }
	    else
				console->debugOut("Unexpected GDB input `%02x'\n", c);
	}

	// Read all extant responses (there's a small chance there could
	// be more than one)

	// Note: In case of a gdb interrupt, it's possible that we will
	// receive one of these responses before interruptProgram is 
	// called. This should not cause a problem as the normal retry
	// mechanism should eat up this response.
	// It might be cleaner to handle JTAG_R_xxx in sendJtagCommand

	// Check the JTAG ICE's message. It can indicate a breakpoint
	// which causes us to return, a sleep status change (ignored),
	// power "event" -- whatever that is (ignored), or a byte of
	// info sent by the program (currently ignored, could be used
	// for something...)
	uint8_t response;

        // This read shouldn't need to be a timeout_read(), but some cygwin
        // systems don't seem to honor the O_NONBLOCK flag on file
        // descriptors.
	while (timeout_read(&response, 1, 1) == 1)
	{
	    uint8_t buf[2];
	    int count;

	    console->debugOut("JTAG box sent %c", response);
	    switch (response)
	    {
	    case JTAG_R_BREAK:
				count = timeout_read(buf, 2, JTAG_RESPONSE_TIMEOUT);
				jtagCheck(count);
				check(count == 2, JTAG_CAUSE);
				*breakpoint = true;
				console->debugOut(": Break Status Register = 0x%02x%02x\n",
                         buf[0], buf[1]);
		break;
	    case JTAG_R_INFO: case JTAG_R_SLEEP:
				// we could do something here, esp. for info
				count = timeout_read(buf, 2, JTAG_RESPONSE_TIMEOUT);
				jtagCheck(count);
				check(count == 2, JTAG_CAUSE);
				console->debugOut(": 0x%02, 0x%02\n", buf[0], buf[1]);
				break;
				
	    case JTAG_R_POWER:
				// apparently no args?
				console->debugOut("\n");
				break;
				
	    default:
				console->debugOut(": Unknown response\n");
				break; 
	    }
	}
#endif
}

bool jtag1::jtagContinue(void)
{
	bool breakpoint = false, gdbInterrupt = false;

#ifdef GDB_FUNCTIONALITY_XXX
	updateBreakpoints();        // download new bp configuration

    if (!doSimpleJtagCommand('G', 0))
    {
			gdbOut("Failed to continue\n");
			return true;
    }

    for (;;)
    {
			pollDevice(&gdbInterrupt, &breakpoint);
	}
#endif

	// We give priority to user interrupts
	if (gdbInterrupt)
	    return false;
	if (breakpoint)
	    return true;
}

