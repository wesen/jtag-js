/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2001 Scott Finneran
 *	Copyright (C) 2002, 2003, 2004 Intel Corporation
 *	Copyright (C) 2005, 2006, 2007 Joerg Wunsch
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License Version 2
 *	as published by the Free Software Foundation.
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
 * This file contains functions for interfacing with the GDB remote protocol.
 *
 * $Id: jtag.h,v 1.29 2008/09/29 20:52:20 joerg_wunsch Exp $
 */

#ifndef JTAG_H
#define JTAG_H

#include <inttypes.h>
#include <sys/types.h>
#include <termios.h>

#include "ioreg.h"
#include "jtag_constants.h"
#include "jtag_device_desc.h"

extern const char *BFDmemoryTypeString[];
extern const int BFDmemorySpaceOffset[];

enum emulator {
  EMULATOR_JTAGICE,
  EMULATOR_DRAGON,
};


// Allocate 1 meg for image buffer. This is where the file data is
// stored before writing occurs.
#define MAX_IMAGE_SIZE 1000000


typedef struct {
    unsigned char val;
    bool  used;
} AVRMemoryByte;


// Struct that holds the memory image. We read from file using BFD
// into this struct, then pass the entire struct to the target writer.
typedef struct {
    AVRMemoryByte image[MAX_IMAGE_SIZE];
    int last_address;
    int first_address;
    bool first_address_ok;
    bool has_data;
    const char *name;
} BFDimage;

// Struct to convert specific bytes received on jtag to a more readable string
// to help while debugging.
//
typedef struct codeToString_s {
  unsigned char code;
  const char *name;
} codeToString_t;

#define countof(arr) (sizeof(arr)/(sizeof(arr[0])))

// The Sync_CRC/EOP message terminator (no real CRC in sight...)
#define JTAG_EOM 0x20, 0x20

// A generic error message when nothing good comes to mind
#define JTAG_CAUSE "JTAG ICE communication failed"

class jtag
{
  protected:
  // The initial serial port parameters. We restore them on exit.
  struct termios oldtio;
  bool oldtioValid;

  // The file descriptor used while talking to the JTAG ICE
  int jtagBox;

  // For the mkII device, is the box attached via USB?
  bool is_usb;

  // A control pipe to talk to the USB daemon.
  int ctrlPipe;

  // The type of our emulator: JTAG ICE, or AVR Dragon.
  emulator emu_type;

  // Whether nSRST is to be applied when connecting (override JTD bit).
  bool apply_nSRST;

  public:
  // Whether we are in "programming mode" (changes how program memory
  // is written, apparently)
  bool programmingEnabled;

  virtual bool isMk2() { return false; }

  // Name of the device controlled by the JTAG ICE
  char *device_name;

  // Daisy chain info
  struct {
    unsigned char units_before;
    unsigned char units_after;
    unsigned char bits_before;
    unsigned char bits_after;
  } dchain;

  protected:
  pid_t openUSB(const char *jtagDeviceName);
  void openSerialPort(const char *portName);
  int safewrite(const void *b, int count);
  void changeLocalBitRate(int newBitRate);
  void restoreSerialPort(void);

  virtual void changeBitRate(int newBitRate) = 0;
  virtual void setDeviceDescriptor(jtag_device_def_type *dev) = 0;
  virtual bool synchroniseAt(int bitrate) = 0;
  virtual void startJtagLink(void) = 0;
  virtual void deviceAutoConfig(void) = 0;
  void jtag_flash_image(BFDimage *image, BFDmemoryType memtype,
			bool program, bool verify);
  // Return page address of
  unsigned int page_addr(unsigned int addr, BFDmemoryType memtype)
  {
    unsigned int page_size = get_page_size( memtype );
    return (unsigned int)(addr & (~(page_size - 1)));
  };

  unsigned int get_page_size(BFDmemoryType memtype);

  // protected debugging helper function
  const char *codeToString(uint8_t code, codeToString_t *arr, int max, const char *defaultName);
  
  
  public:
  jtag(void);
  jtag(const char *dev, char *name, emulator type = EMULATOR_JTAGICE);
  virtual ~jtag(void);

  // Debugging helper functions
  const char *jtagCmdToString(uint8_t command);
  const char *jtagRspToString(uint8_t rsp);
  
  // Basic JTAG I/O
  // -------------

  /** If status < 0: Report JTAG ICE communication error & exit **/
  void jtagCheck(int status);

  /** Send initial configuration to setup the JTAG box itself. 
   **/
  virtual void initJtagBox(void) = 0;

  /**
     Send initial configuration to the JTAG box when starting a new
     debug session.  (Note: when attaching to a running target, fuse
     bits cannot be set so debugging must have been enabled earlier)

     The bitrate sets the JTAG bitrate. The bitrate must be less than
     1/4 that of the target avr frequency or the jtagice will have
     problems reading from the target. The problems are usually
     manifested as failed calls to jtagRead().
  **/
  virtual void initJtagOnChipDebugging(unsigned long bitrate) = 0;


