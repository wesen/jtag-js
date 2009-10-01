/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2001 Scott Finneran
 *      Copyright (C) 2002, 2003, 2004 Intel Corporation
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
 * This file implements those parts of class "jtag" that are common
 * for both, the mkI and mkII protocol.
 *
 * $Id: jtaggeneric.cc,v 1.10 2008/05/26 11:37:14 joerg_wunsch Exp $
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
 

/*
 * Generic functions applicable to both, the mkI and mkII ICE.
 */

void jtag::jtagCheck(int status)
{
    unixCheck(status, JTAG_CAUSE, NULL);
}

jtag::jtag(void)
{
  jtagBox = 0;
  device_name = 0;
  oldtioValid = is_usb = false;
  ctrlPipe = -1;
}

jtag::jtag(const char *jtagDeviceName, char *name, emulator type)
{
    jtagBox = 0;
    oldtioValid = is_usb = false;
    ctrlPipe = -1;
    device_name = name;
    emu_type = type;

    if (strncmp(jtagDeviceName, "usb", 3) == 0) {
#ifdef HAVE_LIBUSB
			is_usb = true;
			openUSB(jtagDeviceName);
#else
			throw "avarice has not been compiled with libusb support\n";
#endif
		} else {
			openSerialPort(jtagDeviceName);
		}
			 
}

// NB: the destructor is virtual; class jtag2 extends it
jtag::~jtag(void)
{
  restoreSerialPort();
}

int jtag::timeout_read(void *buf, size_t count, unsigned long timeout)
{
    char *buffer = (char *)buf;
    size_t actual = 0;

		FDSelect fds;
		fds.add(jtagBox);

    while (actual < count) {
			// retry the select call
			int selected = fds.waitRead(timeout / 1000, true);
			if (selected == 0)
				return actual;
			
			ssize_t thisread = read(jtagBox, &buffer[actual], count - actual);
			if ((thisread < 0) && (errno == EAGAIN))
				continue;
			jtagCheck(thisread);
			
			actual += thisread;
    }
		
    return count;
}

int jtag::safewrite(const void *b, int count)
{
  char *buffer = (char *)b;
  int actual = 0;
  int flags = fcntl(jtagBox, F_GETFL);

  fcntl(jtagBox, F_SETFL, 0); // blocking mode
  while (count > 0)
    {
      int n = write(jtagBox, buffer, count);

      if (n == -1 && errno == EINTR)
	continue;
      if (n == -1)
	{
	  actual = -1;
	  break;
	}

      count -= n;
      actual += n;
      buffer += n;
    }
  fcntl(jtagBox, F_SETFL, flags); 
  return actual;
}


unsigned int jtag::get_page_size(BFDmemoryType memtype)
{
    unsigned int page_size;
    switch( memtype ) {
    case MEM_FLASH:
        page_size = global_p_device_def->flash_page_size;
        break;
    case MEM_EEPROM:
        page_size = global_p_device_def->eeprom_page_size;
        break;
    default:
        page_size = 1;
        break;
    }
    return page_size;
}


static bool pageIsEmpty(BFDimage *image, unsigned int addr, unsigned int size,
                        BFDmemoryType memtype)
{
	bool emptyPage = true;

	// Check if page is used
	for (unsigned int idx=addr; idx<addr+size; idx++)
    {
			if (idx >= image->last_address)
				break;

			// 1. If this address existed in input file, mark as ! empty.
			// 2. If we are programming FLASH, and contents == 0xff, we need
			//    not program (is 0xff after erase).
			if (image->image[idx].used)
        {
					if (!((memtype == MEM_FLASH) &&
								(image->image[idx].val == 0xff)))
            {
							emptyPage = false;
							break;
            }
        }
    }
	return emptyPage;
}


void jtag::jtag_flash_image(BFDimage *image, BFDmemoryType memtype,
                             bool program, bool verify)
{
    unsigned int page_size = get_page_size(memtype);
    static uint8_t buf[MAX_IMAGE_SIZE];
    unsigned int i;
    uint8_t *response = NULL;
    bool emptyPage = true;
    unsigned int addr;

    if (! image->has_data)
    {
        fprintf(stderr, "File contains no data.\n");
        exit(-1);
    }


    if (program)
    {
        // First address must start on page boundary.
        addr = page_addr(image->first_address, memtype);

        console->statusOut("Downloading %s image to target.", image->name);
        console->statusFlush();

        while (addr < image->last_address)
        {
            if (!pageIsEmpty(image, addr, page_size, memtype))
            {
                // Must also convert address to gcc-hacked addr for jtagWrite
                console->debugOut("Writing page at addr 0x%.4lx size 0x%lx\n",
                         addr, page_size);

                // Create raw data buffer
                for (i=0; i<page_size; i++)
                    buf[i] = image->image[i+addr].val;

                check(jtagWrite(BFDmemorySpaceOffset[memtype] + addr,
                                page_size,
                                buf),
                      "Error writing to target");
                // No need for console->statusOut here, since jtagWrite does it.
            }

            addr += page_size;

            console->statusOut(".");
            console->statusFlush();
        }

        console->statusOut("\n");
        console->statusFlush();
    }

    if (verify)
    {
        bool is_verified = true;

        // First address must start on page boundary.
        addr = page_addr(image->first_address, memtype);

        console->statusOut("\nVerifying %s", image->name);
        console->statusFlush();

        while (addr < image->last_address)
        {
            // Must also convert address to gcc-hacked addr for jtagWrite
            console->debugOut("Verifying page at addr 0x%.4lx size 0x%lx\n",
                     addr, page_size);

            response = jtagRead(BFDmemorySpaceOffset[memtype] + addr,
                                page_size);

            // Verify buffer, but only addresses in use.
            for (i=0; i < page_size; i++)
            {
                unsigned int c = i + addr;
                if (image->image[c].used )
                {
                    if (image->image[c].val != response[i])
                    {
                        console->statusOut("\nError verifying target addr %.4x. "
                                  "Expect [0x%02x] Got [0x%02x]",
                                  c, image->image[c].val, response[i]);
                        console->statusFlush();
                        is_verified = false;
                    }
                }
            }

            addr += page_size;

            console->statusOut(".");
            console->statusFlush();
        }
        delete [] response;

        console->statusOut("\n");
        console->statusFlush();

        check(is_verified, "\nVerification failed!");
    }
}

