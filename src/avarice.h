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

#ifndef INCLUDE_AVARICE_H
#define INCLUDE_AVARICE_H

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#include <stdarg.h>
#include <inttypes.h>

#include "autoconf.h"

#include "console.hh"
#include "select.hh"
#include "utils.hh"

/** true if interrupts should be stepped over when stepping */
extern bool ignoreInterrupts;

// XXX throw exception
/** Abort program with error message 'fmt, ...' if !ok.

    Error message is sent via statusOut and gdbOut **/
void check(bool ok, const char *fmt, ...);

#define SYSCALL_CHECK(call) { \
  int _ret = call; \
  unixCheck(_ret, TOSTRING(call) ": %s", strerror(_ret)); \
  }

/** Abort program with error message 'fmt, ...' if status < 0. 
    Error message also includes error indicated by 'errno'

    Error message is sent via statusOut and gdbOut **/
void unixCheck(int status, const char *fmt, ...);
void unixCheck(int status);


#endif // INCLUDE_AVARICE_H
