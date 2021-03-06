/*
 *	avarice - The "avarice" program.
 *	Copyright (C) 2001 Scott Finneran
 *      Copyright (C) 2002, 2003, 2004 Intel Corporation
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
 * This file contains functions for interfacing with the JTAG box.
 *
 * $Id: jtagio.cc,v 1.32 2008/06/12 20:21:44 joerg_wunsch Exp $
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
#include "jtag1.h"

/** Send a command to the jtag, and check result.

    Increase *tries, abort if reaches MAX_JTAG_COMM_ATTEMPS

    Reads first response byte. If no response is received within
    JTAG_RESPONSE_TIMEOUT, returns false. If response byte is 
    JTAG_R_RESP_OK returns true, otherwise returns false.
**/
 
SendResult jtag1::sendJtagCommand(uint8_t *command, int commandSize, int *tries)
{
    check((*tries)++ < MAX_JTAG_COMM_ATTEMPS,
	      "JTAG ICE: Cannot synchronise");

    console->debugOut("\ncommand[%c, %d]: ", command[0], *tries);

    for (int i = 0; i < commandSize; i++)
	console->debugOut("%.2X ", command[i]);

    console->debugOut("\n");

    // before writing, clean up any "unfinished business".
    jtagCheck(tcflush(jtagBox, TCIFLUSH));

    int count = safewrite(command, commandSize);
    if (count < 0)
      jtagCheck(count);
    else // this shouldn't happen
      check(count == commandSize, JTAG_CAUSE);

    // And wait for all characters to go to the JTAG box.... can't hurt!
    jtagCheck(tcdrain(jtagBox));

    // We should get JTAG_R_OK, but we might get JTAG_R_INFO too (we just
    // ignore it)
    for (;;)
      {
	uint8_t ok;
	count = timeout_read(&ok, 1, JTAG_RESPONSE_TIMEOUT);
	jtagCheck(count);

	// timed out
	if (count == 0)
	{
	    console->debugOut("Timed out.\n");
	    return send_failed;
	}

	switch (ok)
	{
	case JTAG_R_OK: return send_ok;
	case JTAG_R_INFO:
	    unsigned char infobuf[2];

	    /* An info ("IDR dirty") response. Ignore it. */
	    console->debugOut("Info response: ");
	    count = timeout_read(infobuf, 2, JTAG_RESPONSE_TIMEOUT);
	    for (int i = 0; i < count; i++)
	    {
		console->debugOut("%.2X ", infobuf[i]);
	    }
	    console->debugOut("\n");
	    if (count != 2 || infobuf[1] != JTAG_R_OK)
		return send_failed;
	    else
		return (SendResult)(mcu_data + infobuf[0]);
	    break;
	default:
	    console->debugOut("Out of sync, reponse was `%02x'\n", ok);
	    return send_failed;
	}
      }
}

/** Get a 'responseSize' byte response from the JTAG ICE

    Returns NULL if no bytes received for JTAG_COMM_TIMEOUT microseconds
    Returns a dynamically allocated buffer containing the reponse (caller
    must free) otherwise
**/
uint8_t *jtag1::getJtagResponse(int responseSize)
{
    uint8_t *response;
    int numCharsRead;

    // Increase by 1 because of the zero termination.
    //
    // note: IT IS THE CALLERS RESPONSIBILITY TO delete() THIS.
    response = new uint8_t[responseSize + 1];
    response[responseSize] = '\0';

    numCharsRead = timeout_read(response, responseSize,
                                JTAG_RESPONSE_TIMEOUT);
    jtagCheck(numCharsRead);

    console->debugOut("response: ");
    for (int i = 0; i < numCharsRead; i++)
    {
	console->debugOut("%.2X ", response[i]);
    }
    console->debugOut("\n");

    if (numCharsRead < responseSize) // timeout problem
    {
	console->debugOut("Timed Out (partial response)\n");
	delete [] response;
	return NULL;
    }

    return response;
}

uint8_t *jtag1::doJtagCommand(uint8_t *command, int  commandSize, int  responseSize)
{
    int tryCount = 0;

    // Send command until we get RESP_OK
    for (;;)
    {
	uint8_t *response;
	static uint8_t sync[] = { ' ' };
	static uint8_t stop[] = { 'S', JTAG_EOM };

	switch (sendJtagCommand(command, commandSize, &tryCount))
	{
	case send_ok:
	    response = getJtagResponse(responseSize);
	    check(response != NULL, JTAG_CAUSE);
	    return response;
	case send_failed:
	    // We're out of sync. Attempt to resync.
	    while (sendJtagCommand(sync, sizeof sync, &tryCount) != send_ok) 
		;
	    break;
	default:
	    /* "IDR dirty", aka I/O debug register dirty, aka we got some
	       data from the target processor. This seems to be
	       indefinitely repeated if we don't do anything.  Asking for
	       the sign on seems to shut it up, so we do that.  (another
	       option is to do a 'forced stop' ('F' command), but that is a
	       bit more intrusive --- it should be ok, as we currently only
	       send commands to stopped targets, but...) */
	    if (sendJtagCommand(stop, sizeof stop, &tryCount) == send_ok)
		getJtagResponse(8);
	    break;
	}
    }

}