void jtag::jtagWriteFuses(char *fuses)
{
    int temp[3];
    uint8_t fuseBits[3];
    uint8_t *readfuseBits;
    unsigned int c;

    check(fuses,
          "Error: No fuses string given");

    // Convert fuses to hex values (this avoids endianess issues)
    c = sscanf(fuses, "%02x%02x%02x", temp+2, temp+1, temp );
    check(c == 3,
          "Error: Fuses specified are not in hexidecimal");

    fuseBits[0] = (uint8_t)temp[0];
    fuseBits[1] = (uint8_t)temp[1];
    fuseBits[2] = (uint8_t)temp[2];

    console->statusOut("\nWriting Fuse Bytes:\n");
    jtagDisplayFuses(fuseBits);

    enableProgramming();

    check(jtagWrite(FUSE_SPACE_ADDR_OFFSET + 0, 3, fuseBits),
          "Error writing fuses");

    readfuseBits = jtagRead(FUSE_SPACE_ADDR_OFFSET + 0, 3);

    disableProgramming();

    check(memcmp(fuseBits, readfuseBits, 3) == 0,
          "Error verifying written fuses");

    delete [] readfuseBits;
}


void jtag::jtagReadFuses(void)
{
    uint8_t *fuseBits = 0;

    enableProgramming();
    console->statusOut("\nReading Fuse Bytes:\n");
    fuseBits = jtagRead(FUSE_SPACE_ADDR_OFFSET + 0, 3);
    disableProgramming();

    check(fuseBits, "Error reading fuses");

    jtagDisplayFuses(fuseBits);

    delete [] fuseBits;
}


void jtag::jtagDisplayFuses(uint8_t *fuseBits)
{
    console->statusOut("  Extended Fuse byte -> 0x%02x\n", fuseBits[2]);
    console->statusOut("      High Fuse byte -> 0x%02x\n", fuseBits[1]);
    console->statusOut("       Low Fuse byte -> 0x%02x\n", fuseBits[0]);
}


void jtag::jtagWriteLockBits(char *lock)
{
    int temp[1];
    uint8_t lockBits[1];
    uint8_t *readlockBits;
    unsigned int c;

    check(lock,
          "Error: No lock bit string given");

    check(strlen(lock) == 2,
          "Error: Fuses must be one byte exactly");

    // Convert lockbits to hex value
    c = sscanf(lock, "%02x", temp);
    check(c == 1,
          "Error: Fuses specified are not in hexidecimal");

    lockBits[0] = (uint8_t)temp[0];

    console->statusOut("\nWriting Lock Bits -> 0x%02x\n", lockBits[0]);

    enableProgramming();

    check(jtagWrite(LOCK_SPACE_ADDR_OFFSET + 0, 1, lockBits),
          "Error writing lockbits" );

    readlockBits = jtagRead(LOCK_SPACE_ADDR_OFFSET + 0, 1);

    disableProgramming();

    check(memcmp(lockBits, readlockBits, 1) == 0,
          "Error verifying written lock bits");

    delete [] readlockBits;
}


void jtag::jtagReadLockBits(void)
{
    uint8_t *lockBits = 0;

    enableProgramming();
    console->statusOut("\nReading Lock Bits:\n");
    lockBits = jtagRead(LOCK_SPACE_ADDR_OFFSET + 0, 1);
    disableProgramming();

    check(lockBits, "Error reading lock bits");

    jtagDisplayLockBits(lockBits);

    delete [] lockBits;
}


void jtag::jtagDisplayLockBits(uint8_t *lockBits)
{
    console->statusOut("Lock bits -> 0x%02x\n\n", lockBits[0]);

    console->statusOut("    Bit 7 [ Reserved ] -> %d\n", (lockBits[0] >> 7) & 1);
    console->statusOut("    Bit 6 [ Reserved ] -> %d\n", (lockBits[0] >> 6) & 1);
    console->statusOut("    Bit 5 [ BLB12    ] -> %d\n", (lockBits[0] >> 5) & 1);
    console->statusOut("    Bit 4 [ BLB11    ] -> %d\n", (lockBits[0] >> 4) & 1);
    console->statusOut("    Bit 3 [ BLB02    ] -> %d\n", (lockBits[0] >> 3) & 1);
    console->statusOut("    Bit 2 [ BLB01    ] -> %d\n", (lockBits[0] >> 2) & 1);
    console->statusOut("    Bit 1 [ LB2      ] -> %d\n", (lockBits[0] >> 1) & 1);
    console->statusOut("    Bit 0 [ LB1      ] -> %d\n", (lockBits[0] >> 0) & 1);
}
