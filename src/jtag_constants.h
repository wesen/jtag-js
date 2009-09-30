#ifndef JTAG_CONSTANTS_H__
#define JTAG_CONSTANTS_H__

// various enums
enum
{
    // Constants common to both mkI and mkII

    // Address space offsets
    FLASH_SPACE_ADDR_OFFSET           = 0x000000,
    DATA_SPACE_ADDR_OFFSET            = 0x800000,

    EEPROM_SPACE_ADDR_OFFSET          = 0x810000,

    FUSE_SPACE_ADDR_OFFSET            = 0x820000,

    LOCK_SPACE_ADDR_OFFSET            = 0x830000,

    SIG_SPACE_ADDR_OFFSET             = 0x840000,

    BREAKPOINT_SPACE_ADDR_OFFSET      = 0x900000,

    ADDR_SPACE_MASK = (DATA_SPACE_ADDR_OFFSET   |
                       EEPROM_SPACE_ADDR_OFFSET |
                       FUSE_SPACE_ADDR_OFFSET   |
                       LOCK_SPACE_ADDR_OFFSET   |
                       SIG_SPACE_ADDR_OFFSET    |
                       BREAKPOINT_SPACE_ADDR_OFFSET),

    // JTAG communication timeouts, in microseconds
    // RESPONSE is for the first response byte
    // COMM is for subsequent response bytes
    MAX_JTAG_COMM_ATTEMPS	      = 10,
    MAX_JTAG_SYNC_ATTEMPS	      = 3,

    JTAG_RESPONSE_TIMEOUT	      = 1000000,
    JTAG_COMM_TIMEOUT		      = 100000,

    // Lock Bit Values
    LOCK_BITS_ALL_UNLOCKED            = 0xff,

    // Fuse Bit Values
    // XXX -- should be configurable, newer devices might no
    // longer match that fixed pattern.  Basically, the pattern
    // is the ATmega64/128 one.
    FUSE_M103C                        = 0x02,
    FUSE_WDTON                        = 0x01,

    FUSE_OCDEN                        = 0x80,
    FUSE_JTAGEN                       = 0x40,
    FUSE_SPIEN                        = 0x20,
    FUSE_CKOPT                        = 0x10,
    FUSE_EESAVE                       = 0x08,
    FUSE_BOOTSZ1                      = 0x04,
    FUSE_BOOTSZ0                      = 0x02,
    FUSE_BOOTRST                      = 0x01,

    FUSE_BODLEVEL                     = 0x80,
    FUSE_BODEN                        = 0x40,
    FUSE_SUT1                         = 0x20,
    FUSE_SUT0                         = 0x10,
    FUSE_CKSEL3                       = 0x08,
    FUSE_CKSEL2                       = 0x04,
    FUSE_CKSEL1                       = 0x02,
    FUSE_CKSEL0                       = 0x01,

    MAX_FLASH_PAGE_SIZE               = 256,
    MAX_EEPROM_PAGE_SIZE              = 16,

    // JTAG ICE mkI protocol constants

    // Address space selector values
    ADDR_PROG_SPACE_PROG_ENABLED      = 0xB0,
    ADDR_PROG_SPACE_PROG_DISABLED     = 0xA0,
    ADDR_DATA_SPACE                   = 0x20,
    ADDR_EEPROM_SPACE                 = 0xB1,
    ADDR_FUSE_SPACE                   = 0xB2,
    ADDR_LOCK_SPACE                   = 0xB3,
    ADDR_SIG_SPACE                    = 0xB4,
    ADDR_BREAKPOINT_SPACE             = 0x60,

    // Comms link bit rates
    BIT_RATE_9600                     = 0xf4,
    BIT_RATE_14400                    = 0xf8,
    BIT_RATE_19200                    = 0xfa,
    BIT_RATE_38400                    = 0xfd,
    BIT_RATE_57600                    = 0xfe,
    BIT_RATE_115200                   = 0xff,

    // Breakpoints (match values returned by JTAG box).
    BREAKPOINT_NONE                   = 0x00,
    BREAKPOINT_X                      = 0x04,
    BREAKPOINT_Y                      = 0x08,
    BREAKPOINT_Z                      = 0x10,