bool jtag1::doSimpleJtagCommand(unsigned char cmd, int responseSize)
{
    uint8_t *response;
    uint8_t command[] = { cmd, JTAG_EOM };
    bool result;

    response = doJtagCommand(command, sizeof command, responseSize);
    result = responseSize == 0 || response[responseSize - 1] == JTAG_R_OK;
    
    delete [] response;

    return result;
}


/** Set PC and JTAG ICE bitrate to BIT_RATE_xxx specified by 'newBitRate' **/
void jtag1::changeBitRate(int newBitRate)
{
    uint8_t jtagrate;

    switch (newBitRate) {
    case 9600:
	jtagrate = BIT_RATE_9600;
	break;
    case 19200:
	jtagrate = BIT_RATE_19200;
	break;
    case 38400:
	jtagrate = BIT_RATE_38400;
	break;
    case 57600:
	jtagrate = BIT_RATE_57600;
	break;
    case 115200:
	jtagrate = BIT_RATE_115200;
	break;
    }
    setJtagParameter(JTAG_P_BITRATE, jtagrate);
    changeLocalBitRate(newBitRate);
}

/** Set the JTAG ICE device descriptor data for specified device type **/
void jtag1::setDeviceDescriptor(jtag_device_def_type *dev)
{
    uint8_t *response = NULL;
    uint8_t *command = (uint8_t *)(&dev->dev_desc1);

    response = doJtagCommand(command, sizeof dev->dev_desc1, 1);
    check(response[0] == JTAG_R_OK,
	      "JTAG ICE: Failed to set device description");

    delete [] response;
}

/** Check for emulator using the 'S' command *without* retries
    (used at startup to check sync only) **/
bool jtag1::checkForEmulator(void)
{
    uint8_t *response;
    uint8_t command[] = { 'S', JTAG_EOM };
    bool result;
    int tries = 0;

    if (!sendJtagCommand(command, sizeof command, &tries))
      return false;

    response = getJtagResponse(8);
    result = response && !strcmp((char *)response, "AVRNOCDA");
    
    delete [] response;

    return result;
}

/** Attempt to synchronise with JTAG at specified bitrate **/
bool jtag1::synchroniseAt(int bitrate)
{
    console->debugOut("Attempting synchronisation at bitrate %d\n", bitrate);

    changeLocalBitRate(bitrate);

    int tries = 0;
    while (tries < MAX_JTAG_SYNC_ATTEMPS)
    {
	// 'S  ' works if this is the first string sent after power up.
	// But if another char is sent, the JTAG seems to go in to some 
	// internal mode. 'SE  ' takes it out of there, apparently (sometimes
	// 'E  ' is enough, but not always...)
	sendJtagCommand((uint8_t *)"SE  ", 4, &tries);
	usleep(2 * JTAG_COMM_TIMEOUT); // let rest of response come before we ignore it
	jtagCheck(tcflush(jtagBox, TCIFLUSH));
	if (checkForEmulator())
	    return true;
    }
    return false;
}

/** Attempt to synchronise with JTAG ICE at all possible bit rates **/
void jtag1::startJtagLink(void)
{
    static int bitrates[] =
    { 115200, 19200, 57600, 38400, 9600 };

    for (unsigned int i = 0; i < sizeof bitrates / sizeof *bitrates; i++)
	if (synchroniseAt(bitrates[i]))
	    return;

    check(false, "Failed to synchronise with the JTAG ICE (is it connected and powered?)");
}

/** Device automatic configuration 
 Determines the device being controlled by the JTAG ICE and configures
 the system accordingly.

 May be overridden by command line parameter.

*/
void jtag1::deviceAutoConfig(void)
{
    unsigned int device_id;
    int i;
    jtag_device_def_type *pDevice = deviceDefinitions;

    // Auto config
    console->debugOut("Automatic device detection: ");

    /* Set daisy chain information */
    configDaisyChain();

    /* Read in the JTAG device ID to determine device */
    device_id = getJtagParameter(JTAG_P_JTAGID_BYTE0);
    /* Reading the first byte resumes the program (weird, no?) */
    interruptProgram();
    device_id += (getJtagParameter(JTAG_P_JTAGID_BYTE1) <<  8) +
      (getJtagParameter(JTAG_P_JTAGID_BYTE2) << 16) + 
      (getJtagParameter(JTAG_P_JTAGID_BYTE3) << 24);

   
    console->debugOut("JTAG id = 0x%0X : Ver = 0x%0x : Device = 0x%0x : Manuf = 0x%0x\n", 
             device_id,
             (device_id & 0xF0000000) >> 28,
             (device_id & 0x0FFFF000) >> 12,
             (device_id & 0x00000FFE) >> 1);
   
    device_id = (device_id & 0x0FFFF000) >> 12;
    console->statusOut("Reported JTAG device ID: 0x%0X\n", device_id);
    
    if (device_name == 0)
    {
        while (pDevice->name)
        {
            if (pDevice->device_id == device_id)
                break;

            pDevice++;
        }
	check((pDevice->device_flags & DEVFL_MKII_ONLY) == 0,
	      "Device is not supported by JTAG ICE mkI");
        check(pDevice->name,
              "No configuration available for device ID: %0x\n",
              device_id); 
    }
    else
    {
        console->debugOut("Looking for device: %s\n", device_name);

        while (pDevice->name)
        {
            if (strcasecmp(pDevice->name, device_name) == 0)
                break;

            pDevice++;
        }
	check((pDevice->device_flags & DEVFL_MKII_ONLY) == 0,
	      "Device is not supported by JTAG ICE mkI");
	check(pDevice->name,
              "No configuration available for Device: %s\n",
              device_name);
    }

    if (device_name)
    {
        if (device_id != pDevice->device_id)
        {
            console->statusOut("Configured for device ID: 0x%0X %s -- FORCED with %s\n",
                      pDevice->device_id, pDevice->name, device_name);
        }
        else
        {
            console->statusOut("Configured for device ID: 0x%0X %s -- Matched with "
                      "%s\n", pDevice->device_id, pDevice->name, device_name);
        }
    }
    else
    {
        console->statusOut("Configured for device ID: 0x%0X %s\n",
                  pDevice->device_id, pDevice->name);
    }

    device_name = (char*)pDevice->name;

    global_p_device_def = pDevice;

    setDeviceDescriptor(pDevice);
}


