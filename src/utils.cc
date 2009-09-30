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

#if 0 // GDB CODE XXX
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