    // Responses from JTAG ICE
    JTAG_R_OK			      = 'A',
    JTAG_R_BREAK		      = 'B',
    JTAG_R_INFO			      = 'G',
    JTAG_R_FAILED		      = 'F',
    JTAG_R_SYNC_ERROR		      = 'E',
    JTAG_R_SLEEP		      = 'H',
    JTAG_R_POWER		      = 'I',

    // JTAG parameters
    JTAG_P_BITRATE		      = 'b',
    JTAG_P_SW_VERSION		      = 0x7b,
    JTAG_P_HW_VERSION		      = 0x7a,
    JTAG_P_IREG_HIGH                  = 0x81,
    JTAG_P_IREG_LOW                   = 0x82,
    JTAG_P_OCD_VTARGET                = 0x84,
    JTAG_P_OCD_BREAK_CAUSE            = 0x85,
    JTAG_P_CLOCK		      = 0x86,
    JTAG_P_EXTERNAL_RESET             = 0x8b, /* W */
    JTAG_P_FLASH_PAGESIZE_LOW         = 0x88, /* W */
    JTAG_P_FLASH_PAGESIZE_HIGH        = 0x89, /* W */
    JTAG_P_EEPROM_PAGESIZE            = 0x8a, /* W */
    JTAG_P_TIMERS_RUNNING	      = 0xa0,
    JTAG_P_BP_FLOW		      = 0xa1,
    JTAG_P_BP_X_HIGH		      = 0xa2,
    JTAG_P_BP_X_LOW		      = 0xa3,
    JTAG_P_BP_Y_HIGH		      = 0xa4,
    JTAG_P_BP_Y_LOW		      = 0xa5,
    JTAG_P_BP_MODE		      = 0xa6,
    JTAG_P_JTAGID_BYTE0               = 0xa7, /* R */
    JTAG_P_JTAGID_BYTE1               = 0xa8, /* R */
    JTAG_P_JTAGID_BYTE2               = 0xa9, /* R */
    JTAG_P_JTAGID_BYTE3               = 0xaa, /* R */
    JTAG_P_UNITS_BEFORE               = 0xab, /* W */
    JTAG_P_UNITS_AFTER                = 0xac, /* W */
    JTAG_P_BIT_BEFORE                 = 0xad, /* W */
    JTAG_P_BIT_AFTER                  = 0xae, /* W */
    JTAG_P_PSB0_LOW                   = 0xaf, /* W */
    JTAG_P_PSBO_HIGH                  = 0xb0, /* W */
    JTAG_P_PSB1_LOW                   = 0xb1, /* W */
    JTAG_P_PSB1_HIGH                  = 0xb2, /* W */
    JTAG_P_MCU_MODE                   = 0xb3, /* R */

    // JTAG commands
    JTAG_C_SET_DEVICE_DESCRIPTOR      = 0xA0,

    // Set JTAG bitrate to 1MHz
    // ff: 1MHz, fe: 500kHz, fd: 250khz, fb: 125Khz
    // JTAG bitrates
    JTAG_BITRATE_1_MHz                = 0xff,
    JTAG_BITRATE_500_KHz              = 0xfe,
    JTAG_BITRATE_250_KHz              = 0xfd,
    JTAG_BITRATE_125_KHz              = 0xfb,

    // JTAG ICE mkII stuff goes here.  Most of this is straight from
    // AppNote AVR067.