void jtag1::initJtagBox(void)
{
    console->statusOut("JTAG config starting.\n");

    startJtagLink();
    changeBitRate(115200);

    uint8_t hw_ver = getJtagParameter(JTAG_P_HW_VERSION);
    console->statusOut("Hardware Version: 0x%02x\n", hw_ver);
    //check(hw_ver == 0xc0, "JTAG ICE: Unknown hardware version");

    uint8_t sw_ver = getJtagParameter(JTAG_P_SW_VERSION);
    console->statusOut("Software Version: 0x%02x\n", sw_ver);

    interruptProgram();

    deviceAutoConfig();

    // Clear out the breakpoints.
    deleteAllBreakpoints();

    console->statusOut("JTAG config complete.\n");
}


void jtag1::initJtagOnChipDebugging(unsigned long bitrate)
{
    console->statusOut("Preparing the target device for On Chip Debugging.\n");

    uint8_t br;
    if (bitrate >= 1000000UL)
	br = JTAG_BITRATE_1_MHz;
    else if (bitrate >= 500000)
	br = JTAG_BITRATE_500_KHz;
    else if (bitrate >= 250000)
	br = JTAG_BITRATE_250_KHz;
    else
	br = JTAG_BITRATE_125_KHz;
    // Set JTAG bitrate
    setJtagParameter(JTAG_P_CLOCK, br);

    // When attaching we can't change fuse bits, etc, as 
    // enabling+disabling programming resets the processor
    enableProgramming();

    // Ensure that all lock bits are "unlocked" ie all 1's
    uint8_t *lockBits = 0;
    lockBits = jtagRead(LOCK_SPACE_ADDR_OFFSET + 0, 1);

    if (*lockBits != LOCK_BITS_ALL_UNLOCKED)
    {
        lockBits[0] = LOCK_BITS_ALL_UNLOCKED;
        jtagWrite(LOCK_SPACE_ADDR_OFFSET + 0, 1, lockBits);
    }

    console->statusOut("\nDisabling lock bits:\n");
    console->statusOut("  LockBits -> 0x%02x\n", *lockBits);

    if (lockBits)
    {
        delete [] lockBits;
        lockBits = 0;
    }

    // Ensure on-chip debug enable fuse is enabled ie '0'
    uint8_t *fuseBits = 0;
    console->statusOut("\nEnabling on-chip debugging:\n");
    fuseBits = jtagRead(FUSE_SPACE_ADDR_OFFSET + 0, 3);

    if ((fuseBits[1] & FUSE_OCDEN) == FUSE_OCDEN)
    {
        fuseBits[1] = fuseBits[1] & ~FUSE_OCDEN; // clear bit
        jtagWrite(FUSE_SPACE_ADDR_OFFSET + 1, 1, &fuseBits[1]);
    }

    jtagDisplayFuses(fuseBits);

    if (fuseBits)
    {
        delete [] fuseBits;
        fuseBits = 0;
    }

    disableProgramming();

    resetProgram(false);
    setJtagParameter(JTAG_P_TIMERS_RUNNING, 0x00);
    resetProgram(true);
}

void jtag1::configDaisyChain(void)
{
    /* Set daisy chain information (if needed) */
    if (dchain.units_before > 0)
	setJtagParameter(JTAG_P_UNITS_BEFORE,dchain.units_before);
    if (dchain.units_after > 0)
	setJtagParameter(JTAG_P_UNITS_AFTER,dchain.units_after);
    if (dchain.bits_before > 0)
	setJtagParameter(JTAG_P_BIT_BEFORE,dchain.bits_before);
    if (dchain.bits_after > 0)
	setJtagParameter(JTAG_P_BIT_AFTER,dchain.bits_after);
}

