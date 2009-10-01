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
 * Interface definition for the include/remote.c file.
 */

#ifndef INCLUDE_REMOTE_H
#define INCLUDE_REMOTE_H

class GdbRemote {
 public:
  /** BUFMAX defines the maximum number of characters in
   * inbound/outbound buffers at least NUMREGBYTES*2 are needed for
   * register packets
   */
  static const int BUFMAX  = 400;

  static const int NUMREGS = 32;
  static const int SREG    = 32;
  static const int SP      = 33;
  static const int PC      = 34;

  /** number of bytes of register, see GDB gdb/avr-tdep.c */
  static const int NUMREGBYTES = (NUMREGS + 1 + 2 + 4);

  static const char *GDB_CAUSE;


 private:
  /** File descriptor for gdb communication. -1 before connection. **/
  int gdbFileDescriptor;

  char remcomInBuffer[BUFMAX];
  char remcomOutBuffer[BUFMAX];

  void ok();
  void error(int n);

  void waitForOutput();
  void waitForInput();
  
  /** Send single char to gdb. Abort in case of problem. **/
  void putDebugChar(char c);

  void reportStatusExtended(int sigval);
  void reportStatus(int sigval);

  bool singleStep();
  char *getpacket();
  void putpack(char *buffer);

  void repStatus(bool breaktime);

  void putpacket(char *buf);

 public:
  GdbRemote() {
    gdbFileDescriptor = -1;
  }

  /** If status < 0: Report gdb communication error & exit **/
  void check(int status);

  /** Talk to gdb over file descriptor 'fd' **/
  void setFile(int fd);

  /** Return single char read from gdb. Abort in case of problem,
      exit cleanly if EOF detected on gdbFileDescriptor. **/
  int getDebugChar();
  int checkForDebugChar();

  /** printf 'fmt, ...' to gdb **/
  void vgdbOut(const char *fmt, va_list args);
  void gdbOut(const char *fmt, ...);
  
  unsigned int readLWord(unsigned int address);
  unsigned int readBWord(unsigned int address);
  unsigned int readSP();

  bool handleInterrupt();

  /** GDB remote protocol interpreter */
  void talkToGdb();
};

extern GdbRemote gdbRemote;

#endif /* INCLUDE_REMOTE_H */