    // Communication with the JTAG ICE works in frames.  The protocol
    // somewhat resembles the STK500v2 protocol, yet it is sufficiently
    // different to prevent a direct code reuse. :-(
    //
    // Frame format:
    //
    //  +---------------------------------------------------------------+
    //  |   0   |  1  .  2  |  3 . 4 . 5 . 6  |   7   | ... | N-1 .  N  |
    //  |       |           |                 |       |     |           |
    //  | start | LSB   MSB | LSB ....... MSB | token | msg | LSB   MSB |
    //  | 0x1B  | sequence# | message size    | 0x0E  |     |   CRC16   |
    //  +---------------------------------------------------------------+
    //
    // Each request message will be returned by a response with a matching
    // sequence #.  Sequence # 0xffff is reserved for asynchronous event
    // notifications that will be sent by the ICE without a request
    // message (e.g. when the target hit a breakpoint).
    //
    // The message size excludes the framing overhead (10 bytes).
    //
    // The first byte of the message is always the request or response
    // code, which is roughly classified as:
    //
    // . Messages (commands) use 0x00 through 0x3f.  (The documentation
    //   claims that messages start at 0x01, but actually CMND_SIGN_OFF is
    //   0x00.)
    // . Internal commands use 0x40 through 0x7f (not documented).
    // . Success responses use 0x80 through 0x9f.
    // . Failure responses use 0xa0 through 0xbf.
    // . Events use 0xe0 through 0xff.

    MESSAGE_START		= 0x1b,
    TOKEN			= 0x0e,

    // Max message size we are willing to accept.  Prevents us from trying
    // to allocate too much VM in case we received a nonsensical packet
    // length.  We have to allocate the buffer as soon as we've got the
    // length information (and thus have to trust that information by that
    // time at first), as the final CRC check can only be done once the
    // entire packet came it.
    MAX_MESSAGE			= 100000,

    // ICE command codes
    CMND_CHIP_ERASE		= 0x13,
    CMND_CLEAR_EVENTS		= 0x22,
    CMND_CLR_BREAK		= 0x1A,
    CMND_ENTER_PROGMODE		= 0x14,
    CMND_ERASEPAGE_SPM		= 0x0D,
    CMND_FORCED_STOP		= 0x0A,
    CMND_GET_BREAK		= 0x12,
    CMND_GET_PARAMETER		= 0x03,
    CMND_GET_SIGN_ON		= 0x01,
    CMND_GET_SYNC		= 0x0f,
    CMND_GO			= 0x08,
    CMND_LEAVE_PROGMODE		= 0x15,
    CMND_READ_MEMORY		= 0x05,
    CMND_READ_PC		= 0x07,
    CMND_RESET			= 0x0B,
    CMND_RESTORE_TARGET		= 0x23,
    CMND_RUN_TO_ADDR		= 0x1C,
    CMND_SELFTEST		= 0x10,
    CMND_SET_BREAK		= 0x11,
    CMND_SET_DEVICE_DESCRIPTOR	= 0x0C,
    CMND_SET_N_PARAMETERS	= 0x16,
    CMND_SET_PARAMETER		= 0x02,
    CMND_SIGN_OFF		= 0x00,
    CMND_SINGLE_STEP		= 0x09,
    CMND_SPI_CMD		= 0x1D,
    CMND_WRITE_MEMORY		= 0x04,
    CMND_WRITE_PC		= 0x06,

    // ICE responses
    RSP_DEBUGWIRE_SYNC_FAILED	= 0xAC,
    RSP_FAILED			= 0xA0,
    RSP_GET_BREAK		= 0x83,
    RSP_ILLEGAL_BREAKPOINT	= 0xA8,
    RSP_ILLEGAL_COMMAND		= 0xAA,
    RSP_ILLEGAL_EMULATOR_MODE	= 0xA4,
    RSP_ILLEGAL_JTAG_ID		= 0xA9,
    RSP_ILLEGAL_MCU_STATE	= 0xA5,
    RSP_ILLEGAL_MEMORY_TYPE	= 0xA2,
    RSP_ILLEGAL_MEMORY_RANGE	= 0xA3,
    RSP_ILLEGAL_PARAMETER	= 0xA1,
    RSP_ILLEGAL_POWER_STATE	= 0xAD,
    RSP_ILLEGAL_VALUE		= 0xA6,
    RSP_MEMORY			= 0x82,
    RSP_NO_TARGET_POWER		= 0xAB,
    RSP_OK			= 0x80,
    RSP_PARAMETER		= 0x81,
    RSP_PC			= 0x84,
    RSP_SELFTEST		= 0x85,
    RSP_SET_N_PARAMETERS	= 0xA7,
    RSP_SIGN_ON			= 0x86,
    RSP_SPI_DATA		= 0x88,

