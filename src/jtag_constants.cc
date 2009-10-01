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
 * This file implements several helper functions dealing with JTAG constants.
 *
 * $Id$
 */

#include "jtag.h"

const char *BFDmemoryTypeString[] = {
    "FLASH",
    "EEPROM",
    "RAM",
};

const int BFDmemorySpaceOffset[] = {
    FLASH_SPACE_ADDR_OFFSET,
    EEPROM_SPACE_ADDR_OFFSET,
    DATA_SPACE_ADDR_OFFSET,
};

/*
 * Debugging helper functions applicable to both the mkI and mkII ICE.
 */
static codeToString_t jtagCmd2String[] = {
  { CMND_ENTER_PROGMODE, "CMND_ENTER_PROGMODE" },
  { CMND_CHIP_ERASE, "CMND_CHIP_ERASE" },
  { CMND_CLEAR_EVENTS, "CMND_CLEAR_EVENTS" },
  { CMND_CLR_BREAK, "CMND_CLR_BREAK" },
  { CMND_ENTER_PROGMODE, "CMND_ENTER_PROGMODE" },
  { CMND_ERASEPAGE_SPM, "CMND_ERASEPAGE_SPM" },
  { CMND_FORCED_STOP, "CMND_FORCED_STOP" },
  { CMND_GET_BREAK, "CMND_GET_BREAK" },
  { CMND_GET_PARAMETER, "CMND_GET_PARAMETER" },
  { CMND_GET_SIGN_ON, "CMND_GET_SIGN_ON" },
  { CMND_GET_SYNC, "CMND_GET_SYNC" },
  { CMND_GO, "CMND_GO" },
  { CMND_LEAVE_PROGMODE, "CMND_LEAVE_PROGMODE" },
  { CMND_READ_MEMORY, "CMND_READ_MEMORY" },
  { CMND_READ_PC, "CMND_READ_PC" },
  { CMND_RESET, "CMND_RESET" },
  { CMND_RESTORE_TARGET, "CMND_RESTORE_TARGET" },
  { CMND_RUN_TO_ADDR, "CMND_RUN_TO_ADDR" },
  { CMND_SELFTEST, "CMND_SELFTEST" },
  { CMND_SET_BREAK, "CMND_SET_BREAK" },
  { CMND_SET_DEVICE_DESCRIPTOR, "CMND_SET_DEVICE_DESCRIPTOR" },
  { CMND_SET_N_PARAMETERS, "CMND_SET_N_PARAMETERS" },
  { CMND_SET_PARAMETER, "CMND_SET_PARAMETER" },
  { CMND_SIGN_OFF, "CMND_SIGN_OFF" },
  { CMND_SINGLE_STEP, "CMND_SINGLE_STEP" },
  { CMND_SPI_CMD, "CMND_SPI_CMD" },
  { CMND_WRITE_MEMORY, "CMND_WRITE_MEMORY" },
  { CMND_WRITE_PC, "CMND_WRITE_PC" },
};

static codeToString_t jtagRsp2String[] = {
    { RSP_DEBUGWIRE_SYNC_FAILED, "RSP_DEBUGWIRE_SYNC_FAILED" },
    { RSP_FAILED, "RSP_FAILED" },
    { RSP_GET_BREAK, "RSP_GET_BREAK" },
    { RSP_ILLEGAL_BREAKPOINT, "RSP_ILLEGAL_BREAKPOINT" },
    { RSP_ILLEGAL_COMMAND, "RSP_ILLEGAL_COMMAND" },
    { RSP_ILLEGAL_EMULATOR_MODE, "RSP_ILLEGAL_EMULATOR_MODE" },
    { RSP_ILLEGAL_JTAG_ID, "RSP_ILLEGAL_JTAG_ID" },
    { RSP_ILLEGAL_MCU_STATE, "RSP_ILLEGAL_MCU_STATE" },
    { RSP_ILLEGAL_MEMORY_TYPE, "RSP_ILLEGAL_MEMORY_TYPE" },
    { RSP_ILLEGAL_MEMORY_RANGE, "RSP_ILLEGAL_MEMORY_RANGE" },
    { RSP_ILLEGAL_PARAMETER, "RSP_ILLEGAL_PARAMETER" },
    { RSP_ILLEGAL_POWER_STATE, "RSP_ILLEGAL_POWER_STATE" },
    { RSP_ILLEGAL_VALUE, "RSP_ILLEGAL_VALUE" },
    { RSP_MEMORY, "RSP_MEMORY" },
    { RSP_NO_TARGET_POWER, "RSP_NO_TARGET_POWER" },
    { RSP_OK, "RSP_OK" },
    { RSP_PARAMETER, "RSP_PARAMETER" },
    { RSP_PC, "RSP_PC" },
    { RSP_SELFTEST, "RSP_SELFTEST" },
    { RSP_SET_N_PARAMETERS, "RSP_SET_N_PARAMETERS" },
    { RSP_SIGN_ON, "RSP_SIGN_ON" },
    { RSP_SPI_DATA, "RSP_SPI_DATA" },
  
};

const char *jtag::codeToString(uint8_t code, codeToString_t *arr,
			       int max, const char *defaultName) {
  for (int i = 0; i < max; i++) {
    if (arr[i].code == code) {
      return arr[i].name;
    }
  }

  return defaultName;
}

const char *jtag::jtagCmdToString(uint8_t command) {
  return codeToString(command, jtagCmd2String, countof(jtagCmd2String), "CMND_UNKNOWN");
}

const char *jtag::jtagRspToString(uint8_t rsp) {
  return codeToString(rsp, jtagRsp2String, countof(jtagRsp2String), "RSP_UNKNOWN");
}
 
