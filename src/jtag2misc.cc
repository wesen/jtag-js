/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2005 Joerg Wunsch
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
 * This file contains miscellaneous routines for the mkII protocol.
 *
 * $Id: jtag2misc.cc,v 1.2 2005/05/27 20:42:13 joerg_wunsch Exp $
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

#include "avarice.h"
#include "jtag.h"
#include "jtag2.h"

void jtag2::setJtagParameter(uint8_t item, uint8_t *newValue, int valSize)
{
    int respsize;
    /*
     * As the maximal parameter length is 4 bytes, we use a fixed-length
     * buffer, as opposed to malloc()ing it.
     */
    unsigned char buf[2 + 4], *resp;

    check(valSize <= 4, "Parameter too large in setJtagParameter");

    buf[0] = CMND_SET_PARAMETER;
    buf[1] = item;
    memcpy(buf + 2, newValue, valSize);

    check(doJtagCommand(buf, valSize + 2, resp, respsize),
	  "set paramater command failed");

    delete [] resp;
}

/*
 * Get a JTAG ICE parameter.  Caller must delete [] the response.  Note
 * that the response still includes the response code at index 0 (to be
 * ignored).
 */
void jtag2::getJtagParameter(uint8_t item, uint8_t *&resp, int &respSize)
{
    /*
     * As the maximal parameter length is 4 bytes, we use a fixed-length
     * buffer, as opposed to malloc()ing it.
     */
    unsigned char buf[2];

    buf[0] = CMND_GET_PARAMETER;
    buf[1] = item;

    check(doJtagCommand(buf, 2, resp, respSize),
	  "get paramater command failed");
    check(resp[0] == RSP_PARAMETER && respSize > 1,
	  "unexpected response to get paramater command");
}