    // ICE events
    EVT_BREAK				= 0xE0,
    EVT_DEBUG				= 0xE6,
    EVT_ERROR_PHY_FORCE_BREAK_TIMEOUT	= 0xE2,
    EVT_ERROR_PHY_MAX_BIT_LENGTH_DIFF	= 0xED,
    EVT_ERROR_PHY_OPT_RECEIVE_TIMEOUT	= 0xF9,
    EVT_ERROR_PHY_OPT_RECEIVED_BREAK	= 0xFA,
    EVT_ERROR_PHY_RECEIVED_BREAK	= 0xF8,
    EVT_ERROR_PHY_RECEIVE_TIMEOUT	= 0xF7,
    EVT_ERROR_PHY_RELEASE_BREAK_TIMEOUT	= 0xE3,
    EVT_ERROR_PHY_SYNC_OUT_OF_RANGE	= 0xF5,
    EVT_ERROR_PHY_SYNC_TIMEOUT		= 0xF0,
    EVT_ERROR_PHY_SYNC_TIMEOUT_BAUD	= 0xF4,
    EVT_ERROR_PHY_SYNC_WAIT_TIMEOUT	= 0xF6,
    EVT_RESULT_PHY_NO_ACTIVITY		= 0xFB,
    EVT_EXT_RESET			= 0xE7,
    EVT_ICE_POWER_ERROR_STATE		= 0xEA,
    EVT_ICE_POWER_OK			= 0xEB,
    EVT_IDR_DIRTY			= 0xEC,
    EVT_NONE				= 0xEF,
    EVT_PDSB_BREAK			= 0xF2,
    EVT_PDSMB_BREAK			= 0xF3,
    EVT_PROGRAM_BREAK			= 0xF1,
    EVT_RUN				= 0xE1,
    EVT_TARGET_POWER_OFF		= 0xE5,
    EVT_TARGET_POWER_ON			= 0xE4,
    EVT_TARGET_SLEEP			= 0xE8,
    EVT_TARGET_WAKEUP			= 0xE9,
    // trailer
    EVT_MAX                             = 0xFF,

    // memory types for CMND_{READ,WRITE}_MEMORY
    MTYPE_IO_SHADOW	= 0x30,	// cached IO registers?
    MTYPE_SRAM		= 0x20,	// target's SRAM or [ext.] IO registers
    MTYPE_EEPROM	= 0x22,	// EEPROM, what way?
    MTYPE_EVENT		= 0x60,	// ICE event memory
    MTYPE_EVENT_COMPRESSED = 0x61, // ICE event memory, bit-mapped
    MTYPE_SPM		= 0xA0,	// flash through LPM/SPM
    MTYPE_FLASH_PAGE	= 0xB0,	// flash in programming mode
    MTYPE_EEPROM_PAGE	= 0xB1,	// EEPROM in programming mode
    MTYPE_FUSE_BITS	= 0xB2,	// fuse bits in programming mode
    MTYPE_LOCK_BITS	= 0xB3,	// lock bits in programming mode
    MTYPE_SIGN_JTAG	= 0xB4,	// signature in programming mode
    MTYPE_OSCCAL_BYTE	= 0xB5,	// osccal cells in programming mode
    MTYPE_CAN		= 0xB6,	// CAN mailbox

    // (some) ICE parameters, for CMND_{GET,SET}_PARAMETER
    PAR_HW_VERSION		= 0x01,
    PAR_FW_VERSION		= 0x02,

    PAR_EMULATOR_MODE		= 0x03,
    EMULATOR_MODE_DEBUGWIRE	= 0x00,
    EMULATOR_MODE_JTAG		= 0x01,
    EMULATOR_MODE_UNKNOWN	= 0x02,
    EMULATOR_MODE_SPI		= 0x03,
    EMULATOR_MODE_JTAG_XMEGA    = 0x05,

    PAR_IREG			= 0x04,

