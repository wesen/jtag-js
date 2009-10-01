/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2001 Scott Finneran
 *      Copyright (C) 2002 Intel Corporation
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
 */

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "avarice.h"
#include "remote.h"

unsigned long b4_to_u32(unsigned char *b) {
	unsigned long l;
	l = (unsigned)b[0];
	l += (unsigned)b[1] << 8;
	l += (unsigned)(unsigned)b[2] << 16;
	l += (unsigned)b[3] << 24;

	return l;
};

void u32_to_b4(unsigned char *b, unsigned long l) {
	b[0] = l & 0xff;
	b[1] = (l >> 8) & 0xff;
	b[2] = (l >> 16) & 0xff;
	b[3] = (l >> 24) & 0xff;
};

unsigned short b2_to_u16(unsigned char *b) {
	unsigned short l;
	l = (unsigned)b[0];
	l += (unsigned)b[1] << 8;

	return l;
};

void u16_to_b2(unsigned char *b, unsigned short l) {
	b[0] = l & 0xff;
	b[1] = (l >> 8) & 0xff;
};

static const unsigned char hexchars[] = "0123456789abcdef";

char *byteToHex(uint8_t x, char *buf)
{
    *buf++ = hexchars[x >> 4];
    *buf++ = hexchars[x & 0xf];

    return buf;
}

int hex(unsigned char ch)
{
    if((ch >= 'a') && (ch <= 'f'))
    {
	return (ch - 'a' + 10);
    }
    if((ch >= '0') && (ch <= '9'))
    {
	return (ch - '0');
    }
    if((ch >= 'A') && (ch <= 'F'))
    {
	return (ch - 'A' + 10);
    }
    return (-1);
}


/** Convert hex string at '*ptr' to an integer.
    Advances '*ptr' to 1st non-hex character found.
    Returns number of characters used in conversion.
 **/
int hexToInt(char **ptr, int *intValue)
{
    int numChars = 0;
    int hexValue;

    *intValue = 0;
    while (**ptr)
    {
	hexValue = hex(**ptr);
	if(hexValue >= 0)
	{
	    *intValue = (*intValue << 4) | hexValue;
	    numChars++;
	}
	else
	{
	    break;
	}
	(*ptr)++;
    }
    return (numChars);
}

/** Convert the memory pointed to by mem into hex, placing result in buf.
    Return a pointer to the last char put in buf (null).
**/
char *mem2hex(uint8_t *mem, char *buf, int count)
{
    for (int i = 0; i < count; i++)
	buf = byteToHex(*mem++, buf);
    *buf = 0;

    return (buf);
}

/** Convert the hex array pointed to by buf into binary to be placed in mem.
    Return a pointer to the character AFTER the last byte written.
**/
uint8_t *hex2mem(char *buf, uint8_t *mem, int count)
{
    int i;
    unsigned char ch;

    for(i = 0; i < count; i++)
    {
	ch = hex(*buf++) << 4;
	ch = ch + hex(*buf++);
	*mem++ = ch;
    }

    return (mem);
}

/** Convert the binary stream in BUF to memory.
    Gdb will escape $, #, and the escape char (0x7d).
    'count' is the total number of bytes to write into
    memory.
**/
uint8_t *bin2mem(char *buf, uint8_t *mem, int count)
{
    int i;

    for(i = 0; i < count; i++)
    {
	// Check for any escaped characters. Be paranoid and
	// only unescape chars that should be escaped.
	if(*buf == 0x7d)
	{
	    switch (*(buf + 1))
	    {
	    case 0x3:	// #
	    case 0x4:	// $
	    case 0x5d:	// escape char
		buf++;
		*buf |= 0x20;
		break;
	    default:
		// nothing
		break;
	    }
	}

	*mem++ = *buf++;
    }

    return mem;
}

static void check_1(bool printUnixError, const char *fmt, va_list args)
{
    int en = errno;
#ifdef va_copy
    va_list copy_args;

    va_copy(copy_args, args);

		if (!Console::quietMode) {
			console->vStatusOut(fmt, copy_args);
		}
#else
		if (!Console::quietMode) {
			console->vStatusOut(fmt, args);
		}
#endif

		if (!Console::quietMode) {
			if (printUnixError) {
				console->statusOut(": %s", strerror(en));
			}
			console->statusOut("\n");
		}

#ifdef GDB_FUNCTIONALITY_XXX
    console->vgdbOut(fmt, args);
    if (printUnixError)
			gdbOut(": %s", strerror(en));
    gdbOut("\n");
#endif

		static char buf[256];
		vsnprintf(buf, sizeof(buf), fmt, args);
		throw buf;
}

void check(bool ok, const char *fmt, ...)
{
	// XXX get all instances of check in the program to add catch/tries
    va_list args;
    va_start(args, fmt);
    if (!ok)
			check_1(false, fmt, args);
    va_end(args);
}

void unixCheck(int status, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (status < 0)
	check_1(true, fmt, args);
    va_end(args);
}