  /** A timed-out read from file descriptor 'fd'.

  'timeout' is in microseconds, it is the maximum interval within which
  the read must make progress (i.e., it's a per-byte timeout)

  Returns the number of bytes read or -1 for errors other than timeout.

  Note: EOF and timeout cannot be distinguished
  **/
  int timeout_read(void *buf, size_t count, unsigned long timeout);

  // Breakpoints
  // -----------

  /** Clear out the breakpoints. */
  virtual void deleteAllBreakpoints(void) = 0;

  /** Delete breakpoint at the specified address. */
  virtual bool deleteBreakpoint(unsigned int address, bpType type, unsigned int length) = 0;

  /** Add a code breakpoint at the specified address. */
  virtual bool addBreakpoint(unsigned int address, bpType type, unsigned int length) = 0;

  /** Send the breakpoint details down to the JTAG box. */
  virtual void updateBreakpoints(void) = 0;

  /** True if there is a breakpoint at address */
  virtual bool codeBreakpointAt(unsigned int address) = 0;

  /** True if there is a breakpoint between start (inclusive) and 
      end (exclusive) */
  virtual bool codeBreakpointBetween(unsigned int start, unsigned int end) = 0;

  virtual bool stopAt(unsigned int address) = 0;

  /** Parse a list of event names to *not* cause a break. */
  virtual void parseEvents(const char *) = 0;

  // Writing to program memory
  // -------------------------

  /** Switch to faster programming mode, allows chip erase */
  virtual void enableProgramming(void) = 0;

  /** Switch back to normal programming mode **/
  virtual void disableProgramming(void) = 0;

  /** Erase all chip memory **/
  virtual void eraseProgramMemory(void) = 0;

  virtual void eraseProgramPage(unsigned long address) = 0;

  /** Download an image contained in the specified file. */
  virtual void downloadToTarget(const char* filename, bool program, bool verify) = 0;

  // Running, single stepping, etc
  // -----------------------------

  /** Retrieve the current Program Counter value, or PC_INVALID if fails */
  virtual unsigned long getProgramCounter(void) = 0;

  /** Set program counter to 'pc'. Return true iff successful **/
  virtual bool setProgramCounter(unsigned long pc) = 0;

  /** Reset AVR. Return true iff successful **/
  virtual bool resetProgram(bool possible_nSRST) = 0;

  /** Interrupt AVR. Return true iff successful **/
  virtual bool interruptProgram(void) = 0;

  /** Resume program execution. Return true iff successful.
      Note: the gdb 'continue' command is handled by jtagContinue,
      this is just the low level command to resume after interruptProgram
  **/
  virtual bool resumeProgram(void) = 0;

  /** Issue a "single step" command to the JTAG box. 
      Return true iff successful **/
  virtual bool jtagSingleStep(bool useHLL = false) = 0;

  /** Send the program on it's merry way, and wait for a breakpoint or
      input from gdb.
      Return true for a breakpoint, false for gdb input. **/
  virtual bool jtagContinue(void) = 0;

	virtual void startPolling(void) = 0;
	
	virtual bool pollDevice(bool *gdbInterrupt, bool *breakpoint) = 0;

  // R/W memory
  // ----------

  /** Read 'numBytes' from target memory address 'addr'.

    The memory space is selected by the high order bits of 'addr' (see
    above).

    Returns a dynamically allocated buffer with the requested bytes if
    successful (caller must free), or NULL if the read failed.
  **/
  virtual unsigned char *jtagRead(unsigned long addr, unsigned int numBytes) = 0;

  /** Write 'numBytes' bytes from 'buffer' to target memory address 'addr'

    The memory space is selected by the high order bits of 'addr' (see
    above).

    Returns true for a successful write, false for a failed one.

    Note: The current behaviour for program-space writes is pretty
    weird (does not match jtagRead). See comments in jtagrw.cc.
  **/
  virtual bool jtagWrite(unsigned long addr, unsigned int numBytes, unsigned char buffer[]) = 0;


  /** Write fuses to target.

    The input parameter is a string from command-line, as produced by
    printf("%x", 0xaabbcc );
  **/
  void jtagWriteFuses(char *fuses);


  /** Read fuses from target.

    Shows extended, high and low fuse byte.
  */
  void jtagReadFuses(void);


  /** Display fuses.

    Shows extended, high and low fuse byte.
  */
  void jtagDisplayFuses(unsigned char *fuseBits);


  /** Write lockbits to target.

    The input parameter is a string from command-line, as produced by
    printf("%x", 0xaa);
  **/
  void jtagWriteLockBits(char *lock);


  /** Read the lock bits from the target and display them.
   **/
  void jtagReadLockBits(void);


  /** Display lockbits.

    Shows raw value and individual bits.
  **/
  void jtagDisplayLockBits(unsigned char *lockBits);

};

extern struct jtag *theJtagICE;

#endif