    PAR_BAUD_RATE		= 0x05,
    PAR_BAUD_2400		= 0x01,
    PAR_BAUD_4800		= 0x02,
    PAR_BAUD_9600		= 0x03,
    PAR_BAUD_19200		= 0x04,	// default
    PAR_BAUD_38400		= 0x05,
    PAR_BAUD_57600		= 0x06,
    PAR_BAUD_115200		= 0x07,
    PAR_BAUD_14400		= 0x08,

    PAR_OCD_VTARGET		= 0x06,
    PAR_OCD_JTAG_CLK		= 0x07,
    PAR_OCD_BREAK_CAUSE		= 0x08,
    PAR_TIMERS_RUNNING		= 0x09,
    PAR_BREAK_ON_CHANGE_FLOW	= 0x0A,
    PAR_BREAK_ADDR1		= 0x0B,
    PAR_BREAK_ADDR2		= 0x0C,
    PAR_COMBBREAKCTRL		= 0x0D,
    PAR_JTAGID			= 0x0E,
    PAR_UNITS_BEFORE		= 0x0F,
    PAR_UNITS_AFTER		= 0x10,
    PAR_BIT_BEFORE		= 0x11,
    PAR_BIT_ATER		= 0x12,
    PAR_EXTERNAL_RESET		= 0x13,
    PAR_FLASH_PAGE_SIZE		= 0x14,
    PAR_EEPROM_PAGE_SIZE	= 0x15,
    PAR_UNUSED1			= 0x16,
    PAR_PSB0			= 0x17,
    PAR_PSB1			= 0x18,
    PAR_PROTOCOL_DEBUG_EVENT	= 0x19,

    PAR_MCU_STATE		= 0x1A,
    STOPPED			= 0x00,
    RUNNING			= 0x01,
    PROGRAMMING			= 0x02,

    PAR_DAISY_CHAIN_INFO	= 0x1B,
    PAR_BOOT_ADDRESS		= 0x1C,
    PAR_TARGET_SIGNATURE	= 0x1D,
    PAR_DEBUGWIRE_BAUDRATE	= 0x1E,
    PAR_PROGRAM_ENTRY_POINT	= 0x1F,
    PAR_PACKET_PARSING_ERRORS	= 0x40,
    PAR_VALID_PACKETS_RECEIVED	= 0x41,
    PAR_INTERCOMMUNICATION_TX_FAILURES = 0x42,
    PAR_INTERCOMMUNICATION_RX_FAILURES = 0x43,
    PAR_CRC_ERRORS		= 0x44,

    PAR_POWER_SOURCE		= 0x45,
    POWER_EXTERNAL		= 0x00,
    POWER_USB			= 0x01,

    PAR_CAN_FLAG		= 0x22,
    DONT_READ_CAN_MAILBOX	= 0x00,
    READ_CAN_MAILBOX		= 0x01,

    PAR_ENABLE_IDR_IN_RUN_MODE	= 0x23,
    ACCESS_OSCCAL		= 0x00,
    ACCESS_IDR			= 0x01,

    PAR_ALLOW_PAGEPROGRAMMING_IN_SCANCHAIN = 0x24,
    PAGEPROG_NOT_ALLOWED	= 0x00,
    PAGEPROG_ALLOWED		= 0x01,
};

enum {
    PC_INVALID			      = 0xffffffff
};

enum bpType
{
    NONE,           // disabled.
    CODE,           // normal code space breakpoint.
    SOFTCODE,       // code software BP (not yet used).
    WRITE_DATA,     // write data space breakpoint (ie "watch").
    READ_DATA,      // read data space breakpoint (ie "watch").
    ACCESS_DATA,    // read/write data space breakpoint (ie "watch").
    DATA_MASK,      // mask for data space breakpoint.
    // keep mask bits last
    HAS_MASK = 0x80000000, // data space BP has an associated mask in
                           // next slot
};

// Enumerations for target memory type.
typedef enum {
    MEM_FLASH = 0,
    MEM_EEPROM = 1,
    MEM_RAM = 2,
} BFDmemoryType;

#endif /* JTAG_CONSTANTS_H__ */
