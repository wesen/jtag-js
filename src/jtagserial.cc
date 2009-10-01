/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2001 Scott Finneran
 *      Copyright (C) 2002, 2003, 2004 Intel Corporation
 *	Copyright (C) 2005, 2007 Joerg Wunsch
 *  Copyright (C) 2009 Manuel Odendahl
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
 * This file implements the opening and closing of serial ports.
 *
 * $Id$
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
#include <errno.h>

#include "avarice.h"
#include "jtag.h"

void jtag::restoreSerialPort()
{
  if (!is_usb && jtagBox >= 0 && oldtioValid)
		tcsetattr(jtagBox, TCSANOW, &oldtio);
}

void jtag::openSerialPort(const char *portName) {
	struct termios newtio;

	// Open modem device for reading and writing and not as controlling
	// tty because we don't want to get killed if linenoise sends
	// CTRL-C.
	jtagBox = open(portName, O_RDWR | O_NOCTTY | O_NONBLOCK);
	unixCheck(jtagBox, "Failed to open %s", portName);

	// save current serial port settings and plan to restore them on exit
	jtagCheck(tcgetattr(jtagBox, &oldtio));
	oldtioValid = true;

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = CS8 | CLOCAL | CREAD;

	// set baud rates in a platform-independent manner
	jtagCheck(cfsetospeed(&newtio, B19200));
	jtagCheck(cfsetispeed(&newtio, B19200));

	// IGNPAR  : ignore bytes with parity errors
	//           otherwise make device raw (no other input processing)
	newtio.c_iflag = IGNPAR;

	// Raw output.
	newtio.c_oflag = 0;

	// Raw input.
	newtio.c_lflag = 0;

	// The following configuration should cause read to return if 2
	// characters are immediately avaible or if the period between
	// characters exceeds 5 * .1 seconds.
	newtio.c_cc[VTIME]    = 5;     // inter-character timer unused
	newtio.c_cc[VMIN]     = 255;   // blocking read until VMIN character
	// arrives

	// now clean the serial line and activate the settings for the port
	jtagCheck(tcflush(jtagBox, TCIFLUSH));
	jtagCheck(tcsetattr(jtagBox,TCSANOW,&newtio));
}

/** Change bitrate of PC's serial port as specified by BIT_RATE_xxx in
    'newBitRate' **/
void jtag::changeLocalBitRate(int newBitRate)
{
	if (is_usb)
		return;

	// Change the local port bitrate.
	struct termios tio;

	jtagCheck(tcgetattr(jtagBox, &tio));

	speed_t newPortSpeed = B19200;
	// Linux doesn't support 14400. Let's hope it doesn't end up there...
	switch(newBitRate)
    {
    case 9600:
			newPortSpeed = B9600;
			break;
    case 19200:
			newPortSpeed = B19200;
			break;
    case 38400:
			newPortSpeed = B38400;
			break;
    case 57600:
			newPortSpeed = B57600;
			break;
    case 115200:
			newPortSpeed = B115200;
			break;
    default:
			console->debugOut("unsupported bitrate: %d\n", newBitRate);
			exit(1);
    }

	cfsetospeed(&tio, newPortSpeed);
	cfsetispeed(&tio, newPortSpeed);

	jtagCheck(tcsetattr(jtagBox,TCSANOW,&tio));
	jtagCheck(tcflush(jtagBox, TCIFLUSH));
}

